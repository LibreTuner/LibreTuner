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

#include "logview.h"

LogView::LogView()
{

}

void LogView::setModel(QAbstractItemModel *model)
{
    if (model_) {
        disconnect(model_, &QAbstractItemModel::rowsInserted, this, &LogView::rowsInserted);
    }
    document()->clear();
    model_ = model;
    if (model) {
        connect(model_, &QAbstractItemModel::rowsInserted, this, &LogView::rowsInserted);
        rowsInserted(QModelIndex(), 0, model->rowCount() - 1);
    }
}



void LogView::rowsInserted(const QModelIndex &parent, int first, int last)
{
    if (parent.isValid()) {
        return;
    }
    for (int i = first; i <= last; ++i)
    {
        QModelIndex index = model_->index(i, 0, parent);

        QTextCharFormat format;

        QVariant foregroundColor = model_->data(index, Qt::TextColorRole);
        if (foregroundColor.isValid())
        {
            format.setForeground(foregroundColor.value<QColor>());
        }

        QVariant backgroundColor = model_->data(index, Qt::BackgroundColorRole);
        if (backgroundColor.isValid())
        {
            format.setBackground(backgroundColor.value<QColor>());
        }

        format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

        QString text = model_->data(index, Qt::DisplayRole).toString();

        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        if (i != 0)
        {
            cursor.insertBlock();
        }

        cursor.insertText(text, format);
    }

    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor, 1);
    setTextCursor(cursor);

 }
