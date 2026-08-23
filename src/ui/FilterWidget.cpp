#include "ui/FilterWidget.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

FilterWidget::FilterWidget(QWidget *parent)
    : QWidget(parent)
{
    initializeUi();
}

QString FilterWidget::nameFilter() const
{
    return nameFilterLineEdit->text().trimmed();
}

QString FilterWidget::valueFilter() const
{
    return valueFilterLineEdit->text().trimmed();
}

bool FilterWidget::searchInNodeNames() const
{
    return searchInNodeNamesCheckBox->isChecked();
}

void FilterWidget::clearFilters()
{
    nameFilterLineEdit->clear();
    valueFilterLineEdit->clear();
}

void FilterWidget::slotClear()
{
    clearFilters();
}

void FilterWidget::initializeUi()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    QLabel *nameFilterLabel = new QLabel(QStringLiteral("Фильтр по названию"), this);

    nameFilterLineEdit = new QLineEdit(this);
    nameFilterLineEdit->setPlaceholderText(QStringLiteral("Текст"));

    QLabel *valueFilterLabel = new QLabel(QStringLiteral("Фильтр по значению"), this);

    valueFilterLineEdit = new QLineEdit(this);
    valueFilterLineEdit->setPlaceholderText(QStringLiteral("Число"));

    searchInNodeNamesCheckBox = new QCheckBox(QStringLiteral("Искать в названиях узлов"), this);
    searchInNodeNamesCheckBox->setChecked(true);

    clearFilterPushButton = new QPushButton(QStringLiteral("Очистить"), this);

    mainLayout->addWidget(nameFilterLabel);
    mainLayout->addWidget(nameFilterLineEdit);
    mainLayout->addWidget(valueFilterLabel);
    mainLayout->addWidget(valueFilterLineEdit);
    mainLayout->addWidget(searchInNodeNamesCheckBox);
    mainLayout->addWidget(clearFilterPushButton);
    mainLayout->addStretch();

    connect(nameFilterLineEdit,
            &QLineEdit::textChanged,
            this,
            &FilterWidget::signalFilterChanged);

    connect(valueFilterLineEdit,
            &QLineEdit::textChanged,
            this,
            &FilterWidget::signalFilterChanged);

    connect(searchInNodeNamesCheckBox,
            &QCheckBox::toggled,
            this,
            &FilterWidget::signalFilterChanged);

    connect(clearFilterPushButton,
            &QPushButton::clicked,
            this,
            &FilterWidget::slotClear);
}
