#include "Events.hpp"

#include <array>
#include <fcntl.h>
#include <signal.h>
#include <stdexcept>
#include <string.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>

#include "../../utils/Clock.hpp"
#include "../../utils/Error.hpp"

namespace frontend::curses {

// Handle
EventHandle::EventHandle(EventLoop& loop, int _Fd) : _Loop(loop), _Fd(_Fd) {
  PosixE(fcntl(_Fd, F_SETFL, PosixE(fcntl(_Fd, F_GETFL, 0)) | O_NONBLOCK));
  _Loop.AddHandle(*this);
}

EventHandle::~EventHandle() { _Loop.RemoveHandle(*this); }

void EventHandle::ScheduleRead() {
  _ReadScheduled = true;
  _Loop.ChangeHandle(*this);
}

void EventHandle::ScheduleWrite() {
  _WriteScheduled = true;
  _Loop.ChangeHandle(*this);
}

void EventHandle::OnEvent(uint32_t events) {
  if (events & EPOLL_EVENTS::EPOLLIN) {
    _ReadScheduled = false;
  }
  if (events & EPOLL_EVENTS::EPOLLOUT) {
    _WriteScheduled = false;
  }

  _Loop.ChangeHandle(*this);

  if (events & EPOLL_EVENTS::EPOLLIN) {
    OnRead();
  }
  if (events & EPOLL_EVENTS::EPOLLOUT) {
    OnWrite();
  }
}

// Notification
EventNotification::EventNotification(EventLoop& loop)
    : EventHandle(loop, PosixE(eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK))) {
  ScheduleRead();
}

void EventNotification::OnRead() {
  uint64_t amount;
  if (PosixE(read(_Fd, &amount, sizeof(amount)), [] { return errno == EWOULDBLOCK; }) <= 0) {
    return;
  }

  OnNotification(amount);
  ScheduleRead();
}

void EventNotification::OnWrite() {
  // This should never happen.
  throw std::runtime_error("EventNotification::OnWrite");
}

void EventNotification::Notify(uint64_t amount) { PosixE(write(_Fd, &amount, sizeof(amount))); }

// Signal
EventSignal::EventSignal(EventLoop& loop, SigNumType signum) : EventHandle(loop, PosixE(CreateSignalFd(signum))) {
  ScheduleRead();
}

void EventSignal::OnRead() {
  struct signalfd_siginfo info;
  if (PosixE(read(_Fd, &info, sizeof(info)), [] { return errno == EWOULDBLOCK; }) <= 0) {
    return;
  }

  OnSignal(info.ssi_signo);
  ScheduleRead();
}

void EventSignal::OnWrite() {
  // This should never happen.
  throw std::runtime_error("EventSignal::OnWrite");
}

int EventSignal::CreateSignalFd(SigNumType signum) {
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, signum);

  signal(signum, [](SigNumType) {});
  sigprocmask(SIG_BLOCK, &set, nullptr);

  return PosixE(signalfd(-1, &set, SFD_CLOEXEC | SFD_NONBLOCK));
}

// Timer
EventTimer::EventTimer(EventLoop& loop, std::chrono::steady_clock::duration timeout,
                       std::chrono::steady_clock::duration repeat)
    : EventHandle(loop, PosixE(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC))) {
  struct itimerspec timer;

  timer.it_value = utils::DurationToTimeSpec(timeout);
  timer.it_interval = utils::DurationToTimeSpec(repeat);

  timerfd_settime(_Fd, 0, &timer, nullptr);
  ScheduleRead();
}

void EventTimer::OnRead() {
  uint64_t amount;
  if (PosixE(read(_Fd, &amount, sizeof(amount)), [] { return errno == EWOULDBLOCK; }) <= 0) {
    return;
  }

  OnTimer();
  ScheduleRead();
}

void EventTimer::OnWrite() {
  // This should never happen.
  throw std::runtime_error("EventTimer::OnWrite");
}

// EventLoop
EventLoop::EventLoop() : _EpollFd(PosixE(epoll_create1(0))), _StopEvent(*this) {}

void EventLoop::Run() {
  while (!_Stopped) {
    std::array<struct epoll_event, 64> events;
    int nfds = PosixE(epoll_wait(_EpollFd, events.data(), events.size(), -1), [] { return errno == EINTR; });
    if (nfds < 0) {
      continue;
    }

    for (int i = 0; i < nfds; ++i) {
      auto& event = events[i];
      EventHandle* handle = reinterpret_cast<EventHandle*>(event.data.ptr);
      handle->OnEvent(event.events);
    }
  }
}

void EventLoop::Stop() {
  _Stopped = true;
  _StopEvent.Notify(1);
}

void EventLoop::AddHandle(EventHandle& handle) {
  struct epoll_event event;
  event.events = EPOLL_EVENTS::EPOLLET;
  event.data.ptr = &handle;
  PosixE(epoll_ctl(_EpollFd, EPOLL_CTL_ADD, handle._Fd, &event));
}

void EventLoop::ChangeHandle(EventHandle& handle) {
  struct epoll_event event;
  event.events = EPOLL_EVENTS::EPOLLET;
  event.data.ptr = &handle;
  if (handle._ReadScheduled) {
    event.events |= EPOLL_EVENTS::EPOLLIN;
  }
  if (handle._WriteScheduled) {
    event.events |= EPOLL_EVENTS::EPOLLOUT;
  }
  PosixE(epoll_ctl(_EpollFd, EPOLL_CTL_MOD, handle._Fd, &event));
}

void EventLoop::RemoveHandle(EventHandle& handle) { PosixE(epoll_ctl(_EpollFd, EPOLL_CTL_DEL, handle._Fd, nullptr)); }

EventLoop::CloseEvent::CloseEvent(EventLoop& loop) : EventNotification(loop), _Loop(loop) {}

void EventLoop::CloseEvent::OnNotification(uint64_t amount) {}

} // namespace frontend::curses
