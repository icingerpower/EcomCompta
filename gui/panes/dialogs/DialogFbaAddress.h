#ifndef DIALOGFBAADDRESS_H
#define DIALOGFBAADDRESS_H

#include <QDialog>
#include <model/orderimporters/Address.h>

namespace Ui {
class DialogFbaAddress;
}

class DialogFbaAddress : public QDialog
{
    Q_OBJECT

public:
    explicit DialogFbaAddress(QWidget *parent = nullptr);
    ~DialogFbaAddress() override;
    Address getAddress() const;
    void clear();

public slots:
    void accept() override;

private:
    Ui::DialogFbaAddress *ui;
};

#endif // DIALOGFBAADDRESS_H
