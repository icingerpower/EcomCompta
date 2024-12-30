#ifndef DIALOGDIFFVATAMAZONUE_H
#define DIALOGDIFFVATAMAZONUE_H

#include <QDialog>

namespace Ui {
class DialogDiffVatAmazonUe;
}

class ModelDiffAmazonUE;

class DialogDiffVatAmazonUe : public QDialog
{
    Q_OBJECT

public:
    explicit DialogDiffVatAmazonUe(QWidget *parent = nullptr);
    ~DialogDiffVatAmazonUe();
    void setModelDiffAmazonUE(ModelDiffAmazonUE *model);
    void filter();
    void exportOrders();
    void resetFilter();

private:
    Ui::DialogDiffVatAmazonUe *ui;
};

#endif // DIALOGDIFFVATAMAZONUE_H
