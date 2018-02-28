#ifndef _THREADPROCEDURE_H_
#define _THREADPROCEDURE_H_

class ThreadProcedure
{
public:
    virtual ~ThreadProcedure() {}
    virtual int run(void) = 0;
};

#endif /*_THREADPROCEDURE_H_*/
