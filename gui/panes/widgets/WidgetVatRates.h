#ifndef WIDGETVATRATES_H
#define WIDGETVATRATES_H

#include <QWidget>

class VatRatesModel;
class VatRatesModelDates;

namespace Ui {
class WidgetVatRates;
}

class WidgetVatRates : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetVatRates(QWidget *parent = nullptr);
    ~WidgetVatRates();
    void setModels(VatRatesModel *ratesModel,
                   VatRatesModelDates *ratesModelDate);
    void add();
    void remove();

private:
    Ui::WidgetVatRates *ui;
    VatRatesModelDates *m_modelVatDates;
};

#endif // WIDGETVATRATES_H
