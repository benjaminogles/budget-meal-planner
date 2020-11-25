
#include "app.h"
#include "ui_app.h"
#include "database.h"
#include "nametoiddelegate.h"

#include <QSqlField>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlTableModel>
#include <QStringListModel>
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
    return removed;
  }

  void query_refresh(QSqlQueryModel *model)
  {
    QString str = model->query().executedQuery();
    model->query().clear();
    model->setQuery(str);
  }

  QList<int> query_remove_ids(QItemSelectionModel *select, QString table, int id_column)
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
  QCompleter *food_completer = nullptr;
  QCompleter *recipe_completer = nullptr;
  int recipe_id = -1;

  Impl(App *app_) :
    app(app_),
    planned(new QSqlQueryModel(app)),
    groceries(new QSqlTableModel(app)),
    recipes(new QSqlQueryModel(app)),
    foods(new QSqlTableModel(app)),
    ingredients(new QSqlTableModel(app))
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

    groceries->setEditStrategy(QSqlTableModel::OnFieldChange);
    groceries->setTable("groceries");
    groceries->select();
  }

  ~Impl()
  {
    if (food_completer)
      delete food_completer;
    if (recipe_completer)
      delete recipe_completer;
  }

  void reset_recipe_completer()
  {
    auto old = recipe_completer;
    recipe_completer = new QCompleter(db_recipe_names());
    recipe_completer->setCompletionMode(QCompleter::InlineCompletion);
    recipe_completer->setCaseSensitivity(Qt::CaseInsensitive);
    app->ui->lePlanned->setCompleter(recipe_completer);
    if (old)
      delete old;
    auto delegate = qobject_cast<NameToIdDelegate*>(app->ui->plannedView->itemDelegate());
    delegate->reset(db_recipe_id_map());
  }

  void reset_recipe_tab()
  {
    recipe_id = -1;
    ingredients->setFilter("recipe is null");
    ingredients->select();
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
      reset_recipe_completer();
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
    reset_recipe_completer();
    app->ui->tabs->setCurrentIndex(recipes_tab_idx);
  }

  void remove_selected_recipes()
  {
    auto select = app->ui->recipesView->selectionModel();
    QList<int> removed = query_remove_ids(select, "recipes", 0);
    if (removed.contains(recipe_id))
      reset_recipe_tab();
    if (removed.size() > 0)
    {
      query_refresh(recipes);
      reset_recipe_completer();
    }
  }

  void reset_food_completer()
  {
    auto old = food_completer;
    food_completer = new QCompleter(db_food_names());
    food_completer->setCompletionMode(QCompleter::InlineCompletion);
    food_completer->setCaseSensitivity(Qt::CaseInsensitive);
    app->ui->leIngredient->setCompleter(food_completer);
    app->ui->leGrocery->setCompleter(food_completer);
    if (old)
      delete old;
    auto delegate = qobject_cast<NameToIdDelegate*>(app->ui->ingredientsView->itemDelegateForColumn(2));
    delegate->reset(db_food_id_map());
    delegate = qobject_cast<NameToIdDelegate*>(app->ui->groceriesView->itemDelegateForColumn(1));
    delegate->reset(db_food_id_map());
  }

  bool add_food(QString name)
  {
    QSqlRecord record;
    record.append(QSqlField("name", QVariant::String));
    record.setValue("name", name);
    if (foods->insertRecord(-1, record))
    {
      reset_food_completer();
      return true;
    }
    return false;
  }

  void remove_selected_foods()
  {
    QList<int> removed = table_remove_rows(app->ui->foodsView->selectionModel(), foods, 0);
    if (removed.size() > 0)
    {
      foods->select();
      reset_food_completer();
    }
  }

  bool add_ingredient(int recipe, QString name)
  {
    int food_id = db_food_id(name);
    if (food_id < 0 && !name.isEmpty())
    {
      if (!add_food(name))
        return false;
      food_id = db_food_id(name);
      if (food_id < 0)
        return false;
    }

    QSqlRecord record;
    record.append(QSqlField("recipe", QVariant::Int));
    record.append(QSqlField("food", QVariant::Int));
    record.setValue("recipe", recipe);
    record.setValue("food", food_id);
    return ingredients->insertRecord(-1, record);
  }

  void remove_selected_ingredients()
  {
    QList<int> removed = table_remove_rows(app->ui->ingredientsView->selectionModel(), ingredients, 0);
    if (removed.size() > 0)
      ingredients->select();
  }

  bool add_grocery(QString name)
  {
    int food_id = db_food_id(name);
    if (food_id < 0)
    {
      if (!add_food(name))
        return false;
      food_id = db_food_id(name);
      if (food_id < 0)
        return false;
    }

    QSqlRecord record;
    record.append(QSqlField("food", QVariant::Int));
    record.setValue("food", food_id);
    return groceries->insertRecord(-1, record);
  }

  void remove_selected_groceries()
  {
    QList<int> removed = table_remove_rows(app->ui->groceriesView->selectionModel(), groceries, 0);
    if (removed.size() > 0)
      groceries->select();
  }

  void reset_groceries()
  {
    db_clear_groceries();
    db_generate_groceries();
    groceries->select();
  }

  bool add_planned(QString name)
  {
    int recipe_id = db_recipe_id(name);
    if (recipe_id < 0)
      return false;
    if (db_add_planned(recipe_id))
    {
      query_refresh(planned);
      return true;
    }
    return false;
  }

  void clear_planned()
  {
    db_clear_planned();
    query_refresh(planned);
  }
};

App::App() : ui(new Ui::App), impl(std::make_unique<Impl>(this))
{
  ui->setupUi(this);

  ui->plannedView->setModel(impl->planned);
  ui->plannedView->setSelectionMode(QAbstractItemView::NoSelection);
  ui->plannedView->setItemDelegate(new NameToIdDelegate(db_recipe_id_map(), this));

  ui->groceriesView->setModel(impl->groceries);
  ui->groceriesView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  ui->groceriesView->setSelectionMode(QAbstractItemView::MultiSelection);
  ui->groceriesView->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->groceriesView->setItemDelegateForColumn(1, new NameToIdDelegate(db_food_id_map(), this));

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
  ui->ingredientsView->setItemDelegateForColumn(2, new NameToIdDelegate(db_food_id_map(), this));
  ui->ingredientsView->setItemDelegateForColumn(3, new NameToIdDelegate(db_unit_id_map(), this));

#ifdef QT_NO_DEBUG
  ui->recipesView->hideColumn(0);
  ui->groceriesView->hideColumn(0);
  ui->foodsView->hideColumn(0);
  ui->ingredientsView->hideColumn(0);
  ui->ingredientsView->hideColumn(1);
#endif

  ui->tabs->setCurrentIndex(recipes_tab_idx);
  ui->recipeTab->setEnabled(false);

  impl->reset_food_completer();
  impl->reset_recipe_completer();

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
    if (impl->add_food(ui->leFood->text()))
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

  connect(ui->leGrocery, &QLineEdit::returnPressed, this, [this]()
  {
    if (impl->add_grocery(ui->leGrocery->text()))
      ui->leGrocery->clear();
  });

  connect(ui->lePlanned, &QLineEdit::returnPressed, this, [this]()
  {
    if (impl->add_planned(ui->lePlanned->text()))
      ui->lePlanned->clear();
  });

  connect(ui->bClearPlanned, &QPushButton::released, this, [this]()
  {
    impl->clear_planned();
  });

  connect(ui->bResetGroceries, &QPushButton::released, this, [this]()
  {
    impl->reset_groceries();
  });

  connect(ui->bDeleteGrocery, &QPushButton::released, this, [this]()
  {
    impl->remove_selected_groceries();
  });

  connect(ui->plannedView, &QListView::doubleClicked, this, [this](const QModelIndex &index)
  {
    impl->start_edit_recipe(index.siblingAtColumn(0).data().toInt());
  });
}

App::~App()
{
  delete ui;
}

