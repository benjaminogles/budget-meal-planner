
#include "database.h"

#include <QSqlDatabase>
#include <QVariant>
#include <QSqlQuery>
#include <QSqlError>

namespace
{
  int schema_version = 0;
  bool initialized = false;

  bool db_init_units()
  {
    QString statement;
    QSqlQuery query;

    statement =
      "insert into units (name) values"
      "('pinch'),"
      "('teaspoon'),"
      "('tablespoon'),"
      "('cup'),"
      "('quart'),"
      "('pint'),"
      "('gallon'),"
      "('ounce'),"
      "('pound');";
    if (!query.exec(statement))
      return false;
    return true;
  }

  QMap<QString, int> db_name_id_map(QString table)
  {
    QMap<QString, int> result;
    QSqlQuery query(QString("select name, id from %1;").arg(table));
    while (query.next())
      result.insert(query.value(0).toString(), query.value(1).toInt());
    return result;
  }

  int db_schema_version()
  {
    QSqlQuery query("select value from schema_versions order by value asc limit 1;");
    if (query.next())
      return query.value(0).toInt();
    return -1;
  }

  bool db_set_field_by_id(int id, QString table, QString field, QVariant value)
  {
    QSqlQuery query;
    if (!query.prepare(QString("update %1 set %2 = :value where id = :id;").arg(table).arg(field)))
      return false;
    query.bindValue(":value", value);
    query.bindValue(":id", id);
    return query.exec();
  }

  int db_id_by_field(QString table, QString field, QVariant value)
  {
    QSqlQuery query;
    if (!query.prepare(QString("select id from %1 where %2 = :value;").arg(table).arg(field)))
      return -1;
    query.bindValue(":value", value);
    if (!query.exec() || !query.next())
      return -1;
    return query.value(0).toInt();
  }

  bool db_set_schema_version()
  {
    QSqlQuery query;
    if (!query.prepare("insert into schema_versions (value) values (?)"))
      return false;
    query.addBindValue(QVariant(schema_version));
    return query.exec();
  }

  QVariant db_field_by_id(QString table, QString field, int id)
  {
    QSqlQuery query;
    if (!query.prepare(QString("select %1 from %2 where id = :id").arg(field).arg(table)))
      return QVariant();
    query.bindValue(":id", id);
    if (!query.exec() || !query.next())
      return QVariant();
    return query.value(0);
  }

  QStringList db_field_list(QString table, QString field)
  {
    QStringList result;
    QSqlQuery query(QString("select %1 from %2;").arg(field).arg(table));
    while (query.next())
      result.append(query.value(0).toString());
    return result;
  }
}

bool db_init(QString src)
{
  if (initialized)
    return false;
  initialized = true;

  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  if (src.length())
    db.setDatabaseName(src);
  else
    db.setDatabaseName(":memory:");
  if (!db.open())
    return false;

  QSqlQuery query("pragma foreign_keys = on;");
  QString statement;

  statement = 
    "create table if not exists schema_versions ("
    "id integer primary key asc,"
    "value integer"
    ");";
  if (!query.exec(statement))
    return false;

  int current_version = db_schema_version();
  bool fresh = current_version < 0;
  if (current_version < schema_version && !db_set_schema_version())
    return false;

  statement =
    "create table if not exists units ("
    "id integer primary key asc,"
    "name text not null,"
    "constraint unit_name_unique unique (name)"
    ");";
  if (!query.exec(statement))
    return false;

  statement =
    "create table if not exists recipes ("
    "id integer primary key asc,"
    "name text not null,"
    "planned integer not null default 0,"
    "meals real not null default 1,"
    "steps text not null default ''"
    ");";
  if (!query.exec(statement))
    return false;

  statement =
    "create table if not exists foods ("
    "id integer primary key asc,"
    "name text not null,"
    "staple integer not null default 0,"
    "price real not null default 0,"
    "constraint food_name_unique unique (name)"
    ");";
  if (!query.exec(statement))
    return false;

  statement =
    "create table if not exists ingredients ("
    "id integer primary key asc,"
    "recipe integer not null references recipes(id) on delete cascade,"
    "food integer not null references foods(id) on delete cascade,"
    "unit integer references units(id),"
    "quantity real not null default 0"
    ");";
  if (!query.exec(statement))
    return false;

  statement =
    "create table if not exists groceries ("
    "id integer primary key asc,"
    "food integer not null references foods(id),"
    "quantity real not null,"
    "generated integer not null default 0"
    ");";
  if (!query.exec(statement))
    return false;

  if (fresh && !db_init_units())
    return false;

  return true;
}

QMap<QString, int> db_unit_id_map()
{
  return db_name_id_map("units");
}

QMap<QString, int> db_food_id_map()
{
  return db_name_id_map("foods");
}

QMap<QString, int> db_recipe_id_map()
{
  return db_name_id_map("recipes");
}

int db_add_recipe(QString name)
{
  QSqlQuery query;
  if (!query.prepare("insert into recipes (name) values (:name);"))
    return -1;
  query.bindValue(":name", name);
  return query.exec() ? query.lastInsertId().toInt() : -1;
}

bool db_remove_id(QString table, int id)
{
  QSqlQuery query;
  if (!query.prepare(QString("delete from %1 where id = :id;").arg(table)))
    return false;
  query.bindValue(":id", id);
  return query.exec();
}

QString db_recipe_name(int id)
{
  return db_field_by_id("recipes", "name", id).toString();
}

QString db_recipe_steps(int id)
{
  return db_field_by_id("recipes", "steps", id).toString();
}

bool db_set_recipe_name(int id, QString name)
{
  return db_set_field_by_id(id, "recipes", "name", name);
}

bool db_set_recipe_steps(int id, QString steps)
{
  return db_set_field_by_id(id, "recipes", "steps", steps);
}

QStringList db_food_names()
{
  return db_field_list("foods", "name");
}

QStringList db_recipe_names()
{
  return db_field_list("recipes", "name");
}

int db_food_id(QString name)
{
  return db_id_by_field("foods", "name", name);
}

int db_recipe_id(QString name)
{
  return db_id_by_field("recipes", "name", name);
}

int db_add_food(QString name)
{
  QSqlQuery query;
  if (!query.prepare("insert into foods (name) values (:name);"))
    return -1;
  query.bindValue(":name", name);
  return query.exec() ? query.lastInsertId().toInt() : -1;
}

void db_clear_planned_groceries()
{
  QSqlQuery query("delete from groceries where generated = 1;");
}

void db_generate_planned_groceries()
{
  QSqlQuery query(
      "insert into groceries (generated, food, quantity) "
      "select 1, f.id, (case f.staple when 0 then sum(i.quantity) else 1 end) "
      "from recipes r join ingredients i on r.id = i.recipe join foods f on f.id = i.food where r.planned = 1 "
      "group by f.id;"
      );
}

bool db_add_planned(int recipe)
{
  QSqlQuery query;
  if (!query.prepare("update recipes set planned = 1 where id = :id"))
    return false;
  query.bindValue(":id", recipe);
  return query.exec();
}

void db_clear_planned()
{
  QSqlQuery query("update recipes set planned = 0;");
}
