#include "hexeditwidget.h"

#include <QByteArray>
#include <QHBoxLayout>
#include <QIODevice>

/*
HexEditWidget::HexEditWidget(QWidget * parent) : QWidget(parent)
{
    buffer_ = new RomHexBuffer;

    auto *document = new QHexDocument(buffer_);
    auto *hexView = new QHexView;
    hexView->setDocument(document);

    auto *layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(hexView);

    setLayout(layout);
}

int RomHexBuffer::length() const
{
    if (!rom_)
    {
        return 0;
    }

    return rom_->size();
}

void RomHexBuffer::insert(int offset, const QByteArray & data) {}

void RomHexBuffer::remove(int offset, int length) {}

QByteArray RomHexBuffer::read(int offset, int length)
{
    if (!rom_)
    {
        return QByteArray();
    }
    if (offset < 0)
    {
        return QByteArray();
    }
    if (offset >= rom_->size())
    {
        return QByteArray();
    }
    int size = std::min<int>(rom_->size() - offset, length);

    return QByteArray(reinterpret_cast<const char *>(&rom_->data()[offset]),
                      size);
}

void RomHexBuffer::read(QIODevice * iodevice) {}

void RomHexBuffer::write(QIODevice * iodevice)
{
    if (!rom_)
    {
        return;
    }
    iodevice->write(reinterpret_cast<const char *>(rom_->data()), rom_->size());
}*/
