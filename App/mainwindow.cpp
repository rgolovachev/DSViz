#include "App/mainwindow.h"
#include "ui_mainwindow.h"

namespace DSViz {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui{std::make_unique<Ui::MainWindow>()} {
  ui->setupUi(this);
}

// пустой деструктор все таки нужен:
// https://stackoverflow.com/questions/34072862/why-is-error-invalid-application-of-sizeof-to-an-incomplete-type-using-uniqu
MainWindow::~MainWindow() {}

} // namespace DSViz
