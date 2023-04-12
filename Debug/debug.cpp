#include "Debug/debug.h"
#include <QTime>

namespace DSViz {

void write_log(const std::string &str) {
  std::ofstream stream{"./log.txt", std::ios::app};
  stream << QTime::currentTime().toString("hh:mm:ss").toStdString() << " "
         << str << "\n";
  stream.close();
}

void clear_log() {
  std::ofstream stream{"./log.txt"};
  stream.close();
}

} // namespace DSViz
