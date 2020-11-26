
TARGET = grocery-gui

CONFIG += c++14

QT += core widgets sql

SOURCES = \
  main.cc \
  database.cc \
  appinit.cc \
  app.cc \
  nametoiddelegate.cc \
  currencydelegate.cc

HEADERS = \
  database.h \
  appinit.h \
  app.h \
  nametoiddelegate.h \
  currencydelegate.h

FORMS = \
  app.ui

