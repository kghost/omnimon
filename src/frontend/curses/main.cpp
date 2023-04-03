#include <clocale>

#include "OmniMon.hpp"

/*
#include "../../backend/process/file/FileIo.hpp"

int main(int argc, char *argv[]) {
  backend::process::file::FileIo o(backend::bpf::BpfManager::GetInstance(), 0);

  auto map = o.GetResultMap();

  while (true) {
    for (auto [key, value] : map) {
      std::cout << std::format("pid {} reads {}", value.pid, value.reads)
                << std::endl;
    }

    sleep(2);
  }
  return 0;
}
*/

int main(int argc, char* argv[]) {
  std::setlocale(LC_ALL, "");

  frontend::curses::OmniMon::GetInstance().Run();

  return 0;
}
