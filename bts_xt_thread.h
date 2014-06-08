#ifndef BTSXTTHREAD_H
#define BTSXTTHREAD_H

#include <memory>
#include <QThread>
#include <QSemaphore>

namespace boost {
    namespace program_options { class variables_map; }
}

namespace bts {
    namespace client { class client; }
}

class BtsXtThread : public QThread
{
    Q_OBJECT
    int _argc;
    char** _argv;
    QSemaphore sem;
    
public: 
    BtsXtThread(int argc, char** argv)
    : QThread(0), _argc(argc), _argv(argv), sem(0)
    {
    }
    
    ~BtsXtThread() {}
    
    void run() Q_DECL_OVERRIDE ;
    void stop();
    void wait_until_initialized() { sem.acquire(1); }

signals:
    void resultReady(const QString &s);
};


#endif
