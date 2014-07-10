#include "MainWindow.hpp"
#include "Utilities.hpp"
#include "config.hpp"

#include <QApplication>
#include <QString>
#include <QMenuBar>
#include <QFileOpenEvent>
#include <QInputDialog>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QStringListModel>
#include <QPushButton>
#include <QFormLayout>
#include <QNetworkReply>
#include <QFileDialog>

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
  if( components.empty() )
  {
    elog("Invalid URL has no contents!");
    QMessageBox::warning(this, tr("Invalid URL"), tr("The URL provided is not valid."));
    return;
  }

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
  else if( components[0].size() >= BTS_BLOCKCHAIN_MIN_SYMBOL_SIZE
           && components[0].size() <= BTS_BLOCKCHAIN_MAX_SYMBOL_SIZE
           && components[0].toUpper() == components[0] )
  {
    //This is an asset symbol.
  }
  else if( components[0] == "Login" )
  {
    //This is a login request
    if( components.size() < 4 )
    {
      elog("Invalid URL has ${url_parts} parts, but should have at least 4.", ("url_parts", components.size()));
      QMessageBox::warning(this, tr("Invalid URL"), tr("The URL provided is not valid."));
      return;
    }
    if( !walletIsUnlocked() )
      return;

    doLogin(components.mid(1));
  }
  else if( components[0] == "Block" )
  {
    //This is a block ID or number
    if( components.size() == 1 )
    {
      elog("Invalid URL has only one part, but should have at least two.");
      QMessageBox::warning(this, tr("Invalid URL"), tr("The URL provided is not valid."));
      return;
    }
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
                                             &promptToUnlock,
                                             Qt::Sheet);

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

std::string MainWindow::getLoginUser(const fc::ecc::public_key& serverKey)
{
  auto serverAccount = _clientWrapper->get_client()->get_chain()->get_account_record(serverKey);
  QString serverName = (serverAccount.valid()? serverAccount->name.c_str() : "UNRECOGNIZED");

  QDialog userSelecterDialog(this);
  userSelecterDialog.setWindowModality(Qt::WindowModal);

  QStringList accounts;
  auto wallet_accounts = _clientWrapper->get_client()->wallet_list_my_accounts();
  if( wallet_accounts.size() == 1 )
  {
    if( QMessageBox::question(this, tr("Login"),
                              tr("You are about to log in with %1 as %2. Would you like to continue?")
                                  .arg(serverName)
                                  .arg(wallet_accounts[0].name.c_str()))
        == QMessageBox::Yes )
      return wallet_accounts[0].name;
    else
      return std::string();
  }
  if( wallet_accounts.size() == 0 )
    return "EMPTY";

  for( auto account : wallet_accounts )
    accounts.push_back(account.name.c_str());

  QComboBox* userSelecterBox = new QComboBox();
  QObject sentry;
  userSelecterBox->setModel(new QStringListModel(accounts, &sentry));
  QPushButton* okButton = new QPushButton(tr("OK"), &userSelecterDialog);
  okButton->setFocus();
  QPushButton* cancelButton = new QPushButton(tr("Cancel"), &userSelecterDialog);

  QFormLayout* userSelecterLayout = new QFormLayout(&userSelecterDialog);
  QHBoxLayout* buttonsLayout = new QHBoxLayout(&userSelecterDialog);
  userSelecterLayout->addRow(tr("You are logging in with %1. Please select the account to login with:").arg(serverName), userSelecterBox);
  userSelecterLayout->addRow(buttonsLayout);
  userSelecterDialog.setLayout(userSelecterLayout);
  buttonsLayout->addStretch();
  buttonsLayout->addWidget(cancelButton);
  buttonsLayout->addWidget(okButton);

  connect(okButton, SIGNAL(clicked()), &userSelecterDialog, SLOT(accept()));
  connect(cancelButton, SIGNAL(clicked()), &userSelecterDialog, SLOT(reject()));

  if( userSelecterDialog.exec() == QDialog::Accepted )
    return userSelecterBox->currentText().toStdString();
  return "";
}

void MainWindow::doLogin(QStringList components)
{
  try
  {
    fc::ecc::private_key myOneTimeKey = fc::ecc::private_key::generate();
    fc::ecc::public_key serverOneTimeKey;
    try {
      serverOneTimeKey = fc::ecc::public_key::from_base58(components[0].toStdString());
    } catch (const fc::exception& e) {
      elog("Unable to parse public key ${key}: ${e}", ("key", components[0].toStdString())("e", e.to_detail_string()));
      QMessageBox::warning(this, tr("Invalid URL"), tr("The URL provided is not valid."));
      return;
    }

    //Calculate server account public key
    fc::ecc::public_key serverAccountKey;
    try {
      serverAccountKey = fc::ecc::public_key(fc::variant(components[1].toStdString()).as<fc::ecc::compact_signature>(),
                                             fc::sha256::hash(serverOneTimeKey.to_base58()));
    } catch (const fc::exception& e) {
      elog("Unable to derive server account public key: ${e}",
           ("e", e.to_detail_string()));
      QMessageBox::warning(this, tr("Invalid URL"), tr("The URL provided is not valid."));
      return;
    }

    //Calculate shared secret
    fc::sha512 secret;
    try {
      secret = myOneTimeKey.get_shared_secret(serverOneTimeKey);
    } catch (const fc::exception& e) {
      elog("Unable to derive shared secret: ${e}", ("e", e.to_detail_string()));
      QMessageBox::warning(this, tr("Invalid URL"), tr("The URL provided is not valid."));
      return;
    }

    //Prompt user to login with server
    std::string loginUser = getLoginUser(serverAccountKey);
    if( loginUser.empty() )
      return;
    if( loginUser == "EMPTY" )
    {
      QMessageBox::warning(this, tr("No Accounts Available"), tr("Could not find any accounts to log in with. Create an account and try again."));
      return goToCreateAccount();
    }

    QUrl url("http://" + QStringList(components.mid(2)).join('/'));
    QUrlQuery query;
    query.addQueryItem("client_key", myOneTimeKey.get_public_key().to_base58().c_str());
    query.addQueryItem("server_key", serverOneTimeKey.to_base58().c_str());
    fc::ecc::compact_signature signature = _clientWrapper->get_client()->wallet_sign_hash(loginUser, fc::sha256::hash(secret.data(), 512/8));
    query.addQueryItem("signed_secret", fc::variant(signature).as_string().c_str());
    url.setQuery(query);

    ilog("Spawning login window with one-time key ${key} and signature ${sgn} to ${host}",
         ("key",myOneTimeKey.get_public_key().to_base58())
         ("sgn", fc::variant(signature).as_string())
         ("host", url.toString().toStdString()));
    Utilities::open_in_external_browser(url);
  }
  catch( const fc::exception& e )
  {
    QMessageBox::warning(this, tr("Unable to Login"), tr("An error occurred during login: %1").arg(e.to_string().c_str()));
  }
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

  connect(_fileMenu->addAction("E&xport Wallet"), &QAction::triggered, [this](){
    QString savePath = QFileDialog::getSaveFileName(this, tr("Export Wallet"), QString(), tr("Wallet Backups (*.json)"));
    if( !savePath.isNull() )
      _clientWrapper->get_client()->wallet_export_to_json(savePath.toStdString());
  });

  _fileMenu->addAction("&Change Password")->setEnabled(false);
#ifndef __APPLE__
  //OSX provides its own Quit menu item which works fine; we don't need to add a second one.
  _fileMenu->addAction("&Quit", qApp, SLOT(quit()));
#endif
  _accountMenu = menuBar->addMenu("&Accounts");
  setMenuBar(menuBar);
}
