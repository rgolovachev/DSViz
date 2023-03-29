#include "vnode.h"

namespace DSViz {

namespace detail {

ReadyTree::~ReadyTree() { Destroy(tree_); }

void ReadyTree::Fill(PNode src, int x0, int y0) {
  Destroy(tree_);
  if (!src) {
    tree_ = nullptr;
    return;
  }
  tree_ = new VNode<int>{};
  FillY_Weight(src, tree_, y0);
  FillX(tree_, x0);
}

ReadyTree::PVNode ReadyTree::Get() { return tree_; }

void ReadyTree::FillY_Weight(PNode src, PVNode dst, int y) {
  dst->node = src;
  dst->y = y;
  int lwidth = kRadius, rwidth = kRadius;
  if (src->left) {
    dst->left = new VNode<int>{};
    FillY_Weight(src->left, dst->left, y - 2 * kRadius - kVerSpace);
    lwidth = kHorSpace / 2 + dst->left->width;
  }
  if (src->right) {
    dst->right = new VNode<int>{};
    FillY_Weight(src->right, dst->right, y - 2 * kRadius - kVerSpace);
    rwidth = kHorSpace / 2 + dst->right->width;
  }
  dst->width = lwidth + rwidth;
  UpdNode(dst);
}

void ReadyTree::FillX(PVNode dst, int x) {
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

void ReadyTree::UpdNode(PVNode node) {
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

void ReadyTree::Destroy(PVNode root) {
  if (!root) {
    return;
  }
  Destroy(root->left);
  Destroy(root->right);
  delete root;
}

} // namespace detail

} // namespace DSViz
