#include <Windows.h>
#include <time.h>
#include <vcclr.h>
//#include <boost/thread.hpp>

#include "../test_common.h"

using namespace System;
using namespace DbOperator;

void process_record(ifm_record *record);
void service(const char *service_name);

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(stdout, "Usage: %s service_name thread_count \n", argv[0]);
		exit(0);
	}

	char service_name[128] = {0};
	strcpy(service_name, argv[1]);

	if (argc >= 3)
	{
// 		int thread_count = atoi(argv[2]);
// 		for (int i = 0; i < thread_count; i++)
// 		{
// 			boost::thread_group thrd_group;
// 			for(int i = 0; i < thread_count; i++)
// 			{
// 				thrd_group.create_thread(boost::bind(service, service_name));
// 			}
// 			thrd_group.join_all();
// 		}
	}
	else
	{
		service(service_name);
	}

	return 0;
}

void service(const char *service_name)
{
	ifm_record record;
	new_ifm_record(&record);


	while (true)
	{
		zero_ifm_record(&record);

		if (ifm_get_request(service_name, &record.sequence_id, record.paras, &record.para_count) != 0)
		{
			fprintf(stdout, "no request\n");
			Sleep(1000);
			continue;
		}

		fprintf(stdout, "request: ");
		print_ifm_record(&record);

		process_record(&record);

		if (ifm_post_response(service_name, record.sequence_id, record.paras, record.para_count) != 0)
		{
			fprintf(stdout, "post response failed, sequence_id=%llu \n", record.sequence_id);
			continue;
		}

		fprintf(stdout, "response: ");
		print_ifm_record(&record);
	}

	delete_ifm_record(&record);
}

const char IFM_RETURN_CODE[20][5] =
{
	"0000",
	"0003",
	"0004",
	"0005",
	"0011",
	"0012",
	"0014",
	"0015",
	"0016",
	"0017",
	"0018",
	"0020",
	"0021",
	"0022",
	"0030",
	"0031",
	"0032",
	"0039",
	"9000",
	"9999"
};

/************************************************************************
MessageId
Result
Sequence
UserID
Birthday
Gender
RegisterTs                                                               
************************************************************************/
//取系统时间函数
void now(char *ts)
{
	time_t tNow = time(NULL);
	struct tm *pTmNow = localtime(&tNow);
	sprintf_s(ts, MAX_PARA_SIZE, "%04d%02d%02d%02d%02d%02d",
		pTmNow->tm_year + 1900, pTmNow->tm_mon + 1, pTmNow->tm_mday,
		pTmNow->tm_hour, pTmNow->tm_min, pTmNow->tm_sec);
}

void make_result(ifm_record * record, int result)
{
	sprintf_s( record->paras[1], MAX_PARA_SIZE, "%04d", result );
}

void make_account_response( ifm_record * record, int sequence ) 
{
	// 	 loginname
 	strcpy_s(record->paras[3], MAX_PARA_SIZE, record->paras[1]);

	// sequence
	sprintf_s(record->paras[2], MAX_PARA_SIZE, "%lu", sequence);

	// birthday gender
	if (sequence % 2 == 0)
	{	
		strcpy_s(record->paras[4], MAX_PARA_SIZE, "19800101");
		strcpy_s(record->paras[5], MAX_PARA_SIZE, "M");
	}
	else
	{
		strcpy_s(record->paras[4], MAX_PARA_SIZE, "20090101");
		strcpy_s(record->paras[5], MAX_PARA_SIZE, "F");
	}

	// registerTS
	now(record->paras[6]);

	record->para_count = 7;
}

void process_record(ifm_record *record)
{
 	//static time_t sequence = time(NULL);
	int sequence = 0;

	int arg = atoi( record->paras[0] );

	const char* accountName = record->paras[1];

	switch( arg )
	{
	case 103:
		{
			int accountId = Pass9DB::AddAccount( gcnew String(accountName), sequence );
			
			make_account_response(record, sequence);

			if( accountId != 0 )
				make_result( record, 40 );
			else
				make_result( record, 22 );
		}
		break;
	case 104:
		{
			int accountId = Pass9DB::InverseActive( gcnew String(accountName), 1, sequence );

			make_account_response(record, sequence);

			if( accountId != 0 )
				make_result( record, 40 );
			else
				make_result( record, 22 );
		}
		break;
	case 109:
		{
			int accountId = Pass9DB::ActiveAccount( gcnew String(record->paras[2]), gcnew String( record->paras[3] ), sequence );
			if( accountId != 0 )
				make_result( record, 0 );
			else
				make_result( record, 40 );
		}
		break;
	}

	return ;
}