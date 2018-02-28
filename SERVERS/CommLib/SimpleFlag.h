#ifndef _SIMPLEFALG_H_
#define _SIMPLEFALG_H_

#include "CommLib.h"

#define DeclareFlags(type) \
        type m_uFlags; \
        void SetField(type uVal, type uMask, type uPos) \
        { \
            m_uFlags = (m_uFlags & ~uMask) | (uVal << uPos); \
        } \
        type GetField(type uMask, type uPos) const \
        { \
            return (m_uFlags & uMask) >> uPos; \
        } \
        void SetBit(type uMask,bool bVal) \
        { \
            if (bVal) \
            { \
                m_uFlags |= uMask; \
            } \
            else \
            { \
                m_uFlags &= ~uMask; \
            } \
        }\
        bool GetBit(type uMask) const \
        { \
            return (m_uFlags & uMask) != 0; \
        }\
		void ClrBit(void) {m_uFlags = 0;}

#endif /*_SIMPLEFALG_H_*/