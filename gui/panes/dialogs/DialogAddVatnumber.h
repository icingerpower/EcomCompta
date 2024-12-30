#ifndef DIALOGADDVATNUMBER_H
#define DIALOGADDVATNUMBER_H

#include <QDialog>
#include "model/vat/VatNumbersModel.h"

namespace Ui {
class DialogAddVatnumber;
}

class DialogAddVatnumber : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAddVatnumber(QWidget *parent = nullptr);
    ~DialogAddVatnumber() override;
    VatNumberData getVatNumber() const;

public slots:
    void clear();
    void accept() override;

private:
    Ui::DialogAddVatnumber *ui;
};

#endif // DIALOGADDVATNUMBER_H
