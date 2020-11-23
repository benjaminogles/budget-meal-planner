
#include "nametoiddelegate.h"

#include <QStringListModel>
#include <QLineEdit>
#include <QCompleter>

struct NameToIdDelegate::Impl
{
  QMap<QString, int> name_to_id;

  Impl(QMap<QString, int> name_to_id) : name_to_id(name_to_id)
  {
  }
};

NameToIdDelegate::NameToIdDelegate(QMap<QString, int> name_to_id, QObject *parent) :
  QStyledItemDelegate(parent),
  impl(std::make_unique<Impl>(name_to_id))
{
}

NameToIdDelegate::~NameToIdDelegate()
{
}

QWidget* NameToIdDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
  QLineEdit *editor = new QLineEdit(parent);
  QCompleter *completer = new QCompleter(impl->name_to_id.uniqueKeys(), parent);
  completer->setCompletionMode(QCompleter::InlineCompletion);
  completer->setCaseSensitivity(Qt::CaseInsensitive);
  editor->setCompleter(completer);
  return editor;
}

void NameToIdDelegate::setEditorData(QWidget *wid, const QModelIndex &index) const
{
  QLineEdit *editor = qobject_cast<QLineEdit*>(wid);
  editor->setText(index.model()->data(index).toString());
}

void NameToIdDelegate::setModelData(QWidget *wid, QAbstractItemModel *model, const QModelIndex &index) const
{
  QLineEdit *editor = qobject_cast<QLineEdit*>(wid);
  int id = -1;
  if (impl->name_to_id.contains(editor->text()))
    id = impl->name_to_id[editor->text()];
  if (id < 0)
    model->setData(index, QVariant());
  else
    model->setData(index, QVariant(id));
}

void NameToIdDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &item, const QModelIndex&) const
{
  editor->setGeometry(item.rect);
}
