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

class ReadyTree {
  using PNode = PNode<int>;
  using PVNode = PVNode<int>;

public:
  ~ReadyTree();

  void Fill(PNode src, int x0 = 0, int y0 = 0);

  PVNode Get();

  static constexpr const int kRadius = 6;
  static constexpr const int kHorSpace = 2;
  static constexpr const int kVerSpace = 2;

private:
  void FillY_Weight(PNode src, PVNode dst, int y);
  void FillX(PVNode dst, int x);
  void UpdNode(PVNode node);

  void Destroy(PVNode root);

  PVNode tree_ = nullptr;
};

} // namespace detail

} // namespace DSViz
#endif // VNODE_H
