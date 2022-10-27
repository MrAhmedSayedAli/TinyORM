#include <QCoreApplication>
#include <QtTest>

#include "orm/db.hpp"
#include "orm/query/querybuilder.hpp"
#include "orm/utils/type.hpp"

#include "databases.hpp"

using Orm::Constants::ID;
using Orm::Constants::UTC;

using Orm::DB;
using Orm::QtTimeZoneConfig;
using Orm::QtTimeZoneType;

using QueryBuilder = Orm::Query::Builder;
using TypeUtils = Orm::Utils::Type;

using TestUtils::Databases;

class tst_MySql_QDateTime : public QObject // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    /* QDateTime with/without timezone */
    /* Raw QSqlQuery */
    /* Server timezone UTC */
    void insert_Qt_QDateTime_UtcTimezone_DatetimeColumn_UtcOnServer() const;
    void insert_Qt_QDateTime_0200Timezone_DatetimeColumn_UtcOnServer() const;
    void insert_Qt_QString_DatetimeColumn_UtcOnServer() const;
    void insert_Qt_QDateTime_UtcTimezone_TimestampColumn_UtcOnServer() const;
    void insert_Qt_QDateTime_0200Timezone_TimestampColumn_UtcOnServer() const;
    void insert_Qt_QString_TimestampColumn_UtcOnServer() const;

    /* Server timezone +02:00 */
    void insert_Qt_QDateTime_UtcTimezone_DatetimeColumn_0200OnServer() const;
    void insert_Qt_QDateTime_0200Timezone_DatetimeColumn_0200OnServer() const;
    void insert_Qt_QString_DatetimeColumn_0200OnServer() const;
    void insert_Qt_QDateTime_UtcTimezone_TimestampColumn_0200OnServer() const;
    void insert_Qt_QDateTime_0200Timezone_TimestampColumn_0200OnServer() const;
    void insert_Qt_QString_TimestampColumn_0200OnServer() const;

    /* Orm::QueryBuilder */
    /* Server timezone UTC */
    void insert_QDateTime_UtcTimezone_DatetimeColumn_UtcOnServer() const;
    void insert_QDateTime_0200Timezone_DatetimeColumn_UtcOnServer() const;
    void insert_QString_DatetimeColumn_UtcOnServer() const;
    void insert_QDateTime_UtcTimezone_TimestampColumn_UtcOnServer() const;
    void insert_QDateTime_0200Timezone_TimestampColumn_UtcOnServer() const;
    void insert_QString_TimestampColumn_UtcOnServer() const;

    /* Server timezone +02:00 */
    void insert_QDateTime_UtcTimezone_DatetimeColumn_0200OnServer() const;
    void insert_QDateTime_0200Timezone_DatetimeColumn_0200OnServer() const;
    void insert_QString_DatetimeColumn_0200OnServer() const;
    void insert_QDateTime_UtcTimezone_TimestampColumn_0200OnServer() const;
    void insert_QDateTime_0200Timezone_TimestampColumn_0200OnServer() const;
    void insert_QString_TimestampColumn_0200OnServer() const;

    /* QDate */
    /* Raw QSqlQuery */
    /* Server timezone UTC */
    void insert_Qt_QDate_UtcTimezone_DateColumn_UtcOnServer() const;
    void insert_Qt_QString_DateColumn_UtcOnServer() const;

    /* Server timezone +02:00 */
    void insert_Qt_QDate_UtcTimezone_DateColumn_0200OnServer() const;
    void insert_Qt_QString_DateColumn_0200OnServer() const;

    /* Orm::QueryBuilder */
    /* Server timezone UTC */
    void insert_QDate_UtcTimezone_DateColumn_UtcOnServer() const;
    void insert_QString_DateColumn_UtcOnServer() const;

    /* Server timezone +02:00 */
    void insert_QDate_UtcTimezone_DateColumn_0200OnServer() const;
    void insert_QString_DateColumn_0200OnServer() const;

// NOLINTNEXTLINE(readability-redundant-access-specifiers)
private:
    /* Common */
    /*! Create QueryBuilder instance for the given connection. */
    [[nodiscard]] std::shared_ptr<QueryBuilder> createQuery() const;

    /* QDateTime with/without timezone */
    /*! Generate a call wrapped for the QVariant::typeId/userType for Qt5/6. */
    inline static auto typeIdWrapper(const QVariant &attribute);

    /*! Set the MySQL timezone session variable to the UTC value. */
    inline void setUtcTimezone() const;
    /*! Set the MySQL timezone session variable to the +02:00 value. */
    inline void set0200Timezone() const;
    /*! Set the MySQL timezone session variable to the given value. */
    void setTimezone(const QString &timezone, QtTimeZoneConfig &&qtTimeZone) const;

    /*! Restore the database after a QDateTime-related test. */
    void restore(quint64 lastId, bool restoreTimezone = false) const;

    /* Data members */
    /*! Connection name used in this test case. */
    QString m_connection {};
};

/* private */

auto tst_MySql_QDateTime::typeIdWrapper(const QVariant &attribute)
{
    /* It helps to avoid #ifdef-s for QT_VERSION in all test methods
       for the QVariant::typeId/userType for Qt5/6. */
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return std::bind_front(&QVariant::typeId, attribute);
#else
    return std::bind_front(&QVariant::userType, attribute);
#endif
}

/* private slots */

void tst_MySql_QDateTime::initTestCase()
{
    m_connection = Databases::createConnection(Databases::MYSQL);

    if (m_connection.isEmpty())
        QSKIP(TestUtils::AutoTestSkipped
              .arg(TypeUtils::classPureBasename(*this), Databases::MYSQL)
              .toUtf8().constData(), );
}

/* QDateTime with/without timezone */

/* Raw QSqlQuery */

/* Server timezone UTC */

void tst_MySql_QDateTime::
insert_Qt_QDateTime_UtcTimezone_DatetimeColumn_UtcOnServer() const
{
    quint64 lastId = 0;

    // Insert
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare("insert into `datetime` (`datetime`) values (?)"));

        qtQuery.addBindValue(QDateTime::fromString("2022-08-28 13:14:15z", Qt::ISODate));

        // QMYSQL driver doesn't care about QDateTime timezone
        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && !qtQuery.isSelect());
        QCOMPARE(qtQuery.numRowsAffected(), 1);

        lastId = qtQuery.lastInsertId().value<quint64>();
    }

    // Verify
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare(
                    "select `id`, `datetime` from `datetime` where `id` = ?"));

        qtQuery.addBindValue(lastId);

        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && qtQuery.isSelect());
        QCOMPARE(qtQuery.size(), 1);

        QVERIFY(qtQuery.first());

        QCOMPARE(qtQuery.value(ID).value<quint64>(), lastId);

        const auto datetimeDbVariant = qtQuery.value("datetime");

        auto typeId = typeIdWrapper(datetimeDbVariant);
        QCOMPARE(typeId(), QMetaType::QDateTime);

        /* QMYSQL driver doesn't care about QDateTime timezone and returns the QDateTime
           in a local timezone. */
        const auto datetimeActual = datetimeDbVariant.value<QDateTime>();
        const auto datetimeExpected = QDateTime::fromString("2022-08-28 13:14:15",
                                                            Qt::ISODate);
        QCOMPARE(datetimeActual, datetimeExpected);
        QCOMPARE(datetimeActual, datetimeExpected.toLocalTime());
        QCOMPARE(datetimeActual.timeZone(), QTimeZone::systemTimeZone());
    }

    // Restore
    restore(lastId);
}

void tst_MySql_QDateTime::
insert_Qt_QDateTime_0200Timezone_DatetimeColumn_UtcOnServer() const
{
    quint64 lastId = 0;

    // Insert
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare("insert into `datetime` (`datetime`) values (?)"));

        qtQuery.addBindValue(
                    QDateTime::fromString("2022-08-28 13:14:15+02:00", Qt::ISODate));

        // QMYSQL driver doesn't care about QDateTime timezone
        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && !qtQuery.isSelect());
        QCOMPARE(qtQuery.numRowsAffected(), 1);

        lastId = qtQuery.lastInsertId().value<quint64>();
    }

    // Verify
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare(
                    "select `id`, `datetime` from `datetime` where `id` = ?"));

        qtQuery.addBindValue(lastId);

        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && qtQuery.isSelect());
        QCOMPARE(qtQuery.size(), 1);

        QVERIFY(qtQuery.first());

        QCOMPARE(qtQuery.value(ID).value<quint64>(), lastId);

        const auto datetimeDbVariant = qtQuery.value("datetime");

        auto typeId = typeIdWrapper(datetimeDbVariant);
        QCOMPARE(typeId(), QMetaType::QDateTime);

        /* QMYSQL driver doesn't care about QDateTime timezone and returns the QDateTime
           in a local timezone. */
        const auto datetimeActual = datetimeDbVariant.value<QDateTime>();
        const auto datetimeExpected = QDateTime::fromString("2022-08-28 13:14:15",
                                                            Qt::ISODate);
        QCOMPARE(datetimeActual, datetimeExpected);
        QCOMPARE(datetimeActual, datetimeExpected.toLocalTime());
        QCOMPARE(datetimeActual.timeZone(), QTimeZone::systemTimeZone());
    }

    // Restore
    restore(lastId);
}

void tst_MySql_QDateTime::insert_Qt_QString_DatetimeColumn_UtcOnServer() const
{
    quint64 lastId = 0;

    // Insert
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare("insert into `datetime` (`datetime`) values (?)"));

        qtQuery.addBindValue(QString("2022-08-28 13:14:15"));

        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && !qtQuery.isSelect());
        QCOMPARE(qtQuery.numRowsAffected(), 1);

        lastId = qtQuery.lastInsertId().value<quint64>();
    }

    // Verify
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare(
                    "select `id`, `datetime` from `datetime` where `id` = ?"));

        qtQuery.addBindValue(lastId);

        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && qtQuery.isSelect());
        QCOMPARE(qtQuery.size(), 1);

        QVERIFY(qtQuery.first());

        QCOMPARE(qtQuery.value(ID).value<quint64>(), lastId);

        const auto datetimeDbVariant = qtQuery.value("datetime");

        auto typeId = typeIdWrapper(datetimeDbVariant);
        QCOMPARE(typeId(), QMetaType::QDateTime);

        /* QMYSQL driver doesn't care about QDateTime timezone and returns the QDateTime
           in a local timezone. */
        const auto datetimeActual = datetimeDbVariant.value<QDateTime>();
        const auto datetimeExpected = QDateTime::fromString("2022-08-28 13:14:15",
                                                            Qt::ISODate);
        QCOMPARE(datetimeActual, datetimeExpected);
        QCOMPARE(datetimeActual, datetimeExpected.toLocalTime());
        QCOMPARE(datetimeActual.timeZone(), QTimeZone::systemTimeZone());
    }

    // Restore
    restore(lastId);
}

void tst_MySql_QDateTime::
insert_Qt_QDateTime_UtcTimezone_TimestampColumn_UtcOnServer() const
{
    quint64 lastId = 0;

    // Insert
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare("insert into `datetime` (`timestamp`) values (?)"));

        qtQuery.addBindValue(QDateTime::fromString("2022-08-29 13:14:15z", Qt::ISODate));

        // QMYSQL driver doesn't care about QDateTime timezone
        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && !qtQuery.isSelect());
        QCOMPARE(qtQuery.numRowsAffected(), 1);

        lastId = qtQuery.lastInsertId().value<quint64>();
    }

    // Verify
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare(
                    "select `id`, `timestamp` from `datetime` where `id` = ?"));

        qtQuery.addBindValue(lastId);

        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && qtQuery.isSelect());
        QCOMPARE(qtQuery.size(), 1);

        QVERIFY(qtQuery.first());

        QCOMPARE(qtQuery.value(ID).value<quint64>(), lastId);

        const auto timestampDbVariant = qtQuery.value("timestamp");

        auto typeId = typeIdWrapper(timestampDbVariant);
        QCOMPARE(typeId(), QMetaType::QDateTime);

        /* QMYSQL driver doesn't care about QDateTime timezone and returns the QDateTime
           in a local timezone. */
        const auto timestampActual = timestampDbVariant.value<QDateTime>();
        const auto timestampExpected = QDateTime::fromString("2022-08-29 13:14:15",
                                                             Qt::ISODate);
        QCOMPARE(timestampActual, timestampExpected);
        QCOMPARE(timestampActual, timestampExpected.toLocalTime());
        QCOMPARE(timestampActual.timeZone(), QTimeZone::systemTimeZone());
    }

    // Restore
    restore(lastId);
}

void tst_MySql_QDateTime::
insert_Qt_QDateTime_0200Timezone_TimestampColumn_UtcOnServer() const
{
    quint64 lastId = 0;

    // Insert
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare("insert into `datetime` (`timestamp`) values (?)"));

        qtQuery.addBindValue(
                    QDateTime::fromString("2022-08-29 13:14:15+02:00", Qt::ISODate));

        // QMYSQL driver doesn't care about QDateTime timezone
        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && !qtQuery.isSelect());
        QCOMPARE(qtQuery.numRowsAffected(), 1);

        lastId = qtQuery.lastInsertId().value<quint64>();
    }

    // Verify
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare(
                    "select `id`, `timestamp` from `datetime` where `id` = ?"));

        qtQuery.addBindValue(lastId);

        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && qtQuery.isSelect());
        QCOMPARE(qtQuery.size(), 1);

        QVERIFY(qtQuery.first());

        QCOMPARE(qtQuery.value(ID).value<quint64>(), lastId);

        const auto timestampDbVariant = qtQuery.value("timestamp");

        auto typeId = typeIdWrapper(timestampDbVariant);
        QCOMPARE(typeId(), QMetaType::QDateTime);

        /* QMYSQL driver doesn't care about QDateTime timezone and returns the QDateTime
           in a local timezone. */
        const auto timestampActual = timestampDbVariant.value<QDateTime>();
        const auto timestampExpected = QDateTime::fromString("2022-08-29 13:14:15",
                                                             Qt::ISODate);
        QCOMPARE(timestampActual, timestampExpected);
        QCOMPARE(timestampActual, timestampExpected.toLocalTime());
        QCOMPARE(timestampActual.timeZone(), QTimeZone::systemTimeZone());
    }

    // Restore
    restore(lastId);
}

void tst_MySql_QDateTime::insert_Qt_QString_TimestampColumn_UtcOnServer() const
{
    quint64 lastId = 0;

    // Insert
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare("insert into `datetime` (`timestamp`) values (?)"));

        qtQuery.addBindValue(QString("2022-08-29 13:14:15"));

        // QMYSQL driver doesn't care about QDateTime timezone
        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && !qtQuery.isSelect());
        QCOMPARE(qtQuery.numRowsAffected(), 1);

        lastId = qtQuery.lastInsertId().value<quint64>();
    }

    // Verify
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare(
                    "select `id`, `timestamp` from `datetime` where `id` = ?"));

        qtQuery.addBindValue(lastId);

        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && qtQuery.isSelect());
        QCOMPARE(qtQuery.size(), 1);

        QVERIFY(qtQuery.first());

        QCOMPARE(qtQuery.value(ID).value<quint64>(), lastId);

        const auto timestampDbVariant = qtQuery.value("timestamp");

        auto typeId = typeIdWrapper(timestampDbVariant);
        QCOMPARE(typeId(), QMetaType::QDateTime);

        /* QMYSQL driver doesn't care about QDateTime timezone and returns the QDateTime
           in a local timezone. */
        const auto timestampActual = timestampDbVariant.value<QDateTime>();
        const auto timestampExpected = QDateTime::fromString("2022-08-29 13:14:15",
                                                             Qt::ISODate);
        QCOMPARE(timestampActual, timestampExpected);
        QCOMPARE(timestampActual, timestampExpected.toLocalTime());
        QCOMPARE(timestampActual.timeZone(), QTimeZone::systemTimeZone());
    }

    // Restore
    restore(lastId);
}

/* Server timezone +02:00 */

void tst_MySql_QDateTime::
insert_Qt_QDateTime_UtcTimezone_DatetimeColumn_0200OnServer() const
{
    set0200Timezone();
    quint64 lastId = 0;

    // Insert
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare("insert into `datetime` (`datetime`) values (?)"));

        qtQuery.addBindValue(QDateTime::fromString("2022-08-28 13:14:15z", Qt::ISODate));

        // QMYSQL driver doesn't care about QDateTime timezone
        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && !qtQuery.isSelect());
        QCOMPARE(qtQuery.numRowsAffected(), 1);

        lastId = qtQuery.lastInsertId().value<quint64>();
    }

    // Verify
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare(
                    "select `id`, `datetime` from `datetime` where `id` = ?"));

        qtQuery.addBindValue(lastId);

        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && qtQuery.isSelect());
        QCOMPARE(qtQuery.size(), 1);

        QVERIFY(qtQuery.first());

        QCOMPARE(qtQuery.value(ID).value<quint64>(), lastId);

        const auto datetimeDbVariant = qtQuery.value("datetime");

        auto typeId = typeIdWrapper(datetimeDbVariant);
        QCOMPARE(typeId(), QMetaType::QDateTime);

        /* QMYSQL driver doesn't care about QDateTime timezone and returns the QDateTime
           in a local timezone. */
        const auto datetimeActual = datetimeDbVariant.value<QDateTime>();
        const auto datetimeExpected = QDateTime::fromString("2022-08-28 13:14:15",
                                                            Qt::ISODate);
        QCOMPARE(datetimeActual, datetimeExpected);
        QCOMPARE(datetimeActual, datetimeExpected.toLocalTime());
        QCOMPARE(datetimeActual.timeZone(), QTimeZone::systemTimeZone());
    }

    // Restore
    restore(lastId, true);
}

void tst_MySql_QDateTime::
insert_Qt_QDateTime_0200Timezone_DatetimeColumn_0200OnServer() const
{
    set0200Timezone();
    quint64 lastId = 0;

    // Insert
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare("insert into `datetime` (`datetime`) values (?)"));

        qtQuery.addBindValue(
                    QDateTime::fromString("2022-08-28 13:14:15+02:00", Qt::ISODate));

        // QMYSQL driver doesn't care about QDateTime timezone
        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && !qtQuery.isSelect());
        QCOMPARE(qtQuery.numRowsAffected(), 1);

        lastId = qtQuery.lastInsertId().value<quint64>();
    }

    // Verify
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare(
                    "select `id`, `datetime` from `datetime` where `id` = ?"));

        qtQuery.addBindValue(lastId);

        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && qtQuery.isSelect());
        QCOMPARE(qtQuery.size(), 1);

        QVERIFY(qtQuery.first());

        QCOMPARE(qtQuery.value(ID).value<quint64>(), lastId);

        const auto datetimeDbVariant = qtQuery.value("datetime");

        auto typeId = typeIdWrapper(datetimeDbVariant);
        QCOMPARE(typeId(), QMetaType::QDateTime);

        /* QMYSQL driver doesn't care about QDateTime timezone and returns the QDateTime
           in a local timezone. */
        const auto datetimeActual = datetimeDbVariant.value<QDateTime>();
        const auto datetimeExpected = QDateTime::fromString("2022-08-28 13:14:15",
                                                            Qt::ISODate);
        QCOMPARE(datetimeActual, datetimeExpected);
        QCOMPARE(datetimeActual, datetimeExpected.toLocalTime());
        QCOMPARE(datetimeActual.timeZone(), QTimeZone::systemTimeZone());
    }

    // Restore
    restore(lastId, true);
}

void tst_MySql_QDateTime::insert_Qt_QString_DatetimeColumn_0200OnServer() const
{
    set0200Timezone();
    quint64 lastId = 0;

    // Insert
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare("insert into `datetime` (`datetime`) values (?)"));

        qtQuery.addBindValue(QString("2022-08-28 13:14:15"));

        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && !qtQuery.isSelect());
        QCOMPARE(qtQuery.numRowsAffected(), 1);

        lastId = qtQuery.lastInsertId().value<quint64>();
    }

    // Verify
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare(
                    "select `id`, `datetime` from `datetime` where `id` = ?"));

        qtQuery.addBindValue(lastId);

        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && qtQuery.isSelect());
        QCOMPARE(qtQuery.size(), 1);

        QVERIFY(qtQuery.first());

        QCOMPARE(qtQuery.value(ID).value<quint64>(), lastId);

        const auto datetimeDbVariant = qtQuery.value("datetime");

        auto typeId = typeIdWrapper(datetimeDbVariant);
        QCOMPARE(typeId(), QMetaType::QDateTime);

        /* QMYSQL driver doesn't care about QDateTime timezone and returns the QDateTime
           in a local timezone. */
        const auto datetimeActual = datetimeDbVariant.value<QDateTime>();
        const auto datetimeExpected = QDateTime::fromString("2022-08-28 13:14:15",
                                                            Qt::ISODate);
        QCOMPARE(datetimeActual, datetimeExpected);
        QCOMPARE(datetimeActual, datetimeExpected.toLocalTime());
        QCOMPARE(datetimeActual.timeZone(), QTimeZone::systemTimeZone());
    }

    // Restore
    restore(lastId, true);
}

void tst_MySql_QDateTime::
insert_Qt_QDateTime_UtcTimezone_TimestampColumn_0200OnServer() const
{
    set0200Timezone();
    quint64 lastId = 0;

    // Insert
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare("insert into `datetime` (`timestamp`) values (?)"));

        qtQuery.addBindValue(QDateTime::fromString("2022-08-29 13:14:15z", Qt::ISODate));

        // QMYSQL driver doesn't care about QDateTime timezone
        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && !qtQuery.isSelect());
        QCOMPARE(qtQuery.numRowsAffected(), 1);

        lastId = qtQuery.lastInsertId().value<quint64>();
    }

    // Verify
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare(
                    "select `id`, `timestamp` from `datetime` where `id` = ?"));

        qtQuery.addBindValue(lastId);

        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && qtQuery.isSelect());
        QCOMPARE(qtQuery.size(), 1);

        QVERIFY(qtQuery.first());

        QCOMPARE(qtQuery.value(ID).value<quint64>(), lastId);

        const auto timestampDbVariant = qtQuery.value("timestamp");

        auto typeId = typeIdWrapper(timestampDbVariant);
        QCOMPARE(typeId(), QMetaType::QDateTime);

        /* QMYSQL driver doesn't care about QDateTime timezone and returns the QDateTime
           in a local timezone. */
        const auto timestampActual = timestampDbVariant.value<QDateTime>();
        const auto timestampExpected = QDateTime::fromString("2022-08-29 13:14:15",
                                                             Qt::ISODate);
        QCOMPARE(timestampActual, timestampExpected);
        QCOMPARE(timestampActual, timestampExpected.toLocalTime());
        QCOMPARE(timestampActual.timeZone(), QTimeZone::systemTimeZone());
    }

    // Restore
    restore(lastId, true);
}

void tst_MySql_QDateTime::
insert_Qt_QDateTime_0200Timezone_TimestampColumn_0200OnServer() const
{
    set0200Timezone();
    quint64 lastId = 0;

    // Insert
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare("insert into `datetime` (`timestamp`) values (?)"));

        qtQuery.addBindValue(
                    QDateTime::fromString("2022-08-29 13:14:15+02:00", Qt::ISODate));

        // QMYSQL driver doesn't care about QDateTime timezone
        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && !qtQuery.isSelect());
        QCOMPARE(qtQuery.numRowsAffected(), 1);

        lastId = qtQuery.lastInsertId().value<quint64>();
    }

    // Verify
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare(
                    "select `id`, `timestamp` from `datetime` where `id` = ?"));

        qtQuery.addBindValue(lastId);

        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && qtQuery.isSelect());
        QCOMPARE(qtQuery.size(), 1);

        QVERIFY(qtQuery.first());

        QCOMPARE(qtQuery.value(ID).value<quint64>(), lastId);

        const auto timestampDbVariant = qtQuery.value("timestamp");

        auto typeId = typeIdWrapper(timestampDbVariant);
        QCOMPARE(typeId(), QMetaType::QDateTime);

        /* QMYSQL driver doesn't care about QDateTime timezone and returns the QDateTime
           in a local timezone. */
        const auto timestampActual = timestampDbVariant.value<QDateTime>();
        const auto timestampExpected = QDateTime::fromString("2022-08-29 13:14:15",
                                                             Qt::ISODate);
        QCOMPARE(timestampActual, timestampExpected);
        QCOMPARE(timestampActual, timestampExpected.toLocalTime());
        QCOMPARE(timestampActual.timeZone(), QTimeZone::systemTimeZone());
    }

    // Restore
    restore(lastId, true);
}

void tst_MySql_QDateTime::insert_Qt_QString_TimestampColumn_0200OnServer() const
{
    set0200Timezone();
    quint64 lastId = 0;

    // Insert
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare("insert into `datetime` (`timestamp`) values (?)"));

        qtQuery.addBindValue(QString("2022-08-29 13:14:15"));

        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && !qtQuery.isSelect());
        QCOMPARE(qtQuery.numRowsAffected(), 1);

        lastId = qtQuery.lastInsertId().value<quint64>();
    }

    // Verify
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare(
                    "select `id`, `timestamp` from `datetime` where `id` = ?"));

        qtQuery.addBindValue(lastId);

        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && qtQuery.isSelect());
        QCOMPARE(qtQuery.size(), 1);

        QVERIFY(qtQuery.first());

        QCOMPARE(qtQuery.value(ID).value<quint64>(), lastId);

        const auto timestampDbVariant = qtQuery.value("timestamp");

        auto typeId = typeIdWrapper(timestampDbVariant);
        QCOMPARE(typeId(), QMetaType::QDateTime);

        /* QMYSQL driver doesn't care about QDateTime timezone and returns the QDateTime
           in a local timezone. */
        const auto timestampActual = timestampDbVariant.value<QDateTime>();
        const auto timestampExpected = QDateTime::fromString("2022-08-29 13:14:15",
                                                             Qt::ISODate);
        QCOMPARE(timestampActual, timestampExpected);
        QCOMPARE(timestampActual, timestampExpected.toLocalTime());
        QCOMPARE(timestampActual.timeZone(), QTimeZone::systemTimeZone());
    }

    // Restore
    restore(lastId, true);
}

/* Orm::QueryBuilder */

/* Server timezone UTC */

void tst_MySql_QDateTime::
insert_QDateTime_UtcTimezone_DatetimeColumn_UtcOnServer() const
{
    // Insert
    quint64 lastId = createQuery()->from("datetime").insertGetId(
                         {{"datetime", QDateTime::fromString("2022-08-28 13:14:15z",
                                                             Qt::ISODate)}});

    // Verify
    {
        auto query = createQuery()->from("datetime").first({ID, "datetime"});

        QCOMPARE(query.size(), 1);

        QCOMPARE(query.value(ID).value<quint64>(), lastId);

        const auto datetimeDbVariant = query.value("datetime");

        auto typeId = typeIdWrapper(datetimeDbVariant);
        QCOMPARE(typeId(), QMetaType::QDateTime);

        /* The time zone must be as is defined in the qt_timezone connection
           configuration, TinyORM QueryBuilder fixes the buggy time zone behavior
           of the QMYSQL driver. */
        const auto datetimeActual = datetimeDbVariant.value<QDateTime>();
        const auto datetimeExpected = QDateTime::fromString("2022-08-28 13:14:15z",
                                                            Qt::ISODate);
        QCOMPARE(datetimeActual, datetimeExpected);
        QCOMPARE(datetimeActual, datetimeExpected.toLocalTime());
        QCOMPARE(datetimeActual.timeZone(), QTimeZone::utc());
    }

    // Restore
    restore(lastId);
}

void tst_MySql_QDateTime::
insert_QDateTime_0200Timezone_DatetimeColumn_UtcOnServer() const
{
    // Insert
    quint64 lastId = createQuery()->from("datetime").insertGetId(
                         {{"datetime", QDateTime::fromString("2022-08-28 13:14:15+02:00",
                                                             Qt::ISODate)}});

    // Verify
    {
        auto query = createQuery()->from("datetime").first({ID, "datetime"});

        QCOMPARE(query.size(), 1);

        QCOMPARE(query.value(ID).value<quint64>(), lastId);

        const auto datetimeDbVariant = query.value("datetime");

        auto typeId = typeIdWrapper(datetimeDbVariant);
        QCOMPARE(typeId(), QMetaType::QDateTime);

        /* The time zone must be as is defined in the qt_timezone connection
           configuration, TinyORM QueryBuilder fixes the buggy time zone behavior
           of the QMYSQL driver. */
        const auto datetimeActual = datetimeDbVariant.value<QDateTime>();
        const auto datetimeExpected = QDateTime::fromString("2022-08-28 11:14:15z",
                                                            Qt::ISODate);
        QCOMPARE(datetimeActual, datetimeExpected);
        QCOMPARE(datetimeActual, datetimeExpected.toLocalTime());
        QCOMPARE(datetimeActual.timeZone(), QTimeZone::utc());
    }

    // Restore
    restore(lastId);
}

void tst_MySql_QDateTime::insert_QString_DatetimeColumn_UtcOnServer() const
{
    // Insert
    quint64 lastId = createQuery()->from("datetime").insertGetId(
                         {{"datetime", QString("2022-08-28 13:14:15")}});

    // Verify
    {
        auto query = createQuery()->from("datetime").first({ID, "datetime"});

        QCOMPARE(query.size(), 1);

        QCOMPARE(query.value(ID).value<quint64>(), lastId);

        const auto datetimeDbVariant = query.value("datetime");

        auto typeId = typeIdWrapper(datetimeDbVariant);
        QCOMPARE(typeId(), QMetaType::QDateTime);

        /* The time zone must be as is defined in the qt_timezone connection
           configuration, TinyORM QueryBuilder fixes the buggy time zone behavior
           of the QMYSQL driver. */
        const auto datetimeActual = datetimeDbVariant.value<QDateTime>();
        const auto datetimeExpected = QDateTime::fromString("2022-08-28 13:14:15z",
                                                            Qt::ISODate);
        QCOMPARE(datetimeActual, datetimeExpected);
        QCOMPARE(datetimeActual, datetimeExpected.toLocalTime());
        QCOMPARE(datetimeActual.timeZone(), QTimeZone::utc());
    }

    // Restore
    restore(lastId);
}

void tst_MySql_QDateTime::
insert_QDateTime_UtcTimezone_TimestampColumn_UtcOnServer() const
{
    // Insert
    quint64 lastId = createQuery()->from("datetime").insertGetId(
                         {{"timestamp", QDateTime::fromString("2022-08-28 13:14:15z",
                                                              Qt::ISODate)}});

    // Verify
    {
        auto query = createQuery()->from("datetime").first({ID, "timestamp"});

        QCOMPARE(query.size(), 1);

        QCOMPARE(query.value(ID).value<quint64>(), lastId);

        const auto timestampDbVariant = query.value("timestamp");

        auto typeId = typeIdWrapper(timestampDbVariant);
        QCOMPARE(typeId(), QMetaType::QDateTime);

        /* The time zone must be as is defined in the qt_timezone connection
           configuration, TinyORM QueryBuilder fixes the buggy time zone behavior
           of the QMYSQL driver. */
        const auto timestampActual = timestampDbVariant.value<QDateTime>();
        const auto timestampExpected = QDateTime::fromString("2022-08-28 13:14:15z",
                                                             Qt::ISODate);
        QCOMPARE(timestampActual, timestampExpected);
        QCOMPARE(timestampActual, timestampExpected.toLocalTime());
        QCOMPARE(timestampActual.timeZone(), QTimeZone::utc());
    }

    // Restore
    restore(lastId);
}

void tst_MySql_QDateTime::
insert_QDateTime_0200Timezone_TimestampColumn_UtcOnServer() const
{
    // Insert
    quint64 lastId = createQuery()->from("datetime").insertGetId(
                         {{"timestamp",
                           QDateTime::fromString("2022-08-28 13:14:15+02:00",
                                                 Qt::ISODate)}});

    // Verify
    {
        auto query = createQuery()->from("datetime").first({ID, "timestamp"});

        QCOMPARE(query.size(), 1);

        QCOMPARE(query.value(ID).value<quint64>(), lastId);

        const auto timestampDbVariant = query.value("timestamp");

        auto typeId = typeIdWrapper(timestampDbVariant);
        QCOMPARE(typeId(), QMetaType::QDateTime);

        /* The time zone must be as is defined in the qt_timezone connection
           configuration, TinyORM QueryBuilder fixes the buggy time zone behavior
           of the QMYSQL driver. */
        const auto timestampActual = timestampDbVariant.value<QDateTime>();
        const auto timestampExpected = QDateTime::fromString("2022-08-28 11:14:15z",
                                                             Qt::ISODate);
        QCOMPARE(timestampActual, timestampExpected);
        QCOMPARE(timestampActual, timestampExpected.toLocalTime());
        QCOMPARE(timestampActual.timeZone(), QTimeZone::utc());
    }

    // Restore
    restore(lastId);
}

void tst_MySql_QDateTime::insert_QString_TimestampColumn_UtcOnServer() const
{
    // Insert
    quint64 lastId = createQuery()->from("datetime").insertGetId(
                         {{"timestamp", QString("2022-08-28 13:14:15")}});

    // Verify
    {
        auto query = createQuery()->from("datetime").first({ID, "timestamp"});

        QCOMPARE(query.size(), 1);

        QCOMPARE(query.value(ID).value<quint64>(), lastId);

        const auto timestampDbVariant = query.value("timestamp");

        auto typeId = typeIdWrapper(timestampDbVariant);
        QCOMPARE(typeId(), QMetaType::QDateTime);

        /* The time zone must be as is defined in the qt_timezone connection
           configuration, TinyORM QueryBuilder fixes the buggy time zone behavior
           of the QMYSQL driver. */
        const auto timestampActual = timestampDbVariant.value<QDateTime>();
        const auto timestampExpected = QDateTime::fromString("2022-08-28 13:14:15z",
                                                             Qt::ISODate);
        QCOMPARE(timestampActual, timestampExpected);
        QCOMPARE(timestampActual, timestampExpected.toLocalTime());
        QCOMPARE(timestampActual.timeZone(), QTimeZone::utc());
    }

    // Restore
    restore(lastId);
}

/* Server timezone +02:00 */

namespace
{
    /*! Time zone +02:00. */
    Q_GLOBAL_STATIC_WITH_ARGS(QTimeZone, TimeZone0200, (QByteArray("UTC+02:00")));
}

void tst_MySql_QDateTime::
insert_QDateTime_UtcTimezone_DatetimeColumn_0200OnServer() const
{
    set0200Timezone();

    // Insert
    quint64 lastId = createQuery()->from("datetime").insertGetId(
                         {{"datetime", QDateTime::fromString("2022-08-28 13:14:15z",
                                                             Qt::ISODate)}});

    // Verify
    {
        auto query = createQuery()->from("datetime").first({ID, "datetime"});

        QCOMPARE(query.size(), 1);

        QCOMPARE(query.value(ID).value<quint64>(), lastId);

        const auto datetimeDbVariant = query.value("datetime");

        auto typeId = typeIdWrapper(datetimeDbVariant);
        QCOMPARE(typeId(), QMetaType::QDateTime);

        /* The time zone must be as is defined in the qt_timezone connection
           configuration, TinyORM QueryBuilder fixes the buggy time zone behavior
           of the QMYSQL driver. */
        const auto datetimeActual = datetimeDbVariant.value<QDateTime>();
        const auto datetimeExpected = QDateTime::fromString("2022-08-28 15:14:15+02:00",
                                                            Qt::ISODate);
        QCOMPARE(datetimeActual, datetimeExpected);
        QCOMPARE(datetimeActual, datetimeExpected.toLocalTime());
        QCOMPARE(datetimeActual.timeZone(), *TimeZone0200);
    }

    // Restore
    restore(lastId, true);
}

void tst_MySql_QDateTime::
insert_QDateTime_0200Timezone_DatetimeColumn_0200OnServer() const
{
    set0200Timezone();

    // Insert
    quint64 lastId = createQuery()->from("datetime").insertGetId(
                         {{"datetime", QDateTime::fromString("2022-08-28 13:14:15+02:00",
                                                             Qt::ISODate)}});

    // Verify
    {
        auto query = createQuery()->from("datetime").first({ID, "datetime"});

        QCOMPARE(query.size(), 1);

        QCOMPARE(query.value(ID).value<quint64>(), lastId);

        const auto datetimeDbVariant = query.value("datetime");

        auto typeId = typeIdWrapper(datetimeDbVariant);
        QCOMPARE(typeId(), QMetaType::QDateTime);

        /* The time zone must be as is defined in the qt_timezone connection
           configuration, TinyORM QueryBuilder fixes the buggy time zone behavior
           of the QMYSQL driver. */
        const auto datetimeActual = datetimeDbVariant.value<QDateTime>();
        const auto datetimeExpected = QDateTime::fromString("2022-08-28 13:14:15+02:00",
                                                            Qt::ISODate);
        QCOMPARE(datetimeActual, datetimeExpected);
        QCOMPARE(datetimeActual, datetimeExpected.toLocalTime());
        QCOMPARE(datetimeActual.timeZone(), *TimeZone0200);
    }

    // Restore
    restore(lastId, true);
}

void tst_MySql_QDateTime::insert_QString_DatetimeColumn_0200OnServer() const
{
    set0200Timezone();

    // Insert
    quint64 lastId = createQuery()->from("datetime").insertGetId(
                         {{"datetime", QString("2022-08-28 13:14:15")}});

    // Verify
    {
        auto query = createQuery()->from("datetime").first({ID, "datetime"});

        QCOMPARE(query.size(), 1);

        QCOMPARE(query.value(ID).value<quint64>(), lastId);

        const auto datetimeDbVariant = query.value("datetime");

        auto typeId = typeIdWrapper(datetimeDbVariant);
        QCOMPARE(typeId(), QMetaType::QDateTime);

        /* The time zone must be as is defined in the qt_timezone connection
           configuration, TinyORM QueryBuilder fixes the buggy time zone behavior
           of the QMYSQL driver. */
        const auto datetimeActual = datetimeDbVariant.value<QDateTime>();
        const auto datetimeExpected = QDateTime::fromString("2022-08-28 13:14:15+02:00",
                                                            Qt::ISODate);
        QCOMPARE(datetimeActual, datetimeExpected);
        QCOMPARE(datetimeActual, datetimeExpected.toLocalTime());
        QCOMPARE(datetimeActual.timeZone(), *TimeZone0200);
    }

    // Restore
    restore(lastId, true);
}

void tst_MySql_QDateTime::
insert_QDateTime_UtcTimezone_TimestampColumn_0200OnServer() const
{
    set0200Timezone();

    // Insert
    quint64 lastId = createQuery()->from("datetime").insertGetId(
                         {{"timestamp", QDateTime::fromString("2022-08-28 13:14:15z",
                                                              Qt::ISODate)}});

    // Verify
    {
        auto query = createQuery()->from("datetime").first({ID, "timestamp"});

        QCOMPARE(query.size(), 1);

        QCOMPARE(query.value(ID).value<quint64>(), lastId);

        const auto timestampDbVariant = query.value("timestamp");

        auto typeId = typeIdWrapper(timestampDbVariant);
        QCOMPARE(typeId(), QMetaType::QDateTime);

        /* The time zone must be as is defined in the qt_timezone connection
           configuration, TinyORM QueryBuilder fixes the buggy time zone behavior
           of the QMYSQL driver. */
        const auto timestampActual = timestampDbVariant.value<QDateTime>();
        const auto timestampExpected = QDateTime::fromString("2022-08-28 15:14:15+02:00",
                                                             Qt::ISODate);
        QCOMPARE(timestampActual, timestampExpected);
        QCOMPARE(timestampActual, timestampExpected.toLocalTime());
        QCOMPARE(timestampActual.timeZone(), *TimeZone0200);
    }

    // Restore
    restore(lastId, true);
}

void tst_MySql_QDateTime::
insert_QDateTime_0200Timezone_TimestampColumn_0200OnServer() const
{
    set0200Timezone();

    // Insert
    quint64 lastId = createQuery()->from("datetime").insertGetId(
                         {{"timestamp",
                           QDateTime::fromString("2022-08-28 13:14:15+02:00",
                                                 Qt::ISODate)}});

    // Verify
    {
        auto query = createQuery()->from("datetime").first({ID, "timestamp"});

        QCOMPARE(query.size(), 1);

        QCOMPARE(query.value(ID).value<quint64>(), lastId);

        const auto timestampDbVariant = query.value("timestamp");

        auto typeId = typeIdWrapper(timestampDbVariant);
        QCOMPARE(typeId(), QMetaType::QDateTime);

        /* The time zone must be as is defined in the qt_timezone connection
           configuration, TinyORM QueryBuilder fixes the buggy time zone behavior
           of the QMYSQL driver. */
        const auto timestampActual = timestampDbVariant.value<QDateTime>();
        const auto timestampExpected = QDateTime::fromString("2022-08-28 13:14:15+02:00",
                                                             Qt::ISODate);
        QCOMPARE(timestampActual, timestampExpected);
        QCOMPARE(timestampActual, timestampExpected.toLocalTime());
        QCOMPARE(timestampActual.timeZone(), *TimeZone0200);
    }

    // Restore
    restore(lastId, true);
}

void tst_MySql_QDateTime::insert_QString_TimestampColumn_0200OnServer() const
{
    set0200Timezone();

    // Insert
    quint64 lastId = createQuery()->from("datetime").insertGetId(
                         {{"timestamp", QString("2022-08-28 13:14:15")}});

    // Verify
    {
        auto query = createQuery()->from("datetime").first({ID, "timestamp"});

        QCOMPARE(query.size(), 1);

        QCOMPARE(query.value(ID).value<quint64>(), lastId);

        const auto timestampDbVariant = query.value("timestamp");

        auto typeId = typeIdWrapper(timestampDbVariant);
        QCOMPARE(typeId(), QMetaType::QDateTime);

        /* The time zone must be as is defined in the qt_timezone connection
           configuration, TinyORM QueryBuilder fixes the buggy time zone behavior
           of the QMYSQL driver. */
        const auto timestampActual = timestampDbVariant.value<QDateTime>();
        const auto timestampExpected = QDateTime::fromString("2022-08-28 13:14:15+02:00",
                                                             Qt::ISODate);
        QCOMPARE(timestampActual, timestampExpected);
        QCOMPARE(timestampActual, timestampExpected.toLocalTime());
        QCOMPARE(timestampActual.timeZone(), *TimeZone0200);
    }

    // Restore
    restore(lastId, true);
}

/* QDate */

/* Raw QSqlQuery */

/* Server timezone UTC */

void tst_MySql_QDateTime::insert_Qt_QDate_UtcTimezone_DateColumn_UtcOnServer() const
{
    quint64 lastId = 0;

    // Insert
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare("insert into `datetime` (`date`) values (?)"));

        qtQuery.addBindValue(QDate::fromString("2022-08-28", Qt::ISODate));

        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && !qtQuery.isSelect());
        QCOMPARE(qtQuery.numRowsAffected(), 1);

        lastId = qtQuery.lastInsertId().value<quint64>();
    }

    // Verify
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare(
                    "select `id`, `date` from `datetime` where `id` = ?"));

        qtQuery.addBindValue(lastId);

        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && qtQuery.isSelect());
        QCOMPARE(qtQuery.size(), 1);

        QVERIFY(qtQuery.first());

        QCOMPARE(qtQuery.value(ID).value<quint64>(), lastId);

        const auto dateDbVariant = qtQuery.value("date");

        auto typeId = typeIdWrapper(dateDbVariant);
        QCOMPARE(typeId(), QMetaType::QDate);

        const auto dateActual = dateDbVariant.value<QDate>();
        const auto dateExpected = QDate::fromString("2022-08-28", Qt::ISODate);
        QCOMPARE(dateActual, dateExpected);
    }

    // Restore
    restore(lastId);
}

void tst_MySql_QDateTime::insert_Qt_QString_DateColumn_UtcOnServer() const
{
    quint64 lastId = 0;

    // Insert
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare("insert into `datetime` (`date`) values (?)"));

        qtQuery.addBindValue(QString("2022-08-28"));

        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && !qtQuery.isSelect());
        QCOMPARE(qtQuery.numRowsAffected(), 1);

        lastId = qtQuery.lastInsertId().value<quint64>();
    }

    // Verify
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare(
                    "select `id`, `date` from `datetime` where `id` = ?"));

        qtQuery.addBindValue(lastId);

        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && qtQuery.isSelect());
        QCOMPARE(qtQuery.size(), 1);

        QVERIFY(qtQuery.first());

        QCOMPARE(qtQuery.value(ID).value<quint64>(), lastId);

        const auto dateDbVariant = qtQuery.value("date");

        auto typeId = typeIdWrapper(dateDbVariant);
        QCOMPARE(typeId(), QMetaType::QDate);

        const auto dateActual = dateDbVariant.value<QDate>();
        const auto dateExpected = QDate::fromString("2022-08-28", Qt::ISODate);
        QCOMPARE(dateActual, dateExpected);
    }

    // Restore
    restore(lastId);
}

/* Server timezone +02:00 */

void tst_MySql_QDateTime::insert_Qt_QDate_UtcTimezone_DateColumn_0200OnServer() const
{
    set0200Timezone();

    quint64 lastId = 0;

    // Insert
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare("insert into `datetime` (`date`) values (?)"));

        qtQuery.addBindValue(QDate::fromString("2022-08-28", Qt::ISODate));

        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && !qtQuery.isSelect());
        QCOMPARE(qtQuery.numRowsAffected(), 1);

        lastId = qtQuery.lastInsertId().value<quint64>();
    }

    // Verify
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare(
                    "select `id`, `date` from `datetime` where `id` = ?"));

        qtQuery.addBindValue(lastId);

        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && qtQuery.isSelect());
        QCOMPARE(qtQuery.size(), 1);

        QVERIFY(qtQuery.first());

        QCOMPARE(qtQuery.value(ID).value<quint64>(), lastId);

        const auto dateDbVariant = qtQuery.value("date");

        auto typeId = typeIdWrapper(dateDbVariant);
        QCOMPARE(typeId(), QMetaType::QDate);

        const auto dateActual = dateDbVariant.value<QDate>();
        const auto dateExpected = QDate::fromString("2022-08-28", Qt::ISODate);
        QCOMPARE(dateActual, dateExpected);
    }

    // Restore
    restore(lastId, true);
}

void tst_MySql_QDateTime::insert_Qt_QString_DateColumn_0200OnServer() const
{
    set0200Timezone();

    quint64 lastId = 0;

    // Insert
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare("insert into `datetime` (`date`) values (?)"));

        qtQuery.addBindValue(QString("2022-08-28"));

        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && !qtQuery.isSelect());
        QCOMPARE(qtQuery.numRowsAffected(), 1);

        lastId = qtQuery.lastInsertId().value<quint64>();
    }

    // Verify
    {
        auto qtQuery = DB::connection(m_connection).getQtQuery();

        QVERIFY(qtQuery.prepare(
                    "select `id`, `date` from `datetime` where `id` = ?"));

        qtQuery.addBindValue(lastId);

        QVERIFY(qtQuery.exec());

        QVERIFY(!qtQuery.lastError().isValid());
        QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && qtQuery.isSelect());
        QCOMPARE(qtQuery.size(), 1);

        QVERIFY(qtQuery.first());

        QCOMPARE(qtQuery.value(ID).value<quint64>(), lastId);

        const auto dateDbVariant = qtQuery.value("date");

        auto typeId = typeIdWrapper(dateDbVariant);
        QCOMPARE(typeId(), QMetaType::QDate);

        const auto dateActual = dateDbVariant.value<QDate>();
        const auto dateExpected = QDate::fromString("2022-08-28", Qt::ISODate);
        QCOMPARE(dateActual, dateExpected);
    }

    // Restore
    restore(lastId, true);
}

/* Orm::QueryBuilder */

/* Server timezone UTC */

void tst_MySql_QDateTime::insert_QDate_UtcTimezone_DateColumn_UtcOnServer() const
{
    // Insert
    quint64 lastId = createQuery()->from("datetime").insertGetId(
                         {{"date", QDate::fromString("2022-08-28", Qt::ISODate)}});

    // Verify
    {
        auto query = createQuery()->from("datetime").first({ID, "date"});

        QCOMPARE(query.size(), 1);

        QCOMPARE(query.value(ID).value<quint64>(), lastId);

        const auto dateDbVariant = query.value("date");

        auto typeId = typeIdWrapper(dateDbVariant);
        QCOMPARE(typeId(), QMetaType::QDate);

        const auto dateActual = dateDbVariant.value<QDate>();
        const auto dateExpected = QDate::fromString("2022-08-28", Qt::ISODate);
        QCOMPARE(dateActual, dateExpected);
    }

    // Restore
    restore(lastId);
}

void tst_MySql_QDateTime::insert_QString_DateColumn_UtcOnServer() const
{
    // Insert
    quint64 lastId = createQuery()->from("datetime").insertGetId(
                         {{"date", QString("2022-08-28")}});

    // Verify
    {
        auto query = createQuery()->from("datetime").first({ID, "date"});

        QCOMPARE(query.size(), 1);

        QCOMPARE(query.value(ID).value<quint64>(), lastId);

        const auto dateDbVariant = query.value("date");

        auto typeId = typeIdWrapper(dateDbVariant);
        QCOMPARE(typeId(), QMetaType::QDate);

        const auto dateActual = dateDbVariant.value<QDate>();
        const auto dateExpected = QDate::fromString("2022-08-28", Qt::ISODate);
        QCOMPARE(dateActual, dateExpected);
    }

    // Restore
    restore(lastId);
}

/* Server timezone +02:00 */

void tst_MySql_QDateTime::insert_QDate_UtcTimezone_DateColumn_0200OnServer() const
{
    set0200Timezone();

    // Insert
    quint64 lastId = createQuery()->from("datetime").insertGetId(
                         {{"date", QDate::fromString("2022-08-28", Qt::ISODate)}});

    // Verify
    {
        auto query = createQuery()->from("datetime").first({ID, "date"});

        QCOMPARE(query.size(), 1);

        QCOMPARE(query.value(ID).value<quint64>(), lastId);

        const auto dateDbVariant = query.value("date");

        auto typeId = typeIdWrapper(dateDbVariant);
        QCOMPARE(typeId(), QMetaType::QDate);

        const auto dateActual = dateDbVariant.value<QDate>();
        const auto dateExpected = QDate::fromString("2022-08-28", Qt::ISODate);
        QCOMPARE(dateActual, dateExpected);
    }

    // Restore
    restore(lastId, true);
}

void tst_MySql_QDateTime::insert_QString_DateColumn_0200OnServer() const
{
    set0200Timezone();

    // Insert
    quint64 lastId = createQuery()->from("datetime").insertGetId(
                         {{"date", QString("2022-08-28")}});

    // Verify
    {
        auto query = createQuery()->from("datetime").first({ID, "date"});

        QCOMPARE(query.size(), 1);

        QCOMPARE(query.value(ID).value<quint64>(), lastId);

        const auto dateDbVariant = query.value("date");

        auto typeId = typeIdWrapper(dateDbVariant);
        QCOMPARE(typeId(), QMetaType::QDate);

        const auto dateActual = dateDbVariant.value<QDate>();
        const auto dateExpected = QDate::fromString("2022-08-28", Qt::ISODate);
        QCOMPARE(dateActual, dateExpected);
    }

    // Restore
    restore(lastId, true);
}

/* private */

/* Common */

std::shared_ptr<QueryBuilder>
tst_MySql_QDateTime::createQuery() const
{
    return DB::connection(m_connection).query();
}

/* QDateTime with/without timezone */

void tst_MySql_QDateTime::setUtcTimezone() const
{
    setTimezone(UTC, {QtTimeZoneType::QtTimeSpec, QVariant::fromValue(Qt::UTC)});
}

void tst_MySql_QDateTime::set0200Timezone() const
{
    setTimezone(QStringLiteral("+02:00"), {QtTimeZoneType::QTimeZone,
                                           QVariant::fromValue(*TimeZone0200)});
}

void tst_MySql_QDateTime::setTimezone(const QString &timezone,
                                      Orm::QtTimeZoneConfig &&qtTimeZone) const
{
    const auto qtQuery = DB::unprepared(
                             QStringLiteral("set time_zone=\"%1\";").arg(timezone),
                             m_connection);

    QVERIFY(!qtQuery.isValid() && qtQuery.isActive() && !qtQuery.isSelect());

    DB::connection(m_connection).setQtTimeZone(std::move(qtTimeZone));
}

void tst_MySql_QDateTime::restore(const quint64 lastId,
                                  const bool restoreTimezone) const
{
    const auto [affected, query] = createQuery()->from("datetime").remove(lastId);

    QVERIFY(!query.lastError().isValid());
    QVERIFY(!query.isValid() && query.isActive() && !query.isSelect());
    QCOMPARE(affected, 1);

    if (!restoreTimezone)
        return;

    // Restore also the MySQL timezone session variable to auto tests default UTC value
    setUtcTimezone();
}

QTEST_MAIN(tst_MySql_QDateTime)

#include "tst_mysql_qdatetime.moc"