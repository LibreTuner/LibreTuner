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

#ifndef CANLOG_H
#define CANLOG_H

#include "caninterface.h"
#include <QAbstractTableModel>
#include <mutex>
#include <vector>

class CanLog : public QAbstractTableModel {
  Q_OBJECT
public:
  void addMessage(const CanMessage &message);

  size_t size() const { return messages_.size(); }

  const CanMessage &get(size_t index) const { return messages_[index]; }

  int columnCount(const QModelIndex &parent) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  int rowCount(const QModelIndex &parent) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role) const override;

  ~CanLog();

private:
  std::vector<CanMessage> messages_;
  mutable std::mutex mutex_;

signals:
  void dataChanged(int first, int last);
  void destructing();
};

#endif // CANLOG_H
