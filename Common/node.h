#ifndef NODE_H
#define NODE_H
#include <map>
#include <memory>
#include <vector>

namespace DSViz {

enum class MsgCode {
  OK,
  WRONG_ID,
  INSERT_ERR,
  REMOVE_ERR,
  MERGE_ERR,
  SPLIT_ERR,
  SEARCH,
  NOT_FOUND,
  FOUND,
  SUCC_DEL,
  UNSUCC_DEL,
  SPLIT_PERF,
  SPLAY_PERF,
  ZIG_PERF,
  ZIG_END,
  ZIGZAG_PERF,
  ZIGZAG_END,
  ZIGZIG_PERF,
  ZIGZIG_END,
  MERGE_PERF,
  R_SEARCH,
  R_FOUND,
  DO_REM,
  DONT_REM,
  INS_DONE,
  NEW_ROOT,
  MERGE_EQUAL,
  MERGE_EMPTY,
  MERGE_END,
  SPLIT_SUCC
};

enum class State {
  REGULAR,
  ON_PATH,
  FOUND,
  NOT_FOUND,
  X_VERTEX,
  P_VERTEX,
  G_VERTEX,
  A_SUBTREE,
  B_SUBTREE,
  C_SUBTREE,
  D_SUBTREE,
  SPLAY_VER,
  DO_REMOVE,
  DONT_REM,
  HIDE_THIS,
  SPLIT_LEFT,
  SPLIT_RIGHT,
  INSERTED,
  NEW_ROOT
};

template <typename T> struct Node;

template <typename T> using PNode = Node<T> *;

template <typename T> using BareTrees = std::map<int, PNode<T>>;

template <typename T> using MsgType = std::pair<MsgCode, BareTrees<T>>;

template <typename T> struct Node {
  PNode<T> par, left, right;
  T value, min, max;
  State state = State::REGULAR;
};

} // namespace DSViz
#endif // NODE_H
