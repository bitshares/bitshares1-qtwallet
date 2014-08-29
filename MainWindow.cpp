#include "MainWindow.hpp"
#include "Utilities.hpp"

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
#include <QClipboard>
#include <QGraphicsWebView>
#include <QWebFrame>
#include <QDir>
#include <QTimer>
#include <QIODevice>
#include <QByteArray>
#include <QFile>

#include <bts/blockchain/config.hpp>
#include <bts/client/client.hpp>
#include <bts/wallet/config.hpp>
#include <bts/wallet/url.hpp>
#include <bts/blockchain/account_record.hpp>

#include <fc/io/raw_variant.hpp>
#include <fc/compress/lzma.hpp>

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

MainWindow::MainWindow()
  : _settings("BitShares", BTS_BLOCKCHAIN_NAME),
    _trayIcon(nullptr),
    _clientWrapper(nullptr)
{
  readSettings();
  initMenu();
}

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
  else if( object == this && event->type() == QEvent::Close && _trayIcon )
  {
    //If we're using a tray icon, close to tray instead of exiting
    if (_settings.value("showTrayIconMessage", true).toBool())
    {
      int showAgain = QMessageBox::information(this,
                               tr("Closing to Tray"),
                               tr("You have closed the %1 window. %1 will continue running in the system tray. To quit, use the Quit option in the menu.")
                                  .arg(qApp->applicationName()),
                               tr("OK"),
                               tr("Don't Show Again"),
                               QString(), 1);
      _settings.setValue("showTrayIconMessage", showAgain == 0);
    }

#ifdef __APPLE__
    //Workaround: if user clicks quit in dock menu, a non-spontaneous close event shows up. We should quit when this happens.
    if( !event->spontaneous() )
      return QMainWindow::eventFilter(object, event);

    ProcessSerialNumber psn = { 0, kCurrentProcess };
    ShowHideProcess(&psn, false);
#else
    setVisible(false);
#endif


    event->ignore();
    return true;
  }
  return QMainWindow::eventFilter(object, event);
}

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
    QString key(components[0].mid(colon+1));

    if(!walletIsUnlocked())
      return;

    getViewer()->loadUrl(_clientWrapper->http_url().toString() + "/#/newcontact?name=" + username + "&key=" + key);
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
      else if( components[1] == "transfer" )
        goToTransfer(components);
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

void MainWindow::navigateTo(const QString& path) 
{
    if( walletIsUnlocked() )
        getViewer()->webView()->page()->mainFrame()->evaluateJavaScript(QString("navigate_to('%1')").arg(path));
}

bool MainWindow::detectCrash()
{
  QString crashState = _settings.value("crash_state", "no_crash").toString();

  //Set to crashed for the duration of execution, but schedule it to be changed back on a clean exit
  _settings.setValue("crash_state", "crashed");
  connect(new QObject(this), &QObject::destroyed, [] {
    QSettings("BitShares", BTS_BLOCKCHAIN_NAME).setValue("crash_state", "no_crash");
  });

  return crashState == "crashed";
}

void MainWindow::goToMyAccounts()
{
    navigateTo("/accounts");
}

void MainWindow::goToAccount(QString accountName)
{
    navigateTo("/accounts/" + accountName);
}

void MainWindow::goToCreateAccount()
{
    navigateTo("/create/account");
}

void MainWindow::goToAddContact()
{
    navigateTo("/newcontact");
}

void MainWindow::takeFocus()
{
  if( !isVisible() )
    setVisible(true);

#ifdef __APPLE__
  ProcessSerialNumber psn = { 0, kCurrentProcess };
  if( !IsProcessVisible(&psn) )
  {
    ShowHideProcess(&psn, true);
    SetFrontProcess(&psn);
  }
#endif

  raise();
  activateWindow();
}

void MainWindow::setupTrayIcon()
{
  if( !QSystemTrayIcon::isSystemTrayAvailable() )
    return;

  _trayIcon = new QSystemTrayIcon;
  _trayIcon->setIcon(QIcon(":/images/tray_icon.png"));
  _trayIcon->show();

  connect(_trayIcon, &QSystemTrayIcon::activated, [this](QSystemTrayIcon::ActivationReason reason){
    if( reason == QSystemTrayIcon::Trigger )
    {
#ifdef __APPLE__
      ProcessSerialNumber psn = { 0, kCurrentProcess };
      bool visible = !IsProcessVisible(&psn);
      ShowHideProcess(&psn, visible);

      if( visible )
        SetFrontProcess(&psn);
#else
      setVisible(!isVisible());
#endif
    }
  });

  connect(qApp, &QApplication::aboutToQuit, [this]{
    _trayIcon->deleteLater();
    _trayIcon = nullptr;
  });
  
  bts::wallet::wallet_ptr wallet = clientWrapper()->get_client()->get_wallet();
  wallet->wallet_claimed_transaction.connect([=](const bts::wallet::ledger_entry& entry) {
      QString receiver = tr("You");
      QString amount = clientWrapper()->get_client()->get_chain()->to_pretty_asset(entry.amount).c_str();
      QString sender = tr("Someone");
      QString memo = entry.memo.c_str();
      
      if (entry.to_account)
          receiver = wallet->get_key_label(*entry.to_account).c_str();
      if (entry.from_account)
          sender = wallet->get_key_label(*entry.from_account).c_str();
      
      _trayIcon->showMessage(tr("%1 sent you %2").arg(sender).arg(amount),
                             tr("%1 just received %2 from %3!\n\nMemo: %4").arg(receiver).arg(amount).arg(sender).arg(memo));
  });
  wallet->update_margin_position.connect([=](const bts::wallet::ledger_entry& entry) {
      QString amount = clientWrapper()->get_client()->get_chain()->to_pretty_asset(entry.amount).c_str();
      _trayIcon->showMessage(tr("Your short order has been filled"),
                             tr("You just sold %1 from your short order.").arg(amount));
  });
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
  if( !_clientWrapper || !_clientWrapper->get_client()->get_wallet()->is_open() )
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
  if( !serverAccount.valid() )
  {
    if(_clientWrapper->get_client()->blockchain_is_synced())
      QMessageBox::critical(this,
                            tr("Misconfigured Website"),
                            tr("The website you are trying to log into is experiencing problems, and cannot accept logins at this time."));
    else
      QMessageBox::warning(this,
                           tr("Out of Sync"),
                           tr("Cannot login right now because your computer is out of sync with the %1 network. Please try again later.").arg(qAppName()));
    return std::string();
  }

  QString serverName = serverAccount->name.c_str();

  QDialog userSelecterDialog(this);
  userSelecterDialog.setWindowModality(Qt::WindowModal);

  QStringList accounts;
  auto wallet_accounts = _clientWrapper->get_client()->wallet_list_my_accounts();
  if( wallet_accounts.size() == 1 )
  {
    QMessageBox loginAuthBox(QMessageBox::Question,
                             tr("Login"),
                             tr("You are about to log in to %1 as %2. Would you like to continue?")
                                .arg(serverName)
                                .arg(wallet_accounts[0].name.c_str()),
                             QMessageBox::Yes | QMessageBox::No,
                             this);
    loginAuthBox.setDefaultButton(QMessageBox::Yes);
    loginAuthBox.setWindowModality(Qt::WindowModal);
    if( loginAuthBox.exec() == QMessageBox::Yes )
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
  QHBoxLayout* buttonsLayout = new QHBoxLayout();
  userSelecterLayout->addRow(tr("You are logging in to %1. Please select the account to login with:").arg(serverName), userSelecterBox);
  userSelecterLayout->addRow(buttonsLayout);
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
    bts::blockchain::public_key_type serverOneTimeKey;

    try {
      serverOneTimeKey = fc::variant(components[0].toStdString()).as<bts::blockchain::public_key_type>();
    } catch (const fc::exception& e) {
      elog("Unable to parse public key ${key}: ${e}", ("key", components[0].toStdString())("e", e.to_detail_string()));
      QMessageBox::warning(this, tr("Invalid URL"), tr("The URL provided is not valid."));
      return;
    }

    //Calculate server account public key
    fc::ecc::public_key serverAccountKey;
    try {
      serverAccountKey = fc::ecc::public_key(fc::variant(components[1].toStdString()).as<fc::ecc::compact_signature>(),
                                             fc::sha256::hash((char*)&serverOneTimeKey,sizeof(serverOneTimeKey)));
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
    query.addQueryItem("client_key",  fc::variant(bts::blockchain::public_key_type(myOneTimeKey.get_public_key())).as_string().c_str());
    query.addQueryItem("client_name", loginUser.c_str());
    query.addQueryItem("server_key", fc::variant(serverOneTimeKey).as_string().c_str());
    fc::ecc::compact_signature signature = _clientWrapper->get_client()->wallet_sign_hash(loginUser, fc::sha256::hash(secret.data(), sizeof(secret)));
    query.addQueryItem("signed_secret", fc::variant(signature).as_string().c_str());
    url.setQuery(query);
    url.setFragment(secret.str().c_str());

    ilog("Spawning login window with one-time key ${key} and signature ${sgn}",
         ("key",myOneTimeKey.get_public_key().to_base58())
         ("sgn", fc::variant(signature).as_string()));
    Utilities::open_in_external_browser(url);
  }
  catch( const fc::exception& e )
  {
    QMessageBox::warning(this, tr("Unable to Login"), tr("An error occurred during login: %1").arg(e.to_string().c_str()));
  }
}

void MainWindow::goToTransfer(QStringList components)
{
  if(!walletIsUnlocked()) return;

  QString sender;
  QString amount;
  QString asset;
  QString memo;
  QStringList parameters = components.mid(2);

  while (!parameters.empty()) {
    QString parameterName = parameters.takeFirst();
    if (parameterName == "amount")
      amount = parameters.takeFirst();
    else if (parameterName == "memo")
      memo = parameters.takeFirst();
    else if (parameterName == "from")
      sender = parameters.takeFirst();
    else if (parameterName == "asset")
      asset = parameters.takeFirst();
    else
      parameters.pop_front();
  }

  QString url = clientWrapper()->http_url().toString() + QStringLiteral("/#/transfer?from=%1&to=%2&amount=%3&asset=%4&memo=%5")
      .arg(sender)
      .arg(components[0])
      .arg(amount)
      .arg(asset)
      .arg(memo);
  getViewer()->loadUrl(url);
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


void MainWindow::importWallet()
{
  QString walletPath = QFileDialog::getOpenFileName(this, tr("Import Wallet"), QDir::homePath(), tr("Wallet Backups (*.json)"));
  if( walletPath.isNull() || !QFileInfo(walletPath).exists() )
    return;

  clientWrapper()->get_client()->wallet_close();

  QDir default_wallet_directory = QString::fromStdString(clientWrapper()->get_client()->get_wallet()->get_data_directory().generic_string());
  QString default_wallet_name = _settings.value("client/default_wallet_name").toString();

  if( QMessageBox::warning(this,
                           tr("Restoring Wallet Backup"),
                           tr("You are about to restore a wallet backup. This will back up and replace your current wallet! Are you sure you wish to continue?"),
                           tr("Yes, back up and replace my wallet"),
                           tr("Cancel"),
                           QString(), 1)
      != 0)
    return;

  QString backup_wallet_name = default_wallet_name + "-backup-" + QDateTime::currentDateTime().toString(Qt::ISODate).replace(':', "");

  bool ok = false;
  QString password = QInputDialog::getText(this,
                                           tr("Import Wallet Passphrase"),
                                           tr("Please enter the passphrase for the wallet you are restoring."),
                                           QLineEdit::Password,
                                           QString(),
                                           &ok);
  if(ok) {
    if( default_wallet_directory.exists(default_wallet_name) )
      default_wallet_directory.rename(default_wallet_name, backup_wallet_name);
    try {
      clientWrapper()->get_client()->wallet_backup_restore(walletPath.toStdString(),
                                                           default_wallet_name.toStdString(),
                                                           password.toStdString());
    } catch (const fc::exception& e) {
      if( default_wallet_directory.exists(default_wallet_name) )
        QDir(default_wallet_directory.absoluteFilePath(default_wallet_name)).removeRecursively();
      if( default_wallet_directory.exists(backup_wallet_name) )
        default_wallet_directory.rename(backup_wallet_name, default_wallet_name);
      QMessageBox::critical(this,
                            tr("Wallet Restore Failed"),
                            tr("Failed to restore wallet backup. Your original wallet has been restored. Error: %1").arg(e.to_string().c_str()));
    }
  } else return;

  getViewer()->loadUrl(clientWrapper()->http_url());
}

void MainWindow::initMenu()
{
  auto menuBar = new QMenuBar(nullptr);

  _fileMenu = menuBar->addMenu("File");

  connect(_fileMenu->addAction("Import Wallet"), &QAction::triggered, this, &MainWindow::importWallet);
  connect(_fileMenu->addAction("Export Wallet"), &QAction::triggered, [this](){
    QString savePath = QFileDialog::getSaveFileName(this,
                                                    tr("Export Wallet"),
                                                    QDir::homePath().append(QStringLiteral("/%1 Wallet Backup.json").arg(qApp->applicationName())),
                                                    tr("Wallet Backups (*.json)"));
    if( !savePath.isNull() )
      _clientWrapper->get_client()->wallet_backup_create(savePath.toStdString());
  });
  _fileMenu->actions().last()->setShortcut(QKeySequence(tr("Ctrl+Shift+X")));
  connect(_fileMenu->addAction("Open URL"), &QAction::triggered, [this]{
    QInputDialog urlGetter(this);
    urlGetter.setWindowTitle(tr("Open URL"));
    urlGetter.setLabelText(tr("Please enter a URL to open"));
    urlGetter.setTextValue(qApp->clipboard()->text().startsWith(CUSTOM_URL_SCHEME ":")
                           ?qApp->clipboard()->text() : CUSTOM_URL_SCHEME ":");
    urlGetter.setWindowModality(Qt::WindowModal);
    urlGetter.resize(width() / 2, 0);

    if( urlGetter.exec() == QInputDialog::Accepted )
      processCustomUrl(urlGetter.textValue());
  });
  _fileMenu->actions().last()->setShortcut(QKeySequence(tr("Ctrl+Shift+U")));

  _fileMenu->addAction(tr("Change Password"))->setEnabled(false);
  _fileMenu->addAction(tr("Check for Updates"), this, SLOT(checkWebUpdates()));
  _fileMenu->addAction(tr("Remove Updates"), this, SLOT(removeWebUpdates()));
  _fileMenu->addAction(tr("Quit"), qApp, SLOT(quit()), QKeySequence(tr("Ctrl+Q")));

  _accountMenu = menuBar->addMenu("Accounts");
  setMenuBar(menuBar);
}

bool MainWindow::verifyUpdateSignature (QByteArray updatePackage, QByteArray signature)
{
  try {
    fc::ecc::public_key verifyingKey = fc::ecc::public_key::from_base58("8H6CdwBH2VP4XkLYr9BxpXq6TwhogZVUB5UcVfMFWJJiu4hWFc");

    std::pair<fc::ecc::compact_signature, fc::time_point_sec> signature_pair;
    fc::datastream<decltype(signature.data())> ds(signature.data(), signature.size());
    fc::raw::unpack(ds, signature_pair);

    for (char c : signature_pair.second.to_iso_string())
        updatePackage.push_back(c);
    fc::sha256 hash = fc::sha256::hash(updatePackage.data(), updatePackage.size());

    if (verifyingKey != fc::ecc::public_key(signature_pair.first, hash)) {
      elog("Signature check failed on web update package! Rejecting package.");
      return false;
    }
  } catch (...) { return false; }
  return true;
}

void MainWindow::checkWebUpdates()
{
  QUrl signatureUrl = QStringLiteral("http://localhost:8888/web.sig");
  QUrl packageUrl = QStringLiteral("http://localhost:8888/web.dat");
  QDir dataDir(QString(clientWrapper()->get_data_dir().c_str()));

  if (dataDir.exists("web.sig") ^ dataDir.exists("web.dat"))
  {
    if (dataDir.exists("web.sig")) {
      elog("Found web.sig but not web.dat. Deleting.");
      dataDir.remove("web.sig");
    }
    if (dataDir.exists("web.dat")) {
      elog("Found web.dat but not web.sig. Deleting.");
      dataDir.remove("web.dat");
    }
  }

  QNetworkAccessManager* downer = new QNetworkAccessManager;
  downer->get(QNetworkRequest(signatureUrl));
  connect(downer, &QNetworkAccessManager::finished, [=](QNetworkReply* reply){
    reply->deleteLater();

    if (reply->url() == signatureUrl) {
      _webPackageSignature = reply->readAll();
      QByteArray oldSignature;
      QFile signatureFile(dataDir.absoluteFilePath("web.sig"));
      signatureFile.open(QIODevice::ReadOnly);
      if (dataDir.exists("web.sig"))
        oldSignature = signatureFile.readAll();

      if (_webPackageSignature != oldSignature)
        downer->get(QNetworkRequest(packageUrl));
      else if (!clientWrapper()->has_web_package())
        //No updates. Load old package if we have one.
        QTimer::singleShot(0, this, SLOT(loadWebUpdates()));
      else {
        QMessageBox noUpdateDialog(this);
        noUpdateDialog.setIcon(QMessageBox::Information);
        noUpdateDialog.addButton(QMessageBox::Ok);
        noUpdateDialog.setDefaultButton(QMessageBox::Ok);
        noUpdateDialog.setWindowModality(Qt::WindowModal);
        noUpdateDialog.setText(tr("No new updates are available."));
        noUpdateDialog.setWindowTitle(tr("%1 Update").arg(qApp->applicationName()));
        noUpdateDialog.exec();
      }
    } else if (reply->url() == packageUrl) {
      auto package = reply->readAll();
      if (!verifyUpdateSignature(package, _webPackageSignature))
        return;

      QMessageBox updateDialog(this);
      updateDialog.setIcon(QMessageBox::Question);
      updateDialog.addButton(QMessageBox::Yes);
      updateDialog.addButton(QMessageBox::No);
      updateDialog.setDefaultButton(QMessageBox::Yes);
      updateDialog.setWindowModality(Qt::WindowModal);
      updateDialog.setText(tr("An update is available for %1. You will not need to restart %1 to install it. "
                              "You may install it later by selecting Check for Updates from the File menu. "
                              "Would you like to install it now?").arg(qApp->applicationName()));
      updateDialog.setWindowTitle(tr("%1 Update").arg(qApp->applicationName()));
      if (updateDialog.exec() != QMessageBox::Yes)
      {
        wlog("User rejected web update package.");
        return;
      }

      QFile webPackage(dataDir.absoluteFilePath("web.dat"));
      webPackage.open(QIODevice::WriteOnly);
      webPackage.write(package);
      QFile webSignature(dataDir.absoluteFilePath("web.sig"));
      webSignature.open(QIODevice::WriteOnly);
      webSignature.write(_webPackageSignature);
      wlog("Downloaded new web package.");

      //We're done here. Queue up a call to loadWebUpdates
      QTimer::singleShot(0, this, SLOT(loadWebUpdates()));
    } else {
      elog("Loaded a page I don't know about: ${url}", ("url", reply->url().toString().toStdString()));
    }
  });
}

void MainWindow::removeWebUpdates()
{
  QMessageBox removeUpdateDialog(this);
  removeUpdateDialog.setIcon(QMessageBox::Question);
  removeUpdateDialog.addButton(QMessageBox::Yes);
  removeUpdateDialog.addButton(QMessageBox::No);
  removeUpdateDialog.setDefaultButton(QMessageBox::No);
  removeUpdateDialog.setWindowModality(Qt::WindowModal);
  removeUpdateDialog.setText(tr("Are you sure you want to remove all installed updates?"));
  removeUpdateDialog.setWindowTitle(tr("%1 Update").arg(qApp->applicationName()));
  if (removeUpdateDialog.exec() == QMessageBox::Yes)
  {
    wlog("User uninstalls web update package.");
    QDir dataDir(clientWrapper()->get_data_dir().c_str());
    dataDir.remove("web.sig");
    dataDir.remove("web.dat");
    clientWrapper()->set_web_package(std::move(std::unordered_map<std::string, std::vector<char>>()));
    clientWrapper()->get_client()->get_wallet()->lock();
    getViewer()->webView()->reload();
  }
}

void MainWindow::loadWebUpdates()
{
  QDir dataDir(QString(clientWrapper()->get_data_dir().c_str()));
  if (!dataDir.exists("web.sig")) {
    wlog("No web update package found.");
    return;
  }
  if (!dataDir.exists("web.dat")) {
    elog("Found web update package signature, but not the package itself.");
    return;
  }

  QByteArray signature;
  QFile signatureFile(dataDir.absoluteFilePath("web.sig"));
  signatureFile.open(QIODevice::ReadOnly);
  signature = signatureFile.readAll();

  QByteArray updatePackage;
  QFile packageFile(dataDir.absoluteFilePath("web.dat"));
  packageFile.open(QIODevice::ReadOnly);
  updatePackage = packageFile.readAll();

  if (!verifyUpdateSignature(updatePackage, signature))
    return;

  using std::vector;
  using std::string;
  using std::pair;

  vector<char> decompressedStream;
  try {
    decompressedStream = fc::lzma_decompress(vector<char>(updatePackage.begin(), updatePackage.end()));
    updatePackage.clear();
  } catch (fc::exception e) {
    elog("Failed to decompress web update package: ${error}", ("error", e.to_detail_string()));
    return;
  }

  vector<pair<string, vector<char>>> deserializedPackage;
  try {
    fc::datastream<const char*> ds(decompressedStream.data(), decompressedStream.size());
    fc::raw::unpack(ds, deserializedPackage);
    decompressedStream.clear();
  } catch (fc::exception e) {
    elog("Failed to deserialize web update package: ${error}", ("error", e.to_detail_string()));
    return;
  }

  std::unordered_map<string, vector<char>> webInterfaceMap;
  for (auto& file : deserializedPackage)
    webInterfaceMap[std::move(file.first)] = std::move(file.second);
  clientWrapper()->get_client()->get_wallet()->lock();
  clientWrapper()->set_web_package(std::move(webInterfaceMap));
  getViewer()->webView()->reload();
}
