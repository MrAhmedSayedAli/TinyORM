#pragma once
#ifndef ORM_TINY_TINYTYPES_HPP
#define ORM_TINY_TINYTYPES_HPP

#include "orm/macros/systemheader.hpp"
TINY_SYSTEM_HEADER

#include <optional>

#include "orm/ormtypes.hpp"
#include "orm/tiny/tinyconcepts.hpp"

TINYORM_BEGIN_COMMON_NAMESPACE

namespace Orm
{
namespace Tiny
{

    template<typename Model>
    class Builder;

    template<typename Model>
    using TinyBuilder = Builder<Model>;

    template<typename Derived, AllRelationsConcept ...AllRelations>
    class Model;

    /*! The type in which the relationships are stored. */
    template<AllRelationsConcept ...AllRelations>
    using RelationsType = std::variant<std::monostate,
                                       QVector<AllRelations>...,
                                       std::optional<AllRelations>...>;

    // TODO pretty print in the debugger silverqx
    /*! Attribute item used in ORM models. */
    struct SHAREDLIB_EXPORT AttributeItem
    {
        QString  key;
        QVariant value;

        /*! Converting operator to the UpdateItem. */
        explicit operator UpdateItem() const;
    };

    /*! Comparison operator for the AttributeItem. */
    SHAREDLIB_EXPORT bool operator==(const AttributeItem &lhs, const AttributeItem &rhs);

    /*! Eager load relation item. */
    struct WithItem
    {
        QString name;
        std::function<void(QueryBuilder &)> constraints {nullptr};
    };

    /*! Comparison operator for the WithItem. */
    [[maybe_unused]]
    SHAREDLIB_EXPORT bool operator==(const WithItem &lhs, const WithItem &rhs);

    /*! Tag for Model::getRelation() family methods to return Related type
        directly ( not container type ). */
    struct One {};

    /*! Tag for Model::getRelationshipFromMethod() to return QVector<Related>
        type ( 'Many' relation types ), only internal type for now, used as the template
        tag in the Model::pushVisited. */
    struct Many {};

    /*! Options parameter type used in Model save() method. */
    struct SaveOptions
    {
        /*! Indicates if timestamps of parent models should be touched. */
        bool touch = true;
    };

    /*! Tag for the model. */
    class IsModel
    {};

namespace Concerns
{
    /*! QueriesRelationships builder type passed to the callback. */
    template<typename Related>
    using QueriesRelationshipsCallback =
            std::conditional_t<std::is_void_v<Related>, QueryBuilder,
                               TinyBuilder<Related>>;
} // namespace Concerns
} // namespace Tiny

    using One = Orm::Tiny::One;

} // namespace Orm

TINYORM_END_COMMON_NAMESPACE

#endif // ORM_TINY_TINYTYPES_HPP
