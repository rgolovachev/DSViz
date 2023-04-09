#include "Core/view.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QMouseEvent>
#include <qwt_plot_curve.h>
#include <qwt_plot_legenditem.h>
#include <qwt_plot_marker.h>
#include <qwt_text.h>

namespace DSViz {

namespace detail {

QColor Palette::GetColor(State state) {
  if (StateToClr.find(state) != StateToClr.end()) {
    return StateToClr.at(state);
  }
  return kDefaultColor;
}

std::string Text::GetMsg(MsgCode code) {
  if (StrMsg.find(code) != StrMsg.end()) {
    return StrMsg.at(code);
  }
  return "";
}

std::string Text::LegendByState(State state) {
  std::string str;
  switch (state) {
  case State::a_subtree:
    str = "A subtree";
    break;
  case State::b_subtree:
    str = "B subtree";
    break;
  case State::c_subtree:
    str = "C subtree";
    break;
  case State::d_subtree:
    str = "D subtree";
    break;
  case State::x_vertex:
    str = "Vertex X";
    break;
  case State::p_vertex:
    str = "Vertex P";
    break;
  case State::g_vertex:
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

// высунул вперед, т.к. компилятор должен смочь вывести тип в auto
auto View::GetCallback() {
  return [this](const MsgType &msg) { HandleMsg(msg.first, msg.second); };
}

View::View()
    : MW_{std::make_unique<MainWindow>()},
      panner_{std::make_unique<CustomPanner>(MW_->GetPlot()->canvas())},
      port_in_{GetCallback()} {
  ConnectWidgets();
  ConfigureWidgets();
  MW_->show();
  // как и в конструкторе model, оно только запишет во внутренние поля
  // Observable это и не будет отправлять потому что еще никто не подписан
  port_out_.Set(UserQuery{QueryType::do_nothing, {0, 0}});
}

void View::SubscribeToController(Observer<UserQuery> *controller_observer) {
  port_out_.Subscribe(controller_observer);
}

Observer<View::MsgType> *View::GetPortIn() { return &port_in_; }

void View::OnPanned(int dx, int dy) {
  x_ += dx;
  y_ += dy;
}

void View::OnPauseOrStop() {
  static int remainingTime = -1;
  if (!stopped_) {
    remainingTime = timer_.remainingTime();
    timer_.stop();
    stopped_ = true;
    MW_->ui->pauseButton->setText("Continue");
  } else {
    MW_->ui->pauseButton->setText("Pause");
    stopped_ = false;
    timer_.start(remainingTime);
  }
}

void View::OnButtonClick() {
  if (sender() == MW_->ui->mergeButton) {
    SetEnabledWidgets(false);
    merge_executing_ = true;
    int left_id = MW_->ui->lefttreeId->currentText().toInt(),
        right_id = MW_->ui->righttreeId->currentText().toInt();
    port_out_.Set(UserQuery{QueryType::merge, {left_id, right_id}});
    merge_executing_ = false;
    SetEnabledWidgets(true);
    MW_->ui->maintreeId->setCurrentText(MW_->ui->lefttreeId->currentText());
    return;
  }

  bool ver_correct{}, id_correct{};
  int id = MW_->ui->maintreeId->currentText().toInt(&id_correct);
  int ver = MW_->ui->vertexId->text().toInt(&ver_correct);

  if (id_correct && (sender() == MW_->ui->deltreeButton)) {
    port_out_.Set(UserQuery{QueryType::deltree, {id, 0}});

  } else if (id_correct && ver_correct) {
    SetEnabledWidgets(false);
    if (sender() == MW_->ui->insertButton) {
      port_out_.Set(UserQuery{QueryType::insert, {id, ver}});
    } else if (sender() == MW_->ui->removeButton) {
      port_out_.Set(UserQuery{QueryType::remove, {id, ver}});
    } else if (sender() == MW_->ui->findButton) {
      port_out_.Set(UserQuery{QueryType::find, {id, ver}});
    } else if (sender() == MW_->ui->splitButton) {
      port_out_.Set(UserQuery{QueryType::split, {id, ver}});
    }
    SetEnabledWidgets(true);
  } else {
    QMessageBox::warning(NULL, QObject::tr("Ошибка"), QObject::tr(kErrMsg));
  }
}

void View::OnZoom(double value) {
  scale_ = value;
  MW_->GetPlot()->setAxisScale(QwtPlot::xBottom, -kBound / scale_,
                               kBound / scale_);
  MW_->GetPlot()->setAxisScale(QwtPlot::yLeft, -kBound / scale_,
                               kBound / scale_);
  Draw();
  panner_->moveCanvas(x_, y_);
}

void View::OnChoiceChange(QString num) {
  Prepare();
  Draw();
}

void View::ConnectWidgets() {
  QObject::connect(MW_->ui->insertButton, SIGNAL(clicked()), this,
                   SLOT(OnButtonClick()));
  QObject::connect(MW_->ui->removeButton, SIGNAL(clicked()), this,
                   SLOT(OnButtonClick()));
  QObject::connect(MW_->ui->findButton, SIGNAL(clicked()), this,
                   SLOT(OnButtonClick()));
  QObject::connect(MW_->ui->splitButton, SIGNAL(clicked()), this,
                   SLOT(OnButtonClick()));
  QObject::connect(MW_->ui->mergeButton, SIGNAL(clicked()), this,
                   SLOT(OnButtonClick()));
  QObject::connect(MW_->ui->deltreeButton, SIGNAL(clicked()), this,
                   SLOT(OnButtonClick()));
  QObject::connect(MW_->ui->qwt_slider, SIGNAL(sliderMoved(double)), this,
                   SLOT(OnZoom(double)));
  QObject::connect(MW_->ui->pauseButton, SIGNAL(clicked()), this,
                   SLOT(OnPauseOrStop()));
  QObject::connect(MW_->ui->maintreeId, SIGNAL(currentTextChanged(QString)),
                   this, SLOT(OnChoiceChange(QString)));
  QObject::connect(panner_.get(), SIGNAL(panned(int, int)), this,
                   SLOT(OnPanned(int, int)));
}

void View::ConfigureWidgets() {
  MW_->GetPlot()->setCanvasBackground(Qt::white);
  MW_->GetPlot()->setAxisScale(QwtPlot::xBottom, -kBound, kBound);
  MW_->GetPlot()->setAxisScale(QwtPlot::yLeft, -kBound, kBound);
  MW_->GetPlot()->enableAxis(0, false);
  MW_->GetPlot()->enableAxis(2, false);
  MW_->ui->qwt_slider->setValue(kSliderBegin);
  MW_->ui->qwt_slider->setScale(kSliderLowerBound, kSliderUpperBound);
  panner_->setMouseButton(Qt::LeftButton);
}

bool View::DoDelay(MsgCode code) {
  if (!MW_->ui->animationOff->isChecked() && (code != MsgCode::succ_del) &&
      (code != MsgCode::unsucc_del) && (code != MsgCode::empty_msg)) {
    return true;
  }
  return false;
}

void View::HandleMsg(MsgCode code, const BareTrees &trees) {
  trees_ = trees;
  UpdateComboBox();
  SetStatus(code);
  Prepare();
  Draw();
  if (DoDelay(code)) {
    Delay(MW_->ui->delayTime->value());
  }
}

void View::UpdateComboBox() {
  QComboBox *maintreeId = MW_->ui->maintreeId,
            *lefttreeId = MW_->ui->lefttreeId,
            *righttreeId = MW_->ui->righttreeId;
  QObject::disconnect(maintreeId, SIGNAL(currentTextChanged(QString)), this,
                      SLOT(OnChoiceChange(QString)));
  QString cur_str = GetText(maintreeId);
  QString merge1_str = GetText(lefttreeId);
  QString merge2_str = GetText(righttreeId);
  ClearBox(maintreeId);
  ClearBox(lefttreeId);
  ClearBox(righttreeId);
  for (auto &[num, tree] : trees_) {
    InsertItem(maintreeId, num);
    InsertItem(lefttreeId, num);
    InsertItem(righttreeId, num);
  }
  QObject::connect(MW_->ui->maintreeId, SIGNAL(currentTextChanged(QString)),
                   this, SLOT(OnChoiceChange(QString)));
  UpdIndex(maintreeId, cur_str);
  UpdIndex(lefttreeId, merge1_str);
  UpdIndex(righttreeId, merge2_str);
}

void View::SetStatus(MsgCode code) {
  std::string msg;
  if (code == MsgCode::split_succ) {
    msg = "The left tree ID is " +
          MW_->ui->maintreeId->currentText().toStdString() + ". The right is " +
          QString::number(next_id_).toStdString();
    ++next_id_;
  } else if (code == MsgCode::merge_end) {
    msg = Text::GetMsg(MsgCode::merge_end) +
          MW_->ui->lefttreeId->currentText().toStdString();
  } else {
    msg = Text::GetMsg(code);
  }
  MW_->ui->statusbar->showMessage(msg.c_str());
}

void View::Prepare() {
  if (!merge_executing_) {
    cur_tree_.Fill(trees_[MW_->ui->maintreeId->currentText().toInt()]);
  } else {
    cur_tree_.Fill(trees_[MW_->ui->lefttreeId->currentText().toInt()]);
  }
}

void View::Draw() {
  auto plot = MW_->GetPlot();

  plot->detachItems();

  QwtPlotLegendItem *leg = new QwtPlotLegendItem{};
  leg->attach(plot);
  auto cur_tree = cur_tree_.Get();
  if (cur_tree) {
    if (cur_tree->node->state == State::hide_this) {
      DrawTwoTrees(plot, cur_tree->left, cur_tree->right);
    } else if (cur_tree->node->state == State::split_left) {
      DrawTwoTrees(plot, cur_tree->left, cur_tree);
    } else if (cur_tree->node->state == State::split_right) {
      DrawTwoTrees(plot, cur_tree, cur_tree->right);
    } else {
      DrawOneTree(plot);
    }
  }

  plot->replot();
}

void View::DrawTwoTrees(QwtPlot *plot, PVNode left, PVNode right) {
  QwtPlotCurve *cur = new QwtPlotCurve{}, *cur2 = new QwtPlotCurve{};
  cur->setItemAttribute(QwtPlotItem::Legend, false);
  cur2->setItemAttribute(QwtPlotItem::Legend, false);
  QPolygonF left_points, right_points;

  AddPoints(left_points, left);
  cur->setSamples(left_points);
  cur->attach(plot);

  AddPoints(right_points, right);
  cur2->setSamples(right_points);
  cur2->attach(plot);
}

void View::DrawOneTree(QwtPlot *plot) {
  QwtPlotCurve *cur = new QwtPlotCurve{};
  cur->setItemAttribute(QwtPlotItem::Legend, false);
  QPolygonF points;
  AddPoints(points, cur_tree_.Get());

  cur->setSamples(points);
  cur->attach(plot);
}

void View::AddPoints(QPolygonF &points, PVNode vnode) {
  if (!vnode) {
    return;
  }

  points << QPointF(vnode->x, vnode->y);
  PushState(vnode);
  if (vnode->node->state != State::split_left) {
    AddPoints(points, vnode->left);
    points << QPointF(vnode->x, vnode->y);
  }
  if (vnode->node->state != State::split_right) {
    AddPoints(points, vnode->right);
    points << QPointF(vnode->x, vnode->y);
  }

  AttachVertex(vnode, GetSymbol(vnode));
}

void View::AttachVertex(PVNode vnode, QwtSymbol *sym) {
  QwtPlotMarker *num = new QwtPlotMarker{};

  num->setValue(vnode->x, vnode->y);
  num->setSymbol(sym);
  QwtText txt = QwtText(QString(std::to_string(vnode->node->value).c_str()));
  int font_sz = kFontSz * scale_;
  QFont font{QString(kFont), font_sz};
  txt.setFont(font);
  num->setLabel(txt);

  if ((IsSubtreeState(vnode->node) && !IsSubtreeState(vnode->node->par)) ||
      vnode->node->state == State::x_vertex ||
      vnode->node->state == State::p_vertex ||
      vnode->node->state == State::g_vertex) {
    num->setLegendIconSize(QSize(kLegSz, kLegSz));
    num->setTitle(Text::LegendByState(vnode->node->state).c_str());
    num->setItemAttribute(QwtPlotItem::Legend, true);
  }

  num->attach(MW_->GetPlot());
}

QwtSymbol *View::GetSymbol(PVNode vnode) {
  QwtSymbol *sym = new QwtSymbol{QwtSymbol::Style::Ellipse};
  int diam = ReadyTree::kRadius * 2;
  sym->setSize((diam * 4) * scale_, (diam * 4) * scale_);
  if (!MW_->ui->animationOff->isChecked()) {
    sym->setColor(Palette::GetColor(vnode->node->state));
  } else {
    sym->setColor(Palette::kDefaultColor);
  }
  return sym;
}

// пропихиваем state в ребенка чтобы закрасить все поддерево одним цветом
void View::PushState(PVNode vnode) {
  if (vnode->left && IsSubtreeState(vnode->node)) {
    vnode->left->node->state = vnode->node->state;
  }
  if (vnode->right && IsSubtreeState(vnode->node)) {
    vnode->right->node->state = vnode->node->state;
  }
}

bool View::IsSubtreeState(PNode node) {
  if (!node) {
    return false;
  }
  if (node->state == State::a_subtree || node->state == State::b_subtree ||
      node->state == State::c_subtree || node->state == State::d_subtree) {
    return true;
  }
  return false;
}

void View::Delay(double sWait) {
  stopped_ = false;
  QEventLoop loop;
  timer_.connect(&timer_, &QTimer::timeout, &loop, &QEventLoop::quit);
  timer_.start(sWait * 1000);
  MW_->ui->pauseButton->setEnabled(true);
  loop.exec();
  MW_->ui->pauseButton->setEnabled(false);
}

void View::SetEnabledWidgets(bool flag) {
  MW_->ui->vertexId->setEnabled(flag);
  MW_->ui->maintreeId->setEnabled(flag);
  MW_->ui->lefttreeId->setEnabled(flag);
  MW_->ui->righttreeId->setEnabled(flag);
  MW_->ui->insertButton->setEnabled(flag);
  MW_->ui->removeButton->setEnabled(flag);
  MW_->ui->findButton->setEnabled(flag);
  MW_->ui->splitButton->setEnabled(flag);
  MW_->ui->mergeButton->setEnabled(flag);
  MW_->ui->deltreeButton->setEnabled(flag);
  MW_->ui->animationOff->setEnabled(flag);
}

QString View::GetText(QComboBox *ptr) {
  if (ptr) {
    return ptr->currentText();
  }
  return "";
}

void View::ClearBox(QComboBox *ptr) {
  if (ptr) {
    ptr->clear();
  }
}

void View::InsertItem(QComboBox *ptr, int num) {
  if (ptr) {
    ptr->addItem(std::to_string(num).c_str());
  }
}

void View::UpdIndex(QComboBox *ptr, const QString &str) {
  int index = ptr->findText(str);
  if (index != -1) {
    ptr->setCurrentIndex(index);
  } else {
    ptr->setCurrentIndex(0);
  }
}

} // namespace DSViz
