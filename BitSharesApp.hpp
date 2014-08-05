#pragma once

#include <QApplication>

#include <list>

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

    /// Overrided from QApplication to catch all exceptions.
    virtual bool notify(QObject * receiver, QEvent * e) override;
  
  /// Class attributes;
  private:
    static BitSharesApp* _instance;
};