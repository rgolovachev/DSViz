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
  MERGE_END
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

inline std::map<MsgCode, const char *> StrMsg = {
    {MsgCode::OK, "OK"},
    {MsgCode::WRONG_ID, "ERROR: There is no tree with this ID"},
    {MsgCode::INSERT_ERR, "ERROR: This key already exists in the tree"},
    {MsgCode::REMOVE_ERR, "ERROR: This key doesn't exist in the tree"},
    {MsgCode::MERGE_ERR,
     "ERROR: The maximum value of the first tree must be lower "
     "than the minimum value of the second one"},
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
    {MsgCode::MERGE_END, "Merge has been executed. The new root is "}};

template <typename T> struct Node;

template <typename T> using PNode = Node<T> *;

template <typename T> using BareTrees = std::map<int, PNode<T>>;

template <typename T> using MsgType = std::pair<const char *, BareTrees<T>>;

template <typename T> struct Node {
  PNode<T> par, left, right;
  T value, min, max;
  State state = State::REGULAR;
};

} // namespace DSViz
#endif // NODE_H
