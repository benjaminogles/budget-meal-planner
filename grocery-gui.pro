
TARGET = grocery-gui

CONFIG += c++14

QT += core widgets sql

SOURCES = \
  main.cc \
  database.cc \
  app.cc \
  tabs.cc \
  nametoiddelegate.cc

HEADERS = \
  database.h \
  app.h \
  tabs.h \
  nametoiddelegate.h

FORMS = \
  tabs.ui

