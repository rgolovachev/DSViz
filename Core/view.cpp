#include "Core/view.h"

namespace DSViz {

namespace detail {

QColor Palette::GetColor(State state) {
  if (StateToClr.find(state) != StateToClr.end()) {
    return StateToClr.at(state);
  }
  return QColorConstants::Gray;
}

std::string Text::GetMsg(MsgCode code) {
  if (StrMsg.find(code) != StrMsg.end()) {
    return StrMsg.at(code);
  }
  return "";
}

QString Text::GetText(QComboBox *ptr) {
  if (ptr) {
    return ptr->currentText();
  }
  return "";
}

void Text::ClearBox(QComboBox *ptr) {
  if (ptr) {
    ptr->clear();
  }
}

void Text::InsertItem(QComboBox *ptr, int num) {
  if (ptr) {
    ptr->addItem(std::to_string(num).c_str());
  }
}

void Text::UpdIndex(QComboBox *ptr, const QString &str) {
  int index = ptr->findText(str);
  if (index != -1) {
    ptr->setCurrentIndex(index);
  } else {
    ptr->setCurrentIndex(0);
  }
}

std::string Text::LegendByState(State state) {
  std::string str;
  switch (state) {
  case State::A_SUBTREE:
    str = "A subtree";
    break;
  case State::B_SUBTREE:
    str = "B subtree";
    break;
  case State::C_SUBTREE:
    str = "C subtree";
    break;
  case State::D_SUBTREE:
    str = "D subtree";
    break;
  case State::X_VERTEX:
    str = "Vertex X";
    break;
  case State::P_VERTEX:
    str = "Vertex P";
    break;
  case State::G_VERTEX:
    str = "Vertex G";
    break;
  default:
    str = "";
    break;
  }
  return str;
}

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
