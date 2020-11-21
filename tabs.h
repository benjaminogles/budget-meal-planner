
#ifndef tabs_h
#define tabs_h

#include <QMainWindow>
#include <memory>

namespace Ui { class Tabs; }

class Tabs : public QMainWindow
{
  Q_OBJECT

  public:
    Tabs();
    ~Tabs();

  private:
    Ui::Tabs *ui;
    struct Impl;
    std::unique_ptr<Impl> impl;
};

#endif

