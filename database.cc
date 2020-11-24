
#include "database.h"

#include <QSqlDatabase>
#include <QVariant>
#include <QSqlQuery>

namespace
{
  int schema_version = 0;
  bool initialized = false;

  bool db_add_conversion(QSqlQuery prepared, QString from, QString to, double factor)
  {
    prepared.bindValue(":from_unit", QVariant(from));
    prepared.bindValue(":to_unit", QVariant(to));
    prepared.bindValue(":factor", QVariant(factor));
    return prepared.exec();
  }

  int db_schema_version()
  {
    QSqlQuery query("select value from schema_versions order by value asc limit 1;");
    if (query.next())
      return query.value(0).toInt();
    return -1;
  }

  bool db_set_schema_version()
  {
    QSqlQuery query;
    if (!query.prepare("insert into schema_versions (value) values (?)"))
      return false;
    query.addBindValue(QVariant(schema_version));
    return query.exec();
  }

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
      "('milliliter'),"
      "('liter'),"
      "('ounce'),"
      "('pound'),"
      "('gram')"
      ";";
    if (!query.exec(statement))
      return false;

    statement =
      "insert into conversions (from_unit, to_unit, factor) values ("
      "(select id from units where name = :from_unit),"
      "(select id from units where name = :to_unit),"
      ":factor"
      ");";
    if (!query.prepare(statement))
      return false;

    if (!db_add_conversion(query, "teaspoon", "tablespoon", 3.0))
      return false;
    if (!db_add_conversion(query, "tablespoon", "teaspoon", 0.333))
      return false;
    if (!db_add_conversion(query, "tablespoon", "cup", 16.231))
      return false;
    if (!db_add_conversion(query, "cup", "tablespoon", 0.0616))
      return false;
    if (!db_add_conversion(query, "cup", "quart", 4.0))
      return false;
    if (!db_add_conversion(query, "quart", "cup", 0.25))
      return false;

    return true;
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

  QSqlQuery query;
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
    "create table if not exists unit_aliases ("
    "id integer primary key asc,"
    "unit integer not null references units(id),"
    "alias text not null"
    ");";
  if (!query.exec(statement))
    return false;

  statement =
    "create table if not exists conversions ("
    "id integer primary key asc,"
    "from_unit integer not null references units(id),"
    "to_unit integer not null references units(id),"
    "factor real not null default 1"
    ");";
  if (!query.exec(statement))
    return false;

  statement =
    "create table if not exists recipes ("
    "id integer primary key asc,"
    "name text not null,"
    "planned integer not null default 0,"
    "meals integer not null default 0,"
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
    "unit integer references units(id),"
    "quantity real not null default 0,"
    "constraint food_name_unique unique (name)"
    ");";
  if (!query.exec(statement))
    return false;

  statement =
    "create table if not exists ingredients ("
    "id integer primary key asc,"
    "recipe integer not null references recipes(id),"
    "food integer not null references foods(id),"
    "unit integer references units(id),"
    "quantity real not null default 0"
    ");";
  if (!query.exec(statement))
    return false;

  statement =
    "create table if not exists groceries ("
    "id integer primary key asc,"
    "food integer not null references foods(id),"
    "quantity real not null default 0"
    ");";
  if (!query.exec(statement))
    return false;

  if (fresh && !db_init_units())
    return false;

  return true;
}

QMap<QString, int> db_unit_id_map()
{
  QMap<QString, int> result;
  QSqlQuery query("select name, id from units;");
  while (query.next())
    result.insert(query.value(0).toString(), query.value(1).toInt());
  return result;
}

int db_add_recipe(QString name)
{
  QSqlQuery query;
  if (!query.prepare("insert into recipes (name) values (?);"))
    return -1;
  query.addBindValue(name);
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

