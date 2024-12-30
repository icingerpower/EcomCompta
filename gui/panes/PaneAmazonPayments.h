#ifndef PANEAMAZONPAYMENTS_H
#define PANEAMAZONPAYMENTS_H

#include <QWidget>
#include <QtCore/qitemselectionmodel.h>

namespace Ui {
class PaneAmazonPayments;
}

class OrderManager;
class PaneAmazonPayments : public QWidget
{
    Q_OBJECT

public:
    explicit PaneAmazonPayments(QWidget *parent = nullptr);
    ~PaneAmazonPayments() override;

public slots:
    void displayReport(const QItemSelection &newSelection,
                       const QItemSelection &previousSelection);
    void setVisible(bool visible) override;
    void compute();

private:
    Ui::PaneAmazonPayments *ui;
    void _connectSlots();
    OrderManager *m_orderManager;
    //bool m_wasVisible;
    QSet<int> m_yearsComputed;
};

#endif // PANEAMAZONPAYMENTS_H
