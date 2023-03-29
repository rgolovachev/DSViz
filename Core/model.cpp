#include "model.h"

namespace DSViz {

namespace detail {

Trees::~Trees() {
  for (auto tree : trees_) {
    destroy(tree.second);
  }
}

void Trees::Destroy(int id) {
  if (trees_.find(id) != trees_.end()) {
    destroy(trees_[id]);
  }
}

void Trees::Insert(std::pair<int, PNode<int>> tree) { trees_.insert(tree); }

void Trees::Erase(int id) {
  if (trees_.find(id) != trees_.end()) {
    trees_.erase(id);
  }
}

size_t Trees::Size() { return trees_.size(); }

PNode<int> &Trees::operator[](int key) { return trees_.at(key); }

BareTrees<int> Trees::Get() { return trees_; }

void Trees::destroy(PNode<int> root) {
  if (!root) {
    return;
  }
  destroy(root->left);
  destroy(root->right);
  delete root;
}

} // namespace detail

Model::Model() : port_out_{std::make_unique<Observable>()} {
  data_.Insert({next_id_++, nullptr});
  // пока еще никто на модель здесь не подписан, так что я просто изменю поле
  // msg_ в Observable
  port_out_->Set(std::make_pair(MsgCode::EMPTY_MSG, data_.Get()));
}

void Model::Insert(int id, int key) {
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

void Model::Remove(int id, int key) {
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

void Model::Merge(int left_id, int right_id) {
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

void Model::Split(int id, int key) {
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

void Model::ExistKey(int id, int key) {
  data_[id] = find(data_[id], key);
  if (data_[id] && data_[id]->value == key) {
    set_regular(data_[id]);
    port_out_->Set(std::make_pair(MsgCode::FOUND, data_.Get()));
  } else {
    set_regular(data_[id]);
    port_out_->Set(std::make_pair(MsgCode::NOT_FOUND, data_.Get()));
  }
}

void Model::DeleteTree(int id) {
  if (data_.Size() == 1) {
    port_out_->Set(std::make_pair(MsgCode::UNSUCC_DEL, data_.Get()));
    return;
  }
  data_.Destroy(id);
  data_.Erase(id);
  port_out_->Set(std::make_pair(MsgCode::SUCC_DEL, data_.Get()));
}

Observable *Model::GetPortOut() { return port_out_.get(); }

void Model::update(PNode v) {
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

void Model::rotate_left(PNode v) {
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

void Model::rotate_right(PNode v) {
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

void Model::splay(PNode v, PNode hidden_root) {
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

void Model::zig(PNode v, PNode hidden_root, bool is_right_zig) {
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

void Model::zig_zig(PNode v, PNode hidden_root, bool is_right_zig_zig) {
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

void Model::zig_zag(PNode v, PNode hidden_root, bool is_right_left) {
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

void Model::set_state(PNode v, PNode A, PNode B, PNode C, PNode D) {
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

void Model::update_root(PNode old_root, PNode new_root) {
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

Model::PNode Model::find(PNode v, int key) {
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

std::pair<Model::PNode, Model::PNode> Model::split(PNode v, int key,
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

Model::PNode Model::insert(PNode v, int key, bool *res) {
  auto [ltree, rtree] = split(v, key, res);
  if (v && v->state == State::HIDE_THIS) {
    delete v;
  }
  PNode new_node{new Node<int>{.par = nullptr,
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

Model::PNode Model::merge(PNode hidden_root) {
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

Model::PNode Model::remove(PNode v, int key, bool *res) {
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

void Model::set_regular(PNode root, PNode prev) {
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

Model::PNode Model::make_hidden_root(PNode ltree, PNode rtree) {
  return new Node<int>{.par = nullptr,
                       .left = ltree,
                       .right = rtree,
                       .value = ltree->max,
                       .min = 0,
                       .max = 0,
                       .state = State::HIDE_THIS};
}

} // namespace DSViz
