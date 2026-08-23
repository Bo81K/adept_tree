#pragma once

#include <QWidget>

class QCheckBox;
class QLineEdit;
class QPushButton;

class FilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FilterWidget(QWidget *parent = nullptr);

    QString nameFilter() const;
    QString valueFilter() const;
    bool searchInNodeNames() const;

    void clearFilters();

signals:
    void signalFilterChanged();

private slots:
    void slotClear();

private:
    void initializeUi();

    QLineEdit *nameFilterLineEdit = nullptr;
    QLineEdit *valueFilterLineEdit = nullptr;
    QCheckBox *searchInNodeNamesCheckBox = nullptr;
    QPushButton *clearFilterPushButton = nullptr;
};
