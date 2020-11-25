
#include "appinit.h"
#include "app.h"

int main(int argc, char **argv)
{
  AppInit init(argc, argv);
  App app;
  app.show();
  return init.exec();
}
