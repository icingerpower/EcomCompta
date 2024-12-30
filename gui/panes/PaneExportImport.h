#ifndef PANEEXPORTIMPORT_H
#define PANEEXPORTIMPORT_H

#include <QWidget>

namespace Ui {
class PaneExportImport;
}

class PaneExportImport : public QWidget
{
    Q_OBJECT

public:
    explicit PaneExportImport(QWidget *parent = nullptr);
    ~PaneExportImport();

private:
    Ui::PaneExportImport *ui;
};

#endif // PANEEXPORTIMPORT_H
