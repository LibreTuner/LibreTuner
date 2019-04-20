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

#include "libretuner.h"

#include "lt/serialize/serialize.h"
#include "lt/serialize/sinks.h"

#include <csignal>
#include <iostream>
#include <algorithm>

int main(int argc, char *argv[]) {
    LibreTuner app(argc, argv);

    std::vector<char> into;
    lt::Serializer<lt::VectorSink<std::vector<char>>> serializer(into);

    std::vector<char> test = {1, 2, 3, 4, 5};

    serializer.serialize(test);

    std::cout << std::accumulate(std::next(into.begin()), into.end(), std::to_string(into[0]), [](std::string s, int n) { return std::move(s) + ' ' + std::to_string(n); }) << std::endl;
    
    return LibreTuner::exec();
}
