#include "Utilities.hpp"

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QFileDialog>

void Utilities::copy_to_clipboard(QString string)
{
    qApp->clipboard()->setText(string);
}

void Utilities::open_in_external_browser(QUrl url)
{
    QDesktopServices::openUrl(url);
}

QString Utilities::prompt_user_to_open_file(QString dialogCaption)
{
    return QFileDialog::getOpenFileName(nullptr, dialogCaption, QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).first());
}
