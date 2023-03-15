#include "App/mainwindow.h"
#include "ui_mainwindow.h"
#include <qwt_plot.h>
#include <qwt_plot_grid.h>

#include <qwt_legend.h>

#include <qwt_plot_curve.h>
#include <qwt_symbol.h>

#include <qwt_plot_magnifier.h>

#include <qwt_plot_panner.h>

#include <qwt_picker_machine.h>
#include <qwt_plot_picker.h>
#include <qwt_symbol.h>

#include <qwt_legend_data.h>
#include <qwt_plot_marker.h>
#include <qwt_slider.h>
#include <qwt_text.h>

namespace DSViz {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
}

MainWindow::~MainWindow() { delete ui; }
} // namespace DSViz
