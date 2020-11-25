
#include "app.h"
#include "ui_app.h"
#include "database.h"
#include "nametoiddelegate.h"

#include <QSqlField>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlTableModel>
#include <QCompleter>

namespace
{
  const int recipes_tab_idx = 0;
  const int recipe_tab_idx = 3;

  QList<int> table_remove_rows(QItemSelectionModel *select, QSqlTableModel *model, int id_column)
  {
    QList<int> removed;
    if (!select->hasSelection())
      return removed;
    auto indexes = select->selectedRows(id_column);
    for (auto index : indexes)
    {
      if (model->removeRows(index.row(), 1))
        removed.append(index.data().toInt());
    }
    model->select();
    return removed;
  }

  void query_refresh(QSqlQueryModel *model)
  {
    QString str = model->query().executedQuery();
    model->query().clear();
    model->setQuery(str);
  }

  QList<int> query_remove_ids(QItemSelectionModel *select, QSqlQueryModel *model, QString table, int id_column)
  {
    QList<int> removed;
    if (!select->hasSelection())
      return removed;
    auto indexes = select->selectedRows(id_column);
    for (auto index : indexes)
    {
      QVariant var = index.data();
      if (var.isNull())
        continue;
      if (db_remove_id(table, var.toInt()))
        removed.append(var.toInt());
    }
    query_refresh(model);
    return removed;
  }
}

struct App::Impl
{
  App *app;
  QSqlQueryModel *planned;
  QSqlTableModel *groceries;
  QSqlQueryModel *recipes;
  QSqlTableModel *foods;
  QSqlTableModel *ingredients;
  QCompleter *food_completer;
  int recipe_id = -1;

  Impl(App *app) :
    app(app),
    planned(new QSqlQueryModel(app)),
    groceries(new QSqlTableModel(app)),
    recipes(new QSqlQueryModel(app)),
    foods(new QSqlTableModel(app)),
    ingredients(new QSqlTableModel(app)),
    food_completer(new QCompleter(db_food_names(), app))
  {
    planned->setQuery("select id, name from recipes where planned != 0");
    groceries->setEditStrategy(QSqlTableModel::OnFieldChange);
    groceries->setTable("groceries");
    groceries->select();
    recipes->setQuery("select id, name from recipes");
    foods->setEditStrategy(QSqlTableModel::OnFieldChange);
    foods->setTable("foods");
    foods->select();
    ingredients->setEditStrategy(QSqlTableModel::OnFieldChange);
    ingredients->setTable("ingredients");
    ingredients->setFilter("recipe is null");
  }
};

App::App() : ui(new Ui::App), impl(std::make_unique<Impl>(this))
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

  ui->ingredientsView->setModel(impl->ingredients);
  ui->ingredientsView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  ui->ingredientsView->setSelectionMode(QAbstractItemView::MultiSelection);
  ui->ingredientsView->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->foodsView->setItemDelegateForColumn(2, new NameToIdDelegate(db_food_id_map(), this));
  ui->foodsView->setItemDelegateForColumn(3, new NameToIdDelegate(db_unit_id_map(), this));

#ifdef QT_NO_DEBUG
  ui->recipesView->hideColumn(0);
  ui->groceriesView->hideColumn(0);
  ui->foodsView->hideColumn(0);
  ui->ingredientsView->hideColumn(0);
  ui->ingredientsView->hideColumn(1);
#endif

  connect(ui->bAddRecipe, &QPushButton::released, this, &App::start_add_recipe);
  connect(ui->bDeleteRecipe, &QPushButton::released, this, &App::remove_recipes);
  connect(ui->leRecipeTitle, &QLineEdit::textEdited, this, &App::set_recipe_name);
  connect(ui->teRecipeSteps, &QPlainTextEdit::textChanged, this, &App::set_recipe_steps);
  connect(ui->bDoneRecipe, &QPushButton::released, this, &App::stop_edit_recipe);
  connect(ui->leFood, &QLineEdit::returnPressed, this, &App::add_food);
  connect(ui->bDeleteFood, &QPushButton::released, this, &App::remove_foods);
  connect(ui->recipesView, &QTableView::doubleClicked, this, &App::recipe_double_clicked);
  connect(ui->leRecipeIngredient, &QLineEdit::returnPressed, this, &App::add_ingredient);
  connect(ui->bDeleteIngredient, &QPushButton::released, this, &App::remove_ingredients);

  ui->tabs->setCurrentIndex(recipes_tab_idx);
  ui->recipeTab->setEnabled(false);
}

App::~App()
{
  delete ui;
}

void App::add_food()
{
  QString name = ui->leFood->text();
  if (name.isEmpty())
    return;

  QSqlRecord record;
  record.append(QSqlField("name", QVariant::String));
  record.setValue("name", name);

  if (impl->foods->insertRecord(-1, record))
    ui->leFood->clear();
}

void App::remove_foods()
{
  table_remove_rows(ui->foodsView->selectionModel(), impl->foods, 0);
}

void App::remove_recipes()
{
  QList<int> removed = query_remove_ids(ui->recipesView->selectionModel(), impl->recipes, "recipes", 0);
  if (removed.contains(impl->recipe_id))
    reset_recipe_tab();
}

void App::set_recipe_name(const QString &name)
{
  if (impl->recipe_id >= 0)
    db_set_recipe_name(impl->recipe_id, name);
}

void App::set_recipe_steps()
{
  QString text = ui->teRecipeSteps->toPlainText();
  if (impl->recipe_id >= 0)
    db_set_recipe_steps(impl->recipe_id, text);
}

void App::reset_recipe_tab()
{
  impl->recipe_id = -1;
  ui->leRecipeTitle->clear();
  ui->teRecipeSteps->clear();
  impl->ingredients->setFilter("recipe is null");
  ui->recipeTab->setEnabled(false);
}

void App::start_edit_recipe(int id)
{
  reset_recipe_tab();
  ui->leRecipeTitle->setText(db_recipe_name(id));
  ui->teRecipeSteps->setPlainText(db_recipe_steps(id));
  impl->ingredients->setFilter(QString("recipe = %1").arg(id));
  ui->recipeTab->setEnabled(true);
  impl->recipe_id = id;
  ui->tabs->setCurrentIndex(recipe_tab_idx);
}

void App::start_add_recipe()
{
  int id = db_add_recipe("Untitled");
  if (id >= 0)
  {
    query_refresh(impl->recipes);
    start_edit_recipe(id);
  }
}

void App::stop_edit_recipe()
{
  query_refresh(impl->recipes);
  reset_recipe_tab();
  ui->tabs->setCurrentIndex(recipes_tab_idx);
}

void App::recipe_double_clicked(const QModelIndex &index)
{
  start_edit_recipe(index.siblingAtColumn(0).data().toInt());
}

void App::add_ingredient()
{
  if (impl->recipe_id < 0)
    return;

  QString name = ui->leRecipeIngredient->text();
  if (name.isEmpty())
    return;

  int food_id = db_food_id(name);
  if (food_id < 0)
  {
    food_id = db_add_food(name);
    if (food_id < 0)
      return;
  }

  QSqlRecord record;
  record.append(QSqlField("recipe", QVariant::Int));
  record.append(QSqlField("food", QVariant::Int));
  record.setValue("recipe", impl->recipe_id);
  record.setValue("food", food_id);

  if (impl->ingredients->insertRecord(-1, record))
    ui->leRecipeIngredient->clear();
}

void App::remove_ingredients()
{
  table_remove_rows(ui->ingredientsView->selectionModel(), impl->ingredients, 0);
}

