#include <filesystem>
#include <iostream>
#include <string>

#include "../../events/Events.hpp"
#include "../CGroupManager.hpp"
#include "../CGroupNode.hpp"

using namespace backend;

static void PrintCGroupTree(const cgroupv2::CGroupNode& node, const std::string& indent) {
  std::cout << indent << node.GetPath().string() << '\n';
  for (const auto& child : node.GetChildren()) {
    PrintCGroupTree(child, indent + "  ");
  }
}

int main(int argc, char* argv[]) {
  std::filesystem::path rootPath = argc > 1 ? argv[1] : "/sys/fs/cgroup";
  if (!std::filesystem::exists(rootPath) || !std::filesystem::is_directory(rootPath)) {
    std::cerr << "Invalid cgroup v2 root path: " << rootPath << '\n';
    return 1;
  }

  try {
    events::EventLoop loop;
    cgroupv2::CGroupManager manager(loop);
    PrintCGroupTree(manager.GetRoot(), "");
    std::cout << "Finished walking cgroup nodes." << std::endl;
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << "Failed to walk cgroup nodes: " << ex.what() << '\n';
    return 1;
  }
}
