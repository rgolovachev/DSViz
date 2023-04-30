#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qwt_plot.h"
#include <QMainWindow>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

namespace DSViz {

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  QwtPlot *Plot();

  std::unique_ptr<Ui::MainWindow> ui;
};
} // namespace DSViz
#endif // MAINWINDOW_H
