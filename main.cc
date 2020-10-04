
#include "app.h"
#include "dashboard.h"

int main(int argc, char **argv)
{
  App application(argc, argv);
  Dashboard dashboard;
  dashboard.show();
  return application.exec();
}
