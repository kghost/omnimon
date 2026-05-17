#include "ProcessListing.hpp"

#include <algorithm>
#include <cassert>
#include <functional>
#include <ranges>
#include <span>

#include "Process.hpp"

namespace backend::process {

const std::filesystem::path ProcessListing::_ProcPath{"/proc"};

class ProcessOrderTree {
public:
  bool operator()(const std::shared_ptr<Process>& a, const std::shared_ptr<Process>& b) const {
    return std::ranges::lexicographical_compare(Process::GetAncestors(a), Process::GetAncestors(b),
                                                [](auto& a, auto& b) { return a->GetPid() < b->GetPid(); });
  }
};

void ProcessListing::UpdateList() {
  auto SkipNonePidDir = [](const std::filesystem::directory_entry& entry) {
    if (!entry.is_directory()) {
      return true;
    }

    std::string filename = entry.path().filename();
    if (!std::all_of(filename.begin(), filename.end(), ::isdigit)) {
      return true;
    }

    return false;
  };

  for (auto& dir : std::filesystem::directory_iterator(_ProcPath)) {
    if (SkipNonePidDir(dir)) {
      continue;
    }

    auto pid = std::stoi(dir.path().filename().string());
    auto& proc = _ProcessCache[pid];
    if (!proc) {
      proc = std::make_shared<Process>(dir);
    }
  }

  for (auto& [_, proc] : _ProcessCache) {
    proc->ParseStatFile();
  }

  std::erase_if(_ProcessCache, [](auto& proc) { return !proc.second->Exists(); });

  for (auto& [_, proc] : _ProcessCache) {
    auto ppid = proc->GetPPid();
    if (ppid != 0) {
      auto it = _ProcessCache.find(ppid);
      if (it != _ProcessCache.end()) {
        proc->SetParent(it->second);
        it->second->AddChild(proc);
      } else {
        proc->SetParent(nullptr);
      }
    } else {
      proc->SetParent(nullptr);
    }
  }
}

std::shared_ptr<Process> ProcessListing::MoveCursor(std::shared_ptr<Process> current, ssize_t offset) {
  if (offset == 0) {
    return current;
  } else if (offset > 0) {
    std::vector<std::shared_ptr<Process>> result(offset + 1);
    auto [_, end] = std::ranges::partial_sort_copy(_ProcessCache | std::views::values | std::views::filter([&](auto p) {
                                                     return !ProcessOrderTree()(p, current);
                                                   }),
                                                   result, ProcessOrderTree());
    return end == result.begin() ? current : *--end;
  } else {
    std::vector<std::shared_ptr<Process>> result(-offset);
    auto [_, end] = std::ranges::partial_sort_copy(
        _ProcessCache | std::views::values | std::views::filter([&](auto p) { return ProcessOrderTree()(p, current); }),
        result, std::not_fn(ProcessOrderTree()));
    return end == result.begin() ? current : *--end;
  }
}

std::shared_ptr<Process> ProcessListing::GetValidAncestor(std::shared_ptr<Process> process) {
  auto selection = Process::GetAncestors(process);
  return *std::find_if(selection.rbegin(), selection.rend(), [](auto p) { return p->Exists(); });
}

std::vector<std::shared_ptr<Process>> ProcessListing::GetTopK(ssize_t k) {
  std::vector<std::shared_ptr<Process>> result(k);
  auto [_, end] = std::ranges::partial_sort_copy(_ProcessCache | std::views::values, result, ProcessOrderTree());
  return {result.begin(), end};
}

std::vector<std::shared_ptr<Process>> ProcessListing::GetAround(std::shared_ptr<Process> process, ssize_t index,
                                                                ssize_t max) {
  if (_ProcessCache.size() <= static_cast<size_t>(max)) {
    std::vector<std::shared_ptr<Process>> result = _ProcessCache | std::views::values | std::ranges::to<std::vector>();
    std::ranges::sort(result, ProcessOrderTree());
    assert(std::lower_bound(result.begin(), result.end(), process, ProcessOrderTree()) != result.end());
    return result;
  } else {
    ssize_t countBefore =
        std::ranges::count_if(_ProcessCache, [&](auto& p) { return ProcessOrderTree()(p.second, process); });
    ssize_t countAfter = static_cast<ssize_t>(_ProcessCache.size()) - countBefore - 1;

    auto before = std::min(countBefore, std::max(index, max - countAfter - 1));
    auto after = std::min(countAfter, std::max(max - index - 1, max - countBefore - 1));

    std::vector<std::shared_ptr<Process>> result(before + 1 + after);

    std::ranges::partial_sort_copy(
        _ProcessCache | std::views::values | std::views::filter([&](auto p) { return ProcessOrderTree()(p, process); }),
        std::span(result.begin(), result.begin() + before) | std::views::reverse, std::not_fn(ProcessOrderTree()));

    std::ranges::partial_sort_copy(_ProcessCache | std::views::values |
                                       std::views::filter([&](auto p) { return !ProcessOrderTree()(p, process); }),
                                   std::span(result.begin() + before, result.end()), ProcessOrderTree());

    return result;
  }
}

} // namespace backend::process
