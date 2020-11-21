
#include "tabs.h"
#include "ui_tabs.h"

#include <QSqlField>
#include <QSqlRecord>
#include <QSqlQueryModel>
#include <QSqlTableModel>

struct Tabs::Impl
{
  QSqlTableModel *foods;
};

Tabs::Tabs() : ui(new Ui::Tabs), impl(std::make_unique<Impl>())
{
  ui->setupUi(this);

  QSqlQueryModel *planned = new QSqlQueryModel(this);
  planned->setQuery("select name from recipes where planned != 0");
  ui->plannedView->setModel(planned);

  QSqlTableModel *groceries = new QSqlTableModel(this);
  groceries->setEditStrategy(QSqlTableModel::OnFieldChange);
  groceries->setTable("groceries");
  groceries->select();
  ui->groceriesView->setModel(groceries);

  QSqlQueryModel *recipes = new QSqlQueryModel(this);
  recipes->setQuery("select name from recipes");
  ui->recipesView->setModel(recipes);
  ui->recipesView->setSortingEnabled(true);

  impl->foods = new QSqlTableModel(this);
  impl->foods->setEditStrategy(QSqlTableModel::OnFieldChange);
  impl->foods->setTable("foods");
  impl->foods->select();
  ui->foodsView->setModel(impl->foods);
  ui->foodsView->setSortingEnabled(true);

  connect(ui->leFood, &QLineEdit::returnPressed, this, &Tabs::add_food);

#ifdef QT_NO_DEBUG
  ui->groceriesView->hideColumn(0);
  ui->foodsView->hideColumn(0);
#endif
}

Tabs::~Tabs()
{
  delete ui;
}

void Tabs::add_food()
{
  QString name = ui->leFood->text();
  if (name.isEmpty())
    return;

  QSqlRecord record;
  record.append(QSqlField("name", QVariant::String));
  record.setValue("name", name);

  if (impl->foods->insertRecord(-1, record))
    ui->leFood->setText("");
}
