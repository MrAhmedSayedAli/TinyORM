#pragma once
#ifndef RELATION_H
#define RELATION_H

#include <QtSql/QSqlQuery>

#include <optional>

#include <range/v3/action/sort.hpp>
#include <range/v3/action/unique.hpp>

#include "orm/concepts.hpp"
#include "orm/tiny/tinytypes.hpp"

#ifdef TINYORM_COMMON_NAMESPACE
namespace TINYORM_COMMON_NAMESPACE
{
#endif
namespace Orm
{
namespace Query
{
    class JoinClause;
}
namespace Tiny
{
    template<class Model>
    class Builder;

    template<typename Derived, AllRelationsConcept ...AllRelations>
    class Model;

namespace Concerns
{
    template<typename Model>
    class QueriesRelationships;
}

namespace Relations
{

    /*! Base relations class. */
    template<class Model, class Related>
    class Relation
    {
        // Used by QueriesRelationships::getHasQueryByExistenceCheck()
        friend Orm::Tiny::Concerns::QueriesRelationships<Model>;

        /*! Model alias. */
        template<typename Derived>
        using BaseModel  = Orm::Tiny::Model<Derived>;
        /*! Expression alias. */
        using Expression = Orm::Query::Expression;
        /*! JoinClause alias. */
        using JoinClause = Orm::Query::JoinClause;
        /*! Alias for the QueriesRelationships callback type. */
        template<typename HasRelated>
        using CallbackType = Concerns::QueriesRelationshipsCallback<HasRelated>;

    protected:
        Relation(std::unique_ptr<Related> &&related, Model &parent,
                 const QString &relatedKey = "");

    public:
        /*! Related instance type passed to the relation. */
        using RelatedType = Related;

        inline virtual ~Relation() = default;

        /*! Set the base constraints on the relation query. */
        virtual void addConstraints() const = 0;

        /*! Run a callback with constraints disabled on the relation. */
        static std::unique_ptr<Relation<Model, Related>>
        noConstraints(const std::function<
                      std::unique_ptr<Relation<Model, Related>>()> &callback);

        /*! Set the constraints for an eager load of the relation. */
        virtual void addEagerConstraints(const QVector<Model> &models) const = 0;
        /*! Initialize the relation on a set of models. */
        virtual QVector<Model> &
        initRelation(QVector<Model> &models, const QString &relation) const = 0;
        /*! Match the eagerly loaded results to their parents. */
        virtual void match(QVector<Model> &models, QVector<Related> results,
                           const QString &relation) const = 0;
        /*! Get the results of the relationship. */
        virtual std::variant<QVector<Related>, std::optional<Related>>
        getResults() const = 0;

        /*! Get the relationship for eager loading. */
        inline QVector<Related> getEager() const
        { return get(); }
        /*! Execute the query as a "select" statement. */
        inline virtual QVector<Related>
        get(const QVector<Column> &columns = {ASTERISK}) const
        { return m_query->get(columns); }

        /* Getters / Setters */
        // CUR check all methods virtual, in child classes should be overriden silverqx
        /*! Get the underlying query for the relation. */
        inline Builder<Related> &getQuery() const
        { return *m_query; }
        /*! Get the base query builder driving the Eloquent builder. */
        inline QueryBuilder &getBaseQuery() const
        { return m_query->getQuery(); }

        /*! Get the parent model of the relation. */
        const Model &getParent() const
        { return m_parent; }
        /*! Get the related model of the relation. */
        const Related &getRelated() const
        { return *m_related; }
        /*! Get the related model of the relation. */
        Related &getRelated()
        { return *m_related; }
        /*! Get the name of the "created at" column. */
        const QString &createdAt() const
        { return m_parent.getCreatedAtColumn(); }
        /*! Get the name of the "updated at" column. */
        const QString &updatedAt() const
        { return m_parent.getUpdatedAtColumn(); }
        /*! Get the name of the related model's "updated at" column. */
        const QString &relatedUpdatedAt() const
        { return m_related->getUpdatedAtColumn(); }
        /*! Get the related key for the relationship. */
        inline const QString &getRelatedKeyName() const
        { return m_relatedKey; }
        /*! Get the fully qualified parent key name. */
        virtual QString getQualifiedParentKeyName() const;
        /*! Get the key for comparing against the parent key in "has" query. */
        virtual QString getExistenceCompareKey() const;

        /* Others */
        /*! Touch all of the related models for the relationship. */
        virtual void touch() const;
        /*! Run a raw update against the base query. */
        std::tuple<int, QSqlQuery>
        rawUpdate(const QVector<UpdateItem> &values = {}) const;

        /* TinyBuilder proxy methods */
        /*! Get a single column's value from the first result of a query. */
        QVariant value(const Column &column) const;

        /*! Get the vector with the values of a given column. */
        QVector<QVariant> pluck(const QString &column) const;
        /*! Get the vector with the values of a given column. */
        template<typename T>
        std::map<T, QVariant> pluck(const QString &column, const QString &key) const;

        /*! Find a model by its primary key. */
        virtual std::optional<Related>
        find(const QVariant &id, const QVector<Column> &columns = {ASTERISK}) const;
        /*! Find a model by its primary key or return fresh model instance. */
        virtual Related
        findOrNew(const QVariant &id, const QVector<Column> &columns = {ASTERISK}) const;
        /*! Find a model by its primary key or throw an exception. */
        virtual Related
        findOrFail(const QVariant &id,
                   const QVector<Column> &columns = {ASTERISK}) const;
        // findMany() is missing intentionally doesn't make sense for one type relations

        /*! Execute the query and get the first result. */
        virtual std::optional<Related>
        first(const QVector<Column> &columns = {ASTERISK}) const;
        /*! Get the first record matching the attributes or instantiate it. */
        virtual Related
        firstOrNew(const QVector<WhereItem> &attributes = {},
                   const QVector<AttributeItem> &values = {}) const;
        /*! Get the first record matching the attributes or create it. */
        Related
        firstOrCreate(const QVector<WhereItem> &attributes = {},
                      const QVector<AttributeItem> &values = {}) const;
        /*! Execute the query and get the first result or throw an exception. */
        virtual Related firstOrFail(const QVector<Column> &columns = {ASTERISK}) const;

        /*! Add a basic where clause to the query, and return the first result. */
        virtual std::optional<Related>
        firstWhere(const Column &column, const QString &comparison,
                   const QVariant &value, const QString &condition = AND) const;
        /*! Add a basic where clause to the query, and return the first result. */
        virtual std::optional<Related>
        firstWhereEq(const Column &column, const QVariant &value,
                     const QString &condition = AND) const;

        /*! Add a where clause on the primary key to the query. */
        const Relation &whereKey(const QVariant &id) const;
        /*! Add a where clause on the primary key to the query. */
        const Relation &whereKey(const QVector<QVariant> &ids) const;
        /*! Add a where clause on the primary key to the query. */
        const Relation &whereKeyNot(const QVariant &id) const;
        /*! Add a where clause on the primary key to the query. */
        const Relation &whereKeyNot(const QVector<QVariant> &ids) const;

        /*! Set the relationships that should be eager loaded. */
        template<typename = void>
        const Relation &with(const QVector<WithItem> &relations) const;
        /*! Set the relationships that should be eager loaded. */
        template<typename = void>
        const Relation &with(const QString &relation) const;
        /*! Begin querying a model with eager loading. */
        const Relation &with(const QVector<QString> &relations) const;
        /*! Begin querying a model with eager loading. */
        const Relation &with(QVector<QString> &&relations) const;

        /*! Prevent the specified relations from being eager loaded. */
        const Relation &without(const QVector<QString> &relations) const;
        /*! Prevent the specified relations from being eager loaded. */
        const Relation &without(const QString &relation) const;

        /*! Set the relationships that should be eager loaded while removing
            any previously added eager loading specifications. */
        const Relation &withOnly(const QVector<WithItem> &relations) const;
        /*! Set the relationship that should be eager loaded while removing
            any previously added eager loading specifications. */
        const Relation &withOnly(const QString &relation) const;

        /* Insert, Update, Delete */
        /*! Create or update a related record matching the attributes, and fill it
            with values. */
        Related updateOrCreate(const QVector<WhereItem> &attributes,
                               const QVector<AttributeItem> &values = {}) const;

        /* Proxies to TinyBuilder -> QueryBuilder */
        /*! Get the SQL representation of the query. */
        QString toSql() const;
        /*! Get the current query value bindings as flattened QVector. */
        QVector<QVariant> getBindings() const;

        /* Insert, Update, Delete */
        /*! Insert a new record into the database. */
        std::optional<QSqlQuery>
        insert(const QVector<AttributeItem> &values) const;
        /*! Insert new records into the database. */
        std::optional<QSqlQuery>
        insert(const QVector<QVector<AttributeItem>> &values) const;
        /*! Insert a new record and get the value of the primary key. */
        quint64 insertGetId(const QVector<AttributeItem> &attributes,
                            const QString &sequence = "") const;

        /*! Insert a new record into the database while ignoring errors. */
        std::tuple<int, std::optional<QSqlQuery>>
        insertOrIgnore(const QVector<AttributeItem> &values) const;
        /*! Insert new records into the database while ignoring errors. */
        std::tuple<int, std::optional<QSqlQuery>>
        insertOrIgnore(const QVector<QVector<AttributeItem>> &values) const;

        /*! Update records in the database. */
        std::tuple<int, QSqlQuery>
        update(const QVector<UpdateItem> &values) const;

        /*! Delete records from the database. */
        std::tuple<int, QSqlQuery> remove() const;
        /*! Delete records from the database. */
        std::tuple<int, QSqlQuery> deleteModels() const;

        /*! Run a truncate statement on the table. */
        void truncate() const;

        /* Select */
        /*! Retrieve the "count" result of the query. */
        quint64 count(const QVector<Column> &columns = {ASTERISK}) const;
        /*! Retrieve the "count" result of the query. */
        template<typename = void>
        quint64 count(const Column &column) const;
        /*! Retrieve the minimum value of a given column. */
        QVariant min(const Column &column) const;
        /*! Retrieve the maximum value of a given column. */
        QVariant max(const Column &column) const;
        /*! Retrieve the sum of the values of a given column. */
        QVariant sum(const Column &column) const;
        /*! Retrieve the average of the values of a given column. */
        QVariant avg(const Column &column) const;
        /*! Alias for the "avg" method. */
        QVariant average(const Column &column) const;

        /*! Execute an aggregate function on the database. */
        QVariant aggregate(const QString &function,
                           const QVector<Column> &columns = {ASTERISK}) const;

        /*! Set the columns to be selected. */
        const Relation &select(const QVector<Column> &columns = {ASTERISK}) const;
        /*! Set the column to be selected. */
        const Relation &select(const Column &column) const;
        /*! Add new select columns to the query. */
        const Relation &addSelect(const QVector<Column> &columns) const;
        /*! Add a new select column to the query. */
        const Relation &addSelect(const Column &column) const;

        /*! Set a select subquery on the query. */
        template<Queryable T>
        const Relation &select(T &&query, const QString &as) const;
        /*! Add a select subquery to the query. */
        template<Queryable T>
        const Relation &addSelect(T &&query, const QString &as) const;

        /*! Add a subselect expression to the query. */
        template<SubQuery T>
        const Relation &selectSub(T &&query, const QString &as) const;
        /*! Add a new "raw" select expression to the query. */
        const Relation &selectRaw(const QString &expression,
                                  const QVector<QVariant> &bindings = {}) const;

        /*! Force the query to only return distinct results. */
        const Relation &distinct() const;
        /*! Force the query to only return distinct results. */
        const Relation &distinct(const QStringList &columns) const;
        /*! Force the query to only return distinct results. */
        const Relation &distinct(QStringList &&columns) const;

        /*! Add a join clause to the query. */
        template<JoinTable T>
        const Relation &join(
                T &&table, const QString &first, const QString &comparison,
                const QString &second, const QString &type = INNER,
                bool where = false) const;
        /*! Add an advanced join clause to the query. */
        template<JoinTable T>
        const Relation &join(
                T &&table, const std::function<void(JoinClause &)> &callback,
                const QString &type = INNER) const;
        /*! Add a "join where" clause to the query. */
        template<JoinTable T>
        const Relation &joinWhere(
                T &&table, const QString &first, const QString &comparison,
                const QVariant &second, const QString &type = INNER) const;

        /*! Add a left join to the query. */
        template<JoinTable T>
        const Relation &leftJoin(
                T &&table, const QString &first, const QString &comparison,
                const QString &second) const;
        /*! Add an advanced left join to the query. */
        template<JoinTable T>
        const Relation &leftJoin(
                T &&table, const std::function<void(JoinClause &)> &callback) const;
        /*! Add a "join where" clause to the query. */
        template<JoinTable T>
        const Relation &leftJoinWhere(
                T &&table, const QString &first, const QString &comparison,
                const QVariant &second) const;

        /*! Add a right join to the query. */
        template<JoinTable T>
        const Relation &rightJoin(
                T &&table, const QString &first, const QString &comparison,
                const QString &second) const;
        /*! Add an advanced right join to the query. */
        template<JoinTable T>
        const Relation &rightJoin(
                T &&table, const std::function<void(JoinClause &)> &callback) const;
        /*! Add a "right join where" clause to the query. */
        template<JoinTable T>
        const Relation &rightJoinWhere(
                T &&table, const QString &first, const QString &comparison,
                const QVariant &second) const;

        /*! Add a "cross join" clause to the query. */
        template<JoinTable T>
        const Relation &crossJoin(
                T &&table, const QString &first, const QString &comparison,
                const QString &second) const;
        /*! Add an advanced "cross join" clause to the query. */
        template<JoinTable T>
        const Relation &crossJoin(
                T &&table, const std::function<void(JoinClause &)> &callback) const;

        /*! Add a subquery join clause to the query. */
        template<SubQuery T>
        const Relation &joinSub(
                T &&query, const QString &as, const QString &first,
                const QString &comparison, const QVariant &second,
                const QString &type = INNER, bool where = false) const;
        /*! Add a subquery join clause to the query. */
        template<SubQuery T>
        const Relation &joinSub(
                T &&query, const QString &as,
                const std::function<void(JoinClause &)> &callback,
                const QString &type = INNER) const;
        /*! Add a subquery left join to the query. */
        template<SubQuery T>
        const Relation &leftJoinSub(
                T &&query, const QString &as, const QString &first,
                const QString &comparison, const QVariant &second) const;
        /*! Add a subquery left join to the query. */
        template<SubQuery T>
        const Relation &leftJoinSub(
                T &&query, const QString &as,
                const std::function<void(JoinClause &)> &callback) const;
        /*! Add a subquery right join to the query. */
        template<SubQuery T>
        const Relation &rightJoinSub(
                T &&query, const QString &as, const QString &first,
                const QString &comparison, const QVariant &second) const;
        /*! Add a subquery right join to the query. */
        template<SubQuery T>
        const Relation &rightJoinSub(
                T &&query, const QString &as,
                const std::function<void(JoinClause &)> &callback) const;

        /*! Add a basic where clause to the query. */
        template<WhereValue T>
        const Relation &where(
                const Column &column, const QString &comparison, T &&value,
                const QString &condition = AND) const;
        /*! Add an "or where" clause to the query. */
        template<WhereValue T>
        const Relation &orWhere(const Column &column, const QString &comparison,
                                T &&value) const;
        /*! Add a basic equal where clause to the query. */
        template<WhereValue T>
        const Relation &whereEq(const Column &column, T &&value,
                                const QString &condition = AND) const;
        /*! Add an equal "or where" clause to the query. */
        template<WhereValue T>
        const Relation &orWhereEq(const Column &column, T &&value) const;

        /*! Add a nested where clause to the query. */
        const Relation &where(const std::function<void(Builder<Related> &)> &callback,
                              const QString &condition = AND) const;
        /*! Add a nested "or where" clause to the query. */
        const Relation &orWhere(
                const std::function<void(Builder<Related> &)> &callback) const;

        /*! Add a vector of basic where clauses to the query. */
        const Relation &where(const QVector<WhereItem> &values,
                              const QString &condition = AND) const;
        /*! Add a vector of basic "or where" clauses to the query. */
        const Relation &orWhere(const QVector<WhereItem> &values) const;

        /*! Add a vector of where clauses comparing two columns to the query. */
        const Relation &whereColumn(const QVector<WhereColumnItem> &values,
                                    const QString &condition = AND) const;
        /*! Add a vector of "or where" clauses comparing two columns to the query. */
        const Relation &orWhereColumn(const QVector<WhereColumnItem> &values) const;

        /*! Add a "where" clause comparing two columns to the query. */
        const Relation &whereColumn(const Column &first, const QString &comparison,
                                    const Column &second,
                                    const QString &condition = AND) const;
        /*! Add a "or where" clause comparing two columns to the query. */
        const Relation &orWhereColumn(const Column &first, const QString &comparison,
                                      const Column &second) const;
        /*! Add an equal "where" clause comparing two columns to the query. */
        const Relation &whereColumnEq(const Column &first, const Column &second,
                                      const QString &condition = AND) const;
        /*! Add an equal "or where" clause comparing two columns to the query. */
        const Relation &orWhereColumnEq(const Column &first,
                                        const Column &second) const;

        /*! Add a "where in" clause to the query. */
        const Relation &whereIn(
                const Column &column, const QVector<QVariant> &values,
                const QString &condition = AND, bool nope = false) const;
        /*! Add an "or where in" clause to the query. */
        const Relation &orWhereIn(const Column &column,
                                  const QVector<QVariant> &values) const;
        /*! Add a "where not in" clause to the query. */
        const Relation &whereNotIn(const Column &column,
                                   const QVector<QVariant> &values,
                                   const QString &condition = AND) const;
        /*! Add an "or where not in" clause to the query. */
        const Relation &orWhereNotIn(const Column &column,
                                     const QVector<QVariant> &values) const;

        /*! Add a "where null" clause to the query. */
        const Relation &whereNull(const QVector<Column> &columns = {ASTERISK},
                                  const QString &condition = AND,
                                  bool nope = false) const;
        /*! Add an "or where null" clause to the query. */
        const Relation &orWhereNull(const QVector<Column> &columns = {ASTERISK}) const;
        /*! Add a "where not null" clause to the query. */
        const Relation &whereNotNull(const QVector<Column> &columns = {ASTERISK},
                                     const QString &condition = AND) const;
        /*! Add an "or where not null" clause to the query. */
        const Relation &
        orWhereNotNull(const QVector<Column> &columns = {ASTERISK}) const;

        /*! Add a "where null" clause to the query. */
        const Relation &whereNull(const Column &column,
                                  const QString &condition = AND,
                                  bool nope = false) const;
        /*! Add an "or where null" clause to the query. */
        const Relation &orWhereNull(const Column &column) const;
        /*! Add a "where not null" clause to the query. */
        const Relation &whereNotNull(const Column &column,
                                     const QString &condition = AND) const;
        /*! Add an "or where not null" clause to the query. */
        const Relation &orWhereNotNull(const Column &column) const;

        /*! Add a basic where clause to the query with a full sub-select column. */
        template<Queryable C, WhereValue V>
        const Relation &where(C &&column, const QString &comparison, V &&value,
                              const QString &condition = AND) const;
        /*! Add an "or where" clause to the query with a full sub-select column. */
        template<Queryable C, WhereValue V>
        const Relation &orWhere(C &&column, const QString &comparison, V &&value) const;
        /*! Add a basic equal where clause to the query with a full sub-select column. */
        template<Queryable C, WhereValue V>
        const Relation &whereEq(C &&column, V &&value,
                                const QString &condition = AND) const;
        /*! Add an equal "or where" clause to the query with a full sub-select column. */
        template<Queryable C, WhereValue V>
        const Relation &orWhereEq(C &&column, V &&value) const;

        /*! Add a full sub-select to the "where" clause. */
        template<WhereValueSubQuery T>
        const Relation &whereSub(const Column &column, const QString &comparison,
                                 T &&query, const QString &condition = AND) const;

        /*! Add a raw "where" clause to the query. */
        const Relation &whereRaw(const QString &sql,
                                 const QVector<QVariant> &bindings = {},
                                 const QString &condition = AND) const;
        /*! Add a raw "or where" clause to the query. */
        const Relation &orWhereRaw(const QString &sql,
                                   const QVector<QVariant> &bindings = {}) const;

        /*! Add a "group by" clause to the query. */
        const Relation &groupBy(const QVector<Column> &groups) const;
        /*! Add a "group by" clause to the query. */
        const Relation &groupBy(const Column &group) const;
        /*! Add a "group by" clause to the query. */
        template<ColumnConcept ...Args>
        const Relation &groupBy(Args &&...groups) const;

        /*! Add a raw "groupBy" clause to the query. */
        const Relation &groupByRaw(const QString &sql,
                                   const QVector<QVariant> &bindings = {}) const;

        /*! Add a "having" clause to the query. */
        const Relation &having(const Column &column, const QString &comparison,
                               const QVariant &value,
                               const QString &condition = AND) const;
        /*! Add an "or having" clause to the query. */
        const Relation &orHaving(const Column &column, const QString &comparison,
                                 const QVariant &value) const;

        /*! Add a raw "having" clause to the query. */
        const Relation &havingRaw(const QString &sql,
                                  const QVector<QVariant> &bindings = {},
                                  const QString &condition = AND) const;
        /*! Add a raw "or having" clause to the query. */
        const Relation &orHavingRaw(const QString &sql,
                                    const QVector<QVariant> &bindings = {}) const;

        /*! Add an "order by" clause to the query. */
        const Relation &orderBy(const Column &column,
                                const QString &direction = ASC) const;
        /*! Add a descending "order by" clause to the query. */
        const Relation &orderByDesc(const Column &column) const;

        /*! Add an "order by" clause to the query with a subquery ordering. */
        template<Queryable T>
        const Relation &orderBy(T &&query, const QString &direction = ASC) const;
        /*! Add a descending "order by" clause to the query with a subquery ordering. */
        template<Queryable T>
        const Relation &orderByDesc(T &&query) const;

        /*! Add a raw "order by" clause to the query. */
        const Relation &orderByRaw(const QString &sql,
                                   const QVector<QVariant> &bindings = {}) const;

        /*! Add an "order by" clause for a timestamp to the query. */
        const Relation &latest(const Column &column = "") const;
        /*! Add an "order by" clause for a timestamp to the query. */
        const Relation &oldest(const Column &column = "") const;
        /*! Remove all existing orders. */
        const Relation &reorder() const;
        /*! Remove all existing orders and optionally add a new order. */
        const Relation &reorder(const Column &column,
                                const QString &direction = ASC) const;

        /*! Set the "limit" value of the query. */
        const Relation &limit(int value) const;
        /*! Alias to set the "limit" value of the query. */
        const Relation &take(int value) const;
        /*! Set the "offset" value of the query. */
        const Relation &offset(int value) const;
        /*! Alias to set the "offset" value of the query. */
        const Relation &skip(int value) const;
        /*! Set the limit and offset for a given page. */
        const Relation &forPage(int page, int perPage = 30) const;

        /*! Increment a column's value by a given amount. */
        template<typename T> requires std::is_arithmetic_v<T>
        std::tuple<int, QSqlQuery>
        increment(const QString &column, T amount = 1,
                  const QVector<UpdateItem> &extra = {}) const;
        /*! Decrement a column's value by a given amount. */
        template<typename T> requires std::is_arithmetic_v<T>
        std::tuple<int, QSqlQuery>
        decrement(const QString &column, T amount = 1,
                  const QVector<UpdateItem> &extra = {}) const;

        /* Pessimistic Locking */
        /*! Lock the selected rows in the table for updating. */
        const Relation &lockForUpdate() const;
        /*! Share lock the selected rows in the table. */
        const Relation &sharedLock() const;
        /*! Lock the selected rows in the table. */
        const Relation &lock(bool value = true) const;
        /*! Lock the selected rows in the table. */
        const Relation &lock(const char *value) const;
        /*! Lock the selected rows in the table. */
        const Relation &lock(const QString &value) const;
        /*! Lock the selected rows in the table. */
        const Relation &lock(QString &&value) const;

        /* Querying Relationship Existence/Absence */
        /*! Add a relationship count / exists condition to the query. */
        template<typename HasRelated = void>
        const Relation &
        has(const QString &relation, const QString &comparison = GE, qint64 count = 1,
            const QString &condition = AND,
            const std::function<void(
                CallbackType<HasRelated> &)> &callback = nullptr) const;

        /*! Add a relationship count / exists condition to the query with an "or". */
        template<typename HasRelated = void>
        const Relation &
        orHas(const QString &relation, const QString &comparison = GE,
              qint64 count = 1) const;
        /*! Add a relationship count / exists condition to the query. */
        template<typename HasRelated = void>
        const Relation &
        doesntHave(const QString &relation, const QString &condition = AND,
                   const std::function<void(
                       CallbackType<HasRelated> &)> &callback = nullptr) const;
        /*! Add a relationship count / exists condition to the query with an "or". */
        template<typename HasRelated = void>
        const Relation &
        orDoesntHave(const QString &relation) const;

        /*! Add a relationship count / exists condition to the query. */
        template<typename HasRelated>
        const Relation &
        has(std::unique_ptr<Relation<Related, HasRelated>> &&relation,
            const QString &comparison = GE, qint64 count = 1,
            const QString &condition = AND,
            const std::function<void(QueryBuilder &)> &callback = nullptr) const;
        /*! Add a relationship count / exists condition to the query, prefer this over
            above overload, void type to avoid ambiguity. */
        template<typename HasRelated, typename = void>
        const Relation &
        has(std::unique_ptr<Relation<Related, HasRelated>> &&relation,
            const QString &comparison = GE, qint64 count = 1,
            const QString &condition = AND,
            const std::function<void(
                TinyBuilder<HasRelated> &)> &callback = nullptr) const;

        /*! Add a relationship count / exists condition to the query. */
#ifdef __clang__
        template<typename HasRelated, typename Method,
                 std::enable_if_t<std::is_member_function_pointer_v<Method>, bool> = true>
#else
        template<typename HasRelated, typename Method>
        requires std::is_member_function_pointer_v<Method>
#endif
        const Relation &
        has(const Method relation, const QString &comparison = GE, qint64 count = 1,
            const QString &condition = AND,
            const std::function<void(
                TinyBuilder<HasRelated> &)> &callback = nullptr) const;

        /*! Add a relationship count / exists condition to the query with where
            clauses. */
        template<typename HasRelated = void>
        const Relation &
        whereHas(const QString &relation,
                 const std::function<void(
                     CallbackType<HasRelated> &)> &callback = nullptr,
                 const QString &comparison = GE, qint64 count = 1) const;

        /*! Add a relationship count / exists condition to the query with where
            clauses and an "or". */
        template<typename HasRelated = void>
        const Relation &
        orWhereHas(const QString &relation,
                   const std::function<void(
                       CallbackType<HasRelated> &)> &callback = nullptr,
                   const QString &comparison = GE, qint64 count = 1) const;
        /*! Add a relationship count / exists condition to the query with where
            clauses. */
        template<typename HasRelated = void>
        const Relation &
        whereDoesntHave(const QString &relation,
                        const std::function<void(
                            CallbackType<HasRelated> &)> &callback = nullptr) const;
        /*! Add a relationship count / exists condition to the query with where
            clauses and an "or". */
        template<typename HasRelated = void>
        const Relation &
        orWhereDoesntHave(const QString &relation,
                          const std::function<void(
                              CallbackType<HasRelated> &)> &callback = nullptr) const;

        /*! Add a relationship count / exists condition to the query with where
            clauses. */
#ifdef __clang__
        template<typename HasRelated, typename Method,
                 std::enable_if_t<std::is_member_function_pointer_v<Method>, bool> = true>
#else
        template<typename HasRelated, typename Method>
        requires std::is_member_function_pointer_v<Method>
#endif
        const Relation &
        whereHas(const Method relation,
                 const std::function<void(TinyBuilder<HasRelated> &)> &callback = nullptr,
                 const QString &comparison = GE, qint64 count = 1) const;

        /* Others */
        /*! The textual representation of the Relation type. */
        virtual QString relationTypeName() const = 0;

    protected:
        /*! Initialize a Relation instance. */
        inline void init() const
        { addConstraints(); }

        /*! Get all of the primary keys for the vector of models. */
        QVector<QVariant>
        getKeys(const QVector<Model> &models, const QString &key = "") const;

        /* Querying Relationship Existence/Absence */
        /*! Add the constraints for an internal relationship existence query.
            Essentially, these queries compare on column names like whereColumn. */
        virtual std::unique_ptr<Builder<Related>>
        getRelationExistenceQuery(
                std::unique_ptr<Builder<Related>> &&query,
                const Builder<Model> &parentQuery,
                const QVector<Column> &columns = {ASTERISK}) const;
        /*! Add the constraints for a relationship count query. */
        std::unique_ptr<Builder<Related>>
        getRelationExistenceCountQuery(
                std::unique_ptr<Builder<Related>> &&query,
                const Builder<Model> &parentQuery) const;

        /* During eager load, we secure m_parent to not become a dangling reference in
           EagerRelationStore::visited() by help of the dummyModel local variable.
           It has to be the reference, because eg. BelongsTo::associate() directly
           modifies attributes of m_parent. */
        /*! The parent model instance. */
        Model &m_parent;
        /*! The related model instance. */
        const std::unique_ptr<Related> m_related;
        /*! The key name of the related model. */
        QString m_relatedKey;
        // TODO next would be good to use TinyBuilder alias instead of Builder silverqx
        /*! The Eloquent query builder instance. */
        std::unique_ptr<Builder<Related>> m_query;
        /*! Indicates if the relation is adding constraints. */
        static bool constraints;
    };

    /*! The tag for one type relation. */
    class OneRelation
    {};

    /*! The tag for many type relation. */
    class ManyRelation
    {};

    /*! The tag for the relation which contains pivot table, like many-to-many. */
    class PivotRelation
    {};

    template<class Model, class Related>
    bool Relation<Model, Related>::constraints = true;

    template<class Model, class Related>
    Relation<Model, Related>::Relation(std::unique_ptr<Related> &&related, Model &parent,
                                       const QString &relatedKey)
        : m_parent(parent)
        , m_related(std::move(related))
        , m_relatedKey(relatedKey.isEmpty() ? m_related->getKeyName() : relatedKey)
        , m_query(m_related->newQuery())
    {}

    template<class Model, class Related>
    std::unique_ptr<Relation<Model, Related>>
    Relation<Model, Related>::noConstraints(
            const std::function<std::unique_ptr<Relation<Model, Related>>()> &callback)
    {
        const auto previous = constraints;

        constraints = false;
        auto relation = std::invoke(callback);
        constraints = previous;

        return relation;
    }

    template<class Model, class Related>
    inline QString Relation<Model, Related>::getQualifiedParentKeyName() const
    {
        return m_parent.getQualifiedKeyName();
    }

    template<class Model, class Related>
    inline QString Relation<Model, Related>::getExistenceCompareKey() const
    {
        throw RuntimeError(QStringLiteral("Method %1() is not implemented for '%2' "
                                          "relation type.")
                           .arg(__func__, relationTypeName()));
    }

    template<class Model, class Related>
    void Relation<Model, Related>::touch() const
    {
        const auto &model = getRelated();

        if (!model.isIgnoringTouch())
            rawUpdate({
                {model.getUpdatedAtColumn(), model.freshTimestampString()}
            });
    }

    template<class Model, class Related>
    std::tuple<int, QSqlQuery>
    Relation<Model, Related>::rawUpdate(const QVector<UpdateItem> &values) const
    {
        // FEATURE scopes silverqx
        return m_query->update(values);
    }

    template<class Model, class Related>
    QVariant Relation<Model, Related>::value(const Column &column) const
    {
        return m_query->value(column);
    }

    template<class Model, class Related>
    QVector<QVariant> Relation<Model, Related>::pluck(const QString &column) const
    {
        return m_query->pluck(column);
    }

    template<class Model, class Related>
    template<typename T>
    std::map<T, QVariant>
    Relation<Model, Related>::pluck(const QString &column, const QString &key) const
    {
        return m_query->template pluck<T>(column, key);
    }

    template<class Model, class Related>
    std::optional<Related>
    Relation<Model, Related>::find(const QVariant &id,
                                   const QVector<Column> &columns) const
    {
        return m_query->find(id, columns);
    }

    template<class Model, class Related>
    Related
    Relation<Model, Related>::findOrNew(const QVariant &id,
                                        const QVector<Column> &columns) const
    {
        return m_query->findOrNew(id, columns);
    }

    template<class Model, class Related>
    Related
    Relation<Model, Related>::findOrFail(const QVariant &id,
                                         const QVector<Column> &columns) const
    {
        return m_query->findOrFail(id, columns);
    }

    template<class Model, class Related>
    std::optional<Related>
    Relation<Model, Related>::first(const QVector<Column> &columns) const
    {
        return m_query->first(columns);
    }

    template<class Model, class Related>
    Related
    Relation<Model, Related>::firstOrNew(const QVector<WhereItem> &attributes,
                                         const QVector<AttributeItem> &values) const
    {
        return m_query->firstOrNew(attributes, values);
    }

    template<class Model, class Related>
    Related
    Relation<Model, Related>::firstOrCreate(const QVector<WhereItem> &attributes,
                                            const QVector<AttributeItem> &values) const
    {
        return m_query->firstOrCreate(attributes, values);
    }

    template<class Model, class Related>
    Related Relation<Model, Related>::firstOrFail(const QVector<Column> &columns) const
    {
        return m_query->firstOrFail(columns);
    }

    template<class Model, class Related>
    std::optional<Related>
    Relation<Model, Related>::firstWhere(
            const Column &column, const QString &comparison,
            const QVariant &value, const QString &condition) const
    {
        return m_query->firstWhere(column, comparison, value, condition);
    }

    template<class Model, class Related>
    std::optional<Related>
    Relation<Model, Related>::firstWhereEq(const Column &column, const QVariant &value,
                                           const QString &condition) const
    {
        return m_query->firstWhereEq(column, value, condition);
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::whereKey(const QVariant &id) const
    {
        m_query->whereKey(id);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::whereKey(const QVector<QVariant> &ids) const
    {
        m_query->whereKey(ids);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::whereKeyNot(const QVariant &id) const
    {
        m_query->whereKeyNot(id);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::whereKeyNot(const QVector<QVariant> &ids) const
    {
        m_query->whereKeyNot(ids);

        return *this;
    }

    template<class Model, class Related>
    template<typename>
    const Relation<Model, Related> &
    Relation<Model, Related>::with(const QVector<WithItem> &relations) const
    {
        m_query->with(relations);

        return *this;
    }

    template<class Model, class Related>
    template<typename>
    const Relation<Model, Related> &
    Relation<Model, Related>::with(const QString &relation) const
    {
        m_query->with(relation);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::with(const QVector<QString> &relations) const
    {
        m_query->with(relations);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::with(QVector<QString> &&relations) const
    {
        m_query->with(std::move(relations));

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::without(const QVector<QString> &relations) const
    {
        m_query->without(relations);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::without(const QString &relation) const
    {
        m_query->without(relation);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::withOnly(const QVector<WithItem> &relations) const
    {
        m_query->withOnly(relations);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::withOnly(const QString &relation) const
    {
        m_query->withOnly(relation);

        return *this;
    }

    template<class Model, class Related>
    Related
    Relation<Model, Related>::updateOrCreate(
            const QVector<WhereItem> &attributes,
            const QVector<AttributeItem> &values) const
    {
        return m_query->updateOrCreate(attributes, values);
    }

    template<class Model, class Related>
    inline QString Relation<Model, Related>::toSql() const
    {
        return getBaseQuery().toSql();
    }

    template<class Model, class Related>
    inline QVector<QVariant> Relation<Model, Related>::getBindings() const
    {
        return getBaseQuery().getBindings();
    }

    template<class Model, class Related>
    std::optional<QSqlQuery>
    Relation<Model, Related>::insert(const QVector<AttributeItem> &values) const
    {
        return m_query->insert(values);
    }

    template<class Model, class Related>
    std::optional<QSqlQuery>
    Relation<Model, Related>::insert(const QVector<QVector<AttributeItem>> &values) const
    {
        return m_query->insert(values);
    }

    // FEATURE dilemma primarykey, Model::KeyType vs QVariant silverqx
    template<class Model, class Related>
    quint64
    Relation<Model, Related>::insertGetId(const QVector<AttributeItem> &attributes,
                                          const QString &sequence) const
    {
        return m_query->insertGetId(attributes, sequence);
    }

    template<class Model, class Related>
    std::tuple<int, std::optional<QSqlQuery>>
    Relation<Model, Related>::insertOrIgnore(const QVector<AttributeItem> &values) const
    {
        return m_query->insertOrIgnore(values);
    }

    template<class Model, class Related>
    std::tuple<int, std::optional<QSqlQuery>>
    Relation<Model, Related>::insertOrIgnore(
            const QVector<QVector<AttributeItem>> &values) const
    {
        return m_query->insertOrIgnore(values);
    }

    template<class Model, class Related>
    std::tuple<int, QSqlQuery>
    Relation<Model, Related>::update(const QVector<UpdateItem> &values) const
    {
        return m_query->update(values);
    }

    template<class Model, class Related>
    std::tuple<int, QSqlQuery>
    Relation<Model, Related>::remove() const
    {
        return m_query->remove();
    }

    template<class Model, class Related>
    std::tuple<int, QSqlQuery>
    Relation<Model, Related>::deleteModels() const
    {
        return m_query->deleteModels();
    }

    template<class Model, class Related>
    void Relation<Model, Related>::truncate() const
    {
        m_query->truncate();
    }

    template<class Model, class Related>
    quint64 Relation<Model, Related>::count(const QVector<Column> &columns) const
    {
        return m_query->count(columns);
    }

    template<class Model, class Related>
    template<typename>
    quint64 Relation<Model, Related>::count(const Column &column) const
    {
        return m_query->count(QVector<Column> {column});
    }

    template<class Model, class Related>
    QVariant Relation<Model, Related>::min(const Column &column) const
    {
        return m_query->min(column);
    }

    template<class Model, class Related>
    QVariant Relation<Model, Related>::max(const Column &column) const
    {
        return m_query->max(column);
    }

    template<class Model, class Related>
    QVariant Relation<Model, Related>::sum(const Column &column) const
    {
        return m_query->sum(column);
    }

    template<class Model, class Related>
    QVariant Relation<Model, Related>::avg(const Column &column) const
    {
        return m_query->avg(column);
    }

    template<class Model, class Related>
    QVariant Relation<Model, Related>::average(const Column &column) const
    {
        return m_query->avg(column);
    }

    template<class Model, class Related>
    QVariant Relation<Model, Related>::aggregate(const QString &function,
                                                 const QVector<Column> &columns) const
    {
        return m_query->aggregate(function, columns);
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::select(const QVector<Column> &columns) const
    {
        m_query->select(columns);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::select(const Column &column) const
    {
        m_query->select(column);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::addSelect(const QVector<Column> &columns) const
    {
        m_query->addSelect(columns);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::addSelect(const Column &column) const
    {
        m_query->addSelect(column);

        return *this;
    }

    template<class Model, class Related>
    template<Queryable T>
    const Relation<Model, Related> &
    Relation<Model, Related>::select(T &&query, const QString &as) const
    {
        m_query->select(std::forward<T>(query), as);

        return *this;
    }

    template<class Model, class Related>
    template<Queryable T>
    const Relation<Model, Related> &
    Relation<Model, Related>::addSelect(T &&query, const QString &as) const
    {
        m_query->addSelect(std::forward<T>(query), as);

        return *this;
    }

    template<class Model, class Related>
    template<SubQuery T>
    const Relation<Model, Related> &
    Relation<Model, Related>::selectSub(T &&query, const QString &as) const
    {
        m_query->selectSub(std::forward<T>(query), as);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::selectRaw(
            const QString &expression, const QVector<QVariant> &bindings) const
    {
        m_query->selectRaw(expression, bindings);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::distinct() const
    {
        m_query->distinct();

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::distinct(const QStringList &columns) const
    {
        m_query->distinct(columns);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::distinct(QStringList &&columns) const
    {
        m_query->distinct(std::move(columns));

        return *this;
    }

    template<class Model, class Related>
    template<JoinTable T>
    const Relation<Model, Related> &
    Relation<Model, Related>::join(
            T &&table, const QString &first, const QString &comparison,
            const QString &second, const QString &type, const bool where) const
    {
        m_query->join(std::forward<T>(table), first, comparison, second, type, where);

        return *this;
    }

    template<class Model, class Related>
    template<JoinTable T>
    const Relation<Model, Related> &
    Relation<Model, Related>::join(
            T &&table, const std::function<void(JoinClause &)> &callback,
            const QString &type) const
    {
        m_query->join(std::forward<T>(table), callback, type);

        return *this;
    }

    template<class Model, class Related>
    template<JoinTable T>
    const Relation<Model, Related> &
    Relation<Model, Related>::joinWhere(
            T &&table, const QString &first, const QString &comparison,
            const QVariant &second, const QString &type) const
    {
        m_query->joinWhere(std::forward<T>(table), first, comparison, second, type);

        return *this;
    }

    template<class Model, class Related>
    template<JoinTable T>
    const Relation<Model, Related> &
    Relation<Model, Related>::leftJoin(
            T &&table, const QString &first, const QString &comparison,
            const QString &second) const
    {
        m_query->leftJoin(std::forward<T>(table), first, comparison, second);

        return *this;
    }

    template<class Model, class Related>
    template<JoinTable T>
    const Relation<Model, Related> &
    Relation<Model, Related>::leftJoin(
            T &&table, const std::function<void(JoinClause &)> &callback) const
    {
        m_query->leftJoin(std::forward<T>(table), callback);

        return *this;
    }

    template<class Model, class Related>
    template<JoinTable T>
    const Relation<Model, Related> &
    Relation<Model, Related>::leftJoinWhere(
            T &&table, const QString &first, const QString &comparison,
            const QVariant &second) const
    {
        m_query->leftJoinWhere(std::forward<T>(table), first, comparison, second);

        return *this;
    }

    template<class Model, class Related>
    template<JoinTable T>
    const Relation<Model, Related> &
    Relation<Model, Related>::rightJoin(
            T &&table, const QString &first, const QString &comparison,
            const QString &second) const
    {
        m_query->rightJoin(std::forward<T>(table), first, comparison, second);

        return *this;
    }

    template<class Model, class Related>
    template<JoinTable T>
    const Relation<Model, Related> &
    Relation<Model, Related>::rightJoin(
            T &&table, const std::function<void(JoinClause &)> &callback) const
    {
        m_query->rightJoin(std::forward<T>(table), callback);

        return *this;
    }

    template<class Model, class Related>
    template<JoinTable T>
    const Relation<Model, Related> &
    Relation<Model, Related>::rightJoinWhere(
            T &&table, const QString &first, const QString &comparison,
            const QVariant &second) const
    {
        m_query->rightJoinWhere(std::forward<T>(table), first, comparison, second);

        return *this;
    }

    template<class Model, class Related>
    template<JoinTable T>
    const Relation<Model, Related> &
    Relation<Model, Related>::crossJoin(
            T &&table, const QString &first, const QString &comparison,
            const QString &second) const
    {
        m_query->crossJoin(std::forward<T>(table), first, comparison, second);

        return *this;
    }

    template<class Model, class Related>
    template<JoinTable T>
    const Relation<Model, Related> &
    Relation<Model, Related>::crossJoin(
            T &&table, const std::function<void(JoinClause &)> &callback) const
    {
        m_query->crossJoin(std::forward<T>(table), callback);

        return *this;
    }

    template<class Model, class Related>
    template<SubQuery T>
    const Relation<Model, Related> &
    Relation<Model, Related>::joinSub(
            T &&query, const QString &as, const QString &first,
            const QString &comparison, const QVariant &second,
            const QString &type, const bool where) const
    {
        m_query->joinSub(std::forward<T>(query), as, first, comparison, second, type,
                         where);

        return *this;
    }

    template<class Model, class Related>
    template<SubQuery T>
    const Relation<Model, Related> &
    Relation<Model, Related>::joinSub(
            T &&query, const QString &as,
            const std::function<void(JoinClause &)> &callback,
            const QString &type) const
    {
        m_query->joinSub(std::forward<T>(query), as, callback, type);

        return *this;
    }

    template<class Model, class Related>
    template<SubQuery T>
    const Relation<Model, Related> &
    Relation<Model, Related>::leftJoinSub(
            T &&query, const QString &as, const QString &first,
            const QString &comparison, const QVariant &second) const
    {
        m_query->leftJoinSub(std::forward<T>(query), as, first, comparison, second);

        return *this;
    }

    template<class Model, class Related>
    template<SubQuery T>
    const Relation<Model, Related> &
    Relation<Model, Related>::leftJoinSub(
            T &&query, const QString &as,
            const std::function<void(JoinClause &)> &callback) const
    {
        m_query->joinSub(std::forward<T>(query), as, callback, LEFT);

        return *this;
    }

    template<class Model, class Related>
    template<SubQuery T>
    const Relation<Model, Related> &
    Relation<Model, Related>::rightJoinSub(
            T &&query, const QString &as, const QString &first,
            const QString &comparison, const QVariant &second) const
    {
        m_query->rightJoinSub(std::forward<T>(query), as, first, comparison, second);

        return *this;
    }

    template<class Model, class Related>
    template<SubQuery T>
    const Relation<Model, Related> &
    Relation<Model, Related>::rightJoinSub(
            T &&query, const QString &as,
            const std::function<void(JoinClause &)> &callback) const
    {
        m_query->joinSub(std::forward<T>(query), as, callback, RIGHT);

        return *this;
    }

    template<class Model, class Related>
    template<WhereValue T>
    const Relation<Model, Related> &
    Relation<Model, Related>::where(const Column &column, const QString &comparison,
                                    T &&value, const QString &condition) const
    {
        m_query->where(column, comparison, std::forward<T>(value), condition);

        return *this;
    }

    template<class Model, class Related>
    template<WhereValue T>
    const Relation<Model, Related> &
    Relation<Model, Related>::orWhere(const Column &column, const QString &comparison,
                                      T &&value) const
    {
        m_query->orWhere(column, comparison, std::forward<T>(value));

        return *this;
    }

    template<class Model, class Related>
    template<WhereValue T>
    const Relation<Model, Related> &
    Relation<Model, Related>::whereEq(const Column &column, T &&value,
                                      const QString &condition) const
    {
        m_query->whereEq(column, std::forward<T>(value), condition);

        return *this;
    }

    template<class Model, class Related>
    template<WhereValue T>
    const Relation<Model, Related> &
    Relation<Model, Related>::orWhereEq(const Column &column, T &&value) const
    {
        m_query->orWhereEq(column, std::forward<T>(value));

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::where(
            const std::function<void(Builder<Related> &)> &callback,
            const QString &condition) const
    {
        m_query->where(callback, condition);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::orWhere(
            const std::function<void(Builder<Related> &)> &callback) const
    {
        m_query->orWhere(callback);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::where(const QVector<WhereItem> &values,
                                    const QString &condition) const
    {
        m_query->where(values, condition);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::orWhere(const QVector<WhereItem> &values) const
    {
        m_query->orWhere(values);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::whereColumn(
            const QVector<WhereColumnItem> &values, const QString &condition) const
    {
        m_query->whereColumn(values, condition);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::orWhereColumn(const QVector<WhereColumnItem> &values) const
    {
        m_query->orWhereColumn(values);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::whereColumn(
            const Column &first, const QString &comparison,
            const Column &second, const QString &condition) const
    {
        m_query->whereColumn(first, comparison, second, condition);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::orWhereColumn(
            const Column &first, const QString &comparison, const Column &second) const
    {
        m_query->orWhereColumn(first, comparison, second);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::whereColumnEq(
            const Column &first, const Column &second, const QString &condition) const
    {
        m_query->whereColumnEq(first, second, condition);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::orWhereColumnEq(const Column &first,
                                              const Column &second) const
    {
        m_query->orWhereColumnEq(first, second);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::whereIn(
            const Column &column, const QVector<QVariant> &values,
            const QString &condition, const bool nope) const
    {
        m_query->whereIn(column, values, condition, nope);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::orWhereIn(const Column &column,
                                        const QVector<QVariant> &values) const
    {
        m_query->orWhereIn(column, values);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::whereNotIn(
            const Column &column, const QVector<QVariant> &values,
            const QString &condition) const
    {
        m_query->whereNotIn(column, values, condition);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::orWhereNotIn(const Column &column,
                                           const QVector<QVariant> &values) const
    {
        m_query->orWhereNotIn(column, values);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::whereNull(
            const QVector<Column> &columns, const QString &condition,
            const bool nope) const
    {
        m_query->whereNull(columns, condition, nope);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::orWhereNull(const QVector<Column> &columns) const
    {
        m_query->orWhereNull(columns);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::whereNotNull(const QVector<Column> &columns,
                                           const QString &condition) const
    {
        m_query->whereNotNull(columns, condition);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::orWhereNotNull(const QVector<Column> &columns) const
    {
        m_query->orWhereNotNull(columns);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::whereNull(
            const Column &column, const QString &condition, const bool nope) const
    {
        m_query->whereNull(column, condition, nope);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::orWhereNull(const Column &column) const
    {
        m_query->orWhereNull(column);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::whereNotNull(const Column &column,
                                           const QString &condition) const
    {
        m_query->whereNotNull(column, condition);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::orWhereNotNull(const Column &column) const
    {
        m_query->orWhereNotNull(column);

        return *this;
    }

    template<class Model, class Related>
    template<Queryable C, WhereValue V>
    const Relation<Model, Related> &
    Relation<Model, Related>::where(
            C &&column, const QString &comparison, V &&value,
            const QString &condition) const
    {
        m_query->where(std::forward<C>(column), comparison, std::forward<V>(value),
                       condition);

        return *this;
    }

    template<class Model, class Related>
    template<Queryable C, WhereValue V>
    const Relation<Model, Related> &
    Relation<Model, Related>::orWhere(
            C &&column, const QString &comparison, V &&value) const
    {
        m_query->where(std::forward<C>(column), comparison, std::forward<V>(value), OR);

        return *this;
    }

    template<class Model, class Related>
    template<Queryable C, WhereValue V>
    const Relation<Model, Related> &
    Relation<Model, Related>::whereEq(
            C &&column, V &&value, const QString &condition) const
    {
        m_query->where(std::forward<C>(column), EQ, std::forward<V>(value), condition);

        return *this;
    }

    template<class Model, class Related>
    template<Queryable C, WhereValue V>
    const Relation<Model, Related> &
    Relation<Model, Related>::orWhereEq(C &&column, V &&value) const
    {
        m_query->where(std::forward<C>(column), EQ, std::forward<V>(value), OR);

        return *this;
    }

    template<class Model, class Related>
    template<WhereValueSubQuery T>
    const Relation<Model, Related> &
    Relation<Model, Related>::whereSub(
            const Column &column, const QString &comparison, T &&query,
            const QString &condition) const
    {
        m_query->whereSub(column, comparison, std::forward<T>(query), condition);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::whereRaw(
            const QString &sql, const QVector<QVariant> &bindings,
            const QString &condition) const
    {
        m_query->whereRaw(sql, bindings, condition);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::orWhereRaw(
            const QString &sql, const QVector<QVariant> &bindings) const
    {
        m_query->whereRaw(sql, bindings, OR);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::groupBy(const QVector<Column> &groups) const
    {
        m_query->groupBy(groups);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::groupBy(const Column &group) const
    {
        m_query->groupBy(group);

        return *this;
    }

    template<class Model, class Related>
    template<ColumnConcept ...Args>
    const Relation<Model, Related> &
    Relation<Model, Related>::groupBy(Args &&...groups) const
    {
        m_query->groupBy(QVector<Column> {std::forward<Args>(groups)...});

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::groupByRaw(const QString &sql,
                                         const QVector<QVariant> &bindings) const
    {
        m_query->groupByRaw(sql, bindings);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::having(
            const Column &column, const QString &comparison,
            const QVariant &value, const QString &condition) const
    {
        m_query->having(column, comparison, value, condition);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::orHaving(const Column &column, const QString &comparison,
                                       const QVariant &value) const
    {
        m_query->orHaving(column, comparison, value);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::havingRaw(
            const QString &sql,  const QVector<QVariant> &bindings,
            const QString &condition) const
    {
        m_query->havingRaw(sql, bindings, condition);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::orHavingRaw(
            const QString &sql,  const QVector<QVariant> &bindings) const
    {
        m_query->havingRaw(sql, bindings, OR);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::orderBy(const Column &column,
                                      const QString &direction) const
    {
        m_query->orderBy(column, direction);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::orderByDesc(const Column &column) const
    {
        m_query->orderByDesc(column);

        return *this;
    }

    template<class Model, class Related>
    template<Queryable T>
    const Relation<Model, Related> &
    Relation<Model, Related>::orderBy(T &&query, const QString &direction) const
    {
        m_query->orderBy(std::forward<T>(query), direction);

        return *this;
    }

    template<class Model, class Related>
    template<Queryable T>
    const Relation<Model, Related> &
    Relation<Model, Related>::orderByDesc(T &&query) const
    {
        m_query->orderBy(std::forward<T>(query), DESC);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::orderByRaw(const QString &sql,
                                         const QVector<QVariant> &bindings) const
    {
        m_query->orderByRaw(sql, bindings);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::latest(const Column &column) const
    {
        m_query->latest(column);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::oldest(const Column &column) const
    {
        m_query->oldest(column);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::reorder() const
    {
        m_query->reorder();

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::reorder(const Column &column,
                                      const QString &direction) const
    {
        m_query->reorder(column, direction);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::limit(const int value) const
    {
        m_query->limit(value);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::take(const int value) const
    {
        m_query->take(value);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::offset(const int value) const
    {
        m_query->offset(value);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::skip(const int value) const
    {
        m_query->skip(value);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::forPage(const int page, const int perPage) const
    {
        m_query->forPage(page, perPage);

        return *this;
    }

    template<class Model, class Related>
    template<typename T> requires std::is_arithmetic_v<T>
    std::tuple<int, QSqlQuery>
    Relation<Model, Related>::increment(const QString &column, const T amount,
                                        const QVector<UpdateItem> &extra) const
    {
        return m_query->increment(column, amount, extra);
    }

    template<class Model, class Related>
    template<typename T> requires std::is_arithmetic_v<T>
    std::tuple<int, QSqlQuery>
    Relation<Model, Related>::decrement(const QString &column, const T amount,
                                        const QVector<UpdateItem> &extra) const
    {
        return m_query->decrement(column, amount, extra);
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::lockForUpdate() const
    {
        m_query->lock(true);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::sharedLock() const
    {
        m_query->lock(false);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::lock(const bool value) const
    {
        m_query->lock(value);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::lock(const char *value) const
    {
        m_query->lock(value);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::lock(const QString &value) const
    {
        m_query->lock(value);

        return *this;
    }

    template<class Model, class Related>
    const Relation<Model, Related> &
    Relation<Model, Related>::lock(QString &&value) const
    {
        m_query->lock(std::move(value));

        return *this;
    }

    template<class Model, class Related>
    template<typename HasRelated>
    const Relation<Model, Related> &
    Relation<Model, Related>::has(
            const QString &relation, const QString &comparison, const qint64 count,
            const QString &condition,
            const std::function<void(CallbackType<HasRelated> &)> &callback) const
    {
        m_query->template has<HasRelated>(relation, comparison, count, condition,
                                          callback);

        return *this;
    }

    template<class Model, class Related>
    template<typename HasRelated>
    const Relation<Model, Related> &
    Relation<Model, Related>::orHas(
            const QString &relation, const QString &comparison, const qint64 count) const
    {
        m_query->template orHas<HasRelated>(relation, comparison, count);

        return *this;
    }

    template<class Model, class Related>
    template<typename HasRelated>
    const Relation<Model, Related> &
    Relation<Model, Related>::doesntHave(
            const QString &relation, const QString &condition,
            const std::function<void(CallbackType<HasRelated> &)> &callback) const
    {
        m_query->template doesntHave<HasRelated>(relation, condition, callback);

        return *this;
    }

    template<class Model, class Related>
    template<typename HasRelated>
    const Relation<Model, Related> &
    Relation<Model, Related>::orDoesntHave(const QString &relation) const
    {
        m_query->template orDoesntHave<HasRelated>(relation);

        return *this;
    }

    template<class Model, class Related>
    template<typename HasRelated>
    const Relation<Model, Related> &
    Relation<Model, Related>::has(
            std::unique_ptr<Relation<Related, HasRelated>> &&relation,
            const QString &comparison, const qint64 count, const QString &condition,
            const std::function<void(QueryBuilder &)> &callback) const
    {
        m_query->template has<HasRelated>(std::move(relation), comparison, count,
                                          condition, callback);

        return *this;
    }

    template<class Model, class Related>
    template<typename HasRelated, typename>
    const Relation<Model, Related> &
    Relation<Model, Related>::has(
            std::unique_ptr<Relation<Related, HasRelated>> &&relation,
            const QString &comparison, const qint64 count, const QString &condition,
            const std::function<void(TinyBuilder<HasRelated> &)> &callback) const
    {
        m_query->template has<HasRelated, void>(std::move(relation), comparison, count,
                                                condition, callback);

        return *this;
    }

    template<class Model, class Related>
#ifdef __clang__
    template<typename HasRelated, typename Method,
             std::enable_if_t<std::is_member_function_pointer_v<Method>, bool>>
#else
    template<typename HasRelated, typename Method>
    requires std::is_member_function_pointer_v<Method>
#endif
    const Relation<Model, Related> &
    Relation<Model, Related>::has(
            const Method relation, const QString &comparison, const qint64 count,
            const QString &condition,
            const std::function<void(TinyBuilder<HasRelated> &)> &callback) const
    {
        m_query->template has<HasRelated>(relation, comparison, count, condition,
                                          callback);

        return *this;
    }

    template<class Model, class Related>
    template<typename HasRelated>
    const Relation<Model, Related> &
    Relation<Model, Related>::whereHas(
            const QString &relation,
            const std::function<void(CallbackType<HasRelated> &)> &callback,
            const QString &comparison, const qint64 count) const
    {
        m_query->template whereHas<HasRelated>(relation, callback, comparison, count);

        return *this;
    }

    template<class Model, class Related>
    template<typename HasRelated>
    const Relation<Model, Related> &
    Relation<Model, Related>::orWhereHas(
            const QString &relation,
            const std::function<void(CallbackType<HasRelated> &)> &callback,
            const QString &comparison, const qint64 count) const
    {
        m_query->template orWhereHas<HasRelated>(relation, callback, comparison, count);

        return *this;
    }

    template<class Model, class Related>
    template<typename HasRelated>
    const Relation<Model, Related> &
    Relation<Model, Related>::whereDoesntHave(
            const QString &relation,
            const std::function<void(CallbackType<HasRelated> &)> &callback) const
    {
        m_query->template whereDoesntHave<HasRelated>(relation, callback);

        return *this;
    }

    template<class Model, class Related>
    template<typename HasRelated>
    const Relation<Model, Related> &
    Relation<Model, Related>::orWhereDoesntHave(
            const QString &relation,
            const std::function<void(CallbackType<HasRelated> &)> &callback) const
    {
        m_query->template orWhereDoesntHave<HasRelated>(relation, callback);

        return *this;
    }

    template<class Model, class Related>
#ifdef __clang__
    template<typename HasRelated, typename Method,
             std::enable_if_t<std::is_member_function_pointer_v<Method>, bool>>
#else
    template<typename HasRelated, typename Method>
    requires std::is_member_function_pointer_v<Method>
#endif
    const Relation<Model, Related> &
    Relation<Model, Related>::whereHas(
            const Method relation,
            const std::function<void(TinyBuilder<HasRelated> &)> &callback,
            const QString &comparison, const qint64 count) const
    {
        m_query->template whereHas<HasRelated>(relation, callback, comparison, count);

        return *this;
    }

    template<class Model, class Related>
    QVector<QVariant>
    Relation<Model, Related>::getKeys(const QVector<Model> &models,
                                      const QString &key) const
    {
        QVector<QVariant> keys;

        for (const auto &model : models)
            keys.append(key.isEmpty() ? model.getKey()
                                      : model.getAttribute(key));

        using namespace ranges;
        return keys |= actions::sort(less {}, &QVariant::value<typename Model::KeyType>)
                       | actions::unique;
    }

    template<class Model, class Related>
    std::unique_ptr<Builder<Related>>
    Relation<Model, Related>::getRelationExistenceQuery(
            std::unique_ptr<Builder<Related>> &&query, const Builder<Model> &,
            const QVector<Column> &columns) const
    {
        query->select(columns).whereColumnEq(getQualifiedParentKeyName(),
                                             getExistenceCompareKey());
        return std::move(query);
    }

    template<class Model, class Related>
    std::unique_ptr<Builder<Related>>
    Relation<Model, Related>::getRelationExistenceCountQuery(
            std::unique_ptr<Builder<Related>> &&query,
            const Builder<Model> &parentQuery) const
    {
        // Ownership of a unique_ptr()
        query = getRelationExistenceQuery(std::move(query), parentQuery,
                                          {Expression("count(*)")});

        query->getQuery().setBindings({}, BindingType::SELECT);

        return std::move(query);
    }

} // namespace Orm::Tiny::Relations
} // namespace Orm::Tiny
} // namespace Orm
#ifdef TINYORM_COMMON_NAMESPACE
} // namespace TINYORM_COMMON_NAMESPACE
#endif

#endif // RELATION_H
