#ifndef DIALOGDISPLAYORDERSMISSINGREPORTS_H
#define DIALOGDISPLAYORDERSMISSINGREPORTS_H

#include <QDialog>

namespace Ui {
class DialogDisplayOrdersMissingReports;
}

class DialogDisplayOrdersMissingReports : public QDialog
{
    Q_OBJECT

public:
    explicit DialogDisplayOrdersMissingReports(QWidget *parent = nullptr);
    ~DialogDisplayOrdersMissingReports();
    void setOrderInfos(const QMap<QString, QList<QStringList> > &orders);

private:
    Ui::DialogDisplayOrdersMissingReports *ui;
};

#endif // DIALOGDISPLAYORDERSMISSINGREPORTS_H
