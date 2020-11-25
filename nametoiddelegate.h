
#ifndef nametoiddelegate_h
#define nametoiddelegate_h

#include <QStyledItemDelegate>
#include <QMap>
#include <memory>

class NameToIdDelegate : public QStyledItemDelegate
{
  Q_OBJECT
  public:
    NameToIdDelegate(QMap<QString, int> name_to_id, QObject *parent = nullptr);
    ~NameToIdDelegate();

    QWidget* createEditor(QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const override;
    void setEditorData(QWidget*, const QModelIndex&) const override;
    void setModelData(QWidget*, QAbstractItemModel*, const QModelIndex&) const override;
    void updateEditorGeometry(QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const override;
    QString displayText(const QVariant&, const QLocale&) const override;

  public slots:
    void reset(QMap<QString, int>);

  private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
#endif
