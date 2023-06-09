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
      panner_{std::make_unique<CustomPanner>(MW_->Plot()->canvas())},
      port_in_{GetCallback()} {
  ConnectWidgets();
  ConfigureWidgets();
  MW_->show();
  // как и в конструкторе model, оно только запишет во внутренние поля
  // Observable это и не будет отправлять потому что еще никто не подписан
  port_out_.Set(UserQuery{QueryType::do_nothing, {0, 0}});
}

void View::SubscribeToUserInput(Observer<UserQuery> *controller_observer) {
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
    port_out_.Set(UserQuery{QueryType::merge, {left_tree_id_, right_tree_id_}});
    merge_executing_ = false;
    SetEnabledWidgets(true);
    main_tree_id_ = left_tree_id_;
    int ind = MW_->ui->maintreeId->findText(QString::number(main_tree_id_));
    MW_->ui->maintreeId->setCurrentIndex(ind);
    return;
  }

  bool ver_correct{};
  int id = main_tree_id_;
  int ver = MW_->ui->vertexId->text().toInt(&ver_correct);

  if (sender() == MW_->ui->deltreeButton) {
    port_out_.Set(UserQuery{QueryType::deltree, {id, 0}});

  } else if (ver_correct) {
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
  MW_->Plot()->setAxisScale(QwtPlot::xBottom, -kBound / scale_,
                            kBound / scale_);
  MW_->Plot()->setAxisScale(QwtPlot::yLeft, -kBound / scale_, kBound / scale_);
  Draw();
  panner_->moveCanvas(x_, y_);
}

void View::OnChoiceChange(QString num) {
  main_tree_id_ = num.toInt();
  Prepare();
  Draw();
}

void View::OnMergeChoiceChange(QString num) {
  if (sender() == MW_->ui->lefttreeId) {
    left_tree_id_ = num.toInt();
  } else {
    right_tree_id_ = num.toInt();
  }
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
  ConnectComboBoxes();
  QObject::connect(panner_.get(), SIGNAL(panned(int, int)), this,
                   SLOT(OnPanned(int, int)));
}

void View::ConfigureWidgets() {
  MW_->Plot()->setCanvasBackground(Qt::white);
  MW_->Plot()->setAxisScale(QwtPlot::xBottom, -kBound, kBound);
  MW_->Plot()->setAxisScale(QwtPlot::yLeft, -kBound, kBound);
  MW_->Plot()->enableAxis(0, false);
  MW_->Plot()->enableAxis(2, false);
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
  trees_ = &trees;
  UpdateComboBox();
  SetStatus(code);
  Prepare();
  Draw();
  if (DoDelay(code)) {
    Delay(MW_->ui->delayTime->value());
  }
}

void View::UpdateTreeId(int &tree_id) {
  // если дерева с номером tree_id уже не существует, то отрисовываю первое
  // попавшееся
  if (trees_->find(tree_id) == trees_->end()) {
    tree_id = trees_->begin()->first;
  }
}

void View::ConnectComboBoxes() {
  QObject::connect(MW_->ui->maintreeId, SIGNAL(currentTextChanged(QString)),
                   this, SLOT(OnChoiceChange(QString)));
  QObject::connect(MW_->ui->lefttreeId, SIGNAL(currentTextChanged(QString)),
                   this, SLOT(OnMergeChoiceChange(QString)));
  QObject::connect(MW_->ui->righttreeId, SIGNAL(currentTextChanged(QString)),
                   this, SLOT(OnMergeChoiceChange(QString)));
}

void View::DisconnectComboBoxes() {
  QObject::disconnect(MW_->ui->maintreeId, SIGNAL(currentTextChanged(QString)),
                      this, SLOT(OnChoiceChange(QString)));
  QObject::disconnect(MW_->ui->lefttreeId, SIGNAL(currentTextChanged(QString)),
                      this, SLOT(OnMergeChoiceChange(QString)));
  QObject::disconnect(MW_->ui->righttreeId, SIGNAL(currentTextChanged(QString)),
                      this, SLOT(OnMergeChoiceChange(QString)));
}

void View::UpdateComboBox() {
  QComboBox *maintree_combobox = MW_->ui->maintreeId,
            *lefttree_combobox = MW_->ui->lefttreeId,
            *righttree_combobox = MW_->ui->righttreeId;
  DisconnectComboBoxes();
  ClearBox(maintree_combobox);
  ClearBox(lefttree_combobox);
  ClearBox(righttree_combobox);
  for (auto &[num, tree] : *trees_) {
    InsertItem(maintree_combobox, num);
    InsertItem(lefttree_combobox, num);
    InsertItem(righttree_combobox, num);
  }
  UpdateTreeId(main_tree_id_);
  UpdateTreeId(left_tree_id_);
  UpdateTreeId(right_tree_id_);
  ConnectComboBoxes();
  UpdComboBoxText(maintree_combobox, main_tree_id_);
  UpdComboBoxText(lefttree_combobox, left_tree_id_);
  UpdComboBoxText(righttree_combobox, right_tree_id_);
}

void View::SetStatus(MsgCode code) {
  std::string msg;
  if (code == MsgCode::split_succ) {
    msg = "The left tree ID is " + std::to_string(main_tree_id_) +
          ". The right is " + QString::number(next_id_).toStdString();
    ++next_id_;
  } else if (code == MsgCode::merge_end) {
    msg = Text::GetMsg(MsgCode::merge_end) + std::to_string(left_tree_id_);
  } else {
    msg = Text::GetMsg(code);
  }
  MW_->ui->statusbar->showMessage(msg.c_str());
}

void View::Prepare() {
  if (!merge_executing_) {
    cur_tree_.Fill(trees_->at(main_tree_id_));
  } else {
    // если я выполняю мерж, то я всегда к левому дереву приливаю правое, так
    // что беру left_tree_id_
    cur_tree_.Fill(trees_->at(left_tree_id_));
  }
}

void View::Draw() {
  auto plot = MW_->Plot();

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

  num->attach(MW_->Plot());
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
  timer_.start(sWait * kSecondsPerMinute);
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

void View::UpdComboBoxText(QComboBox *ptr, int cur_id) {
  // вообще findText может вернуть -1, если не нашел такой строки
  // но я гарантирую, что существует дерево с номером cur_id
  ptr->setCurrentIndex(ptr->findText(QString::number(cur_id)));
}

} // namespace DSViz
