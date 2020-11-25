
#ifndef database_h
#define database_h

#include <QString>
#include <QStringList>
#include <QMap>

bool db_init(QString);

QMap<QString, int> db_unit_id_map();
QMap<QString, int> db_food_id_map();

QStringList db_food_names();
QStringList db_recipe_names();

int db_add_recipe(QString);
int db_add_food(QString);

bool db_remove_id(QString, int);

QString db_recipe_name(int);
QString db_recipe_steps(int);

int db_food_id(QString);

bool db_set_recipe_name(int, QString);
bool db_set_recipe_steps(int, QString);

#endif
