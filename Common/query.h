#ifndef QUERY_H
#define QUERY_H
#include <utility>

namespace DSViz {

enum class QueryType {
  insert,
  remove,
  find,
  split,
  merge,
  deltree,
  do_nothing
};

template <typename T> struct UserQuery {
  QueryType type;
  std::pair<int, T> args;
};

} // namespace DSViz
#endif // QUERY_H
