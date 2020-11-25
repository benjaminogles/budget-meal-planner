
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

  signals:
    void recipe_added(int);
    void recipes_removed();
    void food_added(int);
    void foods_removed();

  private:
    Ui::App *ui;
    struct Impl;
    std::unique_ptr<Impl> impl;
};

#endif

