#ifndef VIEW_H
#define VIEW_H
#include "App/mainwindow.h"
#include "Common/node.h"
#include "Common/query.h"
#include "Core/vnode.h"
#include "Observer/observer.h"
#include <QComboBox>
#include <QTimer>
#include <qwt_plot.h>
#include <qwt_plot_panner.h>
#include <qwt_symbol.h>

namespace DSViz {

namespace detail {

class Palette {
public:
  static QColor GetColor(State state);

  constexpr static const QColor kDefaultColor = QColorConstants::Gray;

private:
  // constexpr сделать не выйдет, у std::map нет constexpr конструктора
  inline static const std::map<State, QColor> StateToClr = {
      {State::on_path, QColor::fromRgb(255, 255, 102)},
      {State::found, QColor::fromRgb(0, 153, 0)},
      {State::not_found, QColor::fromRgb(204, 0, 0)},
      {State::x_vertex, QColor::fromRgb(179, 0, 179)},
      {State::p_vertex, QColor::fromRgb(255, 26, 255)},
      {State::g_vertex, QColor::fromRgb(255, 153, 255)},
      {State::a_subtree, QColor::fromRgb(255, 179, 102)},
      {State::b_subtree, QColor::fromRgb(102, 255, 140)},
      {State::c_subtree, QColor::fromRgb(102, 140, 255)},
      {State::d_subtree, QColor::fromRgb(255, 102, 102)},
      {State::splay_ver, QColor::fromRgb(204, 0, 153)},
      {State::dont_rem, QColor::fromRgb(255, 0, 0)},
      {State::do_remove, QColor::fromRgb(0, 255, 0)},
      {State::inserted, QColor::fromRgb(0, 255, 255)},
      {State::new_root, QColor::fromRgb(255, 215, 0)}};
};

class Text {
public:
  static std::string GetMsg(MsgCode code);

  static std::string LegendByState(State state);

private:
  inline static const std::map<MsgCode, const char *> StrMsg = {
      {MsgCode::OK, "OK"},
      {MsgCode::wrong_id, "ERROR: There is no tree with this ID"},
      {MsgCode::insert_err, "ERROR: This key already exists in the tree"},
      {MsgCode::remove_err, "ERROR: This key doesn't exist in the tree"},
      {MsgCode::merge_err,
       "ERROR: The maximum value of the first tree must be lower "
       "than the minimum value of the second one"},
      {MsgCode::split_err, "The target tree is empty"},
      {MsgCode::search, "Program is searching for vertex"},
      {MsgCode::not_found, "The value hasn't found"},
      {MsgCode::found, "The value has found"},
      {MsgCode::unsucc_del, "You must leave at least one tree"},
      {MsgCode::succ_del, "The tree was deleted successfuly"},
      {MsgCode::split_perf, "Split is performing"},
      {MsgCode::splay_perf, "Splay is performing"},
      {MsgCode::zig_perf, "Zig is performing"},
      {MsgCode::zig_end, "Zig has been executed"},
      {MsgCode::zigzag_perf, "Zig-zag is performing"},
      {MsgCode::zigzag_end, "Zig-zag has been executed"},
      {MsgCode::zigzig_perf, "Zig-zig is performing"},
      {MsgCode::zigzig_end, "Zig-zig has been executed"},
      {MsgCode::merge_perf, "Merge is performing"},
      {MsgCode::r_search, "Search for the most right vertex..."},
      {MsgCode::r_found,
       "The most right vertex in the left subtree has been found"},
      {MsgCode::do_rem, "Remove will be executed"},
      {MsgCode::dont_rem, "Remove won't be executed: vertex hasn't found"},
      {MsgCode::ins_done, "Insertion has been done"},
      {MsgCode::new_root, "There is a new root"},
      {MsgCode::merge_equal,
       "ID of the left tree must be != ID of the right one"},
      {MsgCode::merge_empty, "Both trees must not be empty"},
      {MsgCode::merge_end, "Merge has been executed. The new root is "},
      {MsgCode::empty_msg, ""}};
};

class CustomPanner : public QwtPlotPanner {

public:
  explicit CustomPanner(QWidget *parent);

  virtual bool eventFilter(QObject *object, QEvent *event);
};

} // namespace detail

class View : public QObject {
  Q_OBJECT

  using ReadyTree = detail::ReadyTree;
  using CustomPanner = detail::CustomPanner;
  using Palette = detail::Palette;
  using Text = detail::Text;
  using PVNode = VNode<int> *;
  using PNode = Node<int> *;
  using BareTrees = std::map<int, PNode>;
  using MsgType = std::pair<MsgCode, BareTrees>;
  using UserQuery = UserQuery<int>;

  auto GetCallback();

public:
  View();

  void SubscribeToController(Observer<UserQuery> *controller_observer);
  Observer<MsgType> *GetPortIn();

  static constexpr const int kSecondsPerMinute = 1000;
  static constexpr const char *kFont = "Monaco";
  static constexpr const char *kErrMsg =
      "Номер дерева и/или вершины - не число";
  static constexpr const int kFontSz = 18;
  static constexpr const int kLegSz = 10;
  static constexpr const int kBound = 40;
  static constexpr const double kSliderBegin = 1.0;
  static constexpr const double kSliderLowerBound = 0.2;
  static constexpr const double kSliderUpperBound = 2.0;

public slots:
  void OnPanned(int dx, int dy);
  void OnPauseOrStop();
  void OnButtonClick();
  void OnZoom(double value);
  void OnChoiceChange(QString num);

private:
  void ConnectWidgets();
  void ConfigureWidgets();
  bool DoDelay(MsgCode code);
  void HandleMsg(MsgCode code, const BareTrees &trees);

  void UpdateComboBox();

  void SetStatus(MsgCode code);
  void Prepare();

  void Draw();
  void DrawTwoTrees(QwtPlot *plot, PVNode left, PVNode right);
  void DrawOneTree(QwtPlot *plot);
  void AddPoints(QPolygonF &points, PVNode vnode);
  void AttachVertex(PVNode vnode, QwtSymbol *sym);

  QwtSymbol *GetSymbol(PVNode vnode);
  void PushState(PVNode vnode);
  bool IsSubtreeState(PNode node);
  void Delay(double sWait);
  void SetEnabledWidgets(bool flag);

  static QString GetText(QComboBox *ptr);

  static void ClearBox(QComboBox *ptr);

  static void InsertItem(QComboBox *ptr, int num);

  static void UpdIndex(QComboBox *ptr, const QString &str);

  // data
  ReadyTree cur_tree_ = {};
  BareTrees trees_;
  std::unique_ptr<MainWindow> MW_;
  std::unique_ptr<CustomPanner> panner_;
  QTimer timer_;
  double scale_ = 1;
  bool stopped_ = {};
  bool merge_executing_ = {};
  int x_ = {};
  int y_ = {};
  int next_id_ = 1;

  Observer<MsgType> port_in_;
  Observable<UserQuery> port_out_;
};

} // namespace DSViz
#endif // VIEW_H
