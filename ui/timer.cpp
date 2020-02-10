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
#include "timerrunloop.h"

struct make_shared_enabler : public Timer
{
    template <typename... Args>
    explicit make_shared_enabler(Args &&... args)
        : Timer(std::forward<Args>(args)...)
    {
    }
};

Timer::Timer(Timer::Callback && cb) : callback_(std::move(cb)), active_(false)
{
}

std::chrono::steady_clock::time_point Timer::nextTrigger() const
{
    return nextTrigger_;
}

void Timer::setCallback(Timer::Callback && cb) { callback_ = std::move(cb); }

void Timer::setTimeout(std::chrono::milliseconds timeout)
{
    timeout_ = timeout;
    if (active_)
    {
        // Reinsert
        enable();
    }
}

Timer::~Timer()
{
    // std::lock_guard<std::mutex> lk(mutex_);
}

void Timer::enable()
{
    std::lock_guard<std::mutex> lk(mutex_);
    nextTrigger_ = std::chrono::steady_clock::now() + timeout_;
    if (active_)
    {
        // nextTrigger may have changed, so remove and reinsert
        TimerRunLoop::get().removeTimer(shared_from_this());
    }
    active_ = true;
    TimerRunLoop::get().addTimer(shared_from_this());
}

void Timer::disable()
{
    std::lock_guard<std::mutex> lk(mutex_);
    if (!active_)
    {
        return;
    }
    active_ = false;
    TimerRunLoop::get().removeTimer(shared_from_this());
}

bool Timer::active() const { return active_; }

bool Timer::running() const { return running_; }

bool Timer::tryTrigger()
{
    if (nextTrigger_ < std::chrono::steady_clock::now())
    {
        trigger();
        return true;
    }
    return false;
}

void Timer::trigger()
{
    // Keep the timer alive
    auto self = shared_from_this();

    running_ = true;
    if (callback_)
    {
        callback_();
    }
    disable();
    running_ = false;
}

TimerPtr Timer::create() { return std::make_shared<Timer>(); }

TimerPtr Timer::create(Timer::Callback && cb)
{
    return std::make_shared<Timer>(std::move(cb));
}
