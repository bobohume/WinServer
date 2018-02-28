
#ifndef _FIXEDSTRINGTABLE_H
#define _FIXEDSTRINGTABLE_H

#include "stringTable.h"

class FixedName
{
public:
    FixedName();
    FixedName(const char* pcString);
    FixedName(const FixedName& kString);
    ~FixedName();

    operator const char*() const;

    inline bool Exists() const;

    inline FixedName& operator=(const FixedName& kString);
    inline FixedName& operator=(const char* pcString);

    inline size_t GetLength() const;

    inline bool Equals(const char* pcStr) const;
    inline bool EqualsNoCase(const char* pcStr) const;

    inline bool Contains(const char* pcStr) const;
    inline bool ContainsNoCase(const char* pcStr) const;

    friend bool operator==(const FixedName& s1, const FixedName& s2);
    friend bool operator!=(const FixedName& s1, const FixedName& s2);
   
    friend bool operator==(const FixedName& s1, const char* s2);
    friend bool operator!=(const FixedName& s1, const char* s2);
   
    friend bool operator==(const char* s1, const FixedName& s2);
    friend bool operator!=(const char* s1, const FixedName& s2);
    
protected:
    StringTableEntry m_kHandle;
};

#include "FixedName.inl"

#endif
