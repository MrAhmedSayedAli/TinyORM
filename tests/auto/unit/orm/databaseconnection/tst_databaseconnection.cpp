#include <QCoreApplication>
#include <QtTest>

#include "orm/db.hpp"
#include "orm/mysqlconnection.hpp"

#include "database.hpp"

using Orm::MySqlConnection;

using TestUtils::Database;

// TEST exceptions in tests, qt doesn't care about exceptions, totally ignore it, so when the exception is thrown, I didn't get any exception message or something similar, nothing 👿, try to solve it somehow 🤔 silverqx
class tst_DatabaseConnection : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase_data() const;

    void pingDatabase() const;

    void isNotMaria_OnMySqlConnection() const;
};

void tst_DatabaseConnection::initTestCase_data() const
{
    const auto &connections = Database::createConnections();

    if (connections.isEmpty())
        QSKIP(QStringLiteral("%1 autotest skipped, environment variables "
                             "for ANY connection have not been defined.")
              .arg("tst_DatabaseConnection").toUtf8().constData(), );

    QTest::addColumn<QString>("connection");

    // Run all tests for all supported database connections
    for (const auto &connection : connections)
        QTest::newRow(connection.toUtf8().constData()) << connection;
}

void tst_DatabaseConnection::pingDatabase() const
{
    QFETCH_GLOBAL(QString, connection);

    auto &connection_ = DB::connection(connection);

    if (const auto driverName = connection_.driverName();
        driverName != "QMYSQL"
    )
        QSKIP(QStringLiteral("The '%1' database driver doesn't support ping command.")
              .arg(driverName).toUtf8().constData(), );

    const auto result = connection_.pingDatabase();

    QVERIFY2(result, "Ping database failed.");
}

void tst_DatabaseConnection::isNotMaria_OnMySqlConnection() const
{
    QFETCH_GLOBAL(QString, connection);

    auto &connection_ = DB::connection(connection);

    if (const auto driverName = connection_.driverName();
        driverName != "QMYSQL"
    )
        QSKIP(QStringLiteral("The '%1' database driver doesn't implement isMaria() "
                             "method.")
              .arg(driverName).toUtf8().constData(), );

    const auto expected = !(connection == Database::MYSQL);

    const auto isMaria = dynamic_cast<MySqlConnection &>(connection_).isMaria();

    QCOMPARE(isMaria, expected);
}

QTEST_MAIN(tst_DatabaseConnection)

#include "tst_databaseconnection.moc"
