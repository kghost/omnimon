#include <clocale>

#include "OmniMon.hpp"

int main(int argc, char* argv[]) {
  std::setlocale(LC_ALL, "");
  frontend::ftxui::OmniMon mon;
  mon.Run();
  return 0;
}
