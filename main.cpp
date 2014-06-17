#include "html5viewer/html5viewer.h"
#include "clientwrapper.hpp"
#include "utilities.hpp"

#include <boost/thread.hpp>
#include <bts/blockchain/config.hpp>
#include <signal.h>

#include <QApplication>
#include <QSettings>
#include <QPixmap>
#include <QErrorMessage>
#include <QSplashScreen>
#include <QDir>
#include <QWebSettings>
#include <QWebPage>
#include <QWebFrame>
#include <QJsonDocument>
#include <QGraphicsWebView>
#include <QTimer>

#include <boost/program_options.hpp>

#include <bts/client/client.hpp>
#include <bts/net/upnp.hpp>
#include <bts/blockchain/chain_database.hpp>
#include <bts/rpc/rpc_server.hpp>
#include <bts/cli/cli.hpp>
#include <bts/utilities/git_revision.hpp>
#include <fc/filesystem.hpp>
#include <fc/thread/thread.hpp>
#include <fc/log/file_appender.hpp>
#include <fc/log/logger_config.hpp>
#include <fc/io/json.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/git_revision.hpp>
#include <fc/io/json.hpp>
#include <fc/log/logger_config.hpp>

#include <boost/iostreams/tee.hpp>
#include <boost/iostreams/stream.hpp>
#include <fstream>

#include <QResource>
#include <iostream>
#include <iomanip>

#ifdef NDEBUG
#include "config_prod.hpp"
#else
#include "config_dev.hpp"
#endif

int main( int argc, char** argv )
{
   QCoreApplication::setOrganizationName( "BitShares" );
   QCoreApplication::setOrganizationDomain( "bitshares.org" );
   QCoreApplication::setApplicationName( BTS_BLOCKCHAIN_NAME );
   QApplication app(argc, argv);

   QTimer fc_tasks;
   fc_tasks.connect( &fc_tasks, &QTimer::timeout, [](){ fc::usleep( fc::microseconds( 3000 ) ); } );
   fc_tasks.start();

   QPixmap pixmap(":/images/splash_screen.png");
   QSplashScreen splash(pixmap);
      splash.showMessage(QObject::tr("Loading configuration..."),
                         Qt::AlignCenter | Qt::AlignBottom, Qt::white);
   splash.show();
   qApp->processEvents();

   try {
    ClientWrapper client;
    client.connect(&client, &ClientWrapper::error, [&](QString errorString) {
        splash.showMessage(errorString, Qt::AlignCenter | Qt::AlignBottom, Qt::white);
        fc::usleep( fc::seconds(3) );
    });
    client.initialize();

    ilog( "process events" );
    qApp->processEvents();
    
    Html5Viewer viewer;
    splash.finish(&viewer);

    QWebSettings::globalSettings()->setAttribute( QWebSettings::PluginsEnabled, false );
    viewer.webView()->page()->settings()->setAttribute( QWebSettings::PluginsEnabled, false );
    viewer.setOrientation(Html5Viewer::ScreenOrientationAuto);
    viewer.resize(1200,800);
    viewer.webView()->setAcceptHoverEvents(true);
    viewer.webView()->page()->mainFrame()->addToJavaScriptWindowObject("bitshares", &client);
    viewer.webView()->page()->mainFrame()->addToJavaScriptWindowObject("utilities", new Utilities, QWebFrame::ScriptOwnership);

    /*
    connect(QWebView::page()->networkAccessManager(), SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
            this, SLOT(handleAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
     
    void handleAuthenticationRequired(QNetworkReply*, QAuthenticator* authenticator) {
         authenticator->setUser("username");
           authenticator->setPassword("password");
    }
    */

    
    ilog( "loadURL" );
    // ON COMPLETE..... 
    viewer.loadUrl(client.http_url());
    viewer.show();
    ///

    
    return app.exec();
   }
   catch ( const fc::exception& e) 
   {
      elog( "${e}", ("e",e.to_detail_string() ) );
      QErrorMessage::qtHandler()->showMessage( e.to_string().c_str() );
   }
}
