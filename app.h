
#ifndef app_h
#define app_h

#include <QApplication>
#include <memory>

#define app App::instance()

class App : public QApplication
{
  public:
    App(int &argc, char **argv);
    ~App();

    static App* instance();

  private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

#endif
