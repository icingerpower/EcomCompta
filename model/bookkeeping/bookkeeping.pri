include(entries/entries.pri)
include(invoices/invoices.pri)

HEADERS += \
    $$PWD/ExceptionAccountDeportedMissing.h \
    $$PWD/ExceptionAccountSaleMissing.h \
    $$PWD/ManagerAccountPurchase.h \
    $$PWD/ManagerAccountsAmazon.h \
    $$PWD/ManagerAccountsSales.h \
    $$PWD/ManagerAccountsSalesRares.h \
    $$PWD/ManagerAccountsStockDeported.h \
    $$PWD/ManagerAccountsVatPayments.h \
    $$PWD/ManagerSaleTypes.h \
    $$PWD/SettingBookKeeping.h \
    $$PWD/TableBankBeginBalances.h

SOURCES += \
    $$PWD/ExceptionAccountDeportedMissing.cpp \
    $$PWD/ExceptionAccountSaleMissing.cpp \
    $$PWD/ManagerAccountPurchase.cpp \
    $$PWD/ManagerAccountsAmazon.cpp \
    $$PWD/ManagerAccountsSales.cpp \
    $$PWD/ManagerAccountsSalesRares.cpp \
    $$PWD/ManagerAccountsStockDeported.cpp \
    $$PWD/ManagerAccountsVatPayments.cpp \
    $$PWD/ManagerSaleTypes.cpp \
    $$PWD/SettingBookKeeping.cpp \
    $$PWD/TableBankBeginBalances.cpp
