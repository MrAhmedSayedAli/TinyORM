#pragma once
#ifndef ORM_TINY_UTILS_ATTRIBUTE_HPP
#define ORM_TINY_UTILS_ATTRIBUTE_HPP

#include "orm/macros/systemheader.hpp"
TINY_SYSTEM_HEADER

#include <unordered_set>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/filter.hpp>

#include "orm/tiny/tinytypes.hpp"

TINYORM_BEGIN_COMMON_NAMESPACE

namespace Orm::Tiny::Utils
{

    /*! Library class for the database attribute. */
    class SHAREDLIB_EXPORT Attribute
    {
        Q_DISABLE_COPY_MOVE(Attribute)

    public:
        /*! Deleted default constructor, this is a pure library class. */
        Attribute() = delete;
        /*! Deleted destructor. */
        ~Attribute() = delete;

        /*! Convert a AttributeItem QVector to QVariantMap. */
        static QVariantMap
        convertVectorToMap(const QVector<AttributeItem> &attributes);
        /*! Convert a vector of AttributeItem QVectors to the vector of QVariantMaps. */
        static QVector<QVariantMap>
        convertVectorsToMaps(const QVector<QVector<AttributeItem>> &attributesVector);

        /*! Convert a AttributeItem QVector to UpdateItem QVector. */
        static QVector<UpdateItem>
        convertVectorToUpdateItem(const QVector<AttributeItem> &attributes);
        /*! Convert a AttributeItem QVector to UpdateItem QVector. */
        static QVector<UpdateItem>
        convertVectorToUpdateItem(QVector<AttributeItem> &&attributes);

        /*! Remove attributes which have duplicite keys and leave only the last one. */
        static QVector<AttributeItem>
        removeDuplicitKeys(const QVector<AttributeItem> &attributes);
        /*! Remove attributes which have duplicite keys and leave only the last one. */
        static QVector<AttributeItem>
        removeDuplicitKeys(QVector<AttributeItem> &&attributes);

        /*! Join attributes and values for firstOrXx methods. */
        static QVector<AttributeItem>
        joinAttributesForFirstOr(const QVector<WhereItem> &attributes,
                                 const QVector<AttributeItem> &values,
                                 const QString &keyName);

        /*! Remove a given attributes from the model attributes vector and return
            a copy. */
        template<typename Model>
        static QVector<AttributeItem>
        exceptAttributesForReplicate(const Model &model,
                                     const std::unordered_set<QString> &except = {});
    };

    /* public */

    template<typename Model>
    QVector<AttributeItem>
    Attribute::exceptAttributesForReplicate(const Model &model,
                                            const std::unordered_set<QString> &except)
    {
        std::unordered_set<QString> defaults {
            model.getKeyName(),
            model.getCreatedAtColumn(),
            model.getUpdatedAtColumn(),
        };
        // Remove empty attribute names
        std::erase_if(defaults, [](const auto &attribute)
        {
            return attribute.isEmpty();
        });

        // Merge defaults into except
        std::unordered_set<QString> exceptMerged(defaults.size() + except.size());
        if (!except.empty()) {
            exceptMerged = except;
            exceptMerged.merge(defaults);
        }
        else
            exceptMerged = std::move(defaults);

        // Get all attributes excluding those in the exceptMerged set
        return model.getAttributes()
                | ranges::views::filter([&exceptMerged](const AttributeItem &attribute)
        {
            return !exceptMerged.contains(attribute.key);
        })
                | ranges::to<QVector<AttributeItem>>();
    }

} // namespace Orm::Tiny::Utils

TINYORM_END_COMMON_NAMESPACE

#endif // ORM_TINY_UTILS_ATTRIBUTE_HPP
