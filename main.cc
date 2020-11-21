
#include "app.h"
#include "tabs.h"

int main(int argc, char **argv)
{
  App application(argc, argv);
  Tabs tabs;
  tabs.show();
  return application.exec();
}
