
#include "tabs.h"
#include "ui_tabs.h"

#include <QSqlField>
#include <QSqlRecord>
#include <QSqlQueryModel>
#include <QSqlTableModel>

struct Tabs::Impl
{
  Tabs *tabs;
  QSqlQueryModel *planned;
  QSqlTableModel *groceries;
  QSqlQueryModel *recipes;
  QSqlTableModel *foods;

  Impl(Tabs *tabs) :
    tabs(tabs),
    planned(new QSqlQueryModel(tabs)),
    groceries(new QSqlTableModel(tabs)),
    recipes(new QSqlQueryModel(tabs)),
    foods(new QSqlTableModel(tabs))
  {
    planned->setQuery("select name from recipes where planned != 0");
    groceries->setEditStrategy(QSqlTableModel::OnFieldChange);
    groceries->setTable("groceries");
    groceries->select();
    recipes->setQuery("select name from recipes");
    foods->setEditStrategy(QSqlTableModel::OnFieldChange);
    foods->setTable("foods");
    foods->select();
  }
};

Tabs::Tabs() : ui(new Ui::Tabs), impl(std::make_unique<Impl>(this))
{
  ui->setupUi(this);
  ui->plannedView->setModel(impl->planned);
  ui->groceriesView->setModel(impl->groceries);
  ui->recipesView->setModel(impl->recipes);
  ui->recipesView->setSortingEnabled(true);
  ui->foodsView->setModel(impl->foods);
  ui->foodsView->setSortingEnabled(true);
#ifdef QT_NO_DEBUG
  ui->groceriesView->hideColumn(0);
  ui->foodsView->hideColumn(0);
#endif

  connect(ui->leFood, &QLineEdit::returnPressed, this, &Tabs::add_food);
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
