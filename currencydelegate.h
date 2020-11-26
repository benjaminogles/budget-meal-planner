
#ifndef currencydelegate_h
#define currencydelegate_h

#include <QStyledItemDelegate>

class CurrencyDelegate : public QStyledItemDelegate
{
  public:
    CurrencyDelegate(QObject *parent = nullptr);
    ~CurrencyDelegate();
    QString displayText(const QVariant&, const QLocale&) const override;
};
#endif
