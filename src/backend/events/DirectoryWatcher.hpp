#pragma once

#include <filesystem>
#include <map>
#include <sys/inotify.h>

#include "Events.hpp"
#include "FileReader.hpp"

namespace backend::events {

using WatchDescriptor = int;

class DirectoryWatcher;

class DirectoryWatchDescriptor {
public:
  explicit DirectoryWatchDescriptor(DirectoryWatcher& watcher, const std::filesystem::path& path);
  virtual ~DirectoryWatchDescriptor();

  DirectoryWatchDescriptor(const DirectoryWatchDescriptor&) = delete;
  DirectoryWatchDescriptor(DirectoryWatchDescriptor&&) = delete;
  DirectoryWatchDescriptor& operator=(const DirectoryWatchDescriptor&) = delete;
  DirectoryWatchDescriptor& operator=(DirectoryWatchDescriptor&&) = delete;

  virtual void OnDirectoryCreateChild(const std::string& name) = 0;
  virtual void OnDirectoryDeleteChild(const std::string& name) = 0;

private:
  DirectoryWatcher& _Watcher;
  WatchDescriptor _WatchDescriptor;
};

class DirectoryWatcher : public EventHandle {
public:
  DirectoryWatcher(EventLoop& loop);
  ~DirectoryWatcher() override;

  DirectoryWatcher(const DirectoryWatcher&) = delete;
  DirectoryWatcher(DirectoryWatcher&&) = delete;
  DirectoryWatcher& operator=(const DirectoryWatcher&) = delete;
  DirectoryWatcher& operator=(DirectoryWatcher&&) = delete;

  WatchDescriptor AddDescriptor(DirectoryWatchDescriptor& descriptor, const std::filesystem::path& path);
  void RemoveDescriptor(WatchDescriptor wd);

  void OnRead() override;
  void OnWrite() override;

private:
  FileReadState _State;
  std::map<WatchDescriptor, std::reference_wrapper<DirectoryWatchDescriptor>> _WatchMap;

  void OnInotifyEvent(struct inotify_event event, std::string name);
};

} // namespace backend::events
