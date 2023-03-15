#ifndef VNODE_H
#define VNODE_H
#include "Common/node.h"
#include <QColor>
#include <memory>
#include <type_traits>

namespace DSViz {

constexpr const int kRadius = 6;
constexpr const int kHorSpace = 2;
constexpr const int kVerSpace = 2;

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

template <typename T> void UpdNode(PVNode<T> node) {
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

template <typename T> void FillY_Weight(PNode<T> src, PVNode<T> dst, int y) {
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
  detail::UpdNode(dst);
}

template <typename T> void FillX(PVNode<T> dst, int x) {
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

} // namespace detail

template <typename T> PVNode<T> Fill(PNode<T> src, int x0 = 0, int y0 = 0) {
  if (!src) {
    return nullptr;
  }
  PVNode<T> dst = new VNode<T>{};
  detail::FillY_Weight(src, dst, y0);
  detail::FillX(dst, x0);
  return dst;
}

} // namespace DSViz
#endif // VNODE_H
