include(orderimporters/orderimporters.pri)
include(utils/utils.pri)
include(vat/vat.pri)
include(bookkeeping/bookkeeping.pri)
include(reports/reports.pri)
include(inventory/inventory.pri)

HEADERS += \
    $$PWD/ChangeNotifier.h \
    $$PWD/Customer.h \
    $$PWD/CustomerManager.h \
    $$PWD/LogVat.h \
    $$PWD/SettingManager.h \
    $$PWD/UpdateToCustomer.h

SOURCES += \
    $$PWD/ChangeNotifier.cpp \
    $$PWD/Customer.cpp \
    $$PWD/CustomerManager.cpp \
    $$PWD/LogVat.cpp \
    $$PWD/SettingManager.cpp \
    $$PWD/UpdateToCustomer.cpp
