#pragma once
#ifndef ORM_TINY_CONCERNS_BUILDSQUERIES_HPP
#define ORM_TINY_CONCERNS_BUILDSQUERIES_HPP

#include "orm/macros/systemheader.hpp"
TINY_SYSTEM_HEADER

#include "orm/exceptions/multiplerecordsfounderror.hpp"
#include "orm/exceptions/recordsnotfounderror.hpp"
#include "orm/ormtypes.hpp"
#include "orm/tiny/tinyconcepts.hpp" // IWYU pragma: keep
#include "orm/utils/query.hpp"
#include "orm/utils/type.hpp"

TINYORM_BEGIN_COMMON_NAMESPACE

namespace Orm::Tiny
{
    template<typename Model>
    class Builder;

namespace Concerns
{

    /*! More complex 'Retrieving results' methods that internally build queries. */
    template<ModelConcept Model>
    class BuildsQueries
    {
        /*! Alias for the query utils. */
        using QueryUtils = Orm::Utils::Query;

    public:
        /*! Default constructor. */
        inline BuildsQueries() = default;
        /*! Default destructor. */
        inline ~BuildsQueries() = default;

        /*! Copy constructor. */
        inline BuildsQueries(const BuildsQueries &) = default;
        /*! Deleted copy assignment operator (not needed). */
        BuildsQueries &operator=(const BuildsQueries &) = delete;

        /*! Move constructor. */
        inline BuildsQueries(BuildsQueries &&) noexcept = default;
        /*! Deleted move assignment operator (not needed). */
        BuildsQueries &operator=(BuildsQueries &&) = delete;

        /*! Chunk the results of the query. */
        bool chunk(int count,
                   const std::function<
                       bool(QVector<Model> &&models, int page)> &callback);
        /*! Execute a callback over each item while chunking. */
        bool each(const std::function<bool(Model &&model, int index)> &callback,
                  int count = 1000);

        /*! Run a map over each item while chunking. */
        QVector<Model>
        chunkMap(const std::function<Model(Model &&model)> &callback, int count = 1000);
        /*! Run a map over each item while chunking. */
        template<typename T>
        QVector<T>
        chunkMap(const std::function<T(Model &&model)> &callback, int count = 1000);

        /*! Chunk the results of a query by comparing IDs. */
        bool chunkById(int count,
                       const std::function<
                           bool(QVector<Model> &&models, int page)> &callback,
                       const QString &column = "", const QString &alias = "");
        /*! Execute a callback over each item while chunking by ID. */
        bool eachById(const std::function<bool(Model &&model, int index)> &callback,
                      int count = 1000, const QString &column = "",
                      const QString &alias = "");

        /*! Execute the query and get the first result if it's the sole matching
            record. */
        Model sole(const QVector<Column> &columns = {ASTERISK});

        /*! Pass the query to a given callback. */
        Builder<Model> &tap(const std::function<void(Builder<Model> &query)> &callback);

    private:
        /*! Static cast *this to the QueryBuilder & derived type. */
        inline Builder<Model> &builder() noexcept;
        /*! Static cast *this to the QueryBuilder & derived type, const version. */
        inline const Builder<Model> &builder() const noexcept;
    };

    /* The reason why two BuildsQueries classes exist.
       I have merged QueryBuilder::BuildsQueries and TinyBuilder::BuildsQueries into
       the one templated class and it worked well, no problem, there were only a few
       if constexpr() statements because the SqlQuery can not be iterated using
       the ranged-loop or with iterators, BUT there was a big confusion with parameter
       types passed to the chunk/each() methods, the QueryBuilder needs SqlQuery & and
       TinyBuilder Model && or QVector<Model> && and that is a big confusing problem.
       I was able to compute these types using the metaprogramming at compile time and
       I named them eg. Results but you know nothing from this 'Results' type, so when
       a user fastly looks into the .hpp file it would be fucked up about how these
       chunk/each methods work and what type of callback he needs to define.
       So I have decided to divide it like it's now to the QueryBuilder::BuildsQueries
       and TinyBuilder::BuildsQueries, now is everything crystal clear.
       Clear and nice code has much higher priority for me and because of that I can
       accept this practically duplicate code. 😎 */

    /* public */

    template<ModelConcept Model>
    bool BuildsQueries<Model>::chunk(
            const int count, const std::function<bool(QVector<Model> &&, int)> &callback)
    {
        builder().enforceOrderBy();

        int page = 1;
        int countModels = 0;

        do { // NOLINT(cppcoreguidelines-avoid-do-while)
            /* We'll execute the query for the given page and get the results. If there
               are no results we can just break and return. When there are results
               we will call the callback with the current chunk of these results. */
            auto models = builder().forPage(page, count).get();

            countModels = models.size();

            if (countModels == 0)
                break;

            /* On each chunk result set, we will pass them to the callback and then let
               the developer take care of everything within the callback, which allows
               us to keep the memory low for spinning through large result sets
               for working. */
            if (const auto result = std::invoke(callback, std::move(models), page);
                !result
            )
                return false;

            ++page;

        } while (countModels == count);

        return true;
    }

    template<ModelConcept Model>
    bool BuildsQueries<Model>::each(const std::function<bool(Model &&, int)> &callback,
                                    const int count)
    {
        return chunk(count, [&callback](QVector<Model> &&models, const int /*unused*/)
        {
            int index = 0;

            for (auto &&model : models)
                if (const auto result = std::invoke(callback, std::move(model), index++);
                    !result
                )
                    return false;

            return true;
        });
    }

    template<ModelConcept Model>
    QVector<Model>
    BuildsQueries<Model>::chunkMap(const std::function<Model(Model &&)> &callback,
                                   const int count)
    {
        QVector<Model> result;

        chunk(count, [&result, &callback](QVector<Model> &&models, const int /*unused*/)
        {
            for (auto &&model : models)
                result << std::invoke(callback, std::move(model));

            return true;
        });

        return result;
    }

    template<ModelConcept Model>
    template<typename T>
    QVector<T>
    BuildsQueries<Model>::chunkMap(const std::function<T(Model &&)> &callback,
                                   const int count)
    {
        QVector<T> result;

        chunk(count, [&result, &callback](QVector<Model> &&models, const int /*unused*/)
        {
            for (auto &&model : models)
                result << std::invoke(callback, std::move(model));

            return true;
        });

        return result;
    }

    template<ModelConcept Model>
    bool BuildsQueries<Model>::chunkById(
            const int count, const std::function<bool(QVector<Model> &&, int)> &callback,
            const QString &column, const QString &alias)
    {
        const auto columnName = column.isEmpty() ? builder().defaultKeyName() : column;
        const auto aliasName = alias.isEmpty() ? columnName : alias;

        int page = 1;
        int countModels = 0;

        QVariant lastId;

        do { // NOLINT(cppcoreguidelines-avoid-do-while)
            auto clone = builder().clone();

            /* We'll execute the query for the given page and get the results. If there
               are no results we can just break and return. When there are results
               we will call the callback with the current chunk of these results. */
            auto models = clone.forPageAfterId(count, lastId, columnName, true).get();

            countModels = models.size();

            if (countModels == 0)
                break;

            /* Obtain the lastId before the results is passed to the user's callback
               because an user can leave the results in the invalid/changed state. */
            lastId = models.constLast().getAttribute(aliasName);

            /* And this check can also be made before a callback invocation, it saves
               the unnecessary invocation if the lastId is invalid. It also helps
               to avoid passing invalid data to the user. */
            if (!lastId.isValid() || lastId.isNull())
                throw Orm::Exceptions::RuntimeError(
                        QStringLiteral(
                            "The chunkById operation was aborted because the "
                            "[%1] column is not present in the query result.")
                        .arg(aliasName));

            /* On each chunk result set, we will pass them to the callback and then let
               the developer take care of everything within the callback, which allows
               us to keep the memory low for spinning through large result sets
               for working. */
            if (const auto result = std::invoke(callback, std::move(models), page);
                !result
            )
                return false;

            ++page;

        } while (countModels == count);

        return true;
    }

    template<ModelConcept Model>
    bool BuildsQueries<Model>::eachById(
            const std::function<bool(Model &&, int)> &callback,
            const int count, const QString &column, const QString &alias)
    {
        return chunkById(count, [&callback, count]
                                (QVector<Model> &&models, const int page)
        {
            int index = 0;

            for (auto &&model : models)
                if (const auto result = std::invoke(callback, std::move(model),
                                                    ((page - 1) * count) + index++);
                    !result
                )
                    return false;

            return true;
        },
                column, alias);
    }

    template<ModelConcept Model>
    Model BuildsQueries<Model>::sole(const QVector<Column> &columns)
    {
        auto models = builder().take(2).get(columns);

        if (builder().getConnection().pretending())
            return {};

        const auto count = models.size();

        if (count == 0)
            throw Orm::Exceptions::RecordsNotFoundError(
                    QStringLiteral("No records found in %1().").arg(__tiny_func__));

        if (count > 1)
            throw Orm::Exceptions::MultipleRecordsFoundError(count, __tiny_func__);

        return std::move(models.first());
    }

    template<ModelConcept Model>
    Builder<Model> &
    BuildsQueries<Model>::tap(const std::function<void(Builder<Model> &)> &callback)
    {
        std::invoke(callback, builder());

        return builder();
    }

    /* private */

    template<ModelConcept Model>
    Builder<Model> &BuildsQueries<Model>::builder() noexcept
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
        return static_cast<Builder<Model> &>(*this);
    }

    template<ModelConcept Model>
    const Builder<Model> &BuildsQueries<Model>::builder() const noexcept
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
        return static_cast<const Builder<Model> &>(*this);
    }

} // namespace Concerns
} // namespace Orm::Tiny

TINYORM_END_COMMON_NAMESPACE

#endif // ORM_TINY_CONCERNS_BUILDSQUERIES_HPP
