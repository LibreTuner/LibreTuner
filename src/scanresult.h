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

#ifndef SCANRESULT_H
#define SCANRESULT_H

#include <QAbstractTableModel>

struct DiagnosticCode {
    uint16_t code;
    std::string description;

    std::string codeString() const;
};

class ScanResult : public QAbstractTableModel
{
public:
    ScanResult();

    void add(DiagnosticCode &&code) { beginInsertRows(QModelIndex(), codes_.size(), codes_.size()); codes_.emplace_back(std::move(code)); endInsertRows(); }

    void clear() { codes_.clear(); }

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    std::vector<DiagnosticCode> codes_;
};

#endif // SCANRESULT_H
