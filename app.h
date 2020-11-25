
#ifndef app_h
#define app_h

#include <QMainWindow>
#include <memory>

namespace Ui { class App; }

class App : public QMainWindow
{
  Q_OBJECT

  public:
    App();
    ~App();

  private:
    Ui::App *ui;
    struct Impl;
    std::unique_ptr<Impl> impl;
};

#endif

