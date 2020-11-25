
#include "nametoiddelegate.h"

#include <QStringListModel>
#include <QLineEdit>
#include <QCompleter>

struct NameToIdDelegate::Impl
{
  QMap<QString, int> name_to_id;
  QMap<int, QString> id_to_name;

  Impl(QMap<QString, int> name_to_id)
  {
    reset(name_to_id);
  }

  void reset(QMap<QString, int> source)
  {
    name_to_id = source;
    id_to_name.clear();
    auto i = name_to_id.constBegin();
    while (i != name_to_id.constEnd())
    {
      id_to_name.insert(i.value(), i.key());
      i++;
    }
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
  editor->setText(displayText(index.model()->data(index), QLocale()));
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

QString NameToIdDelegate::displayText(const QVariant &value, const QLocale&) const
{
  if (value.isNull() || !impl->id_to_name.contains(value.toInt()))
    return "";
  return impl->id_to_name[value.toInt()];
}


void NameToIdDelegate::reset(QMap<QString, int> source)
{
  impl->reset(source);
}
