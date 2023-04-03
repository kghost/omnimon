#include "ProcessOrder.hpp"

#include <algorithm>
#include <cassert>
#include <functional>
#include <ranges>
#include <span>

#include "Process.hpp"

namespace frontend::curses {

class ProcessOrderTree {
public:
  bool operator()(std::shared_ptr<Process> a, std::shared_ptr<Process> b) const {
    return std::ranges::lexicographical_compare(Process::GetAncestors(a), Process::GetAncestors(b),
                                                [](auto& a, auto& b) { return a->GetPid() < b->GetPid(); });
  }
};

std::shared_ptr<Process> ProcessCollection::GetProcess(backend::process::PidType pid) const {
  auto it = _ProcessCache.find(pid);
  if (it != _ProcessCache.end()) {
    return it->second;
  }

  return nullptr;
}

void ProcessCollection::UpdateList() {
  _Listing.DoIterate();

  for (auto& [_, proc] : _ProcessCache) {
    proc->Update();
  }

  std::erase_if(_ProcessCache, [](auto& proc) { return !proc.second->Exists(); });

  for (auto& [_, proc] : _ProcessCache) {
    auto parent = GetProcess(proc->GetPPid());
    proc->SetParent(parent);
    if (parent) {
      parent->AddChild(proc);
    }
  }
}

void ProcessCollection::operator()(backend::process::PidType pid, const std::filesystem::path& dir) {
  auto& v = _ProcessCache[pid];
  if (!v) {
    v = std::make_shared<Process>(dir);
  }
}

std::shared_ptr<Process> ProcessCollection::MoveCursor(std::shared_ptr<Process> current, DisplayLength offset) {
  // Move the cursor to the process at the given offset. Return the process at the offset.
  // If the offset is out of bounds, return the first or last process.
  if (offset == 0) {
    return current;
  } else if (offset > 0) {
    std::vector<std::shared_ptr<Process>> result(offset + 1);
    // The std::ranges::partial_sort_copy() call below copies first `offset` number of processes that are after current
    // process into the result vector, including current. The processes are sorted in normal order.
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

std::vector<std::shared_ptr<Process>> ProcessCollection::GetTopK(size_t k) {
  std::vector<std::shared_ptr<Process>> result(k);
  auto [_, end] = std::ranges::partial_sort_copy(_ProcessCache | std::views::values, result, ProcessOrderTree());
  return {result.begin(), end};
}

std::vector<std::shared_ptr<Process>>
ProcessCollection::GetAround(std::shared_ptr<Process> process, DisplayLength& cursor, DisplayLength max, bool update) {
  auto at = [&] {
    if (update) {
      auto selection = Process::GetAncestors(process);
      UpdateList();
      auto at = *std::find_if(selection.rbegin(), selection.rend(), [](auto p) { return p->Exists(); });
      assert(at); // The cursor should point to this process.
      return at;
    } else {
      return process;
    }
  }();

  if (_ProcessCache.size() <= max) {
    auto range = _ProcessCache | std::views::values;
    std::vector<std::shared_ptr<Process>> result(range.begin(), range.end());
    std::ranges::sort(result, ProcessOrderTree());
    auto it = std::lower_bound(result.begin(), result.end(), at, ProcessOrderTree());
    assert(it != result.end());
    cursor = it - result.begin();
    return result;
  } else {
    // Count how many processes are before and after the selected process.
    // The selected process itself is not counted.
    DisplayLength countBefore =
        std::ranges::count_if(_ProcessCache, [&](auto& p) { return ProcessOrderTree()(p.second, at); });
    DisplayLength countAfter = _ProcessCache.size() - countBefore - 1;

    // The countBefore and countAfter variables are now the number of processes before and after the selected process,
    // but they might be too large, because the cursor might be at the beginning or end of the list.
    // The std::min() calls below ensure that the counts never exceed the maximum number of processes that can be
    // displayed.
    auto before = std::min(countBefore, std::max(cursor, max - countAfter - 1));
    auto after = std::min(countAfter, std::max(max - cursor - 1, max - countBefore - 1));
    cursor = before;

    // The result vector will contain the processes that will be displayed, including the selected process.
    // We need to allocate space for the maximum number of processes that can be displayed,
    // because otherwise the std::ranges::partial_sort_copy() call below will not work.
    std::vector<std::shared_ptr<Process>> result(before + 1 + after);

    // At this point the result vector is full of default-constructed shared_ptr<Process> objects.
    // We now need to fill the result vector with the processes that will be displayed.
    // We do this in two steps:
    // 1. We copy the processes that are before the selected process into the beginning of the result vector.
    // 2. We copy the processes that are after the selected process into the end of the result vector.
    // The processes that are before the selected process are copied in reverse order,
    // because the std::ranges::partial_sort_copy() call below sorts the processes in reverse order.
    // The processes that are after the selected process are sorted in normal order,
    // because the std::ranges::partial_sort_copy() call below sorts the processes in normal order.

    // Copy the processes that are before the selected process into the beginning of the result vector.
    std::ranges::partial_sort_copy(
        _ProcessCache | std::views::values | std::views::filter([&](auto p) { return ProcessOrderTree()(p, at); }),
        std::span(result.begin(), result.begin() + before) | std::views::reverse, std::not_fn(ProcessOrderTree()));

    // Copy the processes that are after the selected process into the end of the result vector.
    std::ranges::partial_sort_copy(_ProcessCache | std::views::values |
                                       std::views::filter([&](auto p) { return !ProcessOrderTree()(p, at); }),
                                   std::span(result.begin() + before, result.end()), ProcessOrderTree());

    return result;
  }

  std::vector<std::shared_ptr<Process>> result;
  return result;
}

} // namespace frontend::curses
