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

#include "verticallabel.h"

#include <QPainter>
#include <QStyle>

VerticalLabel::VerticalLabel(QWidget *parent) : QLabel(parent) {}

VerticalLabel::VerticalLabel(const QString &text, QWidget *parent)
    : QLabel(text, parent) {}

void VerticalLabel::paintEvent(QPaintEvent *) {
  QPainter painter(this);
  painter.setPen(palette().color(QPalette::Foreground));
  painter.setBrush(Qt::Dense1Pattern);

  painter.translate(0, height());
  painter.rotate(270);

  painter.drawText(0, 0, height(), width(), alignment(), text());
}

QSize VerticalLabel::minimumSizeHint() const {
  QSize s = QLabel::minimumSizeHint();
  return {s.height(), s.width()};
}

QSize VerticalLabel::sizeHint() const {
  QSize s = QLabel::sizeHint();
  return {s.height(), s.width()};
}
