#ifndef DUMP_HELPER_H
#define DUMP_HELPER_H

#include <winsock2.h>
#include <windows.h>
#include "dbghelp/dbghelp.h"
#include <eh.h>

#include <xutility>
#include "base/Locker.h"
#include "PacketType.h"
#ifndef NTJ_CLIENT
#include "base/types.h"
#endif

#define TINY_DUMP MiniDumpNormal
#define MINI_DUMP MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory | MiniDumpWithDataSegs

#define MIDI_DUMP MiniDumpWithPrivateReadWriteMemory |\
	MiniDumpWithDataSegs |\
	MiniDumpWithHandleData |\
	MiniDumpWithFullMemoryInfo |\
	MiniDumpWithThreadInfo |\
	MiniDumpWithUnloadedModules
#define MAXI_DUMP MiniDumpWithFullMemory |\
	MiniDumpWithFullMemoryInfo |\
	MiniDumpWithHandleData |\
	MiniDumpWithThreadInfo |\
	MiniDumpWithUnloadedModules

#define DUMP_TYPE MAXI_DUMP

#ifdef NTJ_CLIENT
#undef DUMP_TYPE
#ifdef DEBUG
#define DUMP_TYPE MAXI_DUMP
#else
#define DUMP_TYPE TINY_DUMP
#endif
#endif

void debugDump(int dumpType);

class CDumpHelper
{
public:
	CDumpHelper();
	~CDumpHelper();

	void reset();

	static void catchDump(LPEXCEPTION_POINTERS pExceptionPointers);
	static BOOL writeDump(DWORD threadID, LPEXCEPTION_POINTERS pExceptionPointers, int dumpType = DUMP_TYPE);
	static void dumpCallback(void *);

	void SetHandler();

	CMyCriticalSection m_CS;
	LPEXCEPTION_POINTERS m_Ptr;
	HANDLE m_hBegin;
	HANDLE m_hEnd;
	HANDLE m_hQuit;
	HANDLE m_hThread;
	DWORD m_dumpThreadID;
private:
	static LONG WINAPI SystemFail( LPEXCEPTION_POINTERS pExceptionPointers );
	static LPTOP_LEVEL_EXCEPTION_FILTER DefaultFail;
} ;

extern CDumpHelper g_dumper;

#include <sstream>
#include <vector>

#include <assert.h>

namespace DumpHelper
{
	//int max( int a, int b )
	//{
	//	return a > b ? a : b;
	//}

	__interface IStackDumpHandler
	{
	public:
		void __stdcall OnBegin();
		void __stdcall OnEntry(void *pvAddress, LPCSTR szModule, LPCSTR szSymbol);
		void __stdcall OnError(LPCSTR szError);
		void __stdcall OnEnd();
	};

#define ATL_MODULE_NAME_LEN _MAX_PATH
#define ATL_SYMBOL_NAME_LEN 1024

	// Helper class for generating a stack dump
	// This is used internally by AtlDumpStack
	class CStackDumper
	{
	public:
		struct _ATL_SYMBOL_INFO
		{
			ULONG_PTR dwAddress;
			ULONG_PTR dwOffset;
			CHAR	szModule[ATL_MODULE_NAME_LEN];
			CHAR	szSymbol[ATL_SYMBOL_NAME_LEN];
		};

		static LPVOID __stdcall FunctionTableAccess(_In_ HANDLE hProcess, _In_ ULONG_PTR dwPCAddress)
		{
#ifdef _WIN64
			return SymFunctionTableAccess(hProcess, dwPCAddress);
#else
			return SymFunctionTableAccess(hProcess, (ULONG)dwPCAddress);
#endif
		}

		static ULONG_PTR __stdcall GetModuleBase(_In_ HANDLE hProcess, _In_ ULONG_PTR dwReturnAddress)
		{
			IMAGEHLP_MODULE moduleInfo;
			moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE);

#ifdef _WIN64
			if (SymGetModuleInfo(hProcess, dwReturnAddress, &moduleInfo))
#else
			if (SymGetModuleInfo(hProcess, (ULONG)dwReturnAddress, &moduleInfo))
#endif
				return moduleInfo.BaseOfImage;
			else
			{
				MEMORY_BASIC_INFORMATION memoryBasicInfo;

				if (::VirtualQueryEx(hProcess, (LPVOID) dwReturnAddress,
					&memoryBasicInfo, sizeof(memoryBasicInfo)))
				{
					DWORD cch = 0;
					char szFile[MAX_PATH] = { 0 };

					cch = GetModuleFileNameA((HINSTANCE)memoryBasicInfo.AllocationBase,
						szFile, MAX_PATH);

					// Ignore the return code since we can't do anything with it.
					SymLoadModule(hProcess,
						NULL, ((cch) ? szFile : NULL),
#ifdef _WIN64
						NULL, (DWORD_PTR) memoryBasicInfo.AllocationBase, 0);
#else
						NULL, (DWORD)(DWORD_PTR)memoryBasicInfo.AllocationBase, 0);
#endif
					return (DWORD_PTR) memoryBasicInfo.AllocationBase;
				}
			}

			return 0;
		}

		static ULONG_PTR __stdcall GetModuleBaseWithoutDepends(_In_ HANDLE hProcess, _In_ ULONG_PTR dwReturnAddress)
		{
			IMAGEHLP_MODULE moduleInfo;
			moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE);

#ifdef _WIN64
			if (SymGetModuleInfo(hProcess, dwReturnAddress, &moduleInfo))
#else
			if (SymGetModuleInfo(hProcess, (ULONG)dwReturnAddress, &moduleInfo))
#endif
				return moduleInfo.BaseOfImage;
			
			return NULL;
		}

		static BOOL ResolveSymbol(_In_ HANDLE hProcess, _In_ UINT_PTR dwAddress,
			_ATL_SYMBOL_INFO &siSymbol)
		{
			BOOL fRetval = TRUE;

			siSymbol.dwAddress = dwAddress;

			CHAR szUndec[ATL_SYMBOL_NAME_LEN];
			CHAR szWithOffset[ATL_SYMBOL_NAME_LEN+MAX_PATH];
			LPSTR pszSymbol = NULL;
			IMAGEHLP_MODULE mi;

			memset(&siSymbol, 0, sizeof(_ATL_SYMBOL_INFO));
			mi.SizeOfStruct = sizeof(IMAGEHLP_MODULE);

#ifdef _WIN64
			if (!SymGetModuleInfo(hProcess, dwAddress, &mi))
#else
			if (!SymGetModuleInfo(hProcess, (UINT)dwAddress, &mi))
#endif
			{
				strcpy_s(siSymbol.szModule, _countof(siSymbol.szModule), "<no module>");
			}
			else
			{
				LPSTR pszModule = strchr(mi.ImageName, '\\');
				if (pszModule == NULL)
					pszModule = mi.ImageName;
				else
					pszModule++;

				strncpy_s(siSymbol.szModule, _countof(siSymbol.szModule), pszModule, _TRUNCATE);
			}

			__try
			{
				union 
				{
					CHAR rgchSymbol[sizeof(IMAGEHLP_SYMBOL) + ATL_SYMBOL_NAME_LEN];
					IMAGEHLP_SYMBOL  sym;
				} sym;
				memset(&sym.sym, 0x00, sizeof(sym.sym));
				sym.sym.SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
#ifdef _WIN64
				sym.sym.Address = dwAddress;
#else
				sym.sym.Address = (DWORD)dwAddress;
#endif
				sym.sym.MaxNameLength = ATL_SYMBOL_NAME_LEN;

#ifdef _WIN64
				if (SymGetSymFromAddr(hProcess, dwAddress, &(siSymbol.dwOffset), &sym.sym))
#else
				if (SymGetSymFromAddr(hProcess, (DWORD)dwAddress, &(siSymbol.dwOffset), &sym.sym))
#endif
				{
					pszSymbol = sym.sym.Name;

					if (UnDecorateSymbolName(sym.sym.Name, szUndec, sizeof(szUndec)/sizeof(szUndec[0]), 
						UNDNAME_NO_MS_KEYWORDS | UNDNAME_NO_ACCESS_SPECIFIERS))
					{
						pszSymbol = szUndec;
					}
					else if (SymUnDName(&sym.sym, szUndec, sizeof(szUndec)/sizeof(szUndec[0])))
					{
						pszSymbol = szUndec;
					}

					IMAGEHLP_LINE line;
					line.SizeOfStruct = sizeof(IMAGEHLP_LINE);
					DWORD dwDisplacement;
					BOOL bLineNum = SymGetLineFromAddr(hProcess, (DWORD)dwAddress, &dwDisplacement, &line);
	
					if (siSymbol.dwOffset != 0)
					{
						if (bLineNum)
							sprintf_s(szWithOffset, ATL_SYMBOL_NAME_LEN+MAX_PATH, "%s %s Line %d + %d bytes", line.FileName, pszSymbol, line.LineNumber, siSymbol.dwOffset);
						else
							sprintf_s(szWithOffset, ATL_SYMBOL_NAME_LEN, "%s + %d bytes", pszSymbol, siSymbol.dwOffset);

						// ensure null-terminated
						szWithOffset[ATL_SYMBOL_NAME_LEN-1] = '\0';

						pszSymbol = szWithOffset;
					}
				}
				else
					pszSymbol = "<no symbol>";
			}
			__except (EXCEPTION_ACCESS_VIOLATION == GetExceptionCode())
			{
				pszSymbol = "<EX: no symbol>";
				siSymbol.dwOffset = dwAddress - mi.BaseOfImage;
			}

			strncpy_s(siSymbol.szSymbol, _countof(siSymbol.szSymbol), pszSymbol, _TRUNCATE);
			return fRetval;
		}
	};

#ifndef _ATL_MAX_STACK_FRAMES
#define _ATL_MAX_STACK_FRAMES 256
#endif

	class _AtlThreadContextInfo
	{
	public:
		HANDLE             hThread; //Thread to get context for
		CONTEXT            context; //Where to put context
		IStackDumpHandler* pHandler;
		_AtlThreadContextInfo(IStackDumpHandler* p) throw()
		{
			hThread = NULL;
			memset(&context, 0, sizeof(context));
			context.ContextFlags = CONTEXT_FULL;
			pHandler = p;
		}
		~_AtlThreadContextInfo() throw()
		{
			if (hThread != NULL)
				CloseHandle(hThread);
		}
#pragma warning(push)
#pragma warning(disable:4793)
		HRESULT DumpStack() throw()
		{
			// 暂时关闭
			//return S_OK;

			//We can't just call GetThreadContext for the current thread.
			//The docs state this is incorrect and we see problems on x64/Itanium

			//We need to spin up another thread which suspends this thread and gets
			//this thread's context, then unsuspends this thread and signals it.

			HANDLE hThreadPseudo = ::GetCurrentThread();
			HANDLE hProcess = ::GetCurrentProcess();
			if (DuplicateHandle(hProcess, hThreadPseudo, hProcess, &hThread, 0, FALSE, DUPLICATE_SAME_ACCESS) == 0)
				return HRESULT_FROM_WIN32(GetLastError());

			DWORD dwID;
			HANDLE hWorkerThread = CreateThread(NULL, 0, ContextThreadProc, this, 0, &dwID);
			if (hWorkerThread == NULL)
				return HRESULT_FROM_WIN32(GetLastError());
			//Wait for thread to terminate which will indicate it is done.
			//On CE we can't suspend this thread while in the kernel, so loop
			//rather than just waiting.
			while (WaitForSingleObject(hWorkerThread, 0) != WAIT_OBJECT_0)
			{
			}
			CloseHandle(hWorkerThread);
			return S_OK;
		}
		static DWORD WINAPI ContextThreadProc(_In_ LPVOID pv) throw()
		{
			_AtlThreadContextInfo* pThis = 
				reinterpret_cast< _AtlThreadContextInfo* >(pv); 
			return pThis->DoDumpStack();
		}
		DWORD DoDumpStack() throw()
		{
			SuspendThread(hThread);
			GetThreadContext(hThread, &context);

			pHandler->OnBegin();

			//std::vector<void *> adwAddress;
			void * adwAddress[_ATL_MAX_STACK_FRAMES];
			int nStackCount = 0;
			HANDLE hProcess = ::GetCurrentProcess();
			if (SymInitialize(hProcess, NULL, TRUE))
			{
				// force undecorated names to get params
				DWORD dw = SymGetOptions();
				dw &= ~SYMOPT_UNDNAME;
				dw |= SYMOPT_LOAD_LINES;
				SymSetOptions(dw);

				STACKFRAME stackFrame;
				memset(&stackFrame, 0, sizeof(stackFrame));
				stackFrame.AddrPC.Mode = AddrModeFlat;
				stackFrame.AddrFrame.Mode = AddrModeFlat;
				stackFrame.AddrStack.Mode = AddrModeFlat;
				stackFrame.AddrReturn.Mode = AddrModeFlat;
				stackFrame.AddrBStore.Mode = AddrModeFlat;

				DWORD dwMachType;

#if defined(_M_IX86)
				dwMachType                   = IMAGE_FILE_MACHINE_I386;

				// program counter, stack pointer, and frame pointer
				stackFrame.AddrPC.Offset     = context.Eip;
				stackFrame.AddrStack.Offset  = context.Esp;
				stackFrame.AddrFrame.Offset  = context.Ebp;
#elif defined(_M_AMD64)
				// only program counter
				dwMachType                   = IMAGE_FILE_MACHINE_AMD64;
				stackFrame.AddrPC.Offset     = context.Rip;
#elif defined(_M_MRX000)
				// only program counter
				dwMachType                   = IMAGE_FILE_MACHINE_R4000;
				stackFrame.AddrPC.Offset     = context.Fir;
#elif defined(_M_ALPHA)
				// only program counter
				dwMachType                   = IMAGE_FILE_MACHINE_ALPHA;
				stackFrame.AddrPC.Offset     = (unsigned long) context.Fir;
#elif defined(_M_PPC)
				// only program counter
				dwMachType                   = IMAGE_FILE_MACHINE_POWERPC;
				stackFrame.AddrPC.Offset     = context.Iar;
#elif defined(_M_IA64)
				// only program counter
				dwMachType                   = IMAGE_FILE_MACHINE_IA64;
				stackFrame.AddrPC.Offset     = context.StIIP;
#elif defined(_M_ALPHA64)
				// only program counter
				dwMachType                   = IMAGE_FILE_MACHINE_ALPHA64;
				stackFrame.AddrPC.Offset     = context.Fir;
#else
#error("Unknown Target Machine");
#endif

				//adwAddress. .SetCount(0, 16);

				int nFrame;
				for (nFrame = 0; nFrame < _ATL_MAX_STACK_FRAMES; nFrame++)
				{
					if (!StackWalk(dwMachType, hProcess, hThread,
						&stackFrame, &context, NULL,
						CStackDumper::FunctionTableAccess, CStackDumper::GetModuleBase, NULL))
					{
						break;
					}
					if (stackFrame.AddrPC.Offset != 0)
						//adwAddress.push_back((void*)(DWORD_PTR)stackFrame.AddrPC.Offset);
						adwAddress[nStackCount++] = (void*)(DWORD_PTR)stackFrame.AddrPC.Offset;
				}
			}

			// dump it out now
			INT_PTR nAddress;
			INT_PTR cAddresses = nStackCount/*adwAddress.size()*/;
			for (nAddress = 0; nAddress < cAddresses; nAddress++)
			{
				CStackDumper::_ATL_SYMBOL_INFO info;
				UINT_PTR dwAddress = (UINT_PTR)adwAddress[nAddress];

				LPCSTR szModule = NULL;
				LPCSTR szSymbol = NULL;

				if (CStackDumper::ResolveSymbol(hProcess, dwAddress, info))
				{
					szModule = info.szModule;
					szSymbol = info.szSymbol;
				}
				pHandler->OnEntry((void *) dwAddress, szModule, szSymbol);
			}
			pHandler->OnEnd();
			ResumeThread(hThread);
			SymCleanup(hProcess);

			return 0;
		} 

		//不挂起线程，需要在外部设置context
		DWORD DoDumpStackEx() throw()
		{
			pHandler->OnBegin();

			void * adwAddress[_ATL_MAX_STACK_FRAMES];
			int nStackCount = 0;
			HANDLE hProcess = ::GetCurrentProcess();

			static BOOL sysInited = FALSE;
			if (!sysInited)
			{
				sysInited = SymInitialize(hProcess, NULL, TRUE);
			}
			if (sysInited)
			{
				// force undecorated names to get params
				DWORD dw = SymGetOptions();
				dw &= ~SYMOPT_UNDNAME;
				dw |= SYMOPT_LOAD_LINES;
				SymSetOptions(dw);

				STACKFRAME stackFrame;
				memset(&stackFrame, 0, sizeof(stackFrame));
				stackFrame.AddrPC.Mode = AddrModeFlat;
				stackFrame.AddrFrame.Mode = AddrModeFlat;
				stackFrame.AddrStack.Mode = AddrModeFlat;
				stackFrame.AddrReturn.Mode = AddrModeFlat;
				stackFrame.AddrBStore.Mode = AddrModeFlat;

				DWORD dwMachType;
#if defined(_M_IX86)
				dwMachType = IMAGE_FILE_MACHINE_I386;

				// program counter, stack pointer, and frame pointer
				stackFrame.AddrPC.Offset = context.Eip;
				stackFrame.AddrStack.Offset = context.Esp;
				stackFrame.AddrFrame.Offset = context.Ebp;
#elif defined(_M_AMD64)
				// only program counter
				dwMachType = IMAGE_FILE_MACHINE_AMD64;
				stackFrame.AddrPC.Offset = context.Rip;
#elif defined(_M_MRX000)
				// only program counter
				dwMachType = IMAGE_FILE_MACHINE_R4000;
				stackFrame.AddrPC.Offset = context.Fir;
#elif defined(_M_ALPHA)
				// only program counter
				dwMachType = IMAGE_FILE_MACHINE_ALPHA;
				stackFrame.AddrPC.Offset = (unsigned long)context.Fir;
#elif defined(_M_PPC)
				// only program counter
				dwMachType = IMAGE_FILE_MACHINE_POWERPC;
				stackFrame.AddrPC.Offset = context.Iar;
#elif defined(_M_IA64)
				// only program counter
				dwMachType = IMAGE_FILE_MACHINE_IA64;
				stackFrame.AddrPC.Offset = context.StIIP;
#elif defined(_M_ALPHA64)
				// only program counter
				dwMachType = IMAGE_FILE_MACHINE_ALPHA64;
				stackFrame.AddrPC.Offset = context.Fir;
#else
#error("Unknown Target Machine");
#endif

				int nFrame;
				for (nFrame = 0; nFrame < 8/*_ATL_MAX_STACK_FRAMES*/; nFrame++)
				{
					if (!StackWalk(dwMachType, hProcess, hThread,
						&stackFrame, &context, NULL,
						CStackDumper::FunctionTableAccess, CStackDumper::GetModuleBaseWithoutDepends, NULL))
					{
						break;
					}
					if (stackFrame.AddrPC.Offset != 0)
						adwAddress[nStackCount++] = (void*)(DWORD_PTR)stackFrame.AddrPC.Offset;
				}
			}

			// dump it out now
			INT_PTR nAddress;
			INT_PTR cAddresses = nStackCount/*adwAddress.size()*/;
			for (nAddress = 0; nAddress < cAddresses; nAddress++)
			{
				CStackDumper::_ATL_SYMBOL_INFO info;
				UINT_PTR dwAddress = (UINT_PTR)adwAddress[nAddress];

				LPCSTR szModule = NULL;
				LPCSTR szSymbol = NULL;

				if (CStackDumper::ResolveSymbol(hProcess, dwAddress, info))
				{
					szModule = info.szModule;
					szSymbol = info.szSymbol;
				}
				pHandler->OnEntry((void *) dwAddress, szModule, szSymbol);
			}
			pHandler->OnEnd();
			//SymCleanup(hProcess);

			return 0;
		} 

	};


	// Helper function to produce a stack dump
	inline void AtlDumpStack(_In_ IStackDumpHandler *pHandler)
	{
		//ATLASSERT(pHandler);
		assert(pHandler);
		_AtlThreadContextInfo info(pHandler);
		info.DumpStack();
	}
#pragma warning(pop)

#define STACK_TRACE_PART_DELIMITER ';'
#define STACK_TRACE_LINE_DELIMITER '~'

	// CReportHookDumpHandler is a stack dump handler
	// that gathers the stack dump into the format
	// used by CDebugReportHook
	class CReportHookDumpHandler : public IStackDumpHandler
	{
	public:
		CReportHookDumpHandler()
		{
			m_pstr = NULL;
		}

		void GetStackDump(_In_ std::string *pstr)
		{
			//ATLASSERT(pstr);
			assert(pstr);

			SetString(pstr);
			AtlDumpStack(this);
			SetString(NULL);
		}

		void SetString(_In_opt_ std::string *pstr)
		{
			m_pstr = pstr;
		}

		// implementation
		// IStackDumpHandler methods
		void __stdcall OnBegin()
		{
		}

		void __stdcall OnEntry(_In_ void *pvAddress, _In_ LPCSTR szModule, _In_ LPCSTR szSymbol)
		{
			// make sure SetString was called before
			// trying to get a stack dump
			//ATLASSUME(m_pstr);
			if (!m_pstr)
				return;

			char szBuf[100];
			sprintf_s(szBuf, _countof(szBuf), "0x%p;", pvAddress);
			// ensure null-terminated
			szBuf[sizeof(szBuf)-1] = '\0';
			*m_pstr += szBuf;
			if (!szModule)
				szModule = "Unknown";
			if (!szSymbol)
				szSymbol = "<No Info>";

			*m_pstr += szModule;
			*m_pstr += STACK_TRACE_PART_DELIMITER;
			//ATLASSERT(szSymbol);
			assert( szSymbol );

			*m_pstr += szSymbol;
			*m_pstr += STACK_TRACE_PART_DELIMITER;
			*m_pstr += STACK_TRACE_LINE_DELIMITER;
		}

		void __stdcall OnError(LPCSTR /*szError*/)
		{
		}
		void __stdcall OnEnd()
		{
		}

	protected:
		std::string *m_pstr;

	};

#if !( defined(_SXZ_UNITTEST) & !defined(_CONSOLE) )
	static const int AFX_STACK_DUMP_TARGET_TRACE  =                   0x0001;
	static const int AFX_STACK_DUMP_TARGET_CLIPBOARD = 0x0002;
	static const int AFX_STACK_DUMP_TARGET_BOTH    =                  0x0003;
	static const int AFX_STACK_DUMP_TARGET_ODS    =                   0x0004;
#endif

	class CTraceClipboardData : public IStackDumpHandler
	{
		HGLOBAL m_hMemory;
		DWORD	m_dwSize;
		DWORD m_dwUsed;
		DWORD m_dwTarget;

		std::stringstream m_stringStream;

	public:
		void __stdcall OnBegin()
		{
			SendOut("=== begin DumpHelper::DumpStack output ===\r\n");
		}
		void __stdcall OnEntry(void *pvAddress, LPCSTR szModule, LPCSTR szSymbol)
		{
			char sz[40];
			sprintf_s(sz, _countof(sz), "%p: ", pvAddress);
			SendOut(sz);

			if (szModule)
			{
				//ATLASSERT(szSymbol);
				assert( szSymbol );

				SendOut(szModule);
				SendOut("! ");
				SendOut(szSymbol);
			}
			else
				SendOut("symbol not found");
			SendOut("\r\n");
		}

		void __stdcall OnError(LPCSTR szError)
		{
			SendOut(szError);
		}
		void __stdcall OnEnd()
		{
			SendOut("=== end DumpHelper::DumpStack() output ===\r\n");
		}

		std::string string()
		{
			return m_stringStream.str();
		}



		CTraceClipboardData(DWORD dwTarget)
			: m_dwTarget(dwTarget), m_dwSize(0), m_dwUsed(0), m_hMemory(NULL)
		{
		}

		~CTraceClipboardData()
		{
			if (m_hMemory != NULL)
			{
				// chuck it onto the clipboard
				// don't free it unless there's an error

				if (!OpenClipboard(NULL))
					GlobalFree(m_hMemory);
				else if (!EmptyClipboard() ||
					SetClipboardData(CF_TEXT, m_hMemory) == NULL)
				{
					GlobalFree(m_hMemory);
				}
				else
					CloseClipboard();
			}
		}

		void SendOut(LPCSTR pszData)
		{
			int nLength;
			if (pszData == NULL || (nLength = lstrlenA(pszData)) == 0)
				return;

			// send it to TRACE (can be redirected)
			if (m_dwTarget & AFX_STACK_DUMP_TARGET_TRACE)
			{
				char buf[1000] = { 0 };
				sprintf_s( buf, 1000, "%hs", pszData );
				m_stringStream << buf;
			}//TRACE(traceAppMsg, 0, "%hs", pszData);

			// send it to OutputDebugString() (can't redirect)
			if (m_dwTarget & AFX_STACK_DUMP_TARGET_ODS)
				OutputDebugStringA(pszData);

			// build a buffer for the clipboard
			if (m_dwTarget & AFX_STACK_DUMP_TARGET_CLIPBOARD)
			{
				if (m_hMemory == NULL)
				{
					if( nLength > (1024L*1024L) - 1 )
					{
						//TRACE(traceAppMsg, 0, "AfxDumpStack Error: pszData larger than one megabyte.\n");
						m_dwTarget &= ~AFX_STACK_DUMP_TARGET_CLIPBOARD;
					}
					else
					{
						if( nLength + 1 > 1024 )
							m_hMemory = GlobalAlloc(GMEM_MOVEABLE, nLength + 1 );
						else
							m_hMemory = GlobalAlloc(GMEM_MOVEABLE, 1024 );

						if (m_hMemory == NULL)
						{
							//TRACE(traceAppMsg, 0, "AfxDumpStack Error: No memory available for clipboard.\n");
							m_dwTarget &= ~AFX_STACK_DUMP_TARGET_CLIPBOARD;
						}
						else
						{
							m_dwUsed = nLength;
							
							if( nLength + 1 > 1024 )
								m_dwSize = nLength + 1;
							else
								m_dwSize = 1024;

							LPSTR pstr = (LPSTR) GlobalLock(m_hMemory);
							if (pstr != NULL)
							{
								strcpy_s(pstr, ::GlobalSize(m_hMemory), pszData);
								GlobalUnlock(m_hMemory);
							}
							else
							{
								//TRACE(traceAppMsg, 0, "AfxDumpStack Error: Couldn't lock memory!\n");
								GlobalFree(m_hMemory);
								m_hMemory = NULL;
								m_dwTarget &= ~AFX_STACK_DUMP_TARGET_CLIPBOARD;
							}
						}
					}
				}
				else
				{
					if ((m_dwUsed + nLength + 1) >= m_dwSize)
					{
						// grow by leaps and bounds
						DWORD dwNewSize = m_dwSize * 2;
						if (dwNewSize > (1024L*1024L))
						{
							//TRACE(traceAppMsg, 0, "AfxDumpStack Error: more than one megabyte on clipboard.\n");
							m_dwTarget &= ~AFX_STACK_DUMP_TARGET_CLIPBOARD;
						}

						HGLOBAL hMemory = GlobalReAlloc(m_hMemory, dwNewSize, GMEM_MOVEABLE);
						if (hMemory == NULL)
						{
							//TRACE(traceAppMsg, 0, "AfxDumpStack Error: Couldn't get %d bytes!\n", m_dwSize);
							m_dwTarget &= ~AFX_STACK_DUMP_TARGET_CLIPBOARD;
						}
						else
						{
							m_hMemory = hMemory;
							m_dwSize = dwNewSize;
						}
					}

					LPSTR pstr = (LPSTR) GlobalLock(m_hMemory);
					if (pstr != NULL)
					{
						strncpy_s(pstr + m_dwUsed, m_dwSize - m_dwUsed, pszData, _TRUNCATE);
						m_dwUsed += nLength;
						GlobalUnlock(m_hMemory);
					}
					else
					{
						//TRACE(traceAppMsg, 0, "AfxDumpStack Error: Couldn't lock memory!\n");
						m_dwTarget &= ~AFX_STACK_DUMP_TARGET_CLIPBOARD;
					}
				}
			}

			return;
		}

	};

	static std::string DumpStack(DWORD dwTarget = 0x0001 /* = AFX_STACK_DUMP_TARGET_DEFAULT */)
	{
		CTraceClipboardData clipboardData(dwTarget);

		AtlDumpStack(&clipboardData);

		return clipboardData.string();
	}
}

class CVersion
#ifndef NTJ_CLIENT
#endif
{
public:
	static CVersion& GetInstance();
	const char* GetVersion() { return m_vesion; }
	const char* GetMD5() { return m_md5; }

private:
	CVersion();
	CVersion(const CVersion& version);
	CVersion& operator=(const CVersion& version);
	void LoadIni();
	char m_vesion[COMMON_STRING_LENGTH];		// 游戏主版本
	char m_md5[COMMON_STRING_LENGTH];			// MD5校验值

#ifndef NTJ_CLIENT
public:
	virtual void TimeProcess(U32 bExit);
	bool IsAcceptableVersion(char* version);
	bool IsAcceptableBuildVersion(char* version);

private:
	int     m_loadInterval;
	bool    m_isFileExist;
    __int64 m_sVer;
	__int64 m_sBuildVer;
#endif
};

#define G_VERSION CVersion::GetInstance().GetVersion()
#define G_MD5 CVersion::GetInstance().GetMD5()
//#define G_CPUID CVersion::GetInstance().GetCpuId()

#ifndef NTJ_CLIENT
#define DUMP_VERSION G_VERSION_BUILD
#define IS_ACCEPTABLE_VERSION(version) CVersion::GetInstance().IsAcceptableVersion((version))
#define IS_ACCEPTABLE_BUILD_VERSION(version) CVersion::GetInstance().IsAcceptableBuildVersion((version))
#else
#define DUMP_VERSION G_VERSION
#endif

extern void debugDump(int dumpType);

#endif /*DUMP_HELPER_H*/
