
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

  public slots:
    void add_food();

  private:
    Ui::Tabs *ui;
    struct Impl;
    std::unique_ptr<Impl> impl;
};

#endif

