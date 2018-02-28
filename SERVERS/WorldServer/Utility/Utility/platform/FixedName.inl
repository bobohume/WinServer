
inline FixedName::FixedName()
{
    m_kHandle = 0;
}

inline FixedName::FixedName(const char* pcString)
{
    m_kHandle = StringTable->insert(pcString);
}

inline FixedName::FixedName(const FixedName& kString)
{
    m_kHandle = kString.m_kHandle;
}

inline FixedName::~FixedName()
{
}

inline FixedName::operator const char*() const
{
    return m_kHandle;
}

inline FixedName& FixedName::operator=(const FixedName& kString)
{
    if (m_kHandle != kString.m_kHandle)
    {
        m_kHandle = kString.m_kHandle;
    }

    return *this;
}

inline FixedName& FixedName::operator=(const char* pcString)
{
    if (m_kHandle != pcString)
    {
        m_kHandle = StringTable->insert(pcString);
    }

    return *this;
}

inline bool FixedName::Exists() const
{
    return m_kHandle != 0;
}

inline size_t FixedName::GetLength() const
{
    return strlen(m_kHandle);
}

//---------------------------------------------------------------------------
inline bool FixedName::Equals(const char* pcStr) const
{
    // Handle NULL checks and that this points to the exact same location
    // in memory.
    if (m_kHandle == pcStr)
        return true;

    // The previous check would return true if BOTH were NULL. Now check 
    // if the const char* is NULL
    if (pcStr == NULL  || m_kHandle == NULL)
        return false;

    return strcmp((const char*) m_kHandle, pcStr) == 0;
}

//---------------------------------------------------------------------------
inline bool FixedName::EqualsNoCase(const char* pcStr) const
{
    // Handle NULL checks and that this points to the exact same location
    // in memory.
    if (m_kHandle == pcStr)
        return true;

    // The previous check would return true if BOTH were NULL. Now check 
    // if the const char* is NULL
    if (pcStr == NULL  || m_kHandle == NULL)
        return false;

    return _stricmp((const char*) m_kHandle, pcStr) == 0;
}

inline bool FixedName::Contains(const char* pcStr) const
{
    if (m_kHandle == pcStr && pcStr != NULL)
        return true;

    // The previous check would return true if BOTH were NULL. Now check 
    // if the const char* is NULL
    if (pcStr == NULL  || m_kHandle == NULL || pcStr[0] == '\0' || (const char*) m_kHandle == '\0')
    {
        return false;
    }

    return strstr((const char*) m_kHandle, pcStr) != NULL;
}

inline bool FixedName::ContainsNoCase(const char* pcStr) const
{
    if (m_kHandle == pcStr && pcStr != NULL)
        return true;

    // The previous check would return true if BOTH were NULL. Now check 
    // if the const char* is NULL
    if (pcStr == NULL  || m_kHandle == NULL || pcStr[0] == '\0' ||
        (const char*) m_kHandle == '\0')
    {
        return false;
    }

    const char* pcMyString = (const char*) m_kHandle;
    size_t stOtherLength = strlen(pcStr);

    for (unsigned int uiMyIdx = 0; uiMyIdx < GetLength(); uiMyIdx++)
    {
        if (_strnicmp(&pcMyString[uiMyIdx], pcStr, stOtherLength) == 0)
            return true;
    }
    return false;
}

inline bool operator==(const FixedName& s1, const FixedName& s2)
{
    return s1.m_kHandle == s2.m_kHandle;
}
//---------------------------------------------------------------------------
inline bool operator!=(const FixedName& s1, const FixedName& s2)
{
    return s1.m_kHandle != s2.m_kHandle;
}
//---------------------------------------------------------------------------
inline bool operator==(const FixedName& s1, const char* s2)
{
    return s1.Equals(s2);
}
//---------------------------------------------------------------------------
inline bool operator!=(const FixedName& s1, const char* s2)
{
    return !(s1.Equals(s2));
}
//---------------------------------------------------------------------------
inline bool operator==(const char* s1, const FixedName& s2)
{
    return s2.Equals(s1);
}
//---------------------------------------------------------------------------
inline bool operator!=(const char* s1, const FixedName& s2)
{
    return !(s2.Equals(s1));
}
