#ifndef _TVECTOR_H_
#define _TVECTOR_H_

//Includes
#include "types.h"


//-----------------------------------------------------------------------------
// Helper definitions for the vector class.

/// Size of memory blocks to allocate at a time for vectors.
#define VectorBlockSize 16

#ifdef POWER_DEBUG_GUARD
extern bool VectorResize(U32 *aSize, U32 *aCount, void **arrayPtr, U32 newCount, U32 elemSize,
                         const char* fileName,
                         const U32   lineNum);
#else
extern bool VectorResize(U32 *aSize, U32 *aCount, void **arrayPtr, U32 newCount, U32 elemSize);
#endif


/// Use the following macro to bind a vector to a particular line
///  of the owning class for memory tracking purposes
#ifdef POWER_DEBUG_GUARD
#define VECTOR_SET_ASSOCIATION(x) x.setFileAssociation(__FILE__, __LINE__)
#else
#define VECTOR_SET_ASSOCIATION(x)
#endif

// =============================================================================
/// A dynamic array class.
///
/// The vector grows as you insert or append
/// elements.  Insertion is fastest at the end of the array.  Resizing
/// of the array can be avoided by pre-allocating space using the
/// reserve() method.
///
/// <b>***WARNING***</b>
///
/// This template does not initialize, construct or destruct any of
/// it's elements.  This means don't use this template for elements
/// (classes) that need these operations.  This template is intended
/// to be used for simple structures that have no constructors or
/// destructors.
///
/// @nosubgrouping
template<class T>
class Vector
{
  protected:
   U32 mElementCount;
   U32 mArraySize;
   T*  mArray;

#ifdef POWER_DEBUG_GUARD
   const char* mFileAssociation;
   U32         mLineAssociation;
#endif

   bool  resize(U32); // resizes, but does no construction/destruction
   void  destroy(U32 start, U32 end); // destroys from start to end-1
   void  construct(U32 start, U32 end);
   void  construct(U32 start, U32 end, const T* array);
  public:
   Vector(const U32 initialSize = 0);
   Vector(const U32 initialSize, const char* fileName, const U32 lineNum);
   Vector(const char* fileName, const U32 lineNum);
   Vector(const Vector&);
   ~Vector();

#ifdef POWER_DEBUG_GUARD
   void setFileAssociation(const char* file, const U32 line);
#endif

   /// @name STL interface
   /// @{

   typedef T        value_type;
   typedef T&       reference;
   typedef const T& const_reference;

   typedef T*       iterator;
   typedef const T* const_iterator;
   typedef S32    difference_type;
   typedef U32    size_type;

   Vector<T>& operator=(const Vector<T>& p);

   iterator       begin();
   const_iterator begin() const;
   iterator       end();
   const_iterator end() const;

   S32 size() const;
   bool empty() const;

   void insert(iterator, const T&);
   void erase(iterator);

   T&       front();
   const T& front() const;
   T&       back();
   const T& back() const;

   void push_front(const T&);
   void push_back(const T&);
   void pop_front();
   void pop_back();

   T& operator[](U32);
   const T& operator[](U32) const;

   T& operator[](S32 i)              { return operator[](U32(i)); }
   const T& operator[](S32 i ) const { return operator[](U32(i)); }

   void reserve(U32);
   U32 capacity() const;

   /// @}

   /// @name Extended interface
   /// @{

   U32  memSize() const;
   T*   address() const;
   U32  setSize(U32);
   void increment();
   void decrement();
   void increment(U32);
   void decrement(U32);
   void insert(U32);
   void erase(U32);
   void erase_fast(U32);
   void erase_fast(iterator);
   void clear();
   void compact();

   T& first();
   T& last();
   const T& first() const;
   const T& last() const;

   void set(void * addr, U32 sz);

   void merge(const Vector& p);

   /// @}
};

template<class T> inline Vector<T>::~Vector()
{
   dFree(mArray);
}

template<class T> inline Vector<T>::Vector(const U32 initialSize)
{
#ifdef POWER_DEBUG_GUARD
   mFileAssociation = NULL;
   mLineAssociation = 0;
#endif

   mArray        = 0;
   mElementCount = 0;
   mArraySize    = 0;
   if(initialSize)
      reserve(initialSize);
}

template<class T> inline Vector<T>::Vector(const U32 initialSize,
                                           const char* fileName,
                                           const U32   lineNum)
{
#ifdef POWER_DEBUG_GUARD
   mFileAssociation = fileName;
   mLineAssociation = lineNum;
#else
   fileName;
   lineNum;
#endif

   mArray        = 0;
   mElementCount = 0;
   mArraySize    = 0;
   if(initialSize)
      reserve(initialSize);
}

template<class T> inline Vector<T>::Vector(const char* fileName,
                                           const U32   lineNum)
{
#ifdef POWER_DEBUG_GUARD
   mFileAssociation = fileName;
   mLineAssociation = lineNum;
#else
   fileName;
   lineNum;
#endif

   mArray        = 0;
   mElementCount = 0;
   mArraySize    = 0;
}

template<class T> inline Vector<T>::Vector(const Vector& p)
{
#ifdef POWER_DEBUG_GUARD
   mFileAssociation = p.mFileAssociation;
   mLineAssociation = p.mLineAssociation;
#endif

   mArray = 0;
   resize(p.mElementCount);
   construct(0, p.mElementCount, p.mArray);
}


#ifdef POWER_DEBUG_GUARD
template<class T> inline void Vector<T>::setFileAssociation(const char* file,
                                                            const U32   line)
{
   mFileAssociation = file;
   mLineAssociation = line;
}
#endif

template<class T> inline void  Vector<T>::destroy(U32 start, U32 end)
{
   // This check is a little generous as we can legitimately get (0,0) as
   // our parameters... so it won't detect every invalid case but it does
   // remain simple.
   LIB_ASSERT(start <= mElementCount && end <= mElementCount, "Vector<T>::destroy - out of bounds start/end.");

   // destroys from start to end-1
   while(start < end)
      destructInPlace(&mArray[start++]);
}

template<class T> inline void  Vector<T>::construct(U32 start, U32 end)
{
   LIB_ASSERT(start <= mElementCount && end <= mElementCount, "Vector<T>::construct - out of bounds start/end.");
   while(start < end)
      constructInPlace(&mArray[start++]);
}

template<class T> inline void  Vector<T>::construct(U32 start, U32 end, const T* array)
{
   LIB_ASSERT(start <= mElementCount && end <= mElementCount, "Vector<T>::construct - out of bounds start/end.");
   while(start < end)
   {
      constructInPlace(&mArray[start], &array[start]);
      start++;
   }
}

template<class T> inline U32 Vector<T>::memSize() const
{
   return capacity() * sizeof(T);
}

template<class T> inline T* Vector<T>::address() const
{
   return mArray;
}

template<class T> inline U32 Vector<T>::setSize(U32 size)
{
   const U32 oldSize = mElementCount;
   
   if(size > mElementCount)
   {
      if (size > mArraySize)
         resize(size);

      // Set count first so we are in a valid state for construct.
      mElementCount = size;
      construct(oldSize, size);
   }
   else if(size < mElementCount)
   {
      destroy(size, oldSize);
      mElementCount = size;
   }

   return mElementCount;
}

template<class T> inline void Vector<T>::increment()
{
   if(mElementCount == mArraySize)
      resize(mElementCount + 1);
   else
      mElementCount++;
   constructInPlace(&mArray[mElementCount - 1]);
}

template<class T> inline void Vector<T>::decrement()
{
   LIB_ASSERT(mElementCount != 0, "Vector<T>::decrement - cannot decrement zero-length vector.");
   mElementCount--;
   destructInPlace(&mArray[mElementCount]);
}

template<class T> inline void Vector<T>::increment(U32 delta)
{
   U32 count = mElementCount;
   if ((mElementCount += delta) > mArraySize)
      resize(mElementCount);
   construct(count, mElementCount);
}

template<class T> inline void Vector<T>::decrement(U32 delta)
{
   LIB_ASSERT(mElementCount != 0, "Vector<T>::decrement - cannot decrement zero-length vector.");

   const U32 count = mElementCount;

   // Determine new count after decrement...
   U32 newCount = mElementCount;
   if (mElementCount > delta)
      newCount -= delta;
   else
      newCount = 0;

   // Destruct removed items...
   destroy(newCount, count);

   // Note new element count.
   mElementCount = newCount;
}

template<class T> inline void Vector<T>::insert(U32 index)
{
   LIB_ASSERT(index <= mElementCount, "Vector<T>::insert - out of bounds index.");

   if(mElementCount == mArraySize)
      resize(mElementCount + 1);
   else
      mElementCount++;

   memmove(&mArray[index + 1],
                    &mArray[index],
                    (mElementCount - index - 1) * sizeof(value_type));
   
   constructInPlace(&mArray[index]);
}

template<class T> inline void Vector<T>::erase(U32 index)
{
   LIB_ASSERT(index < mElementCount, "Vector<T>::erase - out of bounds index!");

   destructInPlace(&mArray[index]);

   if (index < (mElementCount - 1))
   {
         memmove(&mArray[index],
         &mArray[index + 1],
         (mElementCount - index - 1) * sizeof(value_type));
   }

   mElementCount--;
}

template<class T> inline void Vector<T>::erase_fast(U32 index)
{
   LIB_ASSERT(index < mElementCount, "Vector<T>::erase_fast - out of bounds index.");

   // CAUTION: this operator does NOT maintain list order
   // Copy the last element into the deleted 'hole' and decrement the
   //   size of the vector.
   destructInPlace(&mArray[index]);
   if (index < (mElementCount - 1))
      dMemmove(&mArray[index], &mArray[mElementCount - 1], sizeof(value_type));
   mElementCount--;
}

template<class T> inline T& Vector<T>::first()
{
   LIB_ASSERT(mElementCount != 0, "Vector<T>::first - Error, no first element of a zero sized array!");
   return mArray[0];
}

template<class T> inline const T& Vector<T>::first() const
{
   LIB_ASSERT(mElementCount != 0, "Vector<T>::first - Error, no first element of a zero sized array! (const)");
   return mArray[0];
}

template<class T> inline T& Vector<T>::last()
{
   LIB_ASSERT(mElementCount != 0, "Vector<T>::last - Error, no last element of a zero sized array!");
   return mArray[mElementCount - 1];
}

template<class T> inline const T& Vector<T>::last() const
{
   LIB_ASSERT(mElementCount != 0, "Vector<T>::last - Error, no last element of a zero sized array! (const)");
   return mArray[mElementCount - 1];
}

template<class T> inline void Vector<T>::clear()
{
   destroy(0, mElementCount);
   mElementCount = 0;
}

template<class T> inline void Vector<T>::compact()
{
   resize(mElementCount);
}


//-----------------------------------------------------------------------------

template<class T> inline Vector<T>& Vector<T>::operator=(const Vector<T>& p)
{
   if(mElementCount > p.mElementCount)
   {
      destroy(p.mElementCount, mElementCount);
   }
   
   U32 count = getMin( mElementCount, p.mElementCount );
   U32 i;
   for( i=0; i < count; i++ )
   {
      mArray[i] = p.mArray[i];
   }
   
   resize( p.mElementCount );
   
   if( i < p.mElementCount )
   {
      construct(i, p.mElementCount, p.mArray);
   }
   return *this;
}

template<class T> inline typename Vector<T>::iterator Vector<T>::begin()
{
   return mArray;
}

template<class T> inline typename Vector<T>::const_iterator Vector<T>::begin() const
{
   return mArray;
}

template<class T> inline typename Vector<T>::iterator Vector<T>::end()
{
   return mArray + mElementCount;
}

template<class T> inline typename Vector<T>::const_iterator Vector<T>::end() const
{
   return mArray +mElementCount;
}

template<class T> inline S32 Vector<T>::size() const
{
   return (S32)mElementCount;
}

template<class T> inline bool Vector<T>::empty() const
{
   return (mElementCount == 0);
}

template<class T> inline void Vector<T>::insert(iterator p,const T& x)
{
   U32 index = (U32) (p - mArray);
   insert(index);
   mArray[index] = x;
}

template<class T> inline void Vector<T>::erase(iterator q)
{
   erase(U32(q - mArray));
}

template<class T> inline void Vector<T>::erase_fast(iterator q)
{
   erase_fast(U32(q - mArray));
}

template<class T> inline T& Vector<T>::front()
{
   return *begin();
}

template<class T> inline const T& Vector<T>::front() const
{
   return *begin();
}

template<class T> inline T& Vector<T>::back()
{
   return *end();
}

template<class T> inline const T& Vector<T>::back() const
{
   return *end();
}

template<class T> inline void Vector<T>::push_front(const T& x)
{
   insert(0);
   mArray[0] = x;
}

template<class T> inline void Vector<T>::push_back(const T& x)
{
   increment();
   mArray[mElementCount - 1] = x;
}

template<class T> inline void Vector<T>::pop_front()
{
   LIB_ASSERT(mElementCount != 0, "Vector<T>::pop_front - cannot pop the front of a zero-length vector.");
   erase(U32(0));
}

template<class T> inline void Vector<T>::pop_back()
{
   LIB_ASSERT(mElementCount != 0, "Vector<T>::pop_back - cannot pop the back of a zero-length vector.");
   decrement();
}

template<class T> inline T& Vector<T>::operator[](U32 index)
{
   LIB_ASSERT(index < mElementCount, "Vector<T>::operator[] - out of bounds array access!");
   return mArray[index];
}

template<class T> inline const T& Vector<T>::operator[](U32 index) const
{
   LIB_ASSERT(index < mElementCount, "Vector<T>::operator[] - out of bounds array access!");
   return mArray[index];
}

template<class T> inline void Vector<T>::reserve(U32 size)
{
   if (size <= mArraySize)
      return;

   const U32 ec = mElementCount;
   if (resize(size))
      mElementCount = ec;
}

template<class T> inline U32 Vector<T>::capacity() const
{
    return mArraySize;
}

template<class T> inline void Vector<T>::set(void * addr, U32 sz)
{
   setSize(sz);
   if (addr)
      dMemcpy(address(),addr,sz*sizeof(T));
}

//-----------------------------------------------------------------------------

template<class T> inline bool Vector<T>::resize(U32 ecount)
{
#ifdef POWER_DEBUG_GUARD
   return VectorResize(&mArraySize, &mElementCount, (void**) &mArray, ecount, sizeof(T),
                       mFileAssociation, mLineAssociation);
#else
   return VectorResize(&mArraySize, &mElementCount, (void**) &mArray, ecount, sizeof(T));
#endif
}

template<class T> inline void Vector<T>::merge(const Vector& p)
{
   if (!p.size())
      return;

   const S32 oldsize = size();
   resize(oldsize + p.size());
   dMemcpy( &mArray[oldsize], p.address(), p.size() * sizeof(T) );
}

//-----------------------------------------------------------------------------
/// Template for vectors of pointers.
template <class T>
class VectorPtr : public Vector<void*>
{
   /// @deprecated Disallowed.
   VectorPtr(const VectorPtr&);  // Disallowed

  public:
   VectorPtr();
   VectorPtr(const char* fileName, const U32 lineNum);

   /// @name STL interface
   /// @{

   typedef T        value_type;
   typedef T&       reference;
   typedef const T& const_reference;

   typedef T*       iterator;
   typedef const T* const_iterator;
   typedef U32      difference_type;
   typedef U32      size_type;

   iterator       begin();
   const_iterator begin() const;
   iterator       end();
   const_iterator end() const;

   void insert(iterator,const T&);
   void insert(int idx) { Parent::insert(idx); }
   void erase(iterator);

   T&       front();
   const T& front() const;
   T&       back();
   const T& back() const;
   void     push_front(const T&);
   void     push_back(const T&);

   T&       operator[](U32);
   const T& operator[](U32) const;

   /// @}

   /// @name Extended interface
   /// @{

   typedef Vector<void*> Parent;
   T&       first();
   T&       last();
   const T& first() const;
   const T& last() const;
   void erase_fast(U32);
   void erase_fast(iterator);

   /// @}
};


//-----------------------------------------------------------------------------
template<class T> inline VectorPtr<T>::VectorPtr()
{
   //
}

template<class T> inline VectorPtr<T>::VectorPtr(const char* fileName,
                                                 const U32   lineNum)
   : Vector<void*>(fileName, lineNum)
{
   //
}

template<class T> inline T& VectorPtr<T>::first()
{
   return (T&)Parent::first();
}

template<class T> inline const T& VectorPtr<T>::first() const
{
   return (const T)Parent::first();
}

template<class T> inline T& VectorPtr<T>::last()
{
   return (T&)Parent::last();
}

template<class T> inline const T& VectorPtr<T>::last() const
{
   return (const T&)Parent::last();
}

template<class T> inline typename VectorPtr<T>::iterator VectorPtr<T>::begin()
{
   return (iterator)Parent::begin();
}

template<class T> inline typename VectorPtr<T>::const_iterator VectorPtr<T>::begin() const
{
   return (const_iterator)Parent::begin();
}

template<class T> inline typename VectorPtr<T>::iterator VectorPtr<T>::end()
{
   return (iterator)Parent::end();
}

template<class T> inline typename VectorPtr<T>::const_iterator VectorPtr<T>::end() const
{
   return (const_iterator)Parent::end();
}

template<class T> inline void VectorPtr<T>::insert(iterator i,const T& x)
{
   Parent::insert( (Parent::iterator)i, (Parent::reference)x );
}

template<class T> inline void VectorPtr<T>::erase(iterator i)
{
   Parent::erase( (Parent::iterator)i );
}

template<class T> inline void VectorPtr<T>::erase_fast(U32 index)
{
   LIB_ASSERT(index < mElementCount, "VectorPtr<T>::erase_fast - out of bounds index." );

   // CAUTION: this operator does not maintain list order
   // Copy the last element into the deleted 'hole' and decrement the
   //   size of the vector.
   // Assert: index >= 0 && index < mElementCount
   if (index < (mElementCount - 1))
      mArray[index] = mArray[mElementCount - 1];
   decrement();
}

template<class T> inline void VectorPtr<T>::erase_fast(iterator i)
{
   erase_fast(U32(i - iterator(mArray)));
}

template<class T> inline T& VectorPtr<T>::front()
{
   return *begin();
}

template<class T> inline const T& VectorPtr<T>::front() const
{
   return *begin();
}

template<class T> inline T& VectorPtr<T>::back()
{
   return *end();
}

template<class T> inline const T& VectorPtr<T>::back() const
{
   return *end();
}

template<class T> inline void VectorPtr<T>::push_front(const T& x)
{
   Parent::push_front((Parent::const_reference)x);
}

template<class T> inline void VectorPtr<T>::push_back(const T& x)
{
   Parent::push_back((Parent::const_reference)x);
}

template<class T> inline T& VectorPtr<T>::operator[](U32 index)
{
   return (T&)Parent::operator[](index);
}

template<class T> inline const T& VectorPtr<T>::operator[](U32 index) const
{
   return (const T&)Parent::operator[](index);
}

//------------------------------------------------------------------------------

template <class T> class VectorSet : public Vector<T>
{
  using Vector<T>::begin;
  using Vector<T>::end;
public:
   void insert(T dat)
   {
      if(find(begin(), end(), dat) == end())
         push_back(dat);
   }
};

#endif //_TVECTOR_H_

