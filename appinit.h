
#ifndef appinit_h
#define appinit_h

#include <QApplication>
#include <memory>

class AppInit : public QApplication
{
  public:
    AppInit(int &argc, char **argv);
    ~AppInit();
};

#endif
