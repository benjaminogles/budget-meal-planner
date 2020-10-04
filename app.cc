
#include "app.h"

#include <QSqlDatabase>

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
    return db.open();
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
  check_fatal(init_db(dbsrc), "Unable to connect to database");
  _app = this;
}

App::~App()
{
  _app = nullptr;
}

