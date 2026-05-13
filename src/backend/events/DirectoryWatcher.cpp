#include "DirectoryWatcher.hpp"

#include <cassert>
#include <sys/inotify.h>
#include <unistd.h>

#include "FileReader.hpp"

namespace backend::events {

DirectoryWatchDescriptor::DirectoryWatchDescriptor(DirectoryWatcher& watcher, const std::filesystem::path& path)
    : _Watcher(watcher), _WatchDescriptor(watcher.AddDescriptor(*this, path)) {}

DirectoryWatchDescriptor::~DirectoryWatchDescriptor() { _Watcher.RemoveDescriptor(_WatchDescriptor); }

DirectoryWatcher::DirectoryWatcher(EventLoop& loop)
    : EventHandle(loop, PosixE(inotify_init1(IN_NONBLOCK | IN_CLOEXEC)), true) {
  ScheduleRead();
}

DirectoryWatcher::~DirectoryWatcher() {}

WatchDescriptor DirectoryWatcher::AddDescriptor(DirectoryWatchDescriptor& descriptor,
                                                const std::filesystem::path& path) {
  WatchDescriptor wd =
      PosixE(inotify_add_watch(_Fd, path.c_str(), IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO | IN_ONLYDIR));
  _WatchMap.emplace(wd, std::ref(descriptor));
  return wd;
}

void DirectoryWatcher::RemoveDescriptor(WatchDescriptor wd) {
  assert(_WatchMap.contains(wd));
  _WatchMap.erase(wd);
  PosixE(inotify_rm_watch(_Fd, wd));
}

void DirectoryWatcher::OnRead() {
  FileReader reader(_Fd, _State);

  for (FileReader::ReadResult more = FileReader::ReadResult::Success; more == FileReader::ReadResult::Success;) {
    if (reader.IsReading()) {
      more = reader.Continue();
    } else {
      more = reader.Request(sizeof(struct inotify_event), [this](FileReader& reader, std::vector<char> data) {
        // Handle the inotify_event data
        struct inotify_event* event = reinterpret_cast<struct inotify_event*>(data.data());
        if (event->len > 0) {
          reader.Request(event->len, [this, event](FileReader& reader, std::vector<char> name) {
            std::string s(name.begin(), name.end());
            s.erase(s.find('\0'));
            OnInotifyEvent(*event, s);
          });
        } else {
          OnInotifyEvent(*event, std::string());
        }
      });
    }
  }

  ScheduleRead();
}

void DirectoryWatcher::OnWrite() { throw std::runtime_error("DirectoryWatcher::OnWrite"); }

void DirectoryWatcher::OnInotifyEvent(struct inotify_event event, std::string name) {
  if (event.mask & (IN_CREATE | IN_MOVED_TO)) {
    auto descriptor = _WatchMap.at(event.wd);
    descriptor.get().OnDirectoryCreateChild(name);
  }
  if (event.mask & (IN_DELETE | IN_MOVED_FROM)) {
    auto descriptor = _WatchMap.at(event.wd);
    descriptor.get().OnDirectoryDeleteChild(name);
  }
}

} // namespace backend::events
