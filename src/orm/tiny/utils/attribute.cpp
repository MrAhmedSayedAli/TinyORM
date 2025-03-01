#include "orm/tiny/utils/attribute.hpp"

#include <unordered_set>

#include <range/v3/algorithm/contains.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/transform.hpp>

TINYORM_BEGIN_COMMON_NAMESPACE

namespace Orm::Tiny::Utils
{

QVariantMap Attribute::convertVectorToMap(const QVector<AttributeItem> &attributes)
{
    QVariantMap result;

    for (const auto &attribute : attributes)
        result.insert(attribute.key, attribute.value);

    return result;
}

QVector<QVariantMap>
Attribute::convertVectorsToMaps(const QVector<QVector<AttributeItem>> &attributesVector)
{
    const auto size = attributesVector.size();

    QVector<QVariantMap> result(size);

    for (QVector<QVariantMap>::size_type i = 0; i < size; ++i)
        for (const auto &attribute : attributesVector[i])
            result[i][attribute.key] = attribute.value;

    return result;
}

QVector<UpdateItem>
Attribute::convertVectorToUpdateItem(const QVector<AttributeItem> &attributes)
{
    QVector<UpdateItem> result;
    result.reserve(attributes.size());

    for (const auto &attribute : attributes)
        result.append({attribute.key, attribute.value});

    return result;
}

QVector<UpdateItem>
Attribute::convertVectorToUpdateItem(QVector<AttributeItem> &&attributes)
{
    QVector<UpdateItem> result;
    result.reserve(attributes.size());

    for (auto &&attribute : attributes)
        result.append({std::move(attribute.key),
                       std::move(attribute.value)});

    return result;
}

QVector<AttributeItem>
Attribute::removeDuplicitKeys(const QVector<AttributeItem> &attributes)
{
    const auto size = attributes.size();
    // The helper set, to check duplicit keys
    std::unordered_set<QString> added(static_cast<std::size_t>(size));

    QVector<std::reference_wrapper<const AttributeItem>> dedupedAttributesReversed;
    dedupedAttributesReversed.reserve(size);

    /* If I want to get only the last duplicate, I have to loop in the reverse order,
       so the previous attributes will be skipped and only the last attribute
       will be copied. */
    for (const auto &attribute : attributes | ranges::views::reverse) {
        // If duplicit key then skip
        if (added.contains(attribute.key))
            continue;

        added.emplace(attribute.key);
        dedupedAttributesReversed << std::cref(attribute);
    }

    // Materialize the vector of references in reverse order
    return dedupedAttributesReversed
            | ranges::views::reverse
            | ranges::to<QVector<AttributeItem>>();
}

QVector<AttributeItem>
Attribute::removeDuplicitKeys(QVector<AttributeItem> &&attributes)
{
    const auto size = attributes.size();
    // The helper set, to check duplicit keys
    std::unordered_set<QString> added(static_cast<std::size_t>(size));

    QVector<std::reference_wrapper<AttributeItem>> dedupedAttributesReversed;
    dedupedAttributesReversed.reserve(size);

    /* If I want to get only the last duplicate, I have to loop in the reverse order,
       so the previous attributes will be skipped and only the last attribute
       will be moved. */
    for (auto &attribute : attributes | ranges::views::reverse) {
        // If duplicit key then skip
        if (added.contains(attribute.key))
            continue;

        added.emplace(attribute.key);
        dedupedAttributesReversed << std::ref(attribute);
    }

    // Materialize the vector of references in reverse order
    QVector<AttributeItem> dedupedAttributes;
    dedupedAttributes.reserve(dedupedAttributesReversed.size());

    for (auto &&attribute : dedupedAttributesReversed | ranges::views::reverse)
        dedupedAttributes << std::move(attribute.get());

    return dedupedAttributes;
}

QVector<AttributeItem>
Attribute::joinAttributesForFirstOr(const QVector<WhereItem> &attributes,
                                    const QVector<AttributeItem> &values,
                                    const QString &keyName)
{
    // Remove the primary key from attributes
    auto attributesFiltered = attributes
            | ranges::views::remove_if([&keyName](const WhereItem &attribute)
    {
        /* AttributeItem or more precise TinyORM attributes as such, can not contain
           expression in the column name.
           Of course, I could obtain the QString value from the expression and
           compare it, but I will not support that intentionally, instead
           the std::bad_variant_access exception will be thrown. */
        return keyName == std::get<QString>(attribute.column);
    })
            | ranges::views::transform([](const WhereItem &attribute)
    {
        /* AttributeItem or more precise TinyORM attributes as such, can not contain
           expression in the column name. */
        return AttributeItem {std::get<QString>(attribute.column), attribute.value};
    })
            | ranges::to<QVector<AttributeItem>>();

    // Attributes which already exist in 'attributes' will be removed from 'values'
    auto valuesFiltered = values
            | ranges::views::remove_if([&attributesFiltered](const AttributeItem &value)
    {
        return ranges::contains(attributesFiltered, true,
                                [&value](const AttributeItem &attribute)
        {
            return attribute.key == value.key;
        });
    })
            | ranges::to<QVector<AttributeItem>>();

    return attributesFiltered + valuesFiltered;
}

} // namespace Orm::Tiny::Utils

TINYORM_END_COMMON_NAMESPACE
