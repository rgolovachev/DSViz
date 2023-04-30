#include "App/app.h"
#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication qapp(argc, argv);
  DSViz::App app{};
  return qapp.exec();
}
