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

#ifndef LIBRETUNER_TIMERRUNLOOP_H
#define LIBRETUNER_TIMERRUNLOOP_H

#include "timer.h"
#include <atomic>
#include <functional>
#include <mutex>
#include <set>

class Timer;

class TimerRunLoop
{
public:
    static TimerRunLoop & get();

    void addTimer(const std::shared_ptr<Timer> & timer);

    void removeTimer(const std::shared_ptr<Timer> & timer);

    void startWorker();
    void stopWorker();

    ~TimerRunLoop();

private:
    TimerRunLoop();

    void runLoop();

    struct TCompare
    {
        bool operator()(const std::weak_ptr<Timer> & first,
                        const std::weak_ptr<Timer> & second) const
        {
            if (auto pfirst = first.lock())
            {
                if (auto psecond = second.lock())
                {
                    return pfirst->nextTrigger() < psecond->nextTrigger();
                }
                else
                {
                    return false;
                }
            }
            return true;
        }
    };

    std::mutex mutex_;
    std::condition_variable wake_;
    std::multiset<std::weak_ptr<Timer>, TCompare> queue_;
    std::thread worker_;

    std::atomic<bool> running_;
};

#endif // LIBRETUNER_TIMERRUNLOOP_H
