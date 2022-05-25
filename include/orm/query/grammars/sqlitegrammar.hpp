#pragma once
#ifndef ORM_QUERY_GRAMMARS_SQLITEGRAMMAR_HPP
#define ORM_QUERY_GRAMMARS_SQLITEGRAMMAR_HPP

#include "orm/macros/systemheader.hpp"
TINY_SYSTEM_HEADER

#include "orm/query/grammars/grammar.hpp"

TINYORM_BEGIN_COMMON_NAMESPACE

namespace Orm::Query::Grammars
{

    /*! SQLite grammar. */
    class SHAREDLIB_EXPORT SQLiteGrammar : public Grammar
    {
        Q_DISABLE_COPY(SQLiteGrammar)

    public:
        /*! Default constructor. */
        inline SQLiteGrammar() = default;
        /*! Virtual destructor. */
        inline ~SQLiteGrammar() override = default;

        /*! Compile an insert ignore statement into SQL. */
        QString compileInsertOrIgnore(const QueryBuilder &query,
                                      const QVector<QVariantMap> &values) const override;

        /*! Compile an update statement into SQL. */
        QString compileUpdate(QueryBuilder &query,
                              const QVector<UpdateItem> &values) const override;

        /*! Compile a delete statement into SQL. */
        QString compileDelete(QueryBuilder &query) const override;

        /*! Compile a truncate table statement into SQL. Returns a map of
            the query string and bindings. */
        std::unordered_map<QString, QVector<QVariant>>
        compileTruncate(const QueryBuilder &query) const override;

        /*! Compile the lock into SQL. */
        QString compileLock(const QueryBuilder &query) const override;

        /*! Get the grammar specific operators. */
        const QVector<QString> &getOperators() const override;

    protected:
        /*! Compile the columns for an update statement. */
        QString compileUpdateColumns(const QVector<UpdateItem> &values) const override;

        /*! Map the ComponentType to a Grammar::compileXx() methods. */
        const QMap<SelectComponentType, SelectComponentValue> &
        getCompileMap() const override;
        /*! Map the WhereType to a Grammar::whereXx() methods. */
        const std::function<QString(const WhereConditionItem &)> &
        getWhereMethod(WhereType whereType) const override;

    private:
        /*! Compile an update statement with joins or limit into SQL. */
        QString compileUpdateWithJoinsOrLimit(QueryBuilder &query,
                                              const QVector<UpdateItem> &values) const;

        /*! Compile a delete statement with joins or limit into SQL. */
        QString compileDeleteWithJoinsOrLimit(QueryBuilder &query) const;
    };

} // namespace Orm::Query::Grammars

TINYORM_END_COMMON_NAMESPACE

#endif // ORM_QUERY_GRAMMARS_SQLITEGRAMMAR_HPP
