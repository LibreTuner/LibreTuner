/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2018  <copyright holder> <email>
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
#include <mutex>
#include <thread>
#include <functional>

/**
 * @todo write docs
 */
class Timer {
public:
  using Callback = std::function<void()>;
  /* Called when the timer runs out */
  virtual void timedout() = 0;

  void setTimeout(std::chrono::milliseconds timeout) { timeout_ = timeout; }

  std::chrono::milliseconds timeout() const { return timeout_; }
  
  virtual ~Timer();

protected:
  /* Starts the timeout timer */
  void startTimer();
  /* Stops the timeout timer */
  void stopTimer();

private:
  void run();

  std::chrono::milliseconds timeout_;

  std::thread thread_;

  std::condition_variable cv_;
  std::mutex cv_m_;

  bool canceled_;
};

#endif // TIMED_H
