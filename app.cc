
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

  Impl(App *app_) :
    app(app_),
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

  void reset_recipe_tab()
  {
    recipe_id = -1;
    ingredients->setFilter("recipe is null");
    app->ui->leRecipeTitle->clear();
    app->ui->teRecipeSteps->clear();
    app->ui->recipeTab->setEnabled(false);
  }

  void start_edit_recipe(int id)
  {
    if (recipe_id >= 0)
      reset_recipe_tab();
    ingredients->setFilter(QString("recipe = %1").arg(id));
    app->ui->leRecipeTitle->setText(db_recipe_name(id));
    app->ui->teRecipeSteps->setPlainText(db_recipe_steps(id));
    app->ui->recipeTab->setEnabled(true);
    app->ui->tabs->setCurrentIndex(recipe_tab_idx);
    recipe_id = id;
  }

  void start_add_recipe(QString name)
  {
    int id = db_add_recipe(name);
    if (id >= 0)
    {
      start_edit_recipe(id);
      emit app->recipe_added(id);
    }
  }

  void stop_edit_recipe()
  {
    if (recipe_id < 0)
      return;
    QString name = app->ui->leRecipeTitle->text();
    QString steps = app->ui->teRecipeSteps->toPlainText();
    db_set_recipe_name(recipe_id, name);
    db_set_recipe_steps(recipe_id, steps);
    reset_recipe_tab();
    query_refresh(recipes);
    app->ui->tabs->setCurrentIndex(recipes_tab_idx);
  }

  void remove_selected_recipes()
  {
    auto select = app->ui->recipesView->selectionModel();
    QList<int> removed = query_remove_ids(select, recipes, "recipes", 0);
    if (removed.contains(recipe_id))
      reset_recipe_tab();
    if (removed.size() > 0)
      emit app->recipes_removed();
  }

  int add_food(QString name)
  {
    QSqlRecord record;
    record.append(QSqlField("name", QVariant::String));
    record.setValue("name", name);
    if (foods->insertRecord(-1, record))
    {
      QModelIndex index = foods->index(foods->rowCount(), 0);
      int id = index.data().toInt();
      emit app->food_added(id);
      return id;
    }
    return -1;
  }

  void remove_selected_foods()
  {
    QList<int> removed = table_remove_rows(app->ui->foodsView->selectionModel(), foods, 0);
    if (removed.size() > 0)
      emit app->foods_removed();
  }

  bool add_ingredient(int recipe, QString name)
  {
    int food_id = db_food_id(name);
    if (food_id < 0 && !name.isEmpty())
    {
      food_id = add_food(name);
      if (food_id < 0)
        return -1;
    }

    QSqlRecord record;
    record.append(QSqlField("recipe", QVariant::Int));
    record.append(QSqlField("food", QVariant::Int));
    record.setValue("recipe", recipe);
    record.setValue("food", food_id);

    if (ingredients->insertRecord(-1, record))
    {
      QModelIndex index = ingredients->index(ingredients->rowCount(), 0);
      int id = index.data().toInt();
      return id;
    }
    return -1;
  }

  void remove_selected_ingredients()
  {
    table_remove_rows(app->ui->ingredientsView->selectionModel(), ingredients, 0);
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

  ui->tabs->setCurrentIndex(recipes_tab_idx);
  ui->recipeTab->setEnabled(false);

  connect(ui->bAddRecipe, &QPushButton::released, this, [this]()
  {
    impl->start_add_recipe("Untitled");
  });

  connect(ui->bDeleteRecipe, &QPushButton::released, this, [this]()
  {
    impl->remove_selected_recipes();
  });

  connect(ui->bDoneRecipe, &QPushButton::released, this, [this]()
  {
    impl->stop_edit_recipe();
  });

  connect(ui->leFood, &QLineEdit::returnPressed, this, [this]()
  {
    if (impl->add_food(ui->leFood->text()) >= 0)
      ui->leFood->clear();
  });

  connect(ui->bDeleteFood, &QPushButton::released, this, [this]()
  {
    impl->remove_selected_foods();
  });

  connect(ui->recipesView, &QTableView::doubleClicked, this, [this](const QModelIndex &index)
  {
    impl->start_edit_recipe(index.siblingAtColumn(0).data().toInt());
  });

  connect(ui->leIngredient, &QLineEdit::returnPressed, this, [this]()
  {
    if (impl->add_ingredient(impl->recipe_id, ui->leIngredient->text()))
      ui->leIngredient->clear();
  });

  connect(ui->bDeleteIngredient, &QPushButton::released, this, [this]()
  {
    impl->remove_selected_ingredients();
  });
}

App::~App()
{
  delete ui;
}

