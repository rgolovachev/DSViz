#include "Core/view.h"

namespace DSViz {

namespace detail {

CustomPanner::CustomPanner(QWidget *parent) : QwtPlotPanner(parent) {}

// переопределяю eventFilter чтобы не было такого, что я двигаю qwt_plot
// мышкой, а картинка застыла
bool CustomPanner::eventFilter(QObject *object, QEvent *event) {
  if (object == NULL || object != parentWidget())
    return false;
  switch (event->type()) {
  case QEvent::MouseButtonPress: {
    QMouseEvent *evr = static_cast<QMouseEvent *>(event);
    setMouseButton(evr->button(), evr->modifiers());
    widgetMousePressEvent(evr);
    widgetMouseReleaseEvent(evr);
    break;
  }
  case QEvent::MouseMove: {
    QMouseEvent *evr = static_cast<QMouseEvent *>(event);
    widgetMouseMoveEvent(evr);
    widgetMouseReleaseEvent(evr);
    setMouseButton(evr->button(), evr->modifiers());
    widgetMousePressEvent(evr);
    break;
  }
  case QEvent::MouseButtonRelease: {
    QMouseEvent *evr = static_cast<QMouseEvent *>(event);
    widgetMouseReleaseEvent(evr);
    break;
  }
  case QEvent::KeyPress: {
    widgetKeyPressEvent(static_cast<QKeyEvent *>(event));
    break;
  }
  case QEvent::KeyRelease: {
    widgetKeyReleaseEvent(static_cast<QKeyEvent *>(event));
    break;
  }
  case QEvent::Paint: {
    if (isVisible())
      return true;
    break;
  }
  default:;
  }

  return false;
}

} // namespace detail
} // namespace DSViz
