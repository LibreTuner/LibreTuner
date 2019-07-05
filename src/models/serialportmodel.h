#ifndef LIBRETUNER_SERIALPORTMODEL_H
#define LIBRETUNER_SERIALPORTMODEL_H

#include <QAbstractListModel>

#include <string>
#include <vector>

class SerialPortModel : public QAbstractListModel
{
public:
    explicit SerialPortModel(QObject * parent = nullptr);

    QModelIndex parent(const QModelIndex & child) const override;

    int rowCount(const QModelIndex & parent) const override;

    int columnCount(const QModelIndex & parent) const override;

    QVariant data(const QModelIndex & index, int role) const override;

    template <typename T> void setPorts(T && t)
    {
        beginResetModel();
        ports_ = std::forward<T>(t);
        endResetModel();
    }

private:
    std::vector<std::string> ports_;
};

#endif // LIBRETUNER_SERIALPORTMODEL_H
