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

#include "sockethandler.h"
#include <algorithm>
#include <cassert>

#include <iostream>

SocketHandler::SocketHandler() {}

SocketHandler::~SocketHandler() {
  if (thread_.joinable()) {
    running_ = false;
    thread_.join();
  }
}

SocketHandler *SocketHandler::get() {
  static SocketHandler gSocketHandler_;
  return &gSocketHandler_;
}

void SocketHandler::addSocket(Socket *socket) {
  std::unique_lock<std::mutex> lk(cv_m_);
  sockets_.push_back(socket);
  runLooped_ = false;
  cv_.wait(lk, [this] { return runLooped_ == true; });
}

void SocketHandler::removeSocket(Socket *socket) {
  std::replace(sockets_.begin(), sockets_.end(), socket,
               static_cast<Socket *>(nullptr));
}

void SocketHandler::initialize() {
  assert(!thread_.joinable());
  running_ = true;
  thread_ = std::thread(&SocketHandler::run, this);
}

void SocketHandler::run() {
  fd_set rdfds;
  timeval timeout;
  while (running_) {
    timeout.tv_sec = 0;
    timeout.tv_usec = 500000;
    FD_ZERO(&rdfds);

    int nfds = 0;

    for (Socket *socket : sockets_) {
      if (socket == nullptr) {
        continue;
      }
      FD_SET(socket->fd(), &rdfds);
      if (socket->fd() > nfds) {
        nfds = socket->fd();
      }
    }

    int res = select(nfds + 1, &rdfds, nullptr, nullptr, &timeout);
    if (res == -1) {
      break;
    }

    {
      // trigger runLooped_ for the addSocket() block
      std::lock_guard<std::mutex> lk(cv_m_);
      runLooped_ = true;
    }
    cv_.notify_all();

    if (res == 0) {
      // The timeout expired. The sets will be empty
      continue;
    }

    for (Socket *socket : sockets_) {
      if (socket == nullptr) {
        continue;
      }
      if (FD_ISSET(socket->fd(), &rdfds)) {
        try {
          socket->onRead();
        } catch (const std::exception &e) {
          // std::cerr << "Exception during Socket::onRead(): " << e.what() <<
          // std::endl;
        } catch (...) {
          // std::cerr << "Exception during Socket::onRead()" << std::endl;
        }
      }
    }
  }
}
