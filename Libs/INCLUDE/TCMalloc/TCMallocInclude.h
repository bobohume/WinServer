#ifndef _TCMALLOCINCLUDE_H_
#define _TCMALLOCINCLUDE_H_

/************************************************************************/
/* 使用tcmalloc，包含此文件
/************************************************************************/

//extern bool Init(void)
//{
//    return true;
//}
//
//extern void UnInit(void)
//{
//}


extern inline void* tc_malloc(size_t size);

extern inline void  tc_free(void* ptr);

extern inline void* tc_calloc(size_t n, size_t elem_size);

extern inline void* tc_realloc(void* old_ptr,size_t new_size);

extern inline void* tc_new(size_t size);

extern inline void  tc_delete(void* ptr);

#endif /*_TCMALLOCINCLUDE_H_*/