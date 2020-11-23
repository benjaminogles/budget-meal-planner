
#include "app.h"
#include "database.h"

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
  check_fatal(db_init(dbsrc), "Unable to connect to or initialize database");
  _app = this;
}

App::~App()
{
  _app = nullptr;
}

