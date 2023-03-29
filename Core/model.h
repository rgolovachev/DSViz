#ifndef MODEL_H
#define MODEL_H
#include "Common/node.h"
#include "Observer/observer.h"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <functional>
#include <initializer_list>
#include <map>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

namespace DSViz {

namespace detail {

class Trees {
public:
  ~Trees();

  void Destroy(int id);

  void Insert(std::pair<int, PNode<int>> tree);

  void Erase(int id);

  size_t Size();

  PNode<int> &operator[](int key);

  BareTrees<int> Get();

private:
  void destroy(PNode<int> root);

  BareTrees<int> trees_;
};

} // namespace detail

class Model {
  using Trees = detail::Trees;
  using PNode = PNode<int>;

public:
  Model();

  void Insert(int id, int key);

  void Remove(int id, int key);

  void Merge(int left_id, int right_id);

  void Split(int id, int key);

  void ExistKey(int id, int key);

  void DeleteTree(int id);

  IObservable *GetPort();

private:
  void update(PNode v);
  void rotate_left(PNode v);
  void rotate_right(PNode v);

  // что такое hidden_root стоит посмотреть перед определением функции merge.
  void splay(PNode v, PNode hidden_root = nullptr);
  void zig(PNode v, PNode hidden_root, bool is_right_zig);
  void zig_zig(PNode v, PNode hidden_root, bool is_right_zig_zig);
  void zig_zag(PNode v, PNode hidden_root, bool is_right_left);

  void set_state(PNode v, PNode A, PNode B, PNode C, PNode D = nullptr);
  void update_root(PNode old_root, PNode new_root);

  // find, insert, merge и т.д. я пишу не в camel case чтобы не было путаницы с
  // публичными методами
  PNode find(PNode v, int key);

  std::pair<PNode, PNode> split(PNode v, int key, bool *res = nullptr);

  PNode insert(PNode v, int key, bool *res);

  PNode merge(PNode hidden_root);

  PNode remove(PNode v, int key, bool *res = nullptr);

  void set_regular(PNode root, PNode prev = nullptr);

  static PNode make_hidden_root(PNode ltree, PNode rtree);

  Trees data_ = {};
  std::unique_ptr<IObservable> port_out_;
  int next_id_ = {};
};

} // namespace DSViz
#endif // MODEL_H
