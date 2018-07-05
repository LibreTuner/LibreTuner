/*
 * LibreTuner
 * Copyright (C) 2018  Altenius
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <atomic>
#include <memory>

class TimerRunLoop;

/**
 * @todo write docs
 */
class Timer : public std::enable_shared_from_this<Timer> {
  friend TimerRunLoop;
public:
  using Callback = std::function<void()>;

  static std::shared_ptr<Timer> create();
  static std::shared_ptr<Timer> create(Callback &&cb);

  void setCallback(Callback &&cb);
  void setTimeout(std::chrono::milliseconds timeout);
  std::chrono::milliseconds timeout() const { return timeout_; }
  /* Starts the timeout timer */
  void enable();
  /* Stops the timeout timer */
  void disable();

  bool active() const;
  bool running() const;

  std::chrono::steady_clock::time_point nextTrigger() const;

  ~Timer();

  Timer(const Timer&) = delete;
  Timer(Timer &&) = delete;

protected:
  bool tryTrigger();
  void trigger();

  Timer() = default;
  explicit Timer(Callback &&cb);

private:
  std::chrono::milliseconds timeout_;

  std::chrono::steady_clock::time_point nextTrigger_;

  std::mutex mutex_;

  Callback callback_;

  // true if the timer is waiting
  bool active_;
  // true if the timer is currently being triggered
  std::atomic<bool> running_;
};
using TimerPtr = std::shared_ptr<Timer>;

#endif // TIMED_H
