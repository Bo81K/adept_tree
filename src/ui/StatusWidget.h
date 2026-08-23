#pragma once

#include <QWidget>

class QLabel;

struct TreeStatistics;

class StatusWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StatusWidget(QWidget *parent = nullptr);

    void setStatistics(const TreeStatistics &statistics);

private:
    void initializeUi();

    QLabel *countsLabel = nullptr;
    QLabel *maxValueLabel = nullptr;
    QLabel *minValueLabel = nullptr;
};
