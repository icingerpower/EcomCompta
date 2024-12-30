#ifndef DIALOGADDSERVICESALE_H
#define DIALOGADDSERVICESALE_H

#include <QDialog>
#include <QDate>

namespace Ui {
class DialogAddServiceSale;
}

class DialogAddServiceSale : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAddServiceSale(QWidget *parent = nullptr);
    ~DialogAddServiceSale();
    void init(
        const QDate &date,
        const QString &label,
        double amount,
        const QString &currency);
    bool wasAccepted();
    QDate getDate() const;
    QString getReference() const;
    QString getCurrency() const;
    double getAmountUnit() const;
    double getUnits() const;
    QString getTitle() const;

public slots:
    void accept();
    void reject();

private:
    Ui::DialogAddServiceSale *ui;
    bool m_accepted;
    QDate m_origDate;
};

#endif // DIALOGADDSERVICESALE_H
