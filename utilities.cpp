#include "utilities.hpp"

#include <QApplication>
#include <QClipboard>

void Utilities::copy_to_clipboard(QString string)
{
    qApp->clipboard()->setText(string);
}
