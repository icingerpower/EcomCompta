#include "DialogDisplayOrdersMissingReports.h"
#include "ui_DialogDisplayOrdersMissingReports.h"

//----------------------------------------------------------
DialogDisplayOrdersMissingReports::DialogDisplayOrdersMissingReports(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogDisplayOrdersMissingReports)
{
    ui->setupUi(this);
    ui->tableWidget->horizontalHeader()->resizeSection(1, 150);
}
//----------------------------------------------------------
DialogDisplayOrdersMissingReports::~DialogDisplayOrdersMissingReports()
{
    delete ui;
}
//----------------------------------------------------------
void DialogDisplayOrdersMissingReports::setOrderInfos(
        const QMap<QString, QList<QStringList> > &orders)
{
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(orders.size());
    int idRow = 0;
    for (auto it = orders.begin();
         it != orders.end();
         ++it) {
        QStringList elements = it.key().split(" ");
        ui->tableWidget->setItem(idRow, 0, new QTableWidgetItem(elements[0]));
        ui->tableWidget->setItem(idRow, 1, new QTableWidgetItem(elements[1]));
        QStringList reportSets;
        for (auto reportSet : it.value()) {
            reportSets << reportSet.join(tr(" ET ", " AND "));
        }
        QString reportDisplayText;
        if (reportSets.size() > 0) {
            reportDisplayText = "(" + reportSets.join(tr(") OU (", ") OR (")) + ")";
        } else {
            reportDisplayText = reportSets[0];
        }
        ui->tableWidget->setItem(idRow, 2, new QTableWidgetItem(reportDisplayText));
        ++idRow;
    }
}
//----------------------------------------------------------
