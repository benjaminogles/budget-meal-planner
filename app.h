
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

  public slots:
    void add_food();
    void remove_foods();
    void remove_recipes();
    void set_recipe_name(const QString&);
    void set_recipe_steps();
    void add_ingredient();
    void remove_ingredients();
    void reset_recipe_tab();
    void start_edit_recipe(int);
    void start_add_recipe();
    void stop_edit_recipe();
    void recipe_double_clicked(const QModelIndex&);

  private:
    Ui::App *ui;
    struct Impl;
    std::unique_ptr<Impl> impl;
};

#endif

