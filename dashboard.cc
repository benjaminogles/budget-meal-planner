
#include "dashboard.h"
#include "ui_dashboard.h"

#include <QSqlQueryModel>
#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include <QSqlRecord>

struct Dashboard::Impl
{
  Dashboard *dash;

  Impl(Dashboard *dash) : dash(dash) {}

  void insert_food()
  {
    QString name = dash->ui->leFood->text();
    if (name.isEmpty())
      return;

    QSqlRelationalTableModel *table = static_cast<QSqlRelationalTableModel*>(dash->ui->foodsView->model());
    QSqlRecord record = table->record();
    record.setValue("name", name);
    table->insertRecord(-1, record);
  }

};

Dashboard::Dashboard() : ui(new Ui::Dashboard), impl(std::make_unique<Impl>(this))
{
  ui->setupUi(this);

  QSqlQueryModel *planned = new QSqlQueryModel(this);
  planned->setQuery("select name from recipes where planned != 0");
  ui->plannedView->setModel(planned);

  QSqlRelationalTableModel *groceries = new QSqlRelationalTableModel(this);
  groceries->setEditStrategy(QSqlTableModel::OnFieldChange);
  groceries->setTable("groceries");
  groceries->setRelation(1, QSqlRelation("foods", "id", "name"));
  groceries->select();
  ui->groceriesView->setModel(groceries);
  ui->groceriesView->setItemDelegate(new QSqlRelationalDelegate(ui->groceriesView));

  QSqlQueryModel *recipes = new QSqlQueryModel(this);
  recipes->setQuery("select name from recipes");
  ui->recipesView->setModel(recipes);
  ui->recipesView->setSortingEnabled(true);

  QSqlRelationalTableModel *foods = new QSqlRelationalTableModel(this);
  foods->setEditStrategy(QSqlTableModel::OnFieldChange);
  foods->setTable("foods");
  foods->setRelation(4, QSqlRelation("units", "id", "name"));
  foods->setHeaderData(4, Qt::Horizontal, "unit");
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
