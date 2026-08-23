#include "ui/StatusWidget.h"
#include "service/TreeService.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QString>

StatusWidget::StatusWidget(QWidget *parent)
    : QWidget(parent)
{
    initializeUi();
}

void StatusWidget::setStatistics(const TreeStatistics &statistics)
{
    countsLabel->setText(QStringLiteral("Узлов: %1, листьев: %2")
                             .arg(statistics.nodeCount)
                             .arg(statistics.leafCount));

    if (statistics.hasLeafValues)
    {
        maxValueLabel->setText(QStringLiteral("Макс: %1")
                                   .arg(QString::number(statistics.maxValue, 'g', 12)));
        minValueLabel->setText(QStringLiteral("Мин: %1")
                                   .arg(QString::number(statistics.minValue, 'g', 12)));
    }
    else
    {
        maxValueLabel->setText(QStringLiteral("Макс: -"));
        minValueLabel->setText(QStringLiteral("Мин: -"));
    }
}

void StatusWidget::initializeUi()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    countsLabel = new QLabel(this);
    maxValueLabel = new QLabel(this);
    minValueLabel = new QLabel(this);

    mainLayout->addWidget(countsLabel);
    mainLayout->addWidget(maxValueLabel);
    mainLayout->addWidget(minValueLabel);
    mainLayout->addStretch();
}
