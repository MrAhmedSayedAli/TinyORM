#pragma once
#ifndef ORM_QUERY_GRAMMARS_GRAMMAR_HPP
#define ORM_QUERY_GRAMMARS_GRAMMAR_HPP

#include "orm/macros/systemheader.hpp"
TINY_SYSTEM_HEADER

#include <optional>

#include "orm/basegrammar.hpp"

TINYORM_BEGIN_COMMON_NAMESPACE

namespace Orm::Query::Grammars
{

    // FEATURE savepoints in grammar silverqx
    /*! Sql grammar base class. */
    class SHAREDLIB_EXPORT Grammar : public BaseGrammar
    {
        Q_DISABLE_COPY(Grammar)

    public:
        /*! Default constructor. */
        inline Grammar() = default;
        /*! Pure virtual destructor. */
        inline ~Grammar() override = 0;

        /*! Compile a select query into SQL. */
        QString compileSelect(QueryBuilder &query) const;

        /*! Compile an insert statement into SQL. */
        virtual QString
        compileInsert(const QueryBuilder &query,
                      const QVector<QVariantMap> &values) const;
        /*! Compile an insert ignore statement into SQL. */
        virtual QString
        compileInsertOrIgnore(const QueryBuilder &query,
                              const QVector<QVariantMap> &values) const;
        /*! Compile an insert and get ID statement into SQL. */
        inline virtual QString
        compileInsertGetId(const QueryBuilder &query,
                           const QVector<QVariantMap> &values,
                           const QString &sequence) const;

        /*! Compile an update statement into SQL. */
        virtual QString
        compileUpdate(QueryBuilder &query, const QVector<UpdateItem> &values) const;
        /*! Prepare the bindings for an update statement. */
        QVector<QVariant>
        prepareBindingsForUpdate(const BindingsMap &bindings,
                                 const QVector<UpdateItem> &values) const;

        /*! Compile a delete statement into SQL. */
        virtual QString compileDelete(QueryBuilder &query) const;
        /*! Prepare the bindings for a delete statement. */
        QVector<QVariant>
        prepareBindingsForDelete(const BindingsMap &bindings) const;

        /*! Compile a truncate table statement into SQL. Returns a map of
            the query string and bindings. */
        virtual std::unordered_map<QString, QVector<QVariant>>
        compileTruncate(const QueryBuilder &query) const;

        /*! Get the grammar specific operators. */
        virtual const QVector<QString> &getOperators() const;

    protected:
        /*! Select component types. */
        enum struct SelectComponentType
        {
             AGGREGATE,
             COLUMNS,
             FROM,
             JOINS,
             WHERES,
             GROUPS,
             HAVINGS,
             ORDERS,
             LIMIT,
             OFFSET,
             LOCK,
        };
        /*! The select component compile method and whether the component was set. */
        struct SelectComponentValue
        {
            /*! The component's compile method. */
            std::function<QString(const QueryBuilder &)> compileMethod;
            /*! Determine whether the component is set and is not empty. */
            std::function<bool(const QueryBuilder &)> isset;
        };

        /*! Map the ComponentType to a Grammar::compileXx() methods. */
        virtual const QMap<SelectComponentType, SelectComponentValue> &
        getCompileMap() const = 0;
        /*! Map the WhereType to a Grammar::whereXx() methods. */
        virtual const std::function<QString(const WhereConditionItem &)> &
        getWhereMethod(WhereType whereType) const = 0;

        /*! Determine whether the 'aggregate' component should be compiled. */
        bool shouldCompileAggregate(const std::optional<AggregateItem> &aggregate) const;
        /*! Determine whether the 'columns' component should be compiled. */
        bool shouldCompileColumns(const QueryBuilder &query) const;
        /*! Determine whether the 'from' component should be compiled. */
        bool shouldCompileFrom(const std::variant<std::monostate, QString,
                                                  Query::Expression> &from) const;

        /*! Compile the components necessary for a select clause. */
        QStringList compileComponents(const QueryBuilder &query) const;

        /*! Compile an aggregated select clause. */
        QString compileAggregate(const QueryBuilder &query) const;
        /*! Compile the "select *" portion of the query. */
        virtual QString compileColumns(const QueryBuilder &query) const;

        /*! Compile the "from" portion of the query. */
        QString compileFrom(const QueryBuilder &query) const;

        /*! Compile the "where" portions of the query. */
        QString compileWheres(const QueryBuilder &query) const;
        /*! Get the vector of all the where clauses for the query. */
        QStringList compileWheresToVector(const QueryBuilder &query) const;
        /*! Format the where clause statements into one string. */
        QString concatenateWhereClauses(const QueryBuilder &query,
                                        const QStringList &sql) const;

        /*! Compile the "join" portions of the query. */
        QString compileJoins(const QueryBuilder &query) const;

        /*! Compile the "group by" portions of the query. */
        QString compileGroups(const QueryBuilder &query) const;

        /*! Compile the "having" portions of the query. */
        QString compileHavings(const QueryBuilder &query) const;
        /*! Compile a single having clause. */
        QString compileHaving(const HavingConditionItem &having) const;
        /*! Compile a basic having clause. */
        QString compileBasicHaving(const HavingConditionItem &having) const;

        /*! Compile the "order by" portions of the query. */
        QString compileOrders(const QueryBuilder &query) const;
        /*! Compile the query orders to the vector. */
        QStringList compileOrdersToVector(const QueryBuilder &query) const;
        /*! Compile the "limit" portions of the query. */
        QString compileLimit(const QueryBuilder &query) const;
        /*! Compile the "offset" portions of the query. */
        QString compileOffset(const QueryBuilder &query) const;

        /*! Compile the lock into SQL. */
        virtual QString compileLock(const QueryBuilder &query) const;

        /*! Compile a basic where clause. */
        QString whereBasic(const WhereConditionItem &where) const;
        /*! Compile a nested where clause. */
        QString whereNested(const WhereConditionItem &where) const;
        /*! Compile a where clause comparing two columns. */
        QString whereColumn(const WhereConditionItem &where) const;
        /*! Compile a "where in" clause. */
        QString whereIn(const WhereConditionItem &where) const;
        /*! Compile a "where not in" clause. */
        QString whereNotIn(const WhereConditionItem &where) const;
        /*! Compile a "where null" clause. */
        QString whereNull(const WhereConditionItem &where) const;
        /*! Compile a "where not null" clause. */
        QString whereNotNull(const WhereConditionItem &where) const;
        /*! Compile a raw where clause. */
        QString whereRaw(const WhereConditionItem &where) const;
        /*! Compile a "where exists" clause. */
        QString whereExists(const WhereConditionItem &where) const;
        /*! Compile a "where not exists" clause. */
        QString whereNotExists(const WhereConditionItem &where) const;

        /*! Compile a insert values lists. */
        QStringList compileInsertToVector(const QVector<QVariantMap> &values) const;

        /*! Compile the columns for an update statement. */
        virtual QString
        compileUpdateColumns(const QVector<UpdateItem> &values) const;
        /*! Compile an update statement without joins into SQL. */
        virtual QString
        compileUpdateWithoutJoins(const QueryBuilder &query, const QString &table,
                                  const QString &columns, const QString &wheres) const;
        /*! Compile an update statement with joins into SQL. */
        QString
        compileUpdateWithJoins(const QueryBuilder &query, const QString &table,
                               const QString &columns, const QString &wheres) const;

        /*! Compile a delete statement without joins into SQL. */
        virtual QString
        compileDeleteWithoutJoins(const QueryBuilder &query, const QString &table,
                                  const QString &wheres) const;
        /*! Compile a delete statement with joins into SQL. */
        QString
        compileDeleteWithJoins(const QueryBuilder &query, const QString &table,
                               const QString &wheres) const;

        /*! Concatenate an array of segments, removing empties. */
        QString concatenate(const QStringList &segments) const;
        /*! Remove the leading boolean from a statement. */
        QString removeLeadingBoolean(QString statement) const;

        /*! Flat bindings map and exclude given binding types. */
        QVector<std::reference_wrapper<const QVariant>>
        flatBindingsForUpdateDelete(const BindingsMap &bindings,
                                    const QVector<BindingType> &exclude) const;
    };

    Grammar::~Grammar() = default;

    QString Grammar::compileInsertGetId(
            const QueryBuilder &query, const QVector<QVariantMap> &values,
            const QString &/*unused*/) const
    {
        return compileInsert(query, values);
    }

} // namespace Orm::Query::Grammars

TINYORM_END_COMMON_NAMESPACE

#endif // ORM_QUERY_GRAMMARS_GRAMMAR_HPP
