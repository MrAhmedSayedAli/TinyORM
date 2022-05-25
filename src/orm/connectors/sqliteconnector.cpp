#include "orm/connectors/sqliteconnector.hpp"

#include <QFile>
#include <QtSql/QSqlQuery>

#include "orm/constants.hpp"
#include "orm/exceptions/queryerror.hpp"
#include "orm/utils/type.hpp"

using Orm::Constants::check_database_exists;
using Orm::Constants::database_;
using Orm::Constants::foreign_key_constraints;
using Orm::Constants::NAME;

TINYORM_BEGIN_COMMON_NAMESPACE

namespace Orm::Connectors
{

ConnectionName
SQLiteConnector::connect(const QVariantHash &config) const
{
    auto name = config[NAME].value<QString>();

    const auto options = getOptions(config);

    /* SQLite supports "in-memory" databases that only last as long as the owning
       connection does. These are useful for tests or for short lifetime store
       querying. In-memory databases may only have a single open connection. */
    if (config[database_] == ":memory:") {
        // sqlite :memory: driver
        createConnection(name, config, options);

        return name;
    }

    /* Here we'll verify that the SQLite database exists before going any further
       as the developer probably wants to know if the database exists and this
       SQLite driver will not throw any exception if it does not by default. */
    checkDatabaseExists(config);

    // Create and open new database connection
    const auto connection = createConnection(name, config, options);

    // Foreign key constraints
    configureForeignKeyConstraints(connection, config);

    /* Return only connection name, because QSqlDatabase documentation doesn't
       recommend to store QSqlDatabase instance as a class data member, we can
       simply obtain the connection by QSqlDatabase::connection() when needed. */
    return name;
}

const QVariantHash &
SQLiteConnector::getConnectorOptions() const
{
    return m_options;
}

void SQLiteConnector::parseConfigOptions(QVariantHash &/*unused*/) const
{}

void SQLiteConnector::configureForeignKeyConstraints(
        const QSqlDatabase &connection, const QVariantHash &config) const
{
    // This ensures default SQLite behavior
    if (!config.contains(foreign_key_constraints))
        return;

    const auto foreignKeyConstraints =
            config[foreign_key_constraints].value<bool>() ? QStringLiteral("ON")
                                                          : QStringLiteral("OFF");

    QSqlQuery query(connection);
    // FEATURE schema builder, use DatabaseConnection::statement(), to set recordsHaveBeenModied to true, foreign key constraints silverqx
    if (query.exec(QStringLiteral("PRAGMA foreign_keys = %1;")
                   .arg(foreignKeyConstraints)))
        return;

    throw Exceptions::QueryError(m_configureErrorMessage.arg(__tiny_func__), query);
}

void SQLiteConnector::checkDatabaseExists(const QVariantHash &config) const
{
    const auto path = config[database_].value<QString>();

    // Default behavior is to check database existence
    bool checkDatabaseExists = true;
    if (const auto &configCheckDatabase = config[check_database_exists];
        configCheckDatabase.isValid() && !configCheckDatabase.isNull()
    )
        checkDatabaseExists = config[check_database_exists].value<bool>();

    if (checkDatabaseExists && !QFile::exists(path))
        throw Exceptions::RuntimeError(
                QStringLiteral("SQLite Database file '%1' does not exist.").arg(path));
}

} // namespace Orm::Connectors

TINYORM_END_COMMON_NAMESPACE
