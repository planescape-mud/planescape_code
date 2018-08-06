/* $Id$
 *
 * ruffina, 2009
 */
#ifndef __IOMANAGER_H__
#define __IOMANAGER_H__

#ifndef __MINGW32__
#include <sys/select.h>
#else
#include <winsock.h>
#endif

#include "plugin.h"
#include "schedulertask.h"
#include "oneallocate.h"

class IOManager : public Plugin, public OneAllocate {
    friend class IOInitTask;
    friend class IOPollTask;
    friend class IOReadTask;
    friend class IOWriteTask;
public:
    IOManager();
    virtual ~IOManager();

    virtual void initialization();
    virtual void destruction();

    static IOManager *getThis() {
	return thisClass;
    }
protected:
    void ioInit();
    void ioPoll();
    void ioRead();
    void ioWrite();

private:
    fd_set input_set;
    fd_set output_set;
    fd_set exc_set;
    int maxdesc;
    
    static IOManager *thisClass;
};

struct IOTask : public SchedulerTask { 
    typedef ::Pointer<IOTask> Pointer;
    virtual void after();
};

struct IOInitTask : public IOTask {
    typedef ::Pointer<IOInitTask> Pointer;
    virtual void run();
    virtual int getPriority() const;
};

struct IOPollTask : public IOTask {
    typedef ::Pointer<IOPollTask> Pointer;
    virtual void run();
    virtual int getPriority() const;
};

struct IOReadTask : public IOTask {
    typedef ::Pointer<IOReadTask> Pointer;
    virtual void run();
    virtual int getPriority() const;
};

struct IOWriteTask : public IOTask {
    typedef ::Pointer<IOWriteTask> Pointer;
    virtual void run();
    virtual int getPriority() const;
};

#endif
