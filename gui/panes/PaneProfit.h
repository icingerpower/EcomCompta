#ifndef PANEPROFIT_H
#define PANEPROFIT_H

#include <QWidget>
#include <QItemSelectionModel>

namespace Ui {
class PaneProfit;
}

class PaneProfit : public QWidget
{
    Q_OBJECT

public:
    explicit PaneProfit(QWidget *parent = nullptr);
    ~PaneProfit();

public slots:
    void compute();

    void loadYearSkus();
    void addSku();
    void filterSkus();
    void filterResetSkus();

    void removeGroup();
    void saveInCurrentGroup();
    void saveInNewGroup();


private slots:
    void _onGroupSelected(const QItemSelection &selected,
                          const QItemSelection &unselected);


private:
    Ui::PaneProfit *ui;
    void _connectSlots();
    void _displayChartData(
            const QMap<QDate, int> &chartSales,
            const QMap<QDate, int> &chartRefunds);
};

#endif // PANEPROFIT_H
