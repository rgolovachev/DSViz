#include "model.h"

namespace DSViz {

namespace detail {

Trees::~Trees() {
  for (auto tree : trees_) {
    destroy(tree.second);
  }
}

void Trees::Insert(int id, PNode<int> node) { trees_.insert({id, node}); }

void Trees::DeleteTree(int id) {
  if (trees_.find(id) != trees_.end()) {
    destroy(trees_[id]);
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

Model::Model() {
  data_.Insert(next_id_++, nullptr);
  // пока еще никто на модель здесь не подписан, так что я просто изменю поле
  // msg_ в Observable
  port_out_.Set(std::make_pair(MsgCode::empty_msg, data_.Get()));
}

void Model::Insert(int id, int key) {
  bool flag = false;
  data_[id] = insert(data_[id], key, &flag);
  port_out_.Set(std::make_pair(MsgCode::ins_done, data_.Get()));
  if (!flag) {
    set_regular(data_[id]);
    port_out_.Set(std::make_pair(MsgCode::insert_err, data_.Get()));
    return;
  }

  set_regular(data_[id]);
  port_out_.Set(std::make_pair(MsgCode::OK, data_.Get()));
}

void Model::Remove(int id, int key) {
  bool flag = false;
  data_[id] = remove(data_[id], key, &flag);
  if (!flag) {
    set_regular(data_[id]);
    port_out_.Set(std::make_pair(MsgCode::remove_err, data_.Get()));
    return;
  }

  if (data_[id]) {
    data_[id]->state = State::new_root;
    port_out_.Set(std::make_pair(MsgCode::new_root, data_.Get()));
    data_[id]->state = State::regular;
  }

  port_out_.Set(std::make_pair(MsgCode::OK, data_.Get()));
}

void Model::Merge(int left_id, int right_id) {
  if (left_id == right_id) {
    port_out_.Set(std::make_pair(MsgCode::merge_equal, data_.Get()));
    return;
  }
  if (!data_[left_id] || !data_[right_id]) {
    port_out_.Set(std::make_pair(MsgCode::merge_empty, data_.Get()));
    return;
  }
  if (data_[left_id]->max < data_[right_id]->min) {
    auto ltree = data_[left_id], rtree = data_[right_id];
    auto hidden_root = make_hidden_root(ltree, rtree);
    update(hidden_root);
    rtree->par = hidden_root;
    data_[left_id] = hidden_root;
    data_[left_id] = merge(hidden_root);
    data_.DeleteTree(right_id);
    data_[left_id]->state = State::new_root;
    port_out_.Set(std::make_pair(MsgCode::merge_end, data_.Get()));
    data_[left_id]->state = State::regular;
    port_out_.Set(std::make_pair(MsgCode::OK, data_.Get()));
  } else {
    port_out_.Set(std::make_pair(MsgCode::merge_err, data_.Get()));
  }
}

void Model::Split(int id, int key) {
  if (!data_[id]) {
    port_out_.Set(std::make_pair(MsgCode::split_err, data_.Get()));
    return;
  }
  auto [ltree, rtree] = split(data_[id], key);
  if (data_[id]->state != State::hide_this) {
    if (ltree) {
      ltree->state = State::regular;
    }
    if (rtree) {
      rtree->state = State::regular;
    }
    data_[id] = make_hidden_root(ltree, rtree);
    update(data_[id]);
  }
  port_out_.Set(std::make_pair(MsgCode::split_succ, data_.Get()));
  delete data_[id];
  data_[id] = ltree;
  data_.Insert(next_id_++, rtree);
  port_out_.Set(std::make_pair(MsgCode::OK, data_.Get()));
}

void Model::ExistKey(int id, int key) {
  data_[id] = find(data_[id], key);
  if (data_[id] && data_[id]->value == key) {
    set_regular(data_[id]);
    port_out_.Set(std::make_pair(MsgCode::found, data_.Get()));
  } else {
    set_regular(data_[id]);
    port_out_.Set(std::make_pair(MsgCode::not_found, data_.Get()));
  }
}

void Model::DeleteTree(int id) {
  if (data_.Size() == 1) {
    port_out_.Set(std::make_pair(MsgCode::unsucc_del, data_.Get()));
    return;
  }
  data_.DeleteTree(id);
  port_out_.Set(std::make_pair(MsgCode::succ_del, data_.Get()));
}

void Model::SubscribeToView(Observer<Model::MsgType> *view_observer) {
  port_out_.Subscribe(view_observer);
}

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
  v->state = State::splay_ver;
  port_out_.Set(std::make_pair(MsgCode::splay_perf, data_.Get()));

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
  v->state = State::splay_ver;
  port_out_.Set(std::make_pair(MsgCode::splay_perf, data_.Get()));
}

void Model::zig(PNode v, PNode hidden_root, bool is_right_zig) {
  if (is_right_zig) {
    set_state(v, v->left, v->right, v->par->right);
  } else {
    set_state(v, v->par->left, v->left, v->right);
  }
  port_out_.Set(std::make_pair(MsgCode::zig_perf, data_.Get()));

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
  port_out_.Set(std::make_pair(MsgCode::zig_end, data_.Get()));
}

void Model::zig_zig(PNode v, PNode hidden_root, bool is_right_zig_zig) {
  if (is_right_zig_zig) {
    set_state(v, v->left, v->right, v->par->right, v->par->par->right);
  } else {
    set_state(v, v->par->par->left, v->par->left, v->left, v->right);
  }
  port_out_.Set(std::make_pair(MsgCode::zigzig_perf, data_.Get()));

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
  port_out_.Set(std::make_pair(MsgCode::zigzig_perf, data_.Get()));

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
  port_out_.Set(std::make_pair(MsgCode::zigzig_end, data_.Get()));
}

void Model::zig_zag(PNode v, PNode hidden_root, bool is_right_left) {
  if (is_right_left) {
    set_state(v, v->par->par->left, v->left, v->right, v->par->right);
  } else {
    set_state(v, v->par->left, v->left, v->right, v->par->par->right);
  }
  port_out_.Set(std::make_pair(MsgCode::zigzag_perf, data_.Get()));

  if (is_right_left) {
    rotate_right(v->par);
  } else {
    rotate_left(v->par);
  }
  port_out_.Set(std::make_pair(MsgCode::zigzag_perf, data_.Get()));

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
  port_out_.Set(std::make_pair(MsgCode::zigzag_end, data_.Get()));
}

void Model::set_state(PNode v, PNode A, PNode B, PNode C, PNode D) {
  set_regular(v);
  v->state = State::x_vertex;
  v->par->state = State::p_vertex;
  if (v->par->par) {
    v->par->par->state = State::g_vertex;
  }
  if (A) {
    A->state = State::a_subtree;
  }
  if (B) {
    B->state = State::b_subtree;
  }
  if (C) {
    C->state = State::c_subtree;
  }
  if (D) {
    D->state = State::d_subtree;
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
    v->state = State::found;
    port_out_.Set(std::make_pair(MsgCode::found, data_.Get()));
    set_regular(v);

    splay(v);
    return v;
  }
  if (v->value > key && v->left) {
    v->state = State::on_path;
    port_out_.Set(std::make_pair(MsgCode::search, data_.Get()));

    return find(v->left, key);
  }
  if (v->value < key && v->right) {
    v->state = State::on_path;
    port_out_.Set(std::make_pair(MsgCode::search, data_.Get()));

    return find(v->right, key);
  }

  v->state = State::not_found;
  port_out_.Set(std::make_pair(MsgCode::not_found, data_.Get()));

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
  port_out_.Set(std::make_pair(MsgCode::split_perf, data_.Get()));
  v = find(v, key);
  port_out_.Set(std::make_pair(MsgCode::split_perf, data_.Get()));
  if (v->value == key) {
    v->state = State::hide_this;
    port_out_.Set(std::make_pair(MsgCode::split_perf, data_.Get()));
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
    v->state = State::split_right;
    port_out_.Set(std::make_pair(MsgCode::split_perf, data_.Get()));
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
    v->state = State::split_left;
    port_out_.Set(std::make_pair(MsgCode::split_perf, data_.Get()));
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
  if (v && v->state == State::hide_this) {
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
    ltree->state = State::regular;
  }
  if (rtree) {
    rtree->par = new_node;
    rtree->state = State::regular;
  }
  update(new_node);
  set_regular(new_node);
  new_node->state = State::inserted;
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
  port_out_.Set(std::make_pair(MsgCode::merge_perf, data_.Get()));

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
    ltree->state = State::on_path;
    port_out_.Set(std::make_pair(MsgCode::r_search, data_.Get()));
    ltree = ltree->right;
  }
  ltree->state = State::found;
  port_out_.Set(std::make_pair(MsgCode::r_found, data_.Get()));
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
    v->state = State::do_remove;
    port_out_.Set(std::make_pair(MsgCode::do_rem, data_.Get()));
    v->state = State::hide_this;
    return merge(v);
  }

  v->state = State::dont_rem;
  port_out_.Set(std::make_pair(MsgCode::dont_rem, data_.Get()));
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

  root->state = State::regular;
}

Model::PNode Model::make_hidden_root(PNode ltree, PNode rtree) {
  return new Node<int>{.par = nullptr,
                       .left = ltree,
                       .right = rtree,
                       .value = ltree->max,
                       .min = 0,
                       .max = 0,
                       .state = State::hide_this};
}

} // namespace DSViz
