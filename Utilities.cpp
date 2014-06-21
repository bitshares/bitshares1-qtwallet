#include "Utilities.hpp"

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>

void Utilities::copy_to_clipboard(QString string)
{
    qApp->clipboard()->setText(string);
}

void Utilities::open_in_external_browser(QUrl url)
{
    QDesktopServices::openUrl(url);
}
