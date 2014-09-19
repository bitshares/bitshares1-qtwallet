#pragma once

#include <fc/exception/exception.hpp>

#include <QApplication>

class ClientWrapper;
class Html5Viewer;
class MainWindow;
class QSplashScreen;
class QLocalServer;

/** Subclass needed to reimplement 'notify' method and catch unknown exceptions.
*/
class BitSharesApp : protected QApplication
{
  Q_OBJECT

  public:
    /** Builds & starts the app.
        When this function completes it also destroys application object.
    */
    static int  run(int& argc, char** argv);

  private:
    BitSharesApp(int& argc, char** argv);
    virtual ~BitSharesApp();
    int run();

    void prepareStartupSequence(ClientWrapper* client, Html5Viewer* viewer, MainWindow* mainWindow, QSplashScreen* splash);
    QLocalServer* startSingleInstanceServer(MainWindow* mainWindow);

    void onExceptionCaught(const fc::exception& e);
    void onUnknownExceptionCaught();
    void displayFailureInfo(const std::string& detail);

    /// Overrided from QApplication to catch all exceptions.
    virtual bool notify(QObject * receiver, QEvent * e) override;
  
  /// Class attributes;
  private:
    static BitSharesApp* _instance;
};
