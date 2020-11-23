
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
    void remove_foods();
    void remove_recipes();
    void reset_recipe_tab();
    void start_edit_recipe(int);
    void start_add_recipe();

  private:
    Ui::Tabs *ui;
    struct Impl;
    std::unique_ptr<Impl> impl;
};

#endif

