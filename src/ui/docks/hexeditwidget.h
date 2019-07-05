#ifndef LIBRETUNER_HEXEDITWIDGET_H
#define LIBRETUNER_HEXEDITWIDGET_H

#include <QWidget>
#include <qhexview.h>

#include "lt/rom/rom.h"

class RomHexBuffer : public QHexBuffer
{
    Q_OBJECT
public:
    explicit RomHexBuffer(lt::RomPtr rom = lt::RomPtr()) : rom_(std::move(rom))
    {
    }

    int length() const override;
    void insert(int offset, const QByteArray & data) override;
    void remove(int offset, int length) override;
    QByteArray read(int offset, int length) override;
    void read(QIODevice * iodevice) override;
    void write(QIODevice * iodevice) override;

    inline void setRom(lt::RomPtr rom) noexcept { rom_ = std::move(rom); }

private:
    lt::RomPtr rom_;
};

class HexEditWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HexEditWidget(QWidget * parent = nullptr);

    inline void setRom(lt::RomPtr rom) noexcept
    {
        buffer_->setRom(std::move(rom));
    }

private:
    RomHexBuffer * buffer_;
};

#endif // LIBRETUNER_HEXEDITWIDGET_H
