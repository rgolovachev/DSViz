#include "Debug/debug.h"
#include <QTime>

namespace DSViz {

void WriteLog(const std::string &str) {
  std::ofstream stream{"./log.txt", std::ios::app};
  stream << QTime::currentTime().toString("hh:mm:ss").toStdString() << " "
         << str << "\n";
  stream.close();
}

void ClearLog() {
  std::ofstream stream{"./log.txt"};
  stream.close();
}

} // namespace DSViz
