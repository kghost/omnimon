#pragma once

#include <chrono>
#include <stdint.h>

#include "../../utils/Error.hpp"

namespace frontend::curses {

using SigNumType = int;

class EventLoop;

class EventHandle {
public:
  explicit EventHandle(EventLoop& loop, int _Fd);
  virtual ~EventHandle();

  EventHandle(const EventHandle&) = delete;
  EventHandle(EventHandle&&) = delete;
  EventHandle& operator=(const EventHandle&) = delete;
  EventHandle& operator=(EventHandle&&) = delete;

  void ScheduleRead();
  void ScheduleWrite();
  virtual void OnRead() = 0;
  virtual void OnWrite() = 0;

protected:
  friend class EventLoop;
  void OnEvent(uint32_t events);

  EventLoop& _Loop;
  utils::FileHandle _Fd;
  bool _ReadScheduled = false;
  bool _WriteScheduled = false;
};

class EventNotification : public EventHandle {
public:
  explicit EventNotification(EventLoop& loop);
  virtual ~EventNotification() = default;

  EventNotification(const EventNotification&) = delete;
  EventNotification(EventNotification&&) = delete;
  EventNotification& operator=(const EventNotification&) = delete;
  EventNotification& operator=(EventNotification&&) = delete;

  void OnRead() override final;
  void OnWrite() override final;

  void Notify(uint64_t amount = 1);
  virtual void OnNotification(uint64_t amount) = 0;
};

class EventSignal : public EventHandle {
public:
  explicit EventSignal(EventLoop& loop, SigNumType signum);
  ~EventSignal() override = default;

  EventSignal(const EventSignal&) = delete;
  EventSignal(EventSignal&&) = delete;
  EventSignal& operator=(const EventSignal&) = delete;
  EventSignal& operator=(EventSignal&&) = delete;

  virtual void OnSignal(SigNumType signum) = 0;

  void OnRead() override final;
  void OnWrite() override final;

private:
  int CreateSignalFd(SigNumType signum);
};

class EventTimer : public EventHandle {
public:
  explicit EventTimer(EventLoop& loop, std::chrono::steady_clock::duration timeout,
                      std::chrono::steady_clock::duration repeat);
  virtual ~EventTimer() = default;

  EventTimer(const EventTimer&) = delete;
  EventTimer(EventTimer&&) = delete;
  EventTimer& operator=(const EventTimer&) = delete;
  EventTimer& operator=(EventTimer&&) = delete;

  virtual void OnTimer() = 0;

  void OnRead() override final;
  void OnWrite() override final;
};

class EventLoop {
public:
  explicit EventLoop();
  ~EventLoop() = default;

  EventLoop(const EventLoop&) = delete;
  EventLoop(EventLoop&&) = delete;
  EventLoop& operator=(const EventLoop&) = delete;
  EventLoop& operator=(EventLoop&&) = delete;

  void Run();
  void Stop();

  void AddHandle(EventHandle& handle);
  void ChangeHandle(EventHandle& handle);
  void RemoveHandle(EventHandle& handle);

private:
  class CloseEvent : public EventNotification {
  public:
    explicit CloseEvent(EventLoop& loop);
    ~CloseEvent() override = default;

    void OnNotification(uint64_t amount) override final;

  private:
    EventLoop& _Loop;
  };

  utils::FileHandle _EpollFd;
  CloseEvent _StopEvent;
  bool _Stopped = false;
};

} // namespace frontend::curses
