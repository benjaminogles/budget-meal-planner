
#include "tabs.h"
#include "ui_tabs.h"
#include "database.h"
#include "nametoiddelegate.h"

#include <QSqlField>
#include <QSqlRecord>
#include <QSqlQueryModel>
#include <QSqlTableModel>

namespace
{
  void table_model_remove_rows(QItemSelectionModel *select, QSqlTableModel *model)
  {
    if (!select->hasSelection())
      return;
    auto indexes = select->selectedRows();
    for (auto index : indexes)
      model->removeRows(index.row(), 1);
    model->select();
  }
}

struct Tabs::Impl
{
  Tabs *tabs;
  QSqlQueryModel *planned;
  QSqlTableModel *groceries;
  QSqlQueryModel *recipes;
  QSqlTableModel *foods;
  int recipe_id = -1;

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
  ui->plannedView->setSelectionMode(QAbstractItemView::MultiSelection);
  ui->plannedView->setSelectionBehavior(QAbstractItemView::SelectRows);

  ui->groceriesView->setModel(impl->groceries);
  ui->groceriesView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  ui->groceriesView->setSelectionMode(QAbstractItemView::MultiSelection);
  ui->groceriesView->setSelectionBehavior(QAbstractItemView::SelectRows);

  ui->recipesView->setModel(impl->recipes);
  ui->recipesView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  ui->recipesView->setSelectionMode(QAbstractItemView::MultiSelection);
  ui->recipesView->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->recipesView->setSortingEnabled(true);

  ui->foodsView->setModel(impl->foods);
  ui->foodsView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  ui->foodsView->setSelectionMode(QAbstractItemView::MultiSelection);
  ui->foodsView->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->foodsView->setSortingEnabled(true);
  ui->foodsView->setItemDelegateForColumn(4, new NameToIdDelegate(db_unit_id_map(), this));

#ifdef QT_NO_DEBUG
  ui->groceriesView->hideColumn(0);
  ui->foodsView->hideColumn(0);
#endif

  connect(ui->leFood, &QLineEdit::returnPressed, this, &Tabs::add_food);
  connect(ui->bDeleteFood, &QPushButton::released, this, &Tabs::remove_foods);

  ui->tabs->setCurrentIndex(0);
  ui->recipeTab->setEnabled(false);
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

void Tabs::remove_foods()
{
  table_model_remove_rows(ui->foodsView->selectionModel(), impl->foods);
}

void Tabs::reset_recipe_tab()
{

}

void Tabs::start_edit_recipe(int)
{

}

void Tabs::start_add_recipe()
{
  int id = db_add_recipe("Untitled");
  if (id >= 0)
    start_edit_recipe(id);
}

