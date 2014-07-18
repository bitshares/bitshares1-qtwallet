#pragma once

#include "ClientWrapper.hpp"
#include "html5viewer/html5viewer.h"

#include <QMainWindow>
#include <QSettings>
#include <QMenu>


class MainWindow : public QMainWindow
{
    Q_OBJECT
    QSettings _settings;
    QMenu* _fileMenu;
    QMenu* _accountMenu;
    QString _deferredUrl;
    
  public:
    MainWindow();
    QMenu* fileMenu() { return _fileMenu; }
    QMenu* accountMenu() { return _accountMenu; }

#ifdef __APPLE__
    bool eventFilter(QObject* object, QEvent* event);
#endif
    
    ClientWrapper *clientWrapper() const;
    void setClientWrapper(ClientWrapper* clientWrapper);

    public Q_SLOTS:
    void goToMyAccounts();
    void goToAccount(QString accountName);
    void goToCreateAccount();

    ///Used to schedule a custom URL for processing later, once the app has finished starting
    void deferCustomUrl(QString url);
    ///Triggers the deferred URL processign. Call once app has finished starting
    void processDeferredUrl();
    ///Used to process a custom URL now (only call if app has finished starting)
    void processCustomUrl(QString url);
    void goToBlock(uint32_t blockNumber);
    void goToBlock(QString blockId);
    void goToTransaction(QString transactionId);
  private:
    ClientWrapper* _clientWrapper;

    Html5Viewer* getViewer();
    bool walletIsUnlocked(bool promptToUnlock = true);
    std::string getLoginUser(const fc::ecc::public_key& serverKey);
    void doLogin(QStringList components);
    void readSettings();
    virtual void closeEvent( QCloseEvent* );
    void initMenu();
};
