
#include "app.h"

#include <QSqlDatabase>
#include <QSqlQuery>

namespace
{
  void check_fatal(bool cond, const char *msg)
  {
    if(!cond)
    {
      qCritical("%s\n", msg);
      exit(-1);
    }
  }

  bool init_db(QString src)
  {
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
      "to_unit integer not null references units(id)"
      ");";
    if (!query.exec(statement))
      return false;

    statement =
      "create table if not exists recipes ("
      "id integer primary key asc,"
      "name text not null,"
      "planned integer not null default 0,"
      "meals integer not null default 0,"
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

    return true;
  }
}

struct App::Impl
{
};

static App *_app = nullptr;

App* App::instance()
{
  Q_CHECK_PTR(_app);
  return _app;
}

App::App(int &argc, char **argv) : QApplication(argc, argv), impl(std::make_unique<Impl>())
{
  QString dbsrc;
  for (int i = 1; i < argc; i++)
  {
    QString arg(argv[i]);
    if (arg == "--db")
    {
      check_fatal(argc > i + 1, "Missing argument for --db option");
      dbsrc = argv[i + 1];
    }
  }
  check_fatal(init_db(dbsrc), "Unable to connect or initialize database");
  _app = this;
}

App::~App()
{
  _app = nullptr;
}

