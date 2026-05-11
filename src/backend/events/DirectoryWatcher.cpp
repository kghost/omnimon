#include "DirectoryWatcher.hpp"

#include <deque>
#include <sys/inotify.h>
#include <system_error>
#include <unistd.h>

#include "FileReader.hpp"

namespace backend::events {

DirectoryWatchDescriptor::DirectoryWatchDescriptor(DirectoryWatcher& watcher, const std::filesystem::path& path)
    : _Watcher(watcher), _WatchDescriptor(watcher.AddDescriptor(path)) {}

DirectoryWatchDescriptor::~DirectoryWatchDescriptor() { _Watcher.RemoveDescriptor(_WatchDescriptor); }

DirectoryWatcher::DirectoryWatcher(EventLoop& loop)
    : EventHandle(loop, PosixE(inotify_init1(IN_NONBLOCK | IN_CLOEXEC)), true) {
  ScheduleRead();
}

DirectoryWatcher::~DirectoryWatcher() {}

WatchDescriptor DirectoryWatcher::AddDescriptor(const std::filesystem::path& path) {
  return PosixE(inotify_add_watch(_Fd, path.c_str(), IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO | IN_ONLYDIR));
}

void DirectoryWatcher::RemoveDescriptor(WatchDescriptor wd) { PosixE(inotify_rm_watch(_Fd, wd)); }

void DirectoryWatcher::OnRead() {
  FileReader reader(_Fd, _State);

  while (true) {
    if (reader.IsReading()) {
      reader.Continue();
    } else {
      reader.Request(sizeof(struct inotify_event), [this, &reader](std::vector<char> data) {
        // Handle the inotify_event data
        struct inotify_event* event = reinterpret_cast<struct inotify_event*>(data.data());
        if (event->len > 0) {
          // Read the name of the file/directory that triggered the event.
          reader.Request(event->len, [this, event](std::vector<char> name) {
            // The name is not used in the current implementation, but it can be used for more specific handling if
            // needed.
            OnInotifyEvent(*event, std::string(name.begin(), name.end()));
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
  auto descriptor = _WatchMap.at(event.wd);
  if (event.mask & (IN_CREATE | IN_MOVED_TO)) {
    descriptor.get().OnDirectoryCreateChild(name);
  }
  if (event.mask & (IN_DELETE | IN_MOVED_FROM)) {
    descriptor.get().OnDirectoryDeleteChild(name);
  }
}

} // namespace backend::events
