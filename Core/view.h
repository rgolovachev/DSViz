#ifndef VIEW_H
#define VIEW_H
#include "App/mainwindow.h"
#include "Common/node.h"
#include "Common/query.h"
#include "Core/vnode.h"
#include "Observer/observer.h"
#include "ui_mainwindow.h"
#include <QComboBox>
#include <QMessageBox>
#include <QMouseEvent>
#include <QObject>
#include <QTime>
#include <QTimer>
#include <map>
#include <qwt_legend.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_legenditem.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_panner.h>
#include <qwt_symbol.h>
#include <qwt_text.h>
#include <type_traits>

namespace DSViz {

constexpr const char *kFont = "Monaco";
constexpr const char *kErrMsg = "Номер дерева и/или вершины - не число";
constexpr const int kFontSz = 18;
constexpr const int kLegSz = 10;
constexpr const int kBound = 40;
constexpr const double kSliderBegin = 1.0;
constexpr const double kSliderLowerBound = 0.2;
constexpr const double kSliderUpperBound = 2.0;

inline std::map<State, QColor> StateToClr = {
    {State::ON_PATH, QColor::fromRgb(255, 255, 102)},
    {State::FOUND, QColor::fromRgb(0, 153, 0)},
    {State::NOT_FOUND, QColor::fromRgb(204, 0, 0)},
    {State::X_VERTEX, QColor::fromRgb(179, 0, 179)},
    {State::P_VERTEX, QColor::fromRgb(255, 26, 255)},
    {State::G_VERTEX, QColor::fromRgb(255, 153, 255)},
    {State::A_SUBTREE, QColor::fromRgb(255, 179, 102)},
    {State::B_SUBTREE, QColor::fromRgb(102, 255, 140)},
    {State::C_SUBTREE, QColor::fromRgb(102, 140, 255)},
    {State::D_SUBTREE, QColor::fromRgb(255, 102, 102)},
    {State::SPLAY_VER, QColor::fromRgb(204, 0, 153)},
    {State::DONT_REM, QColor::fromRgb(255, 0, 0)},
    {State::DO_REMOVE, QColor::fromRgb(0, 255, 0)},
    {State::INSERTED, QColor::fromRgb(0, 255, 255)},
    {State::NEW_ROOT, QColor::fromRgb(255, 215, 0)}};

namespace detail {

class CustomPanner : public QwtPlotPanner {

public:
  explicit CustomPanner(QWidget *parent);

  virtual bool eventFilter(QObject *object, QEvent *event);
};

// такой танец с бубном нужен потому что я не могу создать шаблонный класс и там
// определить макрос Q_OBJECT
class BaseView : public QObject {
  Q_OBJECT
public slots:
  virtual void OnPauseOrStop() = 0;
  virtual void OnButtonClick() = 0;
  virtual void OnZoom(double value) = 0;
  virtual void OnPanned(int dx, int dy) = 0;
  virtual void OnChoiceChange(QString num) = 0;
};

template <typename T> class ReadyTree {
public:
  ~ReadyTree();

  void Set(PVNode<T> root);

  PVNode<T> Get();

private:
  void Destroy(PVNode<T> root);

  PVNode<T> tree_ = {};
};

template <typename T> ReadyTree<T>::~ReadyTree() { Destroy(tree_); }

template <typename T> void ReadyTree<T>::Set(PVNode<T> root) {
  Destroy(tree_);
  tree_ = root;
}

template <typename T> PVNode<T> ReadyTree<T>::Get() { return tree_; }

template <typename T> void ReadyTree<T>::Destroy(PVNode<T> root) {
  if (!root) {
    return;
  }
  Destroy(root->left);
  Destroy(root->right);
  delete root;
}

} // namespace detail

template <typename T> class View : public detail::BaseView {
  using ReadyTree = detail::ReadyTree<T>;
  using CustomPanner = detail::CustomPanner;

public:
  View(IObservable *observable);

  IObservable *GetPort();

  void OnPanned(int dx, int dy) override;
  void OnPauseOrStop() override;
  void OnButtonClick() override;
  void OnZoom(double value) override;
  void OnChoiceChange(QString num) override;

private:
  void ConnectWidgets();
  void ConfigureWidgets();
  void SetCallback(IObservable *observable);

  QString GetText(QComboBox *ptr);
  void ClearBox(QComboBox *ptr);
  void InsertItem(QComboBox *ptr, int num);
  void UpdIndex(QComboBox *ptr, const QString &str);
  void UpdateComboBox();

  void SetStatus(const char *msg);
  void Prepare();

  void Draw();
  void DrawTwoTrees(QwtPlot *plot, PVNode<T> left, PVNode<T> right);
  void DrawOneTree(QwtPlot *plot);
  void AddPoints(QPolygonF &points, PVNode<T> vnode);
  void AttachVertex(PVNode<T> vnode, QwtSymbol *sym);

  QColor GetColor(State state);
  QwtSymbol *GetSymbol(PVNode<T> vnode);
  std::string LegendByState(State state);
  void PushState(PVNode<T> vnode);
  bool IsSubtreeState(PNode<T> node);
  void Delay(double sWait);
  void SetEnabledWidgets(bool flag);
  void ConvertString(T &ver, bool &ver_correct);

  // data
  ReadyTree cur_tree_ = {};
  BareTrees<T> trees_;
  std::unique_ptr<MainWindow> MW_;
  std::unique_ptr<CustomPanner> panner_;
  QTimer timer_;
  double scale_ = 1;
  bool stopped_ = {};
  bool merge_executing_ = {};
  int x_ = {};
  int y_ = {};

  std::unique_ptr<IObserver> port_in_;
  std::unique_ptr<IObservable> port_out_;
};

template <typename T>
View<T>::View(IObservable *observable)
    : MW_{std::make_unique<MainWindow>()},
      panner_{std::make_unique<CustomPanner>(MW_->ui->qwt_plot->canvas())},
      port_out_{std::make_unique<Observable>()} {
  ConnectWidgets();
  ConfigureWidgets();
  SetCallback(observable);
  MW_->show();
}

template <typename T> IObservable *View<T>::GetPort() {
  return port_out_.get();
}

template <typename T> void View<T>::OnPanned(int dx, int dy) {
  x_ += dx;
  y_ += dy;
}

template <typename T> void View<T>::OnPauseOrStop() {
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

template <typename T> void View<T>::OnButtonClick() {
  if (sender() == MW_->ui->mergeButton) {
    SetEnabledWidgets(false);
    merge_executing_ = true;
    int left_id = MW_->ui->lefttreeId->currentText().toInt(),
        right_id = MW_->ui->righttreeId->currentText().toInt();
    port_out_->Set(UserQuery<T>{QueryType::MERGE, {left_id, right_id}});
    merge_executing_ = false;
    SetEnabledWidgets(true);
    MW_->ui->maintreeId->setCurrentText(MW_->ui->lefttreeId->currentText());
    return;
  }

  bool ver_correct{}, id_correct{};
  int id = MW_->ui->maintreeId->currentText().toInt(&id_correct);
  T ver{};
  ConvertString(ver, ver_correct);

  if (id_correct && (sender() == MW_->ui->deltreeButton)) {
    port_out_->Set(UserQuery<T>{QueryType::DELTREE, {id, 0}});

  } else if (id_correct && ver_correct) {
    SetEnabledWidgets(false);
    if (sender() == MW_->ui->insertButton) {
      port_out_->Set(UserQuery<T>{QueryType::INSERT, {id, ver}});
    } else if (sender() == MW_->ui->removeButton) {
      port_out_->Set(UserQuery<T>{QueryType::REMOVE, {id, ver}});
    } else if (sender() == MW_->ui->findButton) {
      port_out_->Set(UserQuery<T>{QueryType::FIND, {id, ver}});
    } else if (sender() == MW_->ui->splitButton) {
      port_out_->Set(UserQuery<T>{QueryType::SPLIT, {id, ver}});
    }
    SetEnabledWidgets(true);
  } else {
    QMessageBox::warning(NULL, QObject::tr("Ошибка"), QObject::tr(kErrMsg));
  }
}

template <typename T> void View<T>::OnZoom(double value) {
  scale_ = value;
  MW_->ui->qwt_plot->setAxisScale(QwtPlot::xBottom, -kBound / scale_,
                                  kBound / scale_);
  MW_->ui->qwt_plot->setAxisScale(QwtPlot::yLeft, -kBound / scale_,
                                  kBound / scale_);
  Draw();
  panner_->moveCanvas(x_, y_);
}

template <typename T> void View<T>::OnChoiceChange(QString num) {
  Prepare();
  Draw();
}

template <typename T> void View<T>::ConnectWidgets() {
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

template <typename T> void View<T>::ConfigureWidgets() {
  MW_->ui->qwt_plot->setCanvasBackground(Qt::white);
  MW_->ui->qwt_plot->setAxisScale(QwtPlot::xBottom, -kBound, kBound);
  MW_->ui->qwt_plot->setAxisScale(QwtPlot::yLeft, -kBound, kBound);
  MW_->ui->maintreeId->addItem("0");
  MW_->ui->lefttreeId->addItem("0");
  MW_->ui->righttreeId->addItem("0");
  MW_->ui->qwt_plot->enableAxis(0, false);
  MW_->ui->qwt_plot->enableAxis(2, false);
  MW_->ui->qwt_slider->setValue(kSliderBegin);
  MW_->ui->qwt_slider->setScale(kSliderLowerBound, kSliderUpperBound);
  panner_->setMouseButton(Qt::LeftButton);
}

template <typename T> void View<T>::SetCallback(IObservable *observable) {
  auto callback = [this](const std::any &msg) {
    auto str_msg = std::any_cast<MsgType<T>>(msg).first;
    trees_ = std::any_cast<MsgType<T>>(msg).second;
    this->UpdateComboBox();
    this->SetStatus(str_msg);
    this->Prepare();
    this->Draw();
    if (!this->MW_->ui->animationOff->isChecked() &&
        (std::strcmp(str_msg, StrMsg[MsgCode::SUCC_DEL]) != 0) &&
        (std::strcmp(str_msg, StrMsg[MsgCode::UNSUCC_DEL]) != 0)) {
      this->Delay(this->MW_->ui->delayTime->value());
    }
  };
  port_in_ = std::move(std::make_unique<Observer>(observable, callback));
}

template <typename T> QString View<T>::GetText(QComboBox *ptr) {
  if (ptr) {
    return ptr->currentText();
  }
  return "";
}

template <typename T> void View<T>::ClearBox(QComboBox *ptr) {
  if (ptr) {
    ptr->clear();
  }
}

template <typename T> void View<T>::InsertItem(QComboBox *ptr, int num) {
  if (ptr) {
    ptr->addItem(std::to_string(num).c_str());
  }
}

template <typename T>
void View<T>::UpdIndex(QComboBox *ptr, const QString &str) {
  int index = ptr->findText(str);
  if (index != -1) {
    ptr->setCurrentIndex(index);
  } else {
    ptr->setCurrentIndex(0);
  }
}

template <typename T> void View<T>::UpdateComboBox() {
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

template <typename T> void View<T>::SetStatus(const char *msg) {
  MW_->ui->statusbar->showMessage(msg);
}

template <typename T> void View<T>::Prepare() {
  if (!merge_executing_) {
    cur_tree_.Set(Fill(trees_[MW_->ui->maintreeId->currentText().toInt()]));
  } else {
    cur_tree_.Set(Fill(trees_[MW_->ui->lefttreeId->currentText().toInt()]));
  }
}

template <typename T> void View<T>::Draw() {
  auto plot = MW_->ui->qwt_plot;

  plot->detachItems();

  QwtPlotLegendItem *leg = new QwtPlotLegendItem{};
  leg->attach(plot);
  auto cur_tree = cur_tree_.Get();
  if (cur_tree) {
    if (cur_tree->node->state == State::HIDE_THIS) {
      DrawTwoTrees(plot, cur_tree->left, cur_tree->right);
    } else if (cur_tree->node->state == State::SPLIT_LEFT) {
      DrawTwoTrees(plot, cur_tree->left, cur_tree);
    } else if (cur_tree->node->state == State::SPLIT_RIGHT) {
      DrawTwoTrees(plot, cur_tree, cur_tree->right);
    } else {
      DrawOneTree(plot);
    }
  }

  plot->replot();
}

template <typename T>
void View<T>::DrawTwoTrees(QwtPlot *plot, PVNode<T> left, PVNode<T> right) {
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

template <typename T> void View<T>::DrawOneTree(QwtPlot *plot) {
  QwtPlotCurve *cur = new QwtPlotCurve{};
  cur->setItemAttribute(QwtPlotItem::Legend, false);
  QPolygonF points;
  AddPoints(points, cur_tree_.Get());

  cur->setSamples(points);
  cur->attach(plot);
}

template <typename T>
void View<T>::AddPoints(QPolygonF &points, PVNode<T> vnode) {
  if (!vnode) {
    return;
  }

  points << QPointF(vnode->x, vnode->y);
  PushState(vnode);
  if (vnode->node->state != State::SPLIT_LEFT) {
    AddPoints(points, vnode->left);
    points << QPointF(vnode->x, vnode->y);
  }
  if (vnode->node->state != State::SPLIT_RIGHT) {
    AddPoints(points, vnode->right);
    points << QPointF(vnode->x, vnode->y);
  }

  AttachVertex(vnode, GetSymbol(vnode));
}

template <typename T>
void View<T>::AttachVertex(PVNode<T> vnode, QwtSymbol *sym) {
  QwtPlotMarker *num = new QwtPlotMarker{};

  num->setValue(vnode->x, vnode->y);
  num->setSymbol(sym);
  QwtText txt = QwtText(QString(std::to_string(vnode->node->value).c_str()));
  int font_sz = kFontSz * scale_;
  QFont font{QString(kFont), font_sz};
  txt.setFont(font);
  num->setLabel(txt);

  if ((IsSubtreeState(vnode->node) && !IsSubtreeState(vnode->node->par)) ||
      vnode->node->state == State::X_VERTEX ||
      vnode->node->state == State::P_VERTEX ||
      vnode->node->state == State::G_VERTEX) {
    num->setLegendIconSize(QSize(kLegSz, kLegSz));
    num->setTitle(LegendByState(vnode->node->state).c_str());
    num->setItemAttribute(QwtPlotItem::Legend, true);
  }

  num->attach(MW_->ui->qwt_plot);
}

template <typename T> QColor View<T>::GetColor(State state) {
  if (StateToClr.find(state) != StateToClr.end()) {
    return StateToClr[state];
  }
  return QColorConstants::Gray;
}

template <typename T> QwtSymbol *View<T>::GetSymbol(PVNode<T> vnode) {
  QwtSymbol *sym = new QwtSymbol{QwtSymbol::Style::Ellipse};
  int diam = kRadius * 2;
  sym->setSize((diam * 4) * scale_, (diam * 4) * scale_);
  if (!MW_->ui->animationOff->isChecked()) {
    sym->setColor(GetColor(vnode->node->state));
  } else {
    sym->setColor(QColorConstants::Gray);
  }
  return sym;
}

template <typename T> std::string View<T>::LegendByState(State state) {
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

// пропихиваем state в ребенка чтобы закрасить все поддерево одним цветом
template <typename T> void View<T>::PushState(PVNode<T> vnode) {
  if (vnode->left && IsSubtreeState(vnode->node)) {
    vnode->left->node->state = vnode->node->state;
  }
  if (vnode->right && IsSubtreeState(vnode->node)) {
    vnode->right->node->state = vnode->node->state;
  }
}

template <typename T> bool View<T>::IsSubtreeState(PNode<T> node) {
  if (!node) {
    return false;
  }
  if (node->state == State::A_SUBTREE || node->state == State::B_SUBTREE ||
      node->state == State::C_SUBTREE || node->state == State::D_SUBTREE) {
    return true;
  }
  return false;
}

template <typename T> void View<T>::Delay(double sWait) {
  stopped_ = false;
  QEventLoop loop;
  timer_.connect(&timer_, &QTimer::timeout, &loop, &QEventLoop::quit);
  timer_.start(sWait * 1000);
  MW_->ui->pauseButton->setEnabled(true);
  loop.exec();
  MW_->ui->pauseButton->setEnabled(false);
}

template <typename T> void View<T>::SetEnabledWidgets(bool flag) {
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

template <typename T> void View<T>::ConvertString(T &ver, bool &ver_correct) {
  auto vertexId = MW_->ui->vertexId;
  if (std::is_same_v<T, int>) {
    ver = vertexId->text().toInt(&ver_correct);
  } else if (std::is_same_v<T, double>) {
    ver = vertexId->text().toDouble(&ver_correct);
  } else if (std::is_same_v<T, float>) {
    ver = vertexId->text().toFloat(&ver_correct);
  } else if (std::is_same_v<T, unsigned>) {
    ver = vertexId->text().toUInt(&ver_correct);
  } else if (std::is_same_v<T, unsigned long>) {
    ver = vertexId->text().toULong(&ver_correct);
  } else if (std::is_same_v<T, unsigned long long>) {
    ver = vertexId->text().toULongLong(&ver_correct);
  } else if (std::is_same_v<T, long>) {
    ver = vertexId->text().toLong(&ver_correct);
  } else if (std::is_same_v<T, long long>) {
    ver = vertexId->text().toLongLong(&ver_correct);
  }
}

} // namespace DSViz
#endif // VIEW_H
