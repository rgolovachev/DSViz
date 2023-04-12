#ifndef DEBUG_H
#define DEBUG_H
#include <fstream>
#include <iostream>
#include <string>

namespace DSViz {

static void write_log(const std::string &str);
static void clear_log();

} // namespace DSViz

#endif // DEBUG_H
