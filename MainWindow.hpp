#pragma once

#include "ClientWrapper.hpp"
#include "html5viewer/html5viewer.h"

#include <QMainWindow>
#include <QSettings>
#include <QMenu>


class MainWindow : public QMainWindow
{
    Q_OBJECT
    QSettings settings;
    QMenu* _fileMenu;
    QMenu* _accountMenu;
    
public:
    MainWindow();
    QMenu* fileMenu() { return _fileMenu; }
    QMenu* accountMenu() { return _accountMenu; }

#ifdef __APPLE__
    bool eventFilter(QObject* object, QEvent* event);
#endif
    
    ClientWrapper *clientWrapper() const;
    void setClientWrapper(ClientWrapper *clientWrapper);

public slots:
    void goToMyAccounts();
    void goToCreateAccount();

private:
    ClientWrapper* _clientWrapper;

    Html5Viewer *getViewer();
    bool walletIsUnlocked(bool promptToUnlock = true);
    void readSettings();
    virtual void closeEvent( QCloseEvent* );
    void initMenu();
};
