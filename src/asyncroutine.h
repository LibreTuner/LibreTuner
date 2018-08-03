/*
 * LibreTuner
 * Copyright (C) 2018 Altenius
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

#ifndef ASYNCROUTINE_H
#define ASYNCROUTINE_H

#include <functional>
#include <string>

class AsyncRoutine
{
public:
    using ProgressCallback = std::function<void(float progress)>;
    using ErrorCallback = std::function<void(const std::string &error)>;
    using CompleteCallback = std::function<void()>;

    inline void setProgressCallback(ProgressCallback &&cb);
    inline void setErrorCallback(ErrorCallback &&cb);
    inline void setCompleteCallback(CompleteCallback &&cb);

protected:
  /* Used to safely call callbacks */
  inline void notifyProgress(float progress);
  inline void notifyError(const std::string &error);
  inline void notifyComplete();

private:
  ProgressCallback progressCallback_;
  ErrorCallback errorCallback_;
  CompleteCallback completeCallback_;
};


void AsyncRoutine::notifyProgress(float progress)
{
    if (progressCallback_) {
        progressCallback_(progress);
    }
}

void AsyncRoutine::notifyError(const std::string &error)
{
    if (errorCallback_) {
        errorCallback_(error);
    }
}

void AsyncRoutine::notifyComplete()
{
    if (completeCallback_) {
        completeCallback_();
    }
}

void AsyncRoutine::setProgressCallback(AsyncRoutine::ProgressCallback &&cb)
{
    progressCallback_ = std::move(cb);
}

void AsyncRoutine::setErrorCallback(AsyncRoutine::ErrorCallback &&cb)
{
    errorCallback_ = std::move(cb);
}

void AsyncRoutine::setCompleteCallback(AsyncRoutine::CompleteCallback &&cb)
{
    completeCallback_ = std::move(cb);
}


#endif // ASYNCROUTINE_H
