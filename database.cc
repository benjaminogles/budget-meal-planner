
#include "database.h"

#include <QSqlDatabase>
#include <QVariant>
#include <QSqlQuery>

namespace
{
  bool initialized = false;

  bool db_add_conversion(QSqlQuery prepared, QString from, QString to, double factor)
  {
    prepared.bindValue(":from_unit", QVariant(from));
    prepared.bindValue(":to_unit", QVariant(to));
    prepared.bindValue(":factor", QVariant(factor));
    return prepared.exec();
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
    "id integer primary key asc"
    ");";
  if (!query.exec(statement))
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
    "steps text not null default '',"
    "constraint recipe_name_unique unique (name)"
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

QMap<QString, int> db_unit_id_map()
{
  QMap<QString, int> result;
  QSqlQuery query("select name, id from units;");
  while (query.next())
    result.insert(query.value(0).toString(), query.value(1).toInt());
  return result;
}

int db_add_recipe(QString)
{
  return -1;
}
