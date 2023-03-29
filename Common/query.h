#ifndef QUERY_H
#define QUERY_H
#include <list>

namespace DSViz {

enum class QueryType {
  INSERT,
  REMOVE,
  FIND,
  SPLIT,
  MERGE,
  DELTREE,
  DO_NOTHING
};

template <typename T> struct UserQuery {
  QueryType type;
  std::pair<int, T> args;
};

} // namespace DSViz
#endif // QUERY_H
