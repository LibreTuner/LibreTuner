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

#ifndef MOCKCANINTERFACE_H
#define MOCKCANINTERFACE_H

#include "caninterface.h"

/**
 * A loopback interface used for tests
 */
class MockCanInterface : public CanInterface {
public:
  /**
   * start does nothing in this case
   */
  virtual void start() override {}

  /**
   * this interface is always valid
   */
  virtual bool valid() override { return true; }

  /**
   * Fires the signal.
   */
  virtual void send(const CanMessage &message) override;


  virtual bool recv(CanMessage &message, std::chrono::milliseconds timeout) override;
};

#endif // MOCKCANINTERFACE_H
