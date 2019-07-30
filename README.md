LibreTuner
=========
Open source engine tuning software for Mazda platforms

Interested in contributing? [Join the Discord server](https://discord.gg/QQvX2rB)


Screenshots
-----------
![Main Window](https://user-images.githubusercontent.com/3116133/62159853-ab6d5b80-b2e0-11e9-931d-1b15481e83fb.png)

Supported vehicles
------------------
* Mazdaspeed6 / Mazda 6 MPS / Mazdaspeed Atenza

Planned support
---------------
* Mazdaspeed3 (Gen 1 & 2)
* First Generation RX-8


Layout
------
All interesting code lives at `lib/LibLibreTuner`.

Building
--------
### Requirements
* Qt5
* CMake 3.10 or higher
* A compiler that supports C++17

### Build instructions

#### Linux
1. `git clone https://github.com/Libretuner/LibreTuner.git`
2. `cd LibreTuner`
3. `git submodule update --init --recursive`
4. `cd LibreTuner`
5. `cmake .`
6. `make`
7. `./LibreTuner`

#### Windows
TODO. (Use Qt Creator with MSVC)