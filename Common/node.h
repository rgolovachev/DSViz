#ifndef NODE_H
#define NODE_H
#include <map>

namespace DSViz {

enum class MsgCode {
  OK,
  wrong_id,
  insert_err,
  remove_err,
  merge_err,
  split_err,
  search,
  not_found,
  found,
  succ_del,
  unsucc_del,
  split_perf,
  splay_perf,
  zig_perf,
  zig_end,
  zigzag_perf,
  zigzag_end,
  zigzig_perf,
  zigzig_end,
  merge_perf,
  r_search,
  r_found,
  do_rem,
  dont_rem,
  ins_done,
  new_root,
  merge_equal,
  merge_empty,
  merge_end,
  split_succ,
  empty_msg
};

enum class State {
  regular,
  on_path,
  found,
  not_found,
  x_vertex,
  p_vertex,
  g_vertex,
  a_subtree,
  b_subtree,
  c_subtree,
  d_subtree,
  splay_ver,
  do_remove,
  dont_rem,
  hide_this,
  split_left,
  split_right,
  inserted,
  new_root
};

template <typename T> struct Node {
  Node<T> *par, *left, *right;
  T value, min, max;
  State state = State::regular;
};

} // namespace DSViz
#endif // NODE_H
