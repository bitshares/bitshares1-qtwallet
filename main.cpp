#include "html5viewer/html5viewer.h"
#include "ClientWrapper.hpp"
#include "Utilities.hpp"
#include "MainWindow.hpp"
#include "config.hpp"

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
#include <QAuthenticator>
#include <QNetworkReply>
#include <QResource>
#include <QGraphicsWidget>
#include <QMainWindow>
#include <QMenuBar>
#include <QLayout>
#include <QLocalSocket>
#include <QLocalServer>
#include <QMessageBox>

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
#include <fc/signals.hpp>

#include <boost/iostreams/tee.hpp>
#include <boost/iostreams/stream.hpp>

#include <fstream>
#include <iostream>
#include <iomanip>

#ifdef NDEBUG
#include "config_prod.hpp"
#else
#include "config_dev.hpp"
#endif

void setupMenus(ClientWrapper* client, MainWindow* mainWindow)
{
    auto accountMenu = mainWindow->accountMenu();

    accountMenu->addAction("&Go to My Accounts", mainWindow, SLOT(goToMyAccounts()));
    accountMenu->addAction("&Create Account", mainWindow, SLOT(goToCreateAccount()));
    accountMenu->addAction("&Import Account")->setEnabled(false);
}

void prepareStartupSequence(ClientWrapper* client, Html5Viewer* viewer, MainWindow* mainWindow, QSplashScreen* splash)
{
    viewer->webView()->page()->mainFrame()->addToJavaScriptWindowObject("bitshares", client);
    viewer->webView()->page()->mainFrame()->addToJavaScriptWindowObject("magic_unicorn", new Utilities, QWebFrame::ScriptOwnership);

    QObject::connect(viewer->webView()->page()->networkAccessManager(), &QNetworkAccessManager::authenticationRequired,
                     [client](QNetworkReply*, QAuthenticator *auth) {
       auth->setUser(client->http_url().userName());
       auth->setPassword(client->http_url().password());
    });
    client->connect(client, &ClientWrapper::initialized, [viewer,client,mainWindow]() {
       ilog( "Client initialized; loading web interface from ${url}", ("url", client->http_url().toString().toStdString()) );
       client->status_update("Calculating last 3 digits of pi");
       viewer->webView()->load(client->http_url());
       //Now we know the URL of the app, so we can create the items in the Accounts menu
       setupMenus(client, mainWindow);
    });
    auto loadFinishedConnection = std::make_shared<QMetaObject::Connection>();
    *loadFinishedConnection = viewer->connect(viewer->webView(), &QGraphicsWebView::loadFinished, [mainWindow,splash,viewer,loadFinishedConnection](bool ok) {
       ilog( "Webview loaded: ${status}", ("status", ok) );
       viewer->disconnect(*loadFinishedConnection);
       mainWindow->show();
       splash->finish(mainWindow);
       mainWindow->processDeferredUrl();
    });
    client->connect(client, &ClientWrapper::error, [=](QString errorString) {
       splash->hide();
       QMessageBox::critical(nullptr, QObject::tr("Error"), errorString);
       exit(1);
    });
    client->connect(client, &ClientWrapper::status_update, [=](QString messageString) {
       splash->showMessage(messageString, Qt::AlignCenter | Qt::AlignBottom, Qt::white);
    });
}

QLocalServer* startSingleInstanceServer(MainWindow* mainWindow)
{
    QLocalServer* singleInstanceServer = new QLocalServer();
    if( !singleInstanceServer->listen(BTS_BLOCKCHAIN_NAME) )
    {
        std::cerr << "Could not start new instance listener. Attempting to remove defunct listener... ";
        QLocalServer::removeServer(BTS_BLOCKCHAIN_NAME);
        if( !singleInstanceServer->listen(BTS_BLOCKCHAIN_NAME) )
        {
            std::cerr << "Failed to start new instance listener: " << singleInstanceServer->errorString().toStdString() << std::endl;
            exit(1);
        }
        std::cerr << "Success.\n";
    }

    std::cout << "Listening for new instances on " << singleInstanceServer->fullServerName().toStdString() << std::endl;
    singleInstanceServer->connect(singleInstanceServer, &QLocalServer::newConnection, [singleInstanceServer,mainWindow](){
        QLocalSocket* zygote = singleInstanceServer->nextPendingConnection();
        QEventLoop waitLoop;

        zygote->connect(zygote, &QLocalSocket::readyRead, &waitLoop, &QEventLoop::quit);
        QTimer::singleShot(1000, &waitLoop, SLOT(quit()));
        waitLoop.exec();

        mainWindow->raise();
        mainWindow->activateWindow();

        if( zygote->bytesAvailable() )
        {
            QByteArray message = zygote->readLine();
            ilog("Got message from new instance: ${msg}", ("msg",message.data()));
            mainWindow->processCustomUrl(message);
        }
        zygote->close();

        delete zygote;
    });

    return singleInstanceServer;
}

int main( int argc, char** argv )
{
   QCoreApplication::setOrganizationName( "BitShares" );
   QCoreApplication::setOrganizationDomain( "bitshares.org" );
   QCoreApplication::setApplicationName( BTS_BLOCKCHAIN_NAME );
   QApplication app(argc, argv);
   app.setWindowIcon(QIcon(":/images/qtapp.ico"));

   MainWindow mainWindow;

   //Custom URL handling. OSX handles this differently from Windows and Linux
   //On OSX, the OS will always pass the URL as an event to QApplication.
   //Windows and Linux will just run our program with the URL as an argument.
#ifdef __APPLE__
   //Install OSX event handler
   app.installEventFilter(&mainWindow);
#endif

   //We'll go ahead and leave Win/Lin URL handling available in OSX too
   QLocalSocket* sock = new QLocalSocket();
   sock->connectToServer(BTS_BLOCKCHAIN_NAME);
   if( sock->waitForConnected(100) )
   {
       if( argc > 1 && app.arguments()[1].startsWith(QString(CUSTOM_URL_SCHEME) + ":") )
       {
           //Need to open a custom URL. Pass it to pre-existing instance.
           std::cout << "Found instance already running. Sending message and exiting." << std::endl;
           sock->write(argv[1]);
           sock->waitForBytesWritten();
           sock->close();
       }
       //Note that we connected, but may not have sent anything. This means that another instance is already
       //running. The fact that we connected prompted it to request focus; we will just exit now.
       delete sock;
       return 0;
   }
   else
   {
       if( argc > 1 && app.arguments()[1].startsWith(QString(CUSTOM_URL_SCHEME) + ":") )
       {
           //No other instance running. Handle URL when we get started up.
           mainWindow.deferCustomUrl(app.arguments()[1]);
       }

       //Could not connect to already-running instance. Start a server so future instances connect to us
       QLocalServer* singleInstanceServer = startSingleInstanceServer(&mainWindow);
       app.connect(&app, &QApplication::aboutToQuit, singleInstanceServer, &QLocalServer::deleteLater);
   }
   delete sock;

   auto viewer = new Html5Viewer;
   ClientWrapper client;

#ifdef NDEBUG
   app.connect(&app, &QApplication::aboutToQuit, [&client,&singleInstanceServer](){
       client.get_client()->get_wallet()->close();
       client.get_client()->get_chain()->close();
       exit(0);
   });
#endif

   mainWindow.setCentralWidget(viewer);
   mainWindow.setClientWrapper(&client);

   QTimer fc_tasks;
   fc_tasks.connect( &fc_tasks, &QTimer::timeout, [](){ fc::usleep( fc::microseconds( 1000 ) ); } );
   fc_tasks.start(33);

   QPixmap pixmap(":/images/splash_screen.jpg");
   QSplashScreen splash(pixmap);
   splash.showMessage(QObject::tr("Loading configuration..."),
                      Qt::AlignCenter | Qt::AlignBottom, Qt::white);
   splash.show();

   prepareStartupSequence(&client, viewer, &mainWindow, &splash);

   QWebSettings::globalSettings()->setAttribute( QWebSettings::PluginsEnabled, false );

   try {
    client.initialize();
    return app.exec();
   }
   catch ( const fc::exception& e) 
   {
      elog( "${e}", ("e",e.to_detail_string() ) );
      QErrorMessage::qtHandler()->showMessage( e.to_string().c_str() );
   }
}
