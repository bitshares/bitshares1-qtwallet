#include "html5viewer/html5viewer.h"
#include "bts_xt_thread.h"

#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <fc/thread/thread.hpp>
#include <fc/filesystem.hpp>
#include <bts/blockchain/config.hpp>
#include <signal.h>

#include <QApplication>
#include <QPixmap>
#include <QSplashScreen>

bool exit_signal = false;
std::shared_ptr<BtsXtThread> btsxt;

void handle_signal( int signum )
{
    std::cout<< "Signal " << signum << " caught. exiting.." << std::endl;
    exit_signal = true;
    btsxt->stop();
}

int main( int argc, char** argv )
{
    signal(SIGABRT, &handle_signal);
    signal(SIGTERM, &handle_signal);
    signal(SIGINT, &handle_signal);
    
    btsxt = std::make_shared<BtsXtThread>(argc, argv);
    btsxt->start();
    
    QString initial_url = "http://127.0.0.1:5680";
    //if(!fc::exists( datadir / "default_wallet.dat" ))
    //    initial_url = "http://127.0.0.1:5680/blank.html#/createwallet";
        
    QApplication app(argc, argv);
    
    // TODO: splash_screen.png's path should be loaded from config
    QPixmap pixmap("/Users/vz/work/i3/qt_wallet/splash_screen.png");
    QSplashScreen splash(pixmap);
    splash.show();
    
    splash.showMessage(QObject::tr("Starting RPC Server..."),
                       Qt::AlignCenter | Qt::AlignBottom, Qt::white);    
    qApp->processEvents();
    
    btsxt->wait_until_initialized();
    QThread::sleep(1); // let's give rpc server one more second to bind to port and start listening
    
    Html5Viewer viewer;
    viewer.setOrientation(Html5Viewer::ScreenOrientationAuto);
    viewer.resize(1200,800);
    viewer.show();
    
    
    QUrl url = QUrl(initial_url);
    url.setUserName("user");
    url.setPassword("pass");
    viewer.loadUrl(url);
    
    splash.finish(&viewer);
    
    app.exec();    
  
    btsxt->stop();
    btsxt->wait();
    
    return 0;
}
