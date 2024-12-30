#ifndef REPORTSTOCKDEPORTED_H
#define REPORTSTOCKDEPORTED_H

#include "AbstractReportGenerator.h"

class ReportStockDeported : public AbstractReportGenerator
{
public:
    ReportStockDeported();
    void addTable();
    void addTableRow(const QString &code, int units, double unitPrice);
    void addTableTotal(double totalPrice);
};

#endif // REPORTSTOCKDEPORTED_H
