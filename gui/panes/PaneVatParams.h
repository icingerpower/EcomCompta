#ifndef PANEVATPARAMS_H
#define PANEVATPARAMS_H

#include <QWidget>
#include <QtCore/qitemselectionmodel.h>

class DialogOtherVatRates;

namespace Ui {
class PaneVatParams;
}

class PaneVatParams : public QWidget
{
    Q_OBJECT

public:
    explicit PaneVatParams(QWidget *parent = nullptr);
    ~PaneVatParams();

public slots:
    //void loadCustomer(const QString &name);
    void addOtherRates();
    void removeSelectedOtherRates();
    void addSku();
    void loadYearSkus();
    void removeSelectedSkus();
    void removeSkusFromRight();
    void addSkusToRight();
    void displaySelOtherRates(
            QItemSelection newSelection, QItemSelection previousSelection);

private:
    Ui::PaneVatParams *ui;
    void _connectSlots();
    DialogOtherVatRates *m_dialogOtherVatRates;
    void _hideOtherRatesFields();
    void _showOtherRatesFields();
};

#endif // PANEVATPARAMS_H
