#pragma once
#ifndef ORM_CONCERNS_LOGSQUERIES_HPP
#define ORM_CONCERNS_LOGSQUERIES_HPP

#include "orm/macros/systemheader.hpp"
TINY_SYSTEM_HEADER

#include <QtSql/QSqlQuery>

#include <memory>
#include <optional>

#include "orm/macros/export.hpp"
#include "orm/types/log.hpp"

TINYORM_BEGIN_COMMON_NAMESPACE

namespace Orm
{

class DatabaseConnection;

namespace Concerns
{

    /*! Logs executed queries to the console. */
    class SHAREDLIB_EXPORT LogsQueries
    {
        Q_DISABLE_COPY(LogsQueries)

    public:
        /*! Default constructor. */
        inline LogsQueries() = default;
        /*! Virtual destructor, to pass -Weffc++. */
        inline virtual ~LogsQueries() = default;

        /*! Log a query into the connection's query log. */
        inline void logQuery(const QSqlQuery &query, std::optional<qint64> elapsed) const;
        /*! Log a query into the connection's query log. */
        inline void logQuery(const std::tuple<int, QSqlQuery> &queryResult,
                             std::optional<qint64> elapsed) const;
        /*! Log a query into the connection's query log in the pretending mode. */
        void logQueryForPretend(const QString &query,
                                const QVector<QVariant> &bindings) const;
        /*! Log a transaction query into the connection's query log. */
        void logTransactionQuery(const QString &query,
                                 std::optional<qint64> elapsed) const;
        /*! Log a transaction query into the connection's query log
            in the pretending mode. */
        void logTransactionQueryForPretend(const QString &query) const;

        /*! Get the connection query log. */
        inline std::shared_ptr<QVector<Log>> getQueryLog() const;
        /*! Clear the query log. */
        void flushQueryLog();
        /*! Enable the query log on the connection. */
        void enableQueryLog();
        /*! Disable the query log on the connection. */
        inline void disableQueryLog();
        /*! Determine whether we're logging queries. */
        inline bool logging() const;
        /*! The current order value for a query log record. */
        inline static std::size_t getQueryLogOrder();

        /*! Determine whether debugging SQL queries is enabled/disabled (logging
            to the console using qDebug()). */
        inline bool debugSql() const;
        /*! Disable debugging SQL queries (logging to the console using qDebug()). */
        inline void disableDebugSql();
        /*! Enable debugging SQL queries (logging to the console using qDebug()). */
        inline void enableDebugSql();

    protected:
        /*! Execute the given callback in "dry run" mode. */
        QVector<Log>
        withFreshQueryLog(const std::function<QVector<Log>()> &callback);

        /*! Indicates if changes have been made to the database. */
        bool m_recordsModified = false;
        /*! All of the queries run against the connection. */
        std::shared_ptr<QVector<Log>> m_queryLog = nullptr;
        /*! ID of the query log record. */
        inline static std::atomic<std::size_t> m_queryLogId = 0;

#ifdef TINYORM_DEBUG_SQL
        /*! Indicates whether logging of sql queries is enabled. */
        bool m_debugSql = true;
#else
        /*! Indicates whether logging of sql queries is enabled. */
        bool m_debugSql = false;
#endif

    private:
        /*! Log a query into the connection's query log. */
        void logQueryInternal(const QSqlQuery &query,
                              std::optional<qint64> elapsed) const;

        /*! Convert a named bindings map to the positional bindings vector. */
        QVector<QVariant>
        convertNamedToPositionalBindings(QVariantMap &&bindings) const;

        /*! Dynamic cast *this to the DatabaseConnection & derived type. */
        const DatabaseConnection &databaseConnection() const;

        /*! Indicates whether queries are being logged (private intentionally). */
        bool m_loggingQueries = false;
        /*! All of the queries run against the connection. */
        std::shared_ptr<QVector<Log>> m_queryLogForPretend = nullptr;
    };

    /* public */

    void LogsQueries::logQuery(
            const QSqlQuery &queryResult, std::optional<qint64> elapsed) const
    {
        logQueryInternal(queryResult, elapsed);
    }

    void LogsQueries::logQuery(
            const std::tuple<int, QSqlQuery> &queryResult,
            std::optional<qint64> elapsed) const
    {
        logQueryInternal(std::get<1>(queryResult), elapsed);
    }

    std::shared_ptr<QVector<Log>> LogsQueries::getQueryLog() const
    {
        return m_queryLog;
    }

    void LogsQueries::disableQueryLog()
    {
        m_loggingQueries = false;
    }

    bool LogsQueries::logging() const
    {
        return m_loggingQueries;
    }

    std::size_t LogsQueries::getQueryLogOrder()
    {
        return m_queryLogId;
    }

    bool LogsQueries::debugSql() const
    {
        return m_debugSql;
    }

    void LogsQueries::disableDebugSql()
    {
        m_debugSql = false;
    }

    void LogsQueries::enableDebugSql()
    {
        m_debugSql = true;
    }

} // namespace Concerns
} // namespace Orm

TINYORM_END_COMMON_NAMESPACE

#endif // ORM_CONCERNS_LOGSQUERIES_HPP
