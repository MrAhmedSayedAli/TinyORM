#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QtSql/QSqlQuery>

#include "orm/configuration.hpp"
#include "orm/connectioninterface.hpp"
#include "orm/connectionresolverinterface.hpp"
#include "orm/connectors/connectionfactory.hpp"

#ifdef TINYORM_COMMON_NAMESPACE
namespace TINYORM_COMMON_NAMESPACE
{
#endif
namespace Orm
{
namespace Query
{
    class Builder;
}
    using QueryBuilder = Query::Builder;

    class SHAREDLIB_EXPORT DatabaseManager final : public ConnectionResolverInterface
    {
        friend class DB;

        Q_DISABLE_COPY(DatabaseManager)

    public:
        virtual ~DatabaseManager();

        /*! Factory method to create DatabaseManager instance and register a new connection as default connection at once. */
        static std::unique_ptr<DatabaseManager>
        create(const QVariantHash &config,
               const QString &connection = QLatin1String(defaultConnectionName));
        /*! Factory method to create DatabaseManager instance and set connections at once. */
        static std::unique_ptr<DatabaseManager>
        create(const ConfigurationsType &configs,
               const QString &defaultConnection = QLatin1String(defaultConnectionName));

        /* Proxy methods to the DatabaseConnection */
        /*! Begin a fluent query against a database table for the connection. */
        QSharedPointer<QueryBuilder>
        table(const QString &table, const QString &as = "",
              const QString &connection = "");

        /*! Get a new query builder instance for the connection. */
        QSharedPointer<QueryBuilder> query(const QString &connection = "");
        /*! Get a new QSqlQuery instance for the connection. */
        QSqlQuery qtQuery(const QString &connection = "");

        /*! Create a new raw query expression. */
        inline Query::Expression raw(const QVariant &value)
        { return value; }

        // TODO next add support for named bindings, Using Named Bindings silverqx
        /*! Run a select statement against the database. */
        std::tuple<bool, QSqlQuery>
        select(const QString &query, const QVector<QVariant> &bindings = {});
        /*! Run a select statement and return a single result. */
        std::tuple<bool, QSqlQuery>
        selectOne(const QString &query, const QVector<QVariant> &bindings = {});
        /*! Run an insert statement against the database. */
        std::tuple<bool, QSqlQuery>
        insert(const QString &query, const QVector<QVariant> &bindings = {});
        /*! Run an update statement against the database. */
        std::tuple<int, QSqlQuery>
        update(const QString &query, const QVector<QVariant> &bindings = {});
        /*! Run a delete statement against the database. */
        std::tuple<int, QSqlQuery>
        remove(const QString &query, const QVector<QVariant> &bindings = {});
        /*! Execute an SQL statement and return the boolean result and QSqlQuery. */
        std::tuple<bool, QSqlQuery>
        statement(const QString &query,
                  const QVector<QVariant> &bindings = {});

        /*! Start a new database transaction. */
        bool beginTransaction();
        /*! Commit the active database transaction. */
        bool commit();
        /*! Rollback the active database transaction. */
        bool rollBack();
        /*! Start a new named transaction savepoint. */
        bool savepoint(const QString &id);
        /*! Start a new named transaction savepoint. */
        bool savepoint(size_t id);
        /*! Rollback to a named transaction savepoint. */
        bool rollbackToSavepoint(const QString &id);
        /*! Rollback to a named transaction savepoint. */
        bool rollbackToSavepoint(size_t id);
        /*! Get the number of active transactions. */
        uint transactionLevel();

        /* DatabaseManager */
        /*! Obtain a database connection instance, for now it's singleton. */
        static DatabaseManager *instance();

        /*! Get a database connection instance. */
        ConnectionInterface &connection(const QString &name = "") override;
        /*! Register a connection with the manager. */
        DatabaseManager &
        addConnection(const QVariantHash &config,
                      const QString &name = QLatin1String(defaultConnectionName));

        /*! Reconnect to the given database. */
        ConnectionInterface &reconnect(QString name = "");
        /*! Disconnect from the given database. */
        void disconnect(QString name = "") const;

        /*! Get all of the support drivers. */
        QStringList supportedDrivers() const;
        /*! Returns a list containing the names of all connections. */
        QStringList connectionNames() const;
        /*! Returns a list containing the names of opened connections. */
        QStringList openedConnectionNames() const;

        /*! Get the default connection name. */
        const QString &getDefaultConnection() const override;
        /*! Set the default connection name. */
        void setDefaultConnection(const QString &defaultConnection) override;

        // TODO duplicate, extract to some internal types silverqx
        /*! Reconnector lambda type. */
        using ReconnectorType = std::function<void(const DatabaseConnection &)>;
        /*! Set the database reconnector callback. */
        DatabaseManager &setReconnector(const ReconnectorType &reconnector);

        /* Queries execution time counter */
        /*! Determine whether we're counting queries execution time. */
        bool countingElapsed(const QString &connection = "");
        /*! Enable counting queries execution time on the current connection. */
        DatabaseConnection &enableElapsedCounter(const QString &connection = "");
        /*! Disable counting queries execution time on the current connection. */
        DatabaseConnection &disableElapsedCounter(const QString &connection = "");
        /*! Obtain queries execution time. */
        qint64 getElapsedCounter(const QString &connection = "");
        /*! Obtain and reset queries execution time. */
        qint64 takeElapsedCounter(const QString &connection = "");
        /*! Reset queries execution time. */
        DatabaseConnection &resetElapsedCounter(const QString &connection = "");

        /*! Determine whether any connection is counting queries execution time. */
        bool anyCountingElapsed();
        /*! Enable counting queries execution time on all connections. */
        void enableAllElapsedCounters();
        /*! Disable counting queries execution time on all connections. */
        void disableAllElapsedCounters();
        /*! Obtain queries execution time from all connections. */
        qint64 getAllElapsedCounters();
        /*! Obtain and reset queries execution time on all active connections. */
        qint64 takeAllElapsedCounters();
        /*! Reset queries execution time on all active connections. */
        void resetAllElapsedCounters();

        /*! Enable counting queries execution time on given connections. */
        inline void enableElapsedCounters(const QStringList &connections);
        /*! Disable counting queries execution time on given connections. */
        inline void disableElapsedCounters(const QStringList &connections);
        /*! Obtain queries execution time from given connections. */
        inline qint64 getElapsedCounters(const QStringList &connections);
        /*! Obtain and reset queries execution time on given connections. */
        inline qint64 takeElapsedCounters(const QStringList &connections);
        /*! Reset queries execution time on given connections. */
        inline void resetElapsedCounters(const QStringList &connections);

        /* Queries executed counter */
        /*! Determine whether we're counting the number of executed queries. */
        bool countingStatements(const QString &connection = "");
        /*! Enable counting the number of executed queries on the current connection. */
        DatabaseConnection &
        enableStatementsCounter(const QString &connection = "");
        /*! Disable counting the number of executed queries on the current connection. */
        DatabaseConnection &
        disableStatementsCounter(const QString &connection = "");
        /*! Obtain the number of executed queries. */
        const StatementsCounter &
        getStatementsCounter(const QString &connection = "");
        /*! Obtain and reset the number of executed queries. */
        StatementsCounter
        takeStatementsCounter(const QString &connection = "");
        /*! Reset the number of executed queries. */
        DatabaseConnection &
        resetStatementsCounter(const QString &connection = "");

        /*! Determine whether any connection is counting the number of executed
            queries. */
        bool anyCountingStatements();
        /*! Enable counting the number of executed queries on all connections. */
        void enableAllStatementCounters();
        /*! Disable counting the number of executed queries on all connections. */
        void disableAllStatementCounters();
        /*! Obtain the number of executed queries on all active connections. */
        StatementsCounter getAllStatementCounters();
        /*! Obtain and reset the number of executed queries on all active connections. */
        StatementsCounter takeAllStatementCounters();
        /*! Reset the number of executed queries on all active connections. */
        void resetAllStatementCounters();

        /*! Enable counting the number of executed queries on given connections. */
        inline void enableStatementCounters(const QStringList &connections);
        /*! Disable counting the number of executed queries on given connections. */
        inline void disableStatementCounters(const QStringList &connections);
        /*! Obtain the number of executed queries on given connections. */
        inline StatementsCounter getStatementCounters(const QStringList &connections);
        /*! Obtain and reset the number of executed queries on given connections. */
        inline StatementsCounter takeStatementCounters(const QStringList &connections);
        /*! Reset the number of executed queries on given connections. */
        inline void resetStatementCounters(const QStringList &connections);

    protected:
        /*! Default connection name. */
        static const char *defaultConnectionName;

        explicit DatabaseManager(
                const QString &defaultConnection = QLatin1String(defaultConnectionName));

        explicit DatabaseManager(
                const QVariantHash &config,
                const QString &name = QLatin1String(defaultConnectionName),
                const QString &defaultConnection = QLatin1String(defaultConnectionName));
        explicit DatabaseManager(
                const ConfigurationsType &configs,
                const QString &defaultConnection = QLatin1String(defaultConnectionName));

        /*! Setup the default database connection reconnector. */
        DatabaseManager &setupDefaultReconnector();

        /*! Parse the connection into an array of the name and read / write type. */
        const QString &parseConnectionName(const QString &name) const;

        /*! Make the database connection instance. */
        std::unique_ptr<DatabaseConnection>
        makeConnection(const QString &name);

        /*! Get the configuration for a connection. */
        QVariantHash &configuration(QString name);

        /*! Prepare the database connection instance. */
        std::unique_ptr<DatabaseConnection>
        configure(std::unique_ptr<DatabaseConnection> connection) const;

        /*! Refresh an underlying QSqlDatabase connection on a given connection. */
        DatabaseConnection &refreshQtConnections(const QString &name);

        /*! The database connection factory instance. */
        const Connectors::ConnectionFactory m_factory;
        /*! Database configuration. */
        Configuration m_config;
        /*! The active connection instances. */
        std::unordered_map<QString, std::unique_ptr<DatabaseConnection>> m_connections;
        /*! The callback to be executed to reconnect to a database. */
        ReconnectorType m_reconnector;

    private:
        /*! Database Manager instance. */
        static DatabaseManager *m_instance;
    };

} // namespace Orm
#ifdef TINYORM_COMMON_NAMESPACE
} // namespace TINYORM_COMMON_NAMESPACE
#endif

#endif // DATABASEMANAGER_H
