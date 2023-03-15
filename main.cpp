#include "App/app.h"
#include "App/mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication qapp(argc, argv);
  DSViz::MainWindow win;
  DSViz::App<int> app{&win};
  win.show();
  return qapp.exec();
}
