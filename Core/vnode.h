#ifndef VNODE_H
#define VNODE_H
#include "Common/node.h"
#include <QColor>
#include <memory>
#include <type_traits>

namespace DSViz {

template <typename T> struct VNode;

template <typename T> using PVNode = VNode<T> *;

// Шаблонный по типу хранимого ключа (VNode<int>)
template <typename T> struct VNode {
  PNode<T> node;
  PVNode<T> left, right;
  int x, y, width, x_max, x_min;
  QColor col;
};

namespace detail {

template <typename T> class ReadyTree {
public:
  ~ReadyTree();

  void Fill(PNode<T> src, int x0 = 0, int y0 = 0);

  PVNode<T> Get();

  static constexpr const int kRadius = 6;
  static constexpr const int kHorSpace = 2;
  static constexpr const int kVerSpace = 2;

private:
  void FillY_Weight(PNode<T> src, PVNode<T> dst, int y);
  void FillX(PVNode<T> dst, int x);
  void UpdNode(PVNode<T> node);

  void Destroy(PVNode<T> root);

  PVNode<T> tree_ = nullptr;
};

template <typename T> ReadyTree<T>::~ReadyTree() { Destroy(tree_); }

template <typename T> void ReadyTree<T>::Fill(PNode<T> src, int x0, int y0) {
  Destroy(tree_);
  if (!src) {
    tree_ = nullptr;
    return;
  }
  tree_ = new VNode<T>{};
  FillY_Weight(src, tree_, y0);
  FillX(tree_, x0);
}

template <typename T> PVNode<T> ReadyTree<T>::Get() { return tree_; }

template <typename T>
void ReadyTree<T>::FillY_Weight(PNode<T> src, PVNode<T> dst, int y) {
  dst->node = src;
  dst->y = y;
  int lwidth = kRadius, rwidth = kRadius;
  if (src->left) {
    dst->left = new VNode<T>{};
    FillY_Weight(src->left, dst->left, y - 2 * kRadius - kVerSpace);
    lwidth = kHorSpace / 2 + dst->left->width;
  }
  if (src->right) {
    dst->right = new VNode<T>{};
    FillY_Weight(src->right, dst->right, y - 2 * kRadius - kVerSpace);
    rwidth = kHorSpace / 2 + dst->right->width;
  }
  dst->width = lwidth + rwidth;
  UpdNode(dst);
}

template <typename T> void ReadyTree<T>::FillX(PVNode<T> dst, int x) {
  dst->x = x;
  int ll = x - dst->width / 2;
  int rr = x + dst->width / 2;
  if (dst->left) {
    int lr = ll + dst->left->width;
    FillX(dst->left, (ll + lr) / 2);
  }
  if (dst->right) {
    int rl = rr - dst->right->width;
    FillX(dst->right, (rl + rr) / 2);
  }
}

template <typename T> void ReadyTree<T>::UpdNode(PVNode<T> node) {
  if (!node) {
    return;
  }
  node->x_max = node->x;
  node->x_min = node->x;
  if (node->left) {
    node->x_min = node->left->x_min;
  }
  if (node->right) {
    node->x_max = node->right->x_max;
  }
}

template <typename T> void ReadyTree<T>::Destroy(PVNode<T> root) {
  if (!root) {
    return;
  }
  Destroy(root->left);
  Destroy(root->right);
  delete root;
}
} // namespace detail

} // namespace DSViz
#endif // VNODE_H
