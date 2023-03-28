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

template <typename T> class Trees {
public:
  ~Trees();

  void Destroy(int id);

  void Insert(std::pair<int, PNode<T>> tree);

  void Erase(int id);

  size_t Size();

  PNode<T> &operator[](int key);

  BareTrees<T> Get();

private:
  void destroy(PNode<T> root);

  BareTrees<T> trees_;
};

template <typename T> Trees<T>::~Trees() {
  for (auto tree : trees_) {
    destroy(tree.second);
  }
}

template <typename T> void Trees<T>::Destroy(int id) {
  if (trees_.find(id) != trees_.end()) {
    destroy(trees_[id]);
  }
}

template <typename T> void Trees<T>::Insert(std::pair<int, PNode<T>> tree) {
  trees_.insert(tree);
}

template <typename T> void Trees<T>::Erase(int id) {
  if (trees_.find(id) != trees_.end()) {
    trees_.erase(id);
  }
}

template <typename T> size_t Trees<T>::Size() { return trees_.size(); }

template <typename T> PNode<T> &Trees<T>::operator[](int key) {
  assert(trees_.find(key) != trees_.end());
  return trees_[key];
}

template <typename T> BareTrees<T> Trees<T>::Get() { return trees_; }

template <typename T> void Trees<T>::destroy(PNode<T> root) {
  if (!root) {
    return;
  }
  destroy(root->left);
  destroy(root->right);
  delete root;
}

} // namespace detail

template <typename T> class Model {
  using Trees = detail::Trees<T>;
  using PNode = PNode<T>;

public:
  Model();

  void Insert(int id, const T &key);

  void Remove(int id, const T &key);

  void Merge(int left_id, int right_id);

  void Split(int id, const T &key);

  void ExistKey(int id, const T &key);

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
  PNode find(PNode v, const T &key);

  std::pair<PNode, PNode> split(PNode v, const T &key, bool *res = nullptr);

  PNode insert(PNode v, const T &key, bool *res);

  PNode merge(PNode hidden_root);

  PNode remove(PNode v, const T &key, bool *res = nullptr);

  void set_regular(PNode root, PNode prev = nullptr);

  static PNode make_hidden_root(PNode ltree, PNode rtree);

  Trees data_ = {};
  std::unique_ptr<IObservable> port_out_;
  int next_id_ = {};
};

template <typename T>
Model<T>::Model() : port_out_{std::make_unique<Observable>()} {
  data_.Insert({next_id_++, nullptr});
}

template <typename T> void Model<T>::Insert(int id, const T &key) {
  bool flag = false;
  data_[id] = insert(data_[id], key, &flag);
  port_out_->Set(std::make_pair(MsgCode::INS_DONE, data_.Get()));
  if (!flag) {
    set_regular(data_[id]);
    port_out_->Set(std::make_pair(MsgCode::INSERT_ERR, data_.Get()));
    return;
  }

  set_regular(data_[id]);
  port_out_->Set(std::make_pair(MsgCode::OK, data_.Get()));
}

template <typename T> void Model<T>::Remove(int id, const T &key) {
  bool flag = false;
  data_[id] = remove(data_[id], key, &flag);
  if (!flag) {
    set_regular(data_[id]);
    port_out_->Set(std::make_pair(MsgCode::REMOVE_ERR, data_.Get()));
    return;
  }

  if (data_[id]) {
    data_[id]->state = State::NEW_ROOT;
    port_out_->Set(std::make_pair(MsgCode::NEW_ROOT, data_.Get()));
    data_[id]->state = State::REGULAR;
  }

  port_out_->Set(std::make_pair(MsgCode::OK, data_.Get()));
}

template <typename T> void Model<T>::Merge(int left_id, int right_id) {
  if (left_id == right_id) {
    port_out_->Set(std::make_pair(MsgCode::MERGE_EQUAL, data_.Get()));
    return;
  }
  if (!data_[left_id] || !data_[right_id]) {
    port_out_->Set(std::make_pair(MsgCode::MERGE_EMPTY, data_.Get()));
    return;
  }
  if (data_[left_id]->max < data_[right_id]->min) {
    auto ltree = data_[left_id], rtree = data_[right_id];
    auto hidden_root = make_hidden_root(ltree, rtree);
    update(hidden_root);
    rtree->par = hidden_root;
    data_[left_id] = hidden_root;
    data_[left_id] = merge(hidden_root);
    data_.Erase(right_id);
    data_[left_id]->state = State::NEW_ROOT;
    port_out_->Set(std::make_pair(MsgCode::MERGE_END, data_.Get()));
    data_[left_id]->state = State::REGULAR;
    port_out_->Set(std::make_pair(MsgCode::OK, data_.Get()));
  } else {
    port_out_->Set(std::make_pair(MsgCode::MERGE_ERR, data_.Get()));
  }
}

template <typename T> void Model<T>::Split(int id, const T &key) {
  if (!data_[id]) {
    port_out_->Set(std::make_pair(MsgCode::SPLIT_ERR, data_.Get()));
    return;
  }
  auto [ltree, rtree] = split(data_[id], key);
  if (data_[id]->state != State::HIDE_THIS) {
    if (ltree) {
      ltree->state = State::REGULAR;
    }
    if (rtree) {
      rtree->state = State::REGULAR;
    }
    data_[id] = make_hidden_root(ltree, rtree);
    update(data_[id]);
  }
  port_out_->Set(std::make_pair(MsgCode::SPLIT_SUCC, data_.Get()));
  delete data_[id];
  data_[id] = ltree;
  data_.Insert({next_id_++, rtree});
  port_out_->Set(std::make_pair(MsgCode::OK, data_.Get()));
}

template <typename T> void Model<T>::ExistKey(int id, const T &key) {
  data_[id] = find(data_[id], key);
  if (data_[id] && data_[id]->value == key) {
    set_regular(data_[id]);
    port_out_->Set(std::make_pair(MsgCode::FOUND, data_.Get()));
  } else {
    set_regular(data_[id]);
    port_out_->Set(std::make_pair(MsgCode::NOT_FOUND, data_.Get()));
  }
}

template <typename T> void Model<T>::DeleteTree(int id) {
  if (data_.Size() == 1) {
    port_out_->Set(std::make_pair(MsgCode::UNSUCC_DEL, data_.Get()));
    return;
  }
  data_.Destroy(id);
  data_.Erase(id);
  port_out_->Set(std::make_pair(MsgCode::SUCC_DEL, data_.Get()));
}

template <typename T> IObservable *Model<T>::GetPort() {
  return port_out_.get();
}

template <typename T> void Model<T>::update(PNode v) {
  if (!v) {
    return;
  }
  v->min = v->max = v->value;
  if (v->left) {
    v->min = std::min(v->min, v->left->min);
  }
  if (v->right) {
    v->max = std::max(v->max, v->right->max);
  }
}

template <typename T> void Model<T>::rotate_left(PNode v) {
  auto p = v->par;
  auto r = v->right;
  if (p) {
    if (p->left == v) {
      p->left = r;
    } else {
      p->right = r;
    }
  }
  auto tmp = r->left;
  r->left = v;
  v->right = tmp;
  v->par = r;
  r->par = p;
  if (v->right) {
    v->right->par = v;
  }
  update(v);
  update(r);
  update(p);
}

template <typename T> void Model<T>::rotate_right(PNode v) {
  auto p = v->par;
  auto r = v->left;

  if (p) {
    if (p->left == v) {
      p->left = r;
    } else {
      p->right = r;
    }
  }
  auto tmp = r->right;
  r->right = v;
  v->left = tmp;
  v->par = r;
  r->par = p;
  if (v->left) {
    v->left->par = v;
  }
  update(v);
  update(r);
  update(p);
}

template <typename T> void Model<T>::splay(PNode v, PNode hidden_root) {
  set_regular(v);
  v->state = State::SPLAY_VER;
  port_out_->Set(std::make_pair(MsgCode::SPLAY_PERF, data_.Get()));

  while (v->par) {
    if (v == v->par->left) {
      if (!v->par->par) {
        zig(v, hidden_root, true);

      } else if (v->par == v->par->par->left) {
        zig_zig(v, hidden_root, true);

      } else {
        zig_zag(v, hidden_root, true);
      }
    } else {
      if (!v->par->par) {
        zig(v, hidden_root, false);

      } else if (v->par == v->par->par->right) {
        zig_zig(v, hidden_root, false);

      } else {
        zig_zag(v, hidden_root, false);
      }
    }
  }

  set_regular(v);
  v->state = State::SPLAY_VER;
  port_out_->Set(std::make_pair(MsgCode::SPLAY_PERF, data_.Get()));
}

template <typename T>
void Model<T>::zig(PNode v, PNode hidden_root, bool is_right_zig) {
  if (is_right_zig) {
    set_state(v, v->left, v->right, v->par->right);
  } else {
    set_state(v, v->par->left, v->left, v->right);
  }
  port_out_->Set(std::make_pair(MsgCode::ZIG_PERF, data_.Get()));

  auto old_root = v->par;
  if (is_right_zig) {
    rotate_right(v->par);
  } else {
    rotate_left(v->par);
  }

  if (!hidden_root) {
    update_root(old_root, v);
  } else {
    hidden_root->left = v;
  }
  port_out_->Set(std::make_pair(MsgCode::ZIG_END, data_.Get()));
}

template <typename T>
void Model<T>::zig_zig(PNode v, PNode hidden_root, bool is_right_zig_zig) {
  if (is_right_zig_zig) {
    set_state(v, v->left, v->right, v->par->right, v->par->par->right);
  } else {
    set_state(v, v->par->par->left, v->par->left, v->left, v->right);
  }
  port_out_->Set(std::make_pair(MsgCode::ZIGZIG_PERF, data_.Get()));

  auto old_root = v->par->par;
  if (is_right_zig_zig) {
    rotate_right(v->par->par);
  } else {
    rotate_left(v->par->par);
  }

  if (!v->par->par) {
    if (!hidden_root) {
      update_root(old_root, v->par);
    } else {
      hidden_root->left = v->par;
    }
  }
  port_out_->Set(std::make_pair(MsgCode::ZIGZIG_PERF, data_.Get()));

  old_root = v->par;
  if (is_right_zig_zig) {
    rotate_right(v->par);
  } else {
    rotate_left(v->par);
  }

  if (!v->par) {
    if (!hidden_root) {
      update_root(old_root, v);
    } else {
      hidden_root->left = v;
    }
  }
  port_out_->Set(std::make_pair(MsgCode::ZIGZIG_END, data_.Get()));
}

template <typename T>
void Model<T>::zig_zag(PNode v, PNode hidden_root, bool is_right_left) {
  if (is_right_left) {
    set_state(v, v->par->par->left, v->left, v->right, v->par->right);
  } else {
    set_state(v, v->par->left, v->left, v->right, v->par->par->right);
  }
  port_out_->Set(std::make_pair(MsgCode::ZIGZAG_PERF, data_.Get()));

  if (is_right_left) {
    rotate_right(v->par);
  } else {
    rotate_left(v->par);
  }
  port_out_->Set(std::make_pair(MsgCode::ZIGZAG_PERF, data_.Get()));

  auto old_root = v->par;
  if (is_right_left) {
    rotate_left(v->par);
  } else {
    rotate_right(v->par);
  }
  if (!v->par) {
    if (!hidden_root) {
      update_root(old_root, v);
    } else {
      hidden_root->left = v;
    }
  }
  port_out_->Set(std::make_pair(MsgCode::ZIGZAG_END, data_.Get()));
}

template <typename T>
void Model<T>::set_state(PNode v, PNode A, PNode B, PNode C, PNode D) {
  set_regular(v);
  v->state = State::X_VERTEX;
  v->par->state = State::P_VERTEX;
  if (v->par->par) {
    v->par->par->state = State::G_VERTEX;
  }
  if (A) {
    A->state = State::A_SUBTREE;
  }
  if (B) {
    B->state = State::B_SUBTREE;
  }
  if (C) {
    C->state = State::C_SUBTREE;
  }
  if (D) {
    D->state = State::D_SUBTREE;
  }
}

template <typename T>
void Model<T>::update_root(PNode old_root, PNode new_root) {
  int cur_key = -1;
  for (auto &[key, root] : data_.Get()) {
    if (root == old_root) {
      cur_key = key;
      break;
    }
  }
  if (cur_key != -1) {
    data_[cur_key] = new_root;
  }
}

template <typename T> PNode<T> Model<T>::find(PNode v, const T &key) {
  if (!v) {
    return v;
  }
  if (v->value == key) {
    v->state = State::FOUND;
    port_out_->Set(std::make_pair(MsgCode::FOUND, data_.Get()));
    set_regular(v);

    splay(v);
    return v;
  }
  if (v->value > key && v->left) {
    v->state = State::ON_PATH;
    port_out_->Set(std::make_pair(MsgCode::SEARCH, data_.Get()));

    return find(v->left, key);
  }
  if (v->value < key && v->right) {
    v->state = State::ON_PATH;
    port_out_->Set(std::make_pair(MsgCode::SEARCH, data_.Get()));

    return find(v->right, key);
  }

  v->state = State::NOT_FOUND;
  port_out_->Set(std::make_pair(MsgCode::NOT_FOUND, data_.Get()));

  splay(v);
  return v;
}

template <typename T>
std::pair<PNode<T>, PNode<T>> Model<T>::split(PNode v, const T &key,
                                              bool *res) {
  if (!v) {
    if (res) {
      *res = true;
    }
    return {nullptr, nullptr};
  }
  set_regular(v);
  port_out_->Set(std::make_pair(MsgCode::SPLIT_PERF, data_.Get()));
  v = find(v, key);
  port_out_->Set(std::make_pair(MsgCode::SPLIT_PERF, data_.Get()));
  if (v->value == key) {
    v->state = State::HIDE_THIS;
    port_out_->Set(std::make_pair(MsgCode::SPLIT_PERF, data_.Get()));
    auto ltree = v->left;
    auto rtree = v->right;
    if (ltree) {
      ltree->par = nullptr;
    }
    if (rtree) {
      rtree->par = nullptr;
    }
    if (res) {
      *res = false;
    }
    return {ltree, rtree};
  }
  if (v->value < key) {
    v->state = State::SPLIT_RIGHT;
    port_out_->Set(std::make_pair(MsgCode::SPLIT_PERF, data_.Get()));
    auto rtree = v->right;
    v->right = nullptr;
    if (rtree) {
      rtree->par = nullptr;
    }
    update(v);
    if (res) {
      *res = true;
    }
    return {v, rtree};
  } else {
    v->state = State::SPLIT_LEFT;
    port_out_->Set(std::make_pair(MsgCode::SPLIT_PERF, data_.Get()));
    auto ltree = v->left;
    v->left = nullptr;
    if (ltree) {
      ltree->par = nullptr;
    }
    update(v);
    if (res) {
      *res = true;
    }
    return {ltree, v};
  }
}

template <typename T>
PNode<T> Model<T>::insert(PNode v, const T &key, bool *res) {
  auto [ltree, rtree] = split(v, key, res);
  if (v && v->state == State::HIDE_THIS) {
    delete v;
  }
  PNode new_node{new Node<T>{.par = nullptr,
                             .left = ltree,
                             .right = rtree,
                             .value = key,
                             .min = key,
                             .max = key}};
  if (ltree) {
    ltree->par = new_node;
    ltree->state = State::REGULAR;
  }
  if (rtree) {
    rtree->par = new_node;
    rtree->state = State::REGULAR;
  }
  update(new_node);
  set_regular(new_node);
  new_node->state = State::INSERTED;
  return new_node;
}

// попытаюсь объяснить зачем нам нужен какой то hidden_root.
//
// представим, что мы выполняем операцию remove. Для этого нам нужно
// найти вершину, затем вызвать splay от нее, тем самым подняв ее наверх.
// а дальше удалить ее из дерева и помержить оставшиеся два дерева.
//
// но я не хочу возиться с тем, чтобы отрисовывать два независимых дерева на
// плоскости. Поэтому вершину я удалять эту не буду, а лишь присвою ей
// специальный state, который будет говорить о том, что отрисовывать эту
// вершину не нужно. То есть по сути дела я буду рисовать всегда лишь одно
// дерево, но выглядеть это будет как два разных дерева. Далее мне необходимо
// (следуя алгоритму) найти в "левом" дереве самую правую вершину и вызвать
// от нее splay. Чтобы вызвать splay в "левом" дереве, не задев при этом
// вершины из "правого" дерева, я делаю ltree->par = nullptr;
//
// но а теперь зачем нам нужен hidden_root в функции splay: мы этот
// hidden_root все еще храним в trees_ после "удаления" вершины, и у него в
// ходе операции splay в "левом" дереве может поменяться левый сын и это
// необходимо учитывать

template <typename T> PNode<T> Model<T>::merge(PNode hidden_root) {
  port_out_->Set(std::make_pair(MsgCode::MERGE_PERF, data_.Get()));

  auto ltree = hidden_root->left, rtree = hidden_root->right;
  if (!ltree) {
    if (rtree) {
      rtree->par = nullptr;
    }
    delete hidden_root;
    return rtree;
  }
  ltree->par = nullptr;
  while (ltree->right) {
    ltree->state = State::ON_PATH;
    port_out_->Set(std::make_pair(MsgCode::R_SEARCH, data_.Get()));
    ltree = ltree->right;
  }
  ltree->state = State::FOUND;
  port_out_->Set(std::make_pair(MsgCode::R_FOUND, data_.Get()));
  splay(ltree, hidden_root);
  set_regular(hidden_root);
  ltree->right = rtree;
  if (rtree) {
    rtree->par = ltree;
  }
  update(ltree);
  delete hidden_root;
  return ltree;
}

template <typename T>
PNode<T> Model<T>::remove(PNode v, const T &key, bool *res) {
  if (!v) {
    return v;
  }
  v = find(v, key);
  set_regular(v);
  if (v->value == key) {
    if (res) {
      *res = true;
    }
    v->state = State::DO_REMOVE;
    port_out_->Set(std::make_pair(MsgCode::DO_REM, data_.Get()));
    v->state = State::HIDE_THIS;
    return merge(v);
  }

  v->state = State::DONT_REM;
  port_out_->Set(std::make_pair(MsgCode::DONT_REM, data_.Get()));
  if (res) {
    *res = false;
  }
  return v;
}

template <typename T> void Model<T>::set_regular(PNode root, PNode prev) {
  if (!root) {
    return;
  }

  if (root->left != prev) {
    set_regular(root->left, root);
  }
  if (root->right != prev) {
    set_regular(root->right, root);
  }
  if (root->par != prev) {
    set_regular(root->par, root);
  }

  root->state = State::REGULAR;
}

template <typename T>
PNode<T> Model<T>::make_hidden_root(PNode ltree, PNode rtree) {
  return new Node<T>{.par = nullptr,
                     .left = ltree,
                     .right = rtree,
                     .value = ltree->max,
                     .min = 0,
                     .max = 0,
                     .state = State::HIDE_THIS};
}

} // namespace DSViz
#endif // MODEL_H
