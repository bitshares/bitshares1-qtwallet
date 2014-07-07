#include "MainWindow.hpp"
#include "config.hpp"

#include <QApplication>
#include <QString>
#include <QMenuBar>
#include <QFileOpenEvent>
#include <QInputDialog>
#include <QMessageBox>

#include <bts/blockchain/config.hpp>
#include <bts/client/client.hpp>
#include <bts/wallet/config.hpp>
#include <bts/blockchain/account_record.hpp>

MainWindow::MainWindow()
: _settings("BitShares", BTS_BLOCKCHAIN_NAME),
  _clientWrapper(nullptr)

{
    readSettings();
    initMenu();
}

#ifdef __APPLE__
bool MainWindow::eventFilter(QObject* object, QEvent* event)
{
    if ( event->type() == QEvent::FileOpen )
    {
        QFileOpenEvent* urlEvent = static_cast<QFileOpenEvent*>(event);
        ilog("Got URL to open: ${url}", ("url", urlEvent->file().toStdString()));
        if( isVisible() )
          processCustomUrl(urlEvent->file());
        else
          deferCustomUrl(urlEvent->file());
        return true;
    }
    return false;
}
#endif

void MainWindow::deferCustomUrl(QString url)
{
    if( isVisible() )
    {
        processCustomUrl(url);
        return;
    }

    _deferredUrl = url;
}

void MainWindow::processDeferredUrl()
{
    processCustomUrl(_deferredUrl);
    _deferredUrl.clear();
}

void MainWindow::processCustomUrl(QString url)
{
    if( url.left(url.indexOf(':')).toLower() != CUSTOM_URL_SCHEME )
    {
        elog("Got URL of unknown scheme: ${url}", ("url", url.toStdString()));
        return;
    }

    url = url.mid(url.indexOf(':') + 1);
    while( url.startsWith('/') ) url.remove(0, 1);
    ilog("Processing custom URL request for ${url}", ("url", url.toStdString()));

    QStringList components = url.split('/', QString::SkipEmptyParts);

    if( components[0].contains(':') )
    {
        //This is a username:key pair
        int colon = components[0].indexOf(':');
        QString username = components[0].left(colon);
        bts::blockchain::public_key_type key(components[0].mid(colon+1).toStdString());

        try
        {
            _clientWrapper->get_client()->get_wallet()->add_contact_account(username.toStdString(), key);
            goToAccount(username);
        }
        catch(const fc::exception& e)
        {
            //Display error from backend, but chop off the "Assert exception" stuff before the colon
            QString error = e.to_string().c_str();
            QMessageBox::warning(this, tr("Invalid Account"), tr("Could not create contact account:") + error.mid(error.indexOf(':')+1));
            return;
        }

        if( walletIsUnlocked(false) && components.size() > 1 )
        {
            if( components[1] == "approve" )
                _clientWrapper->confirm_and_set_approval(username, true);
            else if( components[1] == "disapprove" )
                _clientWrapper->confirm_and_set_approval(username, false);
        }
    }
    else if( components[0].toLower() == components[0] )
    {
        //This is a username.
        auto account = _clientWrapper->get_client()->blockchain_get_account(components[0].toStdString());
        if( !account.valid() )
        {
            QMessageBox::warning(this, tr("Invalid Account"), tr("The requested account does not exist."));
            return;
        }
        else
            goToAccount(components[0]);

        if( walletIsUnlocked(false) && components.size() > 1 )
        {
            if( components[1] == "approve" )
                _clientWrapper->confirm_and_set_approval(components[0], true);
            else if( components[1] == "disapprove" )
                _clientWrapper->confirm_and_set_approval(components[0], false);
        }
    }
    else if( components[0].size() > QString(BTS_ADDRESS_PREFIX).size() && components[0].startsWith(BTS_ADDRESS_PREFIX) )
    {
        //This is a key.
    }
    else if( components[0].size() >= BTS_BLOCKCHAIN_MIN_SYMBOL_SIZE && components[0].size() <= BTS_BLOCKCHAIN_MAX_SYMBOL_SIZE && components[0].toUpper() == components[0] )
    {
        //This is an asset symbol.
    }
    else if( components[0] == "Login" )
    {
        //This is a login request
    }
    else if( components[0] == "Block" )
    {
        //This is a block ID or number
        if( components[1] == "num" && components.size() > 2 )
        {
            bool ok = false;
            uint32_t blockNumber = components[2].toInt(&ok);
            if( ok )
                goToBlock(blockNumber);
            else
                QMessageBox::warning(this, tr("Invalid Block Number"), tr("The specified block number does not exist."));
        }
        else
            goToBlock(components[1]);
    }
    else if( components[0] == "Trx" )
    {
        goToTransaction(components[1]);
    }
}

ClientWrapper *MainWindow::clientWrapper() const
{
    return _clientWrapper;
}

void MainWindow::setClientWrapper(ClientWrapper *clientWrapper)
{
    _clientWrapper = clientWrapper;
}

void MainWindow::goToMyAccounts()
{
    if( !walletIsUnlocked() )
      return;

    getViewer()->loadUrl(_clientWrapper->http_url().toString() + "/#/accounts");
}

void MainWindow::goToAccount(QString accountName)
{
    if( !walletIsUnlocked() )
      return;

    getViewer()->loadUrl(_clientWrapper->http_url().toString() + "/#/accounts/" + accountName);
}

void MainWindow::goToCreateAccount()
{
    if( !walletIsUnlocked() )
        return;

    getViewer()->loadUrl(_clientWrapper->http_url().toString() + "/#/create/account");
}

void MainWindow::goToBlock(uint32_t blockNumber)
{
    if( !walletIsUnlocked() )
        return;

    getViewer()->loadUrl(_clientWrapper->http_url().toString() + "/#/blocks/" + QString("%1").arg(blockNumber));
}

void MainWindow::goToBlock(QString blockId)
{
    try
    {
        auto block = _clientWrapper->get_client()->get_chain()->get_block_digest(bts::blockchain::block_id_type(blockId.toStdString()));
        goToBlock(block.block_num);
    }
    catch(...)
    {
        QMessageBox::warning(this, tr("Invalid Block"), tr("The specified block ID does not exist."));
    }
}

void MainWindow::goToTransaction(QString transactionId)
{
    if( !walletIsUnlocked() )
        return;

    getViewer()->loadUrl(_clientWrapper->http_url().toString() + "/#/tx/" + transactionId);
}

Html5Viewer* MainWindow::getViewer()
{
    return static_cast<Html5Viewer*>(centralWidget());
}

bool MainWindow::walletIsUnlocked(bool promptToUnlock)
{
    if( !_clientWrapper )
        return false;
    if( _clientWrapper->get_client()->get_wallet()->is_unlocked() )
        return true;

    bool badPassword = false;
    while( promptToUnlock )
    {
        QString password = QInputDialog::getText(this,
                                                 tr("Unlock Wallet"),
                                                 (badPassword?
                                                      tr("Incorrect password. Please enter your password to continue."):
                                                      tr("Please enter your password to continue.")),
                                                 QLineEdit::Password,
                                                 QString(),
                                                 &promptToUnlock);

        //If user did not click cancel...
        if( promptToUnlock )
        {
            try
            {
                _clientWrapper->get_client()->get_wallet()->unlock( password.toStdString(), BTS_WALLET_DEFAULT_UNLOCK_TIME_SEC );
                promptToUnlock = false;
            }
            catch (...)
            {
                badPassword = true;
            }
        }
    }

    return _clientWrapper->get_client()->get_wallet()->is_unlocked();
}

void MainWindow::readSettings()
{
    if( _settings.contains("geometry") )
    {
        restoreGeometry(_settings.value("geometry").toByteArray());
        restoreState(_settings.value("windowState").toByteArray());
    }
    else {
        resize(1024,768);
    }
}

void MainWindow::closeEvent( QCloseEvent* event )
{
    _settings.setValue("test","bla-bla-bla");
    _settings.setValue("geometry", saveGeometry());
    _settings.setValue("windowState", saveState());
    QMainWindow::closeEvent(event);
}


void MainWindow::initMenu()
{
    auto menuBar = new QMenuBar(nullptr);

    _fileMenu = menuBar->addMenu("&File");
    _fileMenu->addAction("&Import Wallet")->setEnabled(false);
    _fileMenu->addAction("&Export Wallet")->setEnabled(false);
    _fileMenu->addAction("&Change Password")->setEnabled(false);
#ifndef __APPLE__
    //OSX provides its own Quit menu item which works fine; we don't need to add a second one.
    _fileMenu->addAction("&Quit", qApp, SLOT(quit()));
#endif
    _accountMenu = menuBar->addMenu("&Accounts");
    setMenuBar(menuBar);
}
