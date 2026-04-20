#include "merchantrepository.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

MerchantRepository::MerchantRepository(QString connectionName)
    : m_connectionName(std::move(connectionName))
{
}

QList<Merchant> MerchantRepository::loadAllMerchants() const
{
    QList<Merchant> merchants;
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral(
        "SELECT id, name, campus_area, distance_minutes, queue_time_minutes, "
        "supports_dine_in, supports_takeaway, supports_delivery, "
        "delivery_eta_minutes, price_level, notes "
        "FROM merchants ORDER BY name"));

    if (!query.exec()) {
        return merchants;
    }

    while (query.next()) {
        Merchant merchant;
        merchant.id = query.value(0).toInt();
        merchant.name = query.value(1).toString();
        merchant.campusArea = query.value(2).toString();
        merchant.distanceMinutes = query.value(3).toInt();
        merchant.queueTimeMinutes = query.value(4).toInt();
        merchant.supportsDineIn = query.value(5).toBool();
        merchant.supportsTakeaway = query.value(6).toBool();
        merchant.supportsDelivery = query.value(7).toBool();
        merchant.deliveryEtaMinutes = query.value(8).toInt();
        merchant.priceLevel = query.value(9).toString();
        merchant.notes = query.value(10).toString();
        merchants.append(merchant);
    }

    return merchants;
}

int MerchantRepository::merchantCount() const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral("SELECT COUNT(*) FROM merchants"));
    if (!query.exec() || !query.next()) {
        return 0;
    }

    return query.value(0).toInt();
}

bool MerchantRepository::addMerchant(const Merchant &merchant,
                                     QString *errorMessage) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral(
        "INSERT INTO merchants "
        "(name, campus_area, distance_minutes, queue_time_minutes, "
        "supports_dine_in, supports_takeaway, supports_delivery, "
        "delivery_eta_minutes, price_level, notes) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));
    query.addBindValue(merchant.name);
    query.addBindValue(merchant.campusArea);
    query.addBindValue(merchant.distanceMinutes);
    query.addBindValue(merchant.queueTimeMinutes);
    query.addBindValue(merchant.supportsDineIn);
    query.addBindValue(merchant.supportsTakeaway);
    query.addBindValue(merchant.supportsDelivery);
    query.addBindValue(merchant.deliveryEtaMinutes);
    query.addBindValue(merchant.priceLevel);
    query.addBindValue(merchant.notes);

    if (!query.exec()) {
        if (errorMessage != nullptr) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }

    return true;
}

bool MerchantRepository::updateMerchant(const Merchant &merchant,
                                        QString *errorMessage) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral(
        "UPDATE merchants SET "
        "name = ?, campus_area = ?, distance_minutes = ?, queue_time_minutes = ?, "
        "supports_dine_in = ?, supports_takeaway = ?, supports_delivery = ?, "
        "delivery_eta_minutes = ?, price_level = ?, notes = ? "
        "WHERE id = ?"));
    query.addBindValue(merchant.name);
    query.addBindValue(merchant.campusArea);
    query.addBindValue(merchant.distanceMinutes);
    query.addBindValue(merchant.queueTimeMinutes);
    query.addBindValue(merchant.supportsDineIn);
    query.addBindValue(merchant.supportsTakeaway);
    query.addBindValue(merchant.supportsDelivery);
    query.addBindValue(merchant.deliveryEtaMinutes);
    query.addBindValue(merchant.priceLevel);
    query.addBindValue(merchant.notes);
    query.addBindValue(merchant.id);

    if (!query.exec()) {
        if (errorMessage != nullptr) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool MerchantRepository::deleteMerchant(int merchantId,
                                        QString *errorMessage) const
{
    QSqlQuery dependencyQuery(QSqlDatabase::database(m_connectionName));
    dependencyQuery.prepare(QStringLiteral(
        "SELECT COUNT(*) FROM dishes WHERE merchant_id = ?"));
    dependencyQuery.addBindValue(merchantId);
    if (!dependencyQuery.exec() || !dependencyQuery.next()) {
        if (errorMessage != nullptr) {
            *errorMessage = dependencyQuery.lastError().text();
        }
        return false;
    }

    if (dependencyQuery.value(0).toInt() > 0) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral(
                "This merchant still has dishes. Update or delete those dishes first.");
        }
        return false;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral("DELETE FROM merchants WHERE id = ?"));
    query.addBindValue(merchantId);

    if (!query.exec()) {
        if (errorMessage != nullptr) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }

    return query.numRowsAffected() > 0;
}
