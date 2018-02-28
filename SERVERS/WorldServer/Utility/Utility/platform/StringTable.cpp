
#include "platformStrings.h"
#include <assert.h>
#include "StringTable.h"

_StringTable *_gStringTable = NULL;

const U32 _StringTable::csm_stInitSize = 65535; 

//---------------------------------------------------------------
//
// StringTable functions
//
//---------------------------------------------------------------

namespace 
{

    bool sgInitTable = true;
    U8   sgHashTable[256];

    void initTolowerTable()
    {
        for (U32 i = 0; i < 256; i++) {
            U8 c = tolower( (int)i );
            sgHashTable[i] = (c * c) & U8_MAX;
        }

        sgInitTable = false;
    }

} // namespace {}

U32 _StringTable::hashString(const char* str)
{
    if (sgInitTable)
        initTolowerTable();

    if(!str) return -1;

    U32 ret = 0;
    U8 c;
    while((c = *str++) != 0) {
        ret <<= 1;
        ret ^= sgHashTable[c];
    }
    return ret;
}

U32 _StringTable::hashStringn(const char* str, S32 len)
{
    if (sgInitTable)
        initTolowerTable();

    U32 ret = 0;
    U8 c;
    while((c = *str++) != 0 && len--) {
        ret <<= 1;
        ret ^= sgHashTable[c];
    }
    return ret;
}

//--------------------------------------
_StringTable::_StringTable()
{
    buckets = (Node **) dMalloc(csm_stInitSize * sizeof(Node *));
    memset(buckets,0,csm_stInitSize * sizeof(Node *));

    numBuckets = csm_stInitSize;
    itemCount = 0;
}

//--------------------------------------
_StringTable::~_StringTable()
{
    dFree(buckets);
}


//--------------------------------------
void _StringTable::create()
{
    //assert(_gStringTable == NULL, "StringTable::create: StringTable already exists.");
    if(!_gStringTable)
        _gStringTable = new _StringTable;
}

void _StringTable::destroy()
{
    assert(StringTable != NULL && "StringTable::destroy: StringTable does not exist.");
    delete _gStringTable;
    _gStringTable = NULL;
}

StringTableEntry _StringTable::insert(const char* _val, const bool caseSens)
{
    OLD_DO_LOCK(m_cs);

    S32 length = 0;
    // Added 3/29/2007 -- If this is undesirable behavior, let me know -patw
    const char *val = _val;
    if( val == NULL )
        val = "";

    Node **walk, *temp;
    U32 key = hashString(val);
    walk = &buckets[key % numBuckets];
    while((temp = *walk) != NULL)
    {
        if (temp->val == val)
            return temp->val;

        if(caseSens && !dStrcmp(temp->val, val))
            return temp->val;
        else if(!caseSens && !dStricmp(temp->val, val))
            return temp->val;
        walk = &(temp->next);
    }
    char *ret = 0;
    if(!*walk)
    {
        *walk = new Node;
        (*walk)->next = 0;
        length = dStrlen(val) + 1;
        (*walk)->val = (char *)malloc(length);;
        memmove((*walk)->val,val,length);
        ret = (*walk)->val;
        itemCount ++;
    }
    if(itemCount > 2 * numBuckets)
    {
        resize(4 * numBuckets - 1);
    }
    return ret;
}

//--------------------------------------
StringTableEntry _StringTable::insertn(const char* src, S32 len, const bool  caseSens)
{
    OLD_DO_LOCK(m_cs);

    char val[256];
    assert(len < 255 && "Invalid string to insertn");
    dStrncpy(val, sizeof(val), src, len);
    val[len] = 0;
    return insert(val, caseSens);
}

//--------------------------------------
StringTableEntry _StringTable::lookup(const char* val, const bool  caseSens)
{
    OLD_DO_LOCK(m_cs);

    Node **walk, *temp;
    U32 key = hashString(val);
    walk = &buckets[key % numBuckets];
    while((temp = *walk) != NULL)   
    {
        if (temp->val == val)
            return temp->val;

        if(caseSens && !dStrcmp(temp->val, val))
            return temp->val;
        else if(!caseSens && !dStricmp(temp->val, val))
            return temp->val;
        walk = &(temp->next);
    }
    return NULL;
}

//--------------------------------------
StringTableEntry _StringTable::lookupn(const char* val, S32 len, const bool  caseSens)
{
    OLD_DO_LOCK(m_cs);

    Node **walk, *temp;
    U32 key = hashStringn(val, len);
    walk = &buckets[key % numBuckets];
    while((temp = *walk) != NULL) 
    {
        if (temp->val == val)
            return temp->val;

        if(caseSens && !dStrncmp(temp->val, val, len) && temp->val[len] == 0)
            return temp->val;
        else if(!caseSens && !dStrnicmp(temp->val, val, len) && temp->val[len] == 0)
            return temp->val;
        walk = &(temp->next);
    }
    return NULL;
}

//--------------------------------------
void _StringTable::resize(const U32 newSize)
{
    OLD_DO_LOCK(m_cs);

    Node *head = NULL, *walk, *temp;
    U32 i;
    // reverse individual bucket lists
    // we do this because new strings are added at the end of bucket
    // lists so that case sens strings are always after their
    // corresponding case insens strings

    for(i = 0; i < numBuckets; i++) 
	{
        walk = buckets[i];
        while(walk)
        {
            temp = walk->next;
            walk->next = head;
            head = walk;
            walk = temp;
        }
    }

    buckets = (Node **) realloc(buckets, newSize * sizeof(Node *));
    for(i = 0; i < newSize; i++) 
	{
        buckets[i] = 0;
    }

    numBuckets = newSize;
    walk = head;
    while(walk) {
        U32 key;
        Node *temp = walk;

        walk = walk->next;
        key = hashString(temp->val);
        temp->next = buckets[key % newSize];
        buckets[key % newSize] = temp;
    }
}

