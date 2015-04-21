#pragma once

#include "WebUpdates.hpp"
#include "ClientWrapper.hpp"
#include "html5viewer/html5viewer.h"

#include <QMainWindow>
#include <QSettings>
#include <QMenu>
#include <QTimer>
#include <QUuid>

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QSettings _settings;
    QMenu* _fileMenu;
    QMenu* _accountMenu;
    QString _deferredUrl;

    //Temporary storage for a web update description being considered for application.
    //Do not trust this as the in-use web update.
    WebUpdateManifest::UpdateDetails _webUpdateDescription;
    //Version information for the running client and GUI. These values may be trusted
    //as accurate regarding the web update currently being shown to the user.
    uint8_t _majorVersion = 0;
    uint8_t _forkVersion = 0;
    uint8_t _minorVersion = 0;
    uint8_t _patchVersion = 0;

    QTimer* _updateChecker;
    QUuid app_id;
    QString version;

  public:
    MainWindow();
    QMenu* fileMenu() { return _fileMenu; }
    QMenu* accountMenu() { return _accountMenu; }

    bool eventFilter(QObject* object, QEvent* event);

    ClientWrapper *clientWrapper() const;
    void setClientWrapper(ClientWrapper* clientWrapper);
    void navigateTo(const QString& path);

    bool detectCrash();
    QUuid getAppId() const { return app_id; }

public Q_SLOTS:
    void goToMyAccounts();
    void goToAccount(QString accountName);
    void goToCreateAccount();
    void goToAddContact();
    void checkWebUpdates(bool showNoUpdatesAlert = true,
                         std::function<void()> finishedCheckCallback = std::function<void()>());
    void loadWebUpdates();

    //Causes this window to attempt to become the front window on the desktop
    void takeFocus();
    void hideWindow();

    ///Used to schedule a custom URL for processing later, once the app has finished starting
    void deferCustomUrl(QString url);
    ///Triggers the deferred URL processign. Call once app has finished starting
    void processDeferredUrl();
    ///Used to process a custom URL now (only call if app has finished starting)
    void processCustomUrl(QString url);
    void goToBlock(uint32_t blockNumber);
    void goToBlock(QString blockId);
    void goToTransaction(QString transactionId);
    void importWallet();

private Q_SLOTS:
    void removeWebUpdates();

private:
    ClientWrapper* _clientWrapper;

    Html5Viewer* getViewer();
    bool walletIsUnlocked(bool promptToUnlock = true);
    std::string getLoginUser(const fc::ecc::public_key& serverKey);
    void doLogin(QStringList components);
    void goToTransfer(QStringList components);
    void readSettings();
    virtual void closeEvent( QCloseEvent* );
    void initMenu();
    void showNoUpdateAlert();
    bool verifyUpdateSignature(QByteArray updatePackage);
    void goToRefCode(QStringList components);
};
