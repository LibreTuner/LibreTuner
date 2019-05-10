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

namespace lt {

class AsyncRoutine {
public:
    using ProgressCallback = std::function<void(float progress)>;

    inline void setProgressCallback(ProgressCallback &&cb);

protected:
    /* Used to safely call callbacks */
    inline void notifyProgress(float progress);

private:
    ProgressCallback progressCallback_;
};

void AsyncRoutine::notifyProgress(float progress) {
    if (progressCallback_) {
        progressCallback_(progress);
    }
}

void AsyncRoutine::setProgressCallback(AsyncRoutine::ProgressCallback &&cb) {
    progressCallback_ = std::move(cb);
}

} // namespace lt

#endif // ASYNCROUTINE_H
