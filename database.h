
#ifndef database_h
#define database_h

#include <QString>
#include <QMap>

bool db_init(QString);

QMap<QString, int> db_unit_id_map();

#endif
