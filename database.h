
#ifndef database_h
#define database_h

#include <QString>
#include <QMap>

bool db_init(QString);

QMap<QString, int> db_unit_id_map();

int db_add_recipe(QString);

bool db_remove_id(QString, int);

QString db_recipe_name(int);

QString db_recipe_steps(int);

bool db_set_recipe_name(int, QString);

bool db_set_recipe_steps(int, QString);

#endif
