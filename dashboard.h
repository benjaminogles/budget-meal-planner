
#ifndef dashboard_h
#define dashboard_h

#include <QMainWindow>
#include <memory>

namespace Ui { class Dashboard; }

class Dashboard : public QMainWindow
{
  Q_OBJECT

  public:
    Dashboard();
    ~Dashboard();

  private:
    Ui::Dashboard *ui;
    struct Impl;
    std::unique_ptr<Impl> impl;
};

#endif

