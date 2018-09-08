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

#ifndef J2534MANAGER_H
#define J2534MANAGER_H

#include <memory>
#include <vector>

namespace j2534 {
class J2534;
using J2534Ptr = std::shared_ptr<J2534>;
} // namespace j2534

class J2534Manager {
public:
    J2534Manager();
    J2534Manager(J2534Manager &&) = delete;
    J2534Manager(const J2534Manager &) = delete;

    // Returns the singleton
    static J2534Manager &get();

    void init();

    // Empties the vector of discovered interfaces and repopulates.
    void load_interfaces();

    const std::vector<j2534::J2534Ptr> &interfaces() const {
        return interfaces_;
    }

private:
    std::vector<j2534::J2534Ptr> interfaces_;
};

#endif // J2534MANAGER_H
