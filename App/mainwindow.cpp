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
    : QMainWindow(parent), ui{std::make_unique<Ui::MainWindow>()} {
  ui->setupUi(this);
}

// пустой деструктор все таки нужен:
// https://stackoverflow.com/questions/34072862/why-is-error-invalid-application-of-sizeof-to-an-incomplete-type-using-uniqu
MainWindow::~MainWindow() {}

} // namespace DSViz
