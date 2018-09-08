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

#ifndef LIBRETUNER_SETTINGSWIDGET_H
#define LIBRETUNER_SETTINGSWIDGET_H

#include "interface.h"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>

class SettingsWidget : public QWidget {
    Q_OBJECT
public:
    explicit SettingsWidget(QWidget *parent = nullptr);

    virtual InterfaceSettingsPtr settings() = 0;

    virtual void setSettings(const InterfaceSettingsPtr &ptr) = 0;

    /* Creates a new settings UI for the given type. Returns nullptr
     * if the given type is unsupported. */
    static std::unique_ptr<SettingsWidget> create(InterfaceType type);

    static std::unique_ptr<SettingsWidget>
    create(const InterfaceSettingsPtr &settings);

private slots:
    void on_nameChanged(const QString &text);

protected:
    QFormLayout *layout_;
    QLabel *labelError_;

    virtual void setName(const std::string &name) = 0;
    virtual std::string name() = 0;
    void updateUi();

    bool checkName();
    void displayError(const QString &text);

private:
    QLineEdit *editName_;
};

#endif // LIBRETUNER_SETTINGSWIDGET_H
