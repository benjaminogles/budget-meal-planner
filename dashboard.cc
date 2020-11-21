
#include "dashboard.h"
#include "ui_dashboard.h"

#include <QSqlField>
#include <QSqlRecord>
#include <QSqlQueryModel>
#include <QSqlTableModel>

struct Dashboard::Impl
{
  Dashboard *dash;

  Impl(Dashboard *dash) : dash(dash) {}

  void insert_food()
  {
    QString name = dash->ui->leFood->text();
    if (name.isEmpty())
      return;

    QSqlRecord record;
    record.append(QSqlField("name", QVariant::String));
    record.setValue("name", name);

    QSqlTableModel *table = static_cast<QSqlTableModel*>(dash->ui->foodsView->model());
    table->insertRecord(-1, record);
  }

};

Dashboard::Dashboard() : ui(new Ui::Dashboard), impl(std::make_unique<Impl>(this))
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

  QSqlTableModel *foods = new QSqlTableModel(this);
  foods->setEditStrategy(QSqlTableModel::OnFieldChange);
  foods->setTable("foods");
  foods->select();
  ui->foodsView->setModel(foods);
  ui->foodsView->setSortingEnabled(true);

  connect(ui->leFood, &QLineEdit::returnPressed, this, [this](){ impl->insert_food(); });

#ifdef QT_NO_DEBUG
  ui->groceriesView->hideColumn(0);
  ui->foodsView->hideColumn(0);
#endif
}

Dashboard::~Dashboard()
{
  delete ui;
}
