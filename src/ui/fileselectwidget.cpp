#include "fileselectwidget.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>

FileSelectWidget::FileSelectWidget(const QString & caption,
                                   const QString & filter, Mode mode,
                                   QWidget * parent)
    : QWidget(parent), caption_(caption), filter_(filter), mode_(mode)
{
    linePath_ = new QLineEdit;
    linePath_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(linePath_, &QLineEdit::textEdited, this,
            [this](const QString & text) { emit pathChanged(text); });

    buttonBrowse_ = new QPushButton(tr("Browse"));
    connect(buttonBrowse_, &QPushButton::clicked, this,
            &FileSelectWidget::browse);

    auto * layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(linePath_);
    layout->addWidget(buttonBrowse_);
    setLayout(layout);
}

void FileSelectWidget::setPath(const QString & path) noexcept
{
    linePath_->setText(path);
}

QString FileSelectWidget::path() const noexcept { return linePath_->text(); }

void FileSelectWidget::browse()
{
    QString path;
    if (mode_ == MODE_OPEN)
    {
        path = QFileDialog::getOpenFileName(this, caption_, QString(), filter_);
    }
    else if (mode_ == MODE_SAVE)
    {
        path = QFileDialog::getSaveFileName(this, caption_, QString(), filter_);
    }
    if (!path.isNull())
    {
        linePath_->setText(path);
        emit pathChanged(path);
    }
}
