#ifndef DTCMODEL_H
#define DTCMODEL_H

#include "lt/diagnostics/codes.h"

#include <QAbstractTableModel>

class DtcModel : public QAbstractTableModel {
public:
    DtcModel() = default;

    inline void setCodes(lt::DiagnosticCodes &&codes) noexcept {
        codes_ = std::move(codes);
    }

private:
    lt::DiagnosticCodes codes_;

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role) const override;
};

#endif // DTCMODEL_H
