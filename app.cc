
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

  QSqlDatabase choose_db(QString src)
  {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    if (src.length())
      db.setDatabaseName(src);
    else
      db.setDatabaseName(":memory:");
    return db;
  }
}

struct App::Impl
{
  Impl(QString dbsrc) : db(choose_db(dbsrc)) {}

  QSqlDatabase db;
};

static App *_app = nullptr;

App* App::instance()
{
  Q_CHECK_PTR(_app);
  return _app;
}

App::App(int &argc, char **argv) : QApplication(argc, argv)
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

  impl = std::make_unique<Impl>(dbsrc);

  _app = this;
}

App::~App()
{
  _app = nullptr;
}

