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

#include "timer.h"

Timer::Timer(Timer::Callback &&cb) : callback_(std::move(cb)) {

}

void Timer::run() {
  std::unique_lock<std::mutex> lk(cv_m_);
  if (!cv_.wait_for(lk, timeout_, [this]() { return canceled_; })) {
    if (!canceled_) {
      if (callback_) {
        callback_();
      }
    }
  }
}

void Timer::setCallback(Timer::Callback &&cb) {
  callback_ = std::move(cb);
}

Timer::~Timer()
{
  stop();
}

void Timer::start() {
  stop();

  canceled_ = false;
  thread_ = std::thread(&Timer::run, this);
}

void Timer::stop() {
  if (thread_.joinable()) {
    {
      std::lock_guard<std::mutex> lk(cv_m_);
      canceled_ = true;
    }

    cv_.notify_all();
    thread_.join();
  }
}
