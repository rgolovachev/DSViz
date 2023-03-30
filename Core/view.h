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

namespace detail {

class Palette {
public:
  static QColor GetColor(State state);

private:
  // constexpr сделать не выйдет, у std::map нет constexpr конструктора
  inline static const std::map<State, QColor> StateToClr = {
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
};

class Text {
public:
  static std::string GetMsg(MsgCode code);

  static QString GetText(QComboBox *ptr);

  static void ClearBox(QComboBox *ptr);

  static void InsertItem(QComboBox *ptr, int num);

  static void UpdIndex(QComboBox *ptr, const QString &str);

  static std::string LegendByState(State state);

private:
  inline static const std::map<MsgCode, const char *> StrMsg = {
      {MsgCode::OK, "OK"},
      {MsgCode::WRONG_ID, "ERROR: There is no tree with this ID"},
      {MsgCode::INSERT_ERR, "ERROR: This key already exists in the tree"},
      {MsgCode::REMOVE_ERR, "ERROR: This key doesn't exist in the tree"},
      {MsgCode::MERGE_ERR,
       "ERROR: The maximum value of the first tree must be lower "
       "than the minimum value of the second one"},
      {MsgCode::SPLIT_ERR, "The target tree is empty"},
      {MsgCode::SEARCH, "Program is searching for vertex"},
      {MsgCode::NOT_FOUND, "The value hasn't found"},
      {MsgCode::FOUND, "The value has found"},
      {MsgCode::UNSUCC_DEL, "You must leave at least one tree"},
      {MsgCode::SUCC_DEL, "The tree was deleted successfuly"},
      {MsgCode::SPLIT_PERF, "Split is performing"},
      {MsgCode::SPLAY_PERF, "Splay is performing"},
      {MsgCode::ZIG_PERF, "Zig is performing"},
      {MsgCode::ZIG_END, "Zig has been executed"},
      {MsgCode::ZIGZAG_PERF, "Zig-zag is performing"},
      {MsgCode::ZIGZAG_END, "Zig-zag has been executed"},
      {MsgCode::ZIGZIG_PERF, "Zig-zig is performing"},
      {MsgCode::ZIGZIG_END, "Zig-zig has been executed"},
      {MsgCode::MERGE_PERF, "Merge is performing"},
      {MsgCode::R_SEARCH, "Search for the most right vertex..."},
      {MsgCode::R_FOUND,
       "The most right vertex in the left subtree has been found"},
      {MsgCode::DO_REM, "Remove will be executed"},
      {MsgCode::DONT_REM, "Remove won't be executed: vertex hasn't found"},
      {MsgCode::INS_DONE, "Insertion has been done"},
      {MsgCode::NEW_ROOT, "There is a new root"},
      {MsgCode::MERGE_EQUAL,
       "ID of the left tree must be != ID of the right one"},
      {MsgCode::MERGE_EMPTY, "Both trees must not be empty"},
      {MsgCode::MERGE_END, "Merge has been executed. The new root is "},
      {MsgCode::EMPTY_MSG, ""}};
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
  using PVNode = PVNode<int>;
  using PNode = PNode<int>;
  using MsgType = MsgType<int>;
  using UserQuery = UserQuery<int>;

public:
  View();

  Observable<UserQuery> *GetPortOut();
  Observer<MsgType> *GetPortIn();

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
  auto GetCallback();
  void HandleMsg(MsgCode code, const BareTrees<int> &trees);

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

  // data
  ReadyTree cur_tree_ = {};
  BareTrees<int> trees_;
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
