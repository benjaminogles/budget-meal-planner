
TARGET = grocery-gui

CONFIG += c++14

QT += core widgets sql

SOURCES = \
  main.cc \
  database.cc \
  app.cc \
  tabs.cc

HEADERS = \
  database.h \
  app.h \
  tabs.h

FORMS = \
  tabs.ui

