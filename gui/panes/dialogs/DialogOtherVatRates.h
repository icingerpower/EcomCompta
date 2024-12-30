#ifndef DIALOGOTHERVATRATES_H
#define DIALOGOTHERVATRATES_H

#include <QDialog>

namespace Ui {
class DialogOtherVatRates;
}

class DialogOtherVatRates : public QDialog
{
    Q_OBJECT

public:
    explicit DialogOtherVatRates(QWidget *parent = nullptr);
    ~DialogOtherVatRates() override;
    QString getName() const;
    void clear();

public slots:
    void accept() override;

private:
    Ui::DialogOtherVatRates *ui;
};

#endif // DIALOGOTHERVATRATES_H
