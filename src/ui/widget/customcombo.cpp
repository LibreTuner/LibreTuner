#include "customcombo.h"

#include <QAbstractProxyModel>
#include <QComboBox>
#include <QLineEdit>
#include <QVBoxLayout>

class CustomComboProxy : public QAbstractProxyModel
{
public:
    explicit CustomComboProxy(QObject * parent = nullptr)
        : QAbstractProxyModel(parent)
    {
    }

    QModelIndex mapToSource(const QModelIndex & proxyIndex) const override
    {
        if ((proxyIndex.row() == 0 && proxyIndex.column() == 0) ||
            sourceModel() == nullptr)
        {
            return QModelIndex();
        }

        return sourceModel()->index(proxyIndex.row() - 1, proxyIndex.column());
    }

    QModelIndex mapFromSource(const QModelIndex & sourceIndex) const override
    {
        return createIndex(sourceIndex.row() + 1, sourceIndex.column(),
                           sourceIndex.internalPointer());
    }

    QVariant data(const QModelIndex & proxyIndex, int role) const override
    {
        if (proxyIndex.row() == 0 && proxyIndex.column() == 0 &&
            role == Qt::DisplayRole)
        {
            return tr("Custom");
        }

        return QAbstractProxyModel::data(proxyIndex, role);
    }

    Qt::ItemFlags flags(const QModelIndex & index) const override
    {
        if (index.row() == 0 && index.column() == 0)
        {
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        }
        return QAbstractProxyModel::flags(index);
    }

    QModelIndex index(int row, int column,
                      const QModelIndex & parent) const override
    {
        if (sourceModel() == nullptr || parent.isValid() ||
            row >= rowCount(QModelIndex()))
        {
            return QModelIndex();
        }
        if (row == 0 && column == 0)
        {
            return createIndex(0, 0, nullptr);
        }
        return mapFromSource(sourceModel()->index(row - 1, column));
    }

    QModelIndex parent(const QModelIndex & /*child*/) const override
    {
        return QModelIndex();
    }

    int rowCount(const QModelIndex & parent) const override
    {
        if (sourceModel() == nullptr || parent.isValid())
        {
            return 0;
        }
        return sourceModel()->rowCount(QModelIndex()) + 1;
    }

    int columnCount(const QModelIndex & /*parent*/) const override { return 1; }

    void resetProxy()
    {
        beginResetModel();
        endResetModel();
    }

    void setSourceModel(QAbstractItemModel * sourceModel) override
    {
        if (this->sourceModel() != nullptr)
        {
            disconnect(this->sourceModel(), &QAbstractItemModel::modelReset,
                       this, &CustomComboProxy::resetProxy);
        }
        beginResetModel();
        QAbstractProxyModel::setSourceModel(sourceModel);
        endResetModel();

        if (sourceModel != nullptr)
        {
            connect(sourceModel, &QAbstractItemModel::modelReset, this,
                    &CustomComboProxy::resetProxy);
        }
    }
};

CustomCombo::CustomCombo(QWidget * parent) : QWidget(parent)
{
    model_ = new CustomComboProxy(this);

    combo_ = new QComboBox;
    combo_->setModel(model_);
    line_ = new QLineEdit;
    line_->setEnabled(false);

    auto * layout = new QVBoxLayout;
    layout->addWidget(combo_);
    layout->addWidget(line_);
    layout->setContentsMargins(0, 0, 0, 0);

    setLayout(layout);

    connect(combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int index) {
                line_->setEnabled(index == 0);
                emit valueChanged();
            });

    connect(line_, &QLineEdit::textChanged,
            [this](const QString & /*s*/) { emit valueChanged(); });
}

void CustomCombo::setModel(QAbstractItemModel * model)
{
    model_->setSourceModel(model);
}

QString CustomCombo::value()
{
    if (combo_->currentIndex() == 0)
    {
        return line_->text();
    }

    QVariant var = combo_->currentData(Qt::UserRole);
    return var.toString();
}

void CustomCombo::setValue(const QString & value)
{
    // Check model for value
    for (int i = 0; i < model_->rowCount(QModelIndex()); ++i)
    {
        if (model_->data(model_->index(i, 0, QModelIndex()), Qt::UserRole)
                .toString() == value)
        {
            combo_->setCurrentIndex(i);
            line_->clear();
            return;
        }
    }
    combo_->setCurrentIndex(0);
    line_->setText(value);
}
