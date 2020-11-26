
#include "currencydelegate.h"

CurrencyDelegate::CurrencyDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

CurrencyDelegate::~CurrencyDelegate()
{
}

QString CurrencyDelegate::displayText(const QVariant &var, const QLocale&) const
{
  return QString::number(var.toDouble(), 'f', 2);
}
