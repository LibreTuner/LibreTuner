#ifndef ROMEXPLORER_H
#define ROMEXPLORER_H

#include <QWidget>

/**
 * A interface for exploring an image and adding tables.
 */
class RomExplorer : public QWidget
{
    Q_OBJECT
public:
    explicit RomExplorer(QWidget * parent = nullptr);
};

#endif // ROMEXPLORER_H
