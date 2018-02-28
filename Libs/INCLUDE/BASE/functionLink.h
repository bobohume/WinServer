#ifndef _BITSET_H_
#define _BITSET_H_
#include "types.h"

struct FunctionLink
{
	virtual ~FunctionLink() {}
};

template <class RR, class AA, class BB, class CC, class DD,
class EE, class FF, class GG, class HH>
struct FunctionBase: public FunctionLink 
{
	virtual RR operator()(AA,BB,CC,DD,EE,FF,GG,HH) = 0;
};

template <class RR, class AA, class BB, class CC, class DD,
class EE, class FF, class GG>
struct FunctionBase<RR,AA,BB,CC,DD,EE,FF,GG,
	EmptyType>: public FunctionLink 
{
	virtual RR operator()(AA,BB,CC,DD,EE,FF,GG) = 0;
};

template <class RR, class AA, class BB, class CC, class DD, 
class EE, class FF>
struct FunctionBase<RR,AA,BB,CC,DD,EE,FF,
	EmptyType,EmptyType>: public FunctionLink 
{
	virtual RR operator()(AA,BB,CC,DD,EE,FF) = 0;
};

template <class RR, class AA, class BB, class CC, class DD,
class EE>
struct FunctionBase<RR,AA,BB,CC,DD,EE,
	EmptyType,EmptyType,EmptyType> : public FunctionLink 
{
	virtual RR operator()(AA,BB,CC,DD,EE) = 0;
};

template <class RR, class AA, class BB, class CC, class DD>
struct FunctionBase<RR,AA,BB,CC,DD,EmptyType,EmptyType,
	EmptyType,EmptyType>: public FunctionLink 
{
	virtual RR operator()(AA,BB,CC,DD) = 0;
};

template <class RR, class AA, class BB, class CC>
struct FunctionBase<RR,AA,BB,CC,EmptyType,EmptyType,
	EmptyType,EmptyType,EmptyType>: public FunctionLink 
{
	virtual RR operator()(AA,BB,CC) = 0;
};

template <class RR, class AA, class BB>
struct FunctionBase<RR,AA,BB,EmptyType,EmptyType,EmptyType,EmptyType,
	EmptyType,EmptyType>: public FunctionLink 
{
	virtual RR operator()(AA,BB) = 0;
};

template <class RR, class AA>
struct FunctionBase<RR,AA,EmptyType,EmptyType,EmptyType,
	EmptyType,EmptyType,EmptyType,EmptyType>: public FunctionLink 
{
	virtual RR operator()(AA) = 0;
};

template <class RR>
struct FunctionBase<RR,EmptyType,EmptyType,EmptyType,EmptyType,EmptyType,
	EmptyType,EmptyType,EmptyType> : public FunctionLink 
{
	virtual RR operator()() = 0;
};
	
//------------------------FunctionMethod---------------------------//
template <class T, class U, class R, class A, class B, class C, class D,
class E, class F, class G, class H>
struct FunctionMethod: public FunctionBase<R,A,B,C,D,E,F,G,H> {
	T object;
	U func;
	FunctionMethod(T ptr,U ff): object(ptr),func(ff) {}
	R operator()(A a,B b,C c,D d,E e,F f,G g,H h) {  return (object->*func)(a,b,c,d,e,f,g,h); }
};

template <class T, class U, class R, class A, class B, class C, class D,
class E, class F, class G>
struct FunctionMethod<T, U, R, A, B, C, D, E, F, G, EmptyType>: public FunctionBase<R,A,B,C,D,E,F,G,EmptyType> {
	T object;
	U func;
	FunctionMethod(T ptr,U ff): object(ptr),func(ff) {}
	R operator()(A a,B b,C c,D d,E e,F f,G g) {  return (object->*func)(a,b,c,d,e,f,g); }
};

template <class T, class U, class R, class A, class B, class C, class D,
class E, class F>
struct FunctionMethod<T, U, R, A, B, C, D, E, F, EmptyType, EmptyType>: public FunctionBase<R,A,B,C,D,E,F,EmptyType,EmptyType> {
	T object;
	U func;
	FunctionMethod(T ptr,U ff): object(ptr),func(ff) {}
	R operator()(A a,B b,C c,D d,E e,F f) {  return (object->*func)(a,b,c,d,e,f); }
};

template <class T, class U, class R, class A, class B, class C, class D,
class E>
struct FunctionMethod<T, U, R, A, B, C, D, E, EmptyType, EmptyType, EmptyType>: public FunctionBase<R,A,B,C,D,E,EmptyType,EmptyType,EmptyType> {
	T object;
	U func;
	FunctionMethod(T ptr,U ff): object(ptr),func(ff) {}
	R operator()(A a,B b,C c,D d,E e) {  return (object->*func)(a,b,c,d,e); }
};

template <class T, class U, class R, class A, class B, class C, class D>
struct FunctionMethod<T, U, R, A, B, C, D, EmptyType, EmptyType, EmptyType, EmptyType>: public FunctionBase<R,A,B,C,D,EmptyType,EmptyType,EmptyType,EmptyType> {
	T object;
	U func;
	FunctionMethod(T ptr,U ff): object(ptr),func(ff) {}
	R operator()(A a,B b,C c,D d) {  return (object->*func)(a,b,c,d); }
};

template <class T, class U, class R, class A, class B, class C>
struct FunctionMethod<T, U, R, A, B, C, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>: public FunctionBase<R,A,B,C,EmptyType,EmptyType,EmptyType,EmptyType,EmptyType> {
	T object;
	U func;
	FunctionMethod(T ptr,U ff): object(ptr),func(ff) {}
	R operator()(A a,B b,C c) {  return (object->*func)(a,b,c); }
};

template <class T, class U, class R, class A, class B>
struct FunctionMethod<T, U, R, A, B, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>: public FunctionBase<R,A,B,EmptyType,EmptyType,EmptyType,EmptyType,EmptyType,EmptyType> {
	T object;
	U func;
	FunctionMethod(T ptr,U ff): object(ptr),func(ff) {}
	R operator()(A a,B b) {  return (object->*func)(a,b); }
};

template <class T, class U, class R, class A>
struct FunctionMethod<T, U, R, A, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>: public FunctionBase<R,A,EmptyType,EmptyType,EmptyType,EmptyType,EmptyType,EmptyType,EmptyType> {
	T object;
	U func;
	FunctionMethod(T ptr,U ff): object(ptr),func(ff) {}
	R operator()(A a) {  return (object->*func)(a); }
};

template <class T, class U, class R>
struct FunctionMethod<T, U, R, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>: public FunctionBase<R,EmptyType,EmptyType,EmptyType,EmptyType,EmptyType,EmptyType,EmptyType,EmptyType> {
	T object;
	U func;
	FunctionMethod(T ptr,U ff): object(ptr),func(ff) {}
	R operator()() {  return (object->*func)(); }
};

//------------------------FunctionGlobal---------------------------//
template <class T, class R, class A, class B, class C, class D,
class E, class F, class G, class H>
struct FunctionGlobal: public FunctionBase<R,A,B,C,D,E,F,G,H> {
	T func;
	FunctionGlobal(T ff): func(ff) {}
	R operator()(A a,B b,C c,D d,E e,F f,G g,H h) {  return (*func)(a,b,c,d,e,f,g,h); }
};

template <class T, class R, class A, class B, class C, class D,
class E, class F, class G>
struct FunctionGlobal<T, R, A, B, C, D, E, F, G, EmptyType>: public FunctionBase<R,A,B,C,D,E,F,G,EmptyType> {
	T func;
	FunctionGlobal(T ff): func(ff) {}
	R operator()(A a,B b,C c,D d,E e,F f,G g) {  return (*func)(a,b,c,d,e,f,g); }
};

template <class T, class R, class A, class B, class C, class D,
class E, class F>
struct FunctionGlobal<T, R, A, B, C, D, E, F, EmptyType, EmptyType>: public FunctionBase<R,A,B,C,D,E,F,EmptyType,EmptyType> {
	T func;
	FunctionGlobal(T ff): func(ff) {}
	R operator()(A a,B b,C c,D d,E e,F f) {  return (*func)(a,b,c,d,e,f); }
};

template <class T, class R, class A, class B, class C, class D,
class E>
struct FunctionGlobal<T, R, A, B, C, D, E, EmptyType, EmptyType, EmptyType>: public FunctionBase<R,A,B,C,D,E,EmptyType,EmptyType,EmptyType> {
	T func;
	FunctionGlobal(T ff): func(ff) {}
	R operator()(A a,B b,C c,D d,E e) {  return (*func)(a,b,c,d,e); }
};

template <class T, class R, class A, class B, class C, class D>
struct FunctionGlobal<T, R, A, B, C, D, EmptyType, EmptyType, EmptyType, EmptyType>: public FunctionBase<R,A,B,C,D,EmptyType,EmptyType,EmptyType,EmptyType> {
	T func;
	FunctionGlobal(T ff): func(ff) {}
	R operator()(A a,B b,C c,D d) {  return (*func)(a,b,c,d); }
};

template <class T, class R, class A, class B, class C>
struct FunctionGlobal<T, R, A, B, C, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>: public FunctionBase<R,A,B,C,EmptyType,EmptyType,EmptyType,EmptyType,EmptyType> {
	T func;
	FunctionGlobal(T ff): func(ff) {}
	R operator()(A a,B b,C c) {  return (*func)(a,b,c); }
};

template <class T, class R, class A, class B>
struct FunctionGlobal<T, R, A, B, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>: public FunctionBase<R,A,B,EmptyType,EmptyType,EmptyType,EmptyType,EmptyType,EmptyType> {
	T func;
	FunctionGlobal(T ff): func(ff) {}
	R operator()(A a,B b) {  return (*func)(a,b); }
};

template <class T, class R, class A>
struct FunctionGlobal<T, R, A, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>: public FunctionBase<R,A,EmptyType,EmptyType,EmptyType,EmptyType,EmptyType,EmptyType,EmptyType> {
	T func;
	FunctionGlobal(T ff): func(ff) {}
	R operator()(A a) {  return (*func)(a); }
};

template <class T, class R>
struct FunctionGlobal<T, R, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>: public FunctionBase<R,EmptyType,EmptyType,EmptyType,EmptyType,EmptyType,EmptyType,EmptyType,EmptyType> {
	T func;
	FunctionGlobal(T ff): func(ff) {}
	R operator()() {  return (*func)(); }
};

//--------------------CreateFunctionMethod---------------------------//
template <class T, class U, class R, class A, class B, class C, class D,
class E, class F, class G, class H>
FunctionLink* CreateFunctionMethod(T obj, U fun)
{
	return 	new FunctionMethod<T, U, R, A, B, C, D, E, F, G, H>(obj, fun);
};

template <class T, class U, class R, class A, class B, class C, class D,
class E, class F, class G>
FunctionLink* CreateFunctionMethod(T obj, U fun)
{
	return 	new FunctionMethod<T, U, R, A, B, C, D, E, F, G, EmptyType>(obj, fun);
};

template <class T, class U, class R, class A, class B, class C, class D,
class E, class F>
FunctionLink* CreateFunctionMethod(T obj, U fun)
{
	return 	new FunctionMethod<T, U, R, A, B, C, D, E, F, EmptyType, EmptyType>(obj, fun);
};

template <class T, class U, class R, class A, class B, class C, class D,
class E>
FunctionLink* CreateFunctionMethod(T obj, U fun)
{
	return 	new FunctionMethod<T, U, R, A, B, C, D, E, EmptyType, EmptyType, EmptyType>(obj, fun);
};

template <class T, class U, class R, class A, class B, class C, class D>
FunctionLink* CreateFunctionMethod(T obj, U fun)
{
	return 	new FunctionMethod<T, U, R, A, B, C, D, EmptyType, EmptyType, EmptyType, EmptyType>(obj, fun);
};

template <class T, class U, class R, class A, class B, class C>
FunctionLink* CreateFunctionMethod(T obj, U fun)
{
	return 	new FunctionMethod<T, U, R, A, B, C, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>(obj, fun);
};

template <class T, class U, class R, class A, class B>
FunctionLink* CreateFunctionMethod(T obj, U fun)
{
	return 	new FunctionMethod<T, U, R, A, B, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>(obj, fun);
};

template <class T, class U, class R, class A>
FunctionLink* CreateFunctionMethod(T obj, U fun)
{
	return 	new FunctionMethod<T, U, R, A, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>(obj, fun);
};

template <class T, class U, class R>
FunctionLink* CreateFunctionMethod(T obj, U fun)
{
	return 	new FunctionMethod<T, U, R, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>(obj, fun);
};

//--------------------CreateFunctionGlobal---------------------------//
template <class T, class R, class A, class B, class C, class D,
class E, class F, class G, class H>
FunctionLink* CreateFunctionGlobal(T fun)
{
	return 	new FunctionGlobal<T, R, A, B, C, D, E, F, G, H>(fun);
};

template <class T, class R, class A, class B, class C, class D,
class E, class F, class G>
FunctionLink* CreateFunctionGlobal(T fun)
{
	return 	new FunctionGlobal<T, R, A, B, C, D, E, F, G, EmptyType>(fun);
};

template <class T, class R, class A, class B, class C, class D,
class E, class F>
FunctionLink* CreateFunctionGlobal(T fun)
{
	return 	new FunctionGlobal<T, R, A, B, C, D, E, F, EmptyType, EmptyType>(fun);
};

template <class T, class R, class A, class B, class C, class D,
class E>
FunctionLink* CreateFunctionGlobal(T fun)
{
	return 	new FunctionGlobal<T, R, A, B, C, D, E, EmptyType, EmptyType, EmptyType>(fun);
};

template <class T, class R, class A, class B, class C, class D>
FunctionLink* CreateFunctionGlobal(T fun)
{
	return 	new FunctionGlobal<T, R, A, B, C, D, EmptyType, EmptyType, EmptyType, EmptyType>(fun);
};

template <class T, class R, class A, class B, class C>
FunctionLink* CreateFunctionGlobal(T fun)
{
	return 	new FunctionGlobal<T, R, A, B, C, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>(fun);
};

template <class T, class R, class A, class B>
FunctionLink* CreateFunctionGlobal(T fun)
{
	return 	new FunctionGlobal<T, R, A, B, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>(fun);
};

template <class T, class R, class A>
FunctionLink* CreateFunctionGlobal(T fun)
{
	return 	new FunctionGlobal<T, R, A, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>(fun);
};

template <class T, class R>
FunctionLink* CreateFunctionGlobal(T fun)
{
	return 	new FunctionGlobal<T, R, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>(fun);
};

//快捷创建void类型的函数
//--------------------CreateRVFunctionMethod---------------------------//
template <class T, class U, class A, class B, class C, class D,
class E, class F, class G, class H>
FunctionLink* CreateRVFunctionMethod(T obj, U fun)
{
	return 	new FunctionMethod<T, U, void, A, B, C, D, E, F, G, H>(obj, fun);
};

template <class T, class U, class A, class B, class C, class D,
class E, class F, class G>
FunctionLink* CreateRVFunctionMethod(T obj, U fun)
{
	return 	new FunctionMethod<T, U, void, A, B, C, D, E, F, G, EmptyType>(obj, fun);
};

template <class T, class U, class A, class B, class C, class D,
class E, class F>
FunctionLink* CreateRVFunctionMethod(T obj, U fun)
{
	return 	new FunctionMethod<T, U, void, A, B, C, D, E, F, EmptyType, EmptyType>(obj, fun);
};

template <class T, class U, class A, class B, class C, class D,
class E>
FunctionLink* CreateRVFunctionMethod(T obj, U fun)
{
	return 	new FunctionMethod<T, U, void, A, B, C, D, E, EmptyType, EmptyType, EmptyType>(obj, fun);
};

template <class T, class U, class A, class B, class C, class D>
FunctionLink* CreateRVFunctionMethod(T obj, U fun)
{
	return 	new FunctionMethod<T, U, void, A, B, C, D, EmptyType, EmptyType, EmptyType, EmptyType>(obj, fun);
};

template <class T, class U, class A, class B, class C>
FunctionLink* CreateRVFunctionMethod(T obj, U fun)
{
	return 	new FunctionMethod<T, U, void, A, B, C, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>(obj, fun);
};

template <class T, class U, class A, class B>
FunctionLink* CreateRVFunctionMethod(T obj, U fun)
{
	return 	new FunctionMethod<T, U, void, A, B, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>(obj, fun);
};

template <class T, class U, class A>
FunctionLink* CreateRVFunctionMethod(T obj, U fun)
{
	return 	new FunctionMethod<T, U, void, A, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>(obj, fun);
};

template <class T, class U>
FunctionLink* CreateRVFunctionMethod(T obj, U fun)
{
	return 	new FunctionMethod<T, U, void, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>(obj, fun);
};

//--------------------CreateRVFunctionGlobal---------------------------//
template <class T, class A, class B, class C, class D,
class E, class F, class G, class H>
	FunctionLink* CreateRVFunctionGlobal(T fun)
{
	return 	new FunctionGlobal<T,void, A, B, C, D, E, F, G, H>(fun);
};

template <class T, class A, class B, class C, class D,
class E, class F, class G>
	FunctionLink* CreateRVFunctionGlobal(T fun)
{
	return 	new FunctionGlobal<T, void, A, B, C, D, E, F, G, EmptyType>(fun);
};

template <class T, class A, class B, class C, class D,
class E, class F>
	FunctionLink* CreateRVFunctionGlobal(T fun)
{
	return 	new FunctionGlobal<T, void, A, B, C, D, E, F, EmptyType, EmptyType>(fun);
};

template <class T, class A, class B, class C, class D,
class E>
	FunctionLink* CreateRVFunctionGlobal(T fun)
{
	return 	new FunctionGlobal<T, void, A, B, C, D, E, EmptyType, EmptyType, EmptyType>(fun);
};

template <class T, class A, class B, class C, class D>
FunctionLink* CreateRVFunctionGlobal(T fun)
{
	return 	new FunctionGlobal<T, void, A, B, C, D, EmptyType, EmptyType, EmptyType, EmptyType>(fun);
};

template <class T, class A, class B, class C>
FunctionLink* CreateRVFunctionGlobal(T fun)
{
	return 	new FunctionGlobal<T, void, A, B, C, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>(fun);
};

template <class T, class A, class B>
FunctionLink* CreateRVFunctionGlobal(T fun)
{
	return 	new FunctionGlobal<T, void, A, B, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>(fun);
};

template <class T, class A>
FunctionLink* CreateRVFunctionGlobal(T fun)
{
	return 	new FunctionGlobal<T, void, A, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>(fun);
};

template <class T>
FunctionLink* CreateRVFunctionGlobal(T fun)
{
	return 	new FunctionGlobal<T, void, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>(fun);
};

#endif