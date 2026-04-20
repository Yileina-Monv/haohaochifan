#pragma once

#include "../core/domainmodels.h"

#include <QString>

class MerchantRepository
{
public:
    explicit MerchantRepository(QString connectionName);

    QList<Merchant> loadAllMerchants() const;
    int merchantCount() const;
    bool addMerchant(const Merchant &merchant,
                     QString *errorMessage = nullptr) const;
    bool updateMerchant(const Merchant &merchant,
                        QString *errorMessage = nullptr) const;
    bool deleteMerchant(int merchantId,
                        QString *errorMessage = nullptr) const;

private:
    QString m_connectionName;
};
