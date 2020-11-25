
#include "appinit.h"
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

AppInit::AppInit(int &argc, char **argv) : QApplication(argc, argv)
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
}

AppInit::~AppInit()
{
}

