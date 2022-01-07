#pragma once
#ifndef ORM_TINY_CONCERNS_HASRELATIONSHIPS_HPP
#define ORM_TINY_CONCERNS_HASRELATIONSHIPS_HPP

#include "orm/macros/systemheader.hpp"
TINY_SYSTEM_HEADER

#ifdef __GNUG__
#include <map>
#else
#include <unordered_map>
#endif
#include <unordered_set>

#include <range/v3/algorithm/contains.hpp>

#include "orm/exceptions/invalidargumenterror.hpp"
#include "orm/tiny/concerns/hasrelationstore.hpp"
#include "orm/tiny/exceptions/relationnotfounderror.hpp"
#include "orm/tiny/exceptions/relationnotloadederror.hpp"
#include "orm/tiny/macros/crtpmodelwithbase.hpp"
#include "orm/tiny/relations/belongsto.hpp"
#include "orm/tiny/relations/belongstomany.hpp"
#include "orm/tiny/relations/hasmany.hpp"
#include "orm/tiny/relations/hasone.hpp"

TINYORM_BEGIN_COMMON_NAMESPACE

namespace Orm::Tiny
{
namespace Relations
{
    class IsPivotModel;
}
namespace Concerns
{

    /*! Model relationships. */
    template<typename Derived, AllRelationsConcept ...AllRelations>
    class HasRelationships :
            private Tiny::Concerns::HasRelationStore<Derived, AllRelations...>
    {
        /* Using starts in the BaseRelationStore::visit() and is used to access private
           visited methods. */
        friend Concerns::HasRelationStore<Derived, AllRelations...>;
        // To access XyzVisitor()-s, replaceRelations() and few other private methods
        friend Model<Derived, AllRelations...>;
        // To access eagerLoadRelationWithVisitor()
        friend TinyBuilder<Derived>;
        // To access private queriesRelationshipsWithVisitor()
        friend Concerns::QueriesRelationships<Derived>;

    public:
        /*! Get a relationship for Many types relation. */
        template<typename Related,
                 template<typename> typename Container = QVector>
        const Container<Related *>
        getRelationValue(const QString &relation);
        /*! Get a relationship for a One type relation. */
        template<typename Related, typename Tag> requires std::same_as<Tag, One>
        Related *
        getRelationValue(const QString &relation);

        /*! Get a specified relationship. */
        template<typename Related,
                 template<typename> typename Container = QVector>
        const Container<Related *>
        getRelation(const QString &relation);
        /*! Get a specified relationship as Related type, for use with HasOne and
            BelongsTo relation types. */
        template<typename Related, typename Tag> requires std::same_as<Tag, One>
        Related *getRelation(const QString &relation);

        /*! Determine if the given relation is loaded. */
        bool relationLoaded(const QString &relation) const;

        /*! Set the given relationship on the model. */
        template<typename Related>
        Derived &setRelation(const QString &relation, const QVector<Related> &models);
        /*! Set the given relationship on the model. */
        template<typename Related>
        Derived &setRelation(const QString &relation, QVector<Related> &&models);
        /*! Set the given relationship on the model. */
        template<typename Related>
        Derived &setRelation(const QString &relation,
                             const std::optional<Related> &model);
        /*! Set the given relationship on the model. */
        template<typename Related>
        Derived &setRelation(const QString &relation, std::optional<Related> &&model);
        /*! Set the given relationship on the model. */
        template<typename Related>
        requires std::is_base_of_v<Relations::IsPivotModel, Related>
        Derived &setRelation(const QString &relation,
                             const std::optional<Related> &model);
        /*! Set the given relationship on the model. */
        template<typename Related>
        requires std::is_base_of_v<Relations::IsPivotModel, Related>
        Derived &setRelation(const QString &relation, std::optional<Related> &&model);

        /* Relationships factory methods */
        /*! Define a one-to-one relationship. */
        template<typename Related>
        std::unique_ptr<Relations::HasOne<Derived, Related>>
        hasOne(QString foreignKey = "", QString localKey = "");
        /*! Define an inverse one-to-one or many relationship. */
        template<typename Related>
        std::unique_ptr<Relations::BelongsTo<Derived, Related>>
        belongsTo(QString foreignKey = "", QString ownerKey = "", QString relation = "");
        /*! Define a one-to-many relationship. */
        template<typename Related>
        std::unique_ptr<Relations::HasMany<Derived, Related>>
        hasMany(QString foreignKey = "", QString localKey = "");
        /*! Define a many-to-many relationship. */
        template<typename Related, typename PivotType = Relations::Pivot>
        std::unique_ptr<Relations::BelongsToMany<Derived, Related, PivotType>>
        belongsToMany(QString table = "", QString foreignPivotKey = "",
                      QString relatedPivotKey = "", QString parentKey = "",
                      QString relatedKey = "", QString relation = "");

        /*! Touch the owning relations of the model. */
        void touchOwners();

        /*! Get the relationships that are touched on save. */
        inline const QStringList &getTouchedRelations() const;
        /*! Determine if the model touches a given relation. */
        inline bool touches(const QString &relation) const;

        /*! Get all the loaded relations for the instance. */
        inline
#ifdef __GNUG__
        const std::map<QString, RelationsType<AllRelations...>> &
#else
        const std::unordered_map<QString, RelationsType<AllRelations...>> &
#endif
        getRelations() const;
        /*! Get all the loaded relations for the instance. */
        inline
#ifdef __GNUG__
        std::map<QString, RelationsType<AllRelations...>> &
#else
        std::unordered_map<QString, RelationsType<AllRelations...>> &
#endif
        getRelations();

        /*! Unset all the loaded relations for the instance. */
        Derived &unsetRelations();
        /*! Unset a loaded relationship. */
        Derived &unsetRelation(const QString &relation);

    protected:
        /*! Relation visitor lambda type. */
        using RelationVisitor = std::function<void(
                typename Concerns::HasRelationStore<Derived, AllRelations...>
                                 ::BaseRelationStore &)>;

        /*! Get a relationship value from a method. */
        template<class Related,
                 template<typename> typename Container = QVector>
        Container<Related *>
        getRelationshipFromMethod(const QString &relation);
        /*! Get a relationship value from a method. */
        template<class Related, typename Tag> requires std::same_as<Tag, One>
        Related *
        getRelationshipFromMethod(const QString &relation);

        /*! Set the entire relations hash on the model. */
        Derived &setRelations(
#ifdef __GNUG__
                const std::map<QString, RelationsType<AllRelations...>> &relations);
#else
                const std::unordered_map<QString,
                                         RelationsType<AllRelations...>> &relations);
#endif
        /*! Set the entire relations hash on the model. */
        Derived &setRelations(
#ifdef __GNUG__
                std::map<QString, RelationsType<AllRelations...>> &&relations);
#else
                std::unordered_map<QString, RelationsType<AllRelations...>> &&relations);
#endif

        /* Relationships factory methods */
        /*! Instantiate a new HasOne relationship. */
        template<typename Related>
        inline std::unique_ptr<Relations::HasOne<Derived, Related>>
        newHasOne(std::unique_ptr<Related> &&related, Derived &parent,
                  const QString &foreignKey, const QString &localKey) const;
        /*! Instantiate a new BelongsTo relationship. */
        template<typename Related>
        inline std::unique_ptr<Relations::BelongsTo<Derived, Related>>
        newBelongsTo(std::unique_ptr<Related> &&related,
                     Derived &child, const QString &foreignKey,
                     const QString &ownerKey, const QString &relation) const;
        /*! Instantiate a new HasMany relationship. */
        template<typename Related>
        inline std::unique_ptr<Relations::HasMany<Derived, Related>>
        newHasMany(std::unique_ptr<Related> &&related, Derived &parent,
                   const QString &foreignKey, const QString &localKey) const;
        /*! Instantiate a new BelongsToMany relationship. */
        template<typename Related, typename PivotType>
        inline std::unique_ptr<Relations::BelongsToMany<Derived, Related, PivotType>>
        newBelongsToMany(std::unique_ptr<Related> &&related, Derived &parent,
                         const QString &table, const QString &foreignPivotKey,
                         const QString &relatedPivotKey, const QString &parentKey,
                         const QString &relatedKey, const QString &relation) const;

        /*! Create a new model instance for a related model. */
        template<typename Related>
        std::unique_ptr<Related> newRelatedInstance() const;

        /*! Guess the "belongs to" relationship name. */
        template<typename Related>
        QString guessBelongsToRelation() const;
        /*! Guess the "belongs to many" relationship name. */
        template<typename Related>
        QString guessBelongsToManyRelation() const;
        /*! Get the joining table name for a many-to-many relation. */
        template<typename Related>
        QString pivotTableName() const;

        /* Data members */
        /*! Map of relation names to methods. */
        QHash<QString, RelationVisitor> u_relations;

        // BUG std::unordered_map prevents to compile on GCC, if I comment out std::optional<AllRelations>... in the RelationsType<AllRelations...>, or I change it to the QHash, then it compile, I'm absolutelly lost why this is happening 😞😭, I can't change to the QHash because of 25734deb, I have created simple test project gcc_trivial_bug_test in merydeye-gentoo, but std::map works so it is a big win, because now I can compile whole project on gcc ✨🚀 silverqx
        /*! The loaded relationships for the model. */
#ifdef __GNUG__
        std::map<QString, RelationsType<AllRelations...>> m_relations;
#else
        std::unordered_map<QString, RelationsType<AllRelations...>> m_relations;
#endif
        /*! The relationships that should be touched on save. */
        QStringList u_touches;
        // CUR1 use sets instead of QStringList where appropriate silverqx
        /*! Currently loaded Pivot relation names. */
        std::unordered_set<QString> m_pivots;

    private:
        /*! Alias for the enum struct RelationNotFoundError::From. */
        using RelationFrom = Tiny::Exceptions::RelationNotFoundError::From;

        /*! Throw exception if a relation is not defined. */
        void validateUserRelation(const QString &name,
                                  RelationFrom from = RelationFrom::UNDEFINED) const;

        /*! Obtain related models from "relationships" data member hash
            without any checks. */
        template<class Related,
                 template<typename> typename Container = QVector>
        Container<Related *>
        getRelationFromHash(const QString &relation);
        /*! Obtain related models from "relationships" data member hash
            without any checks. */
        template<class Related, typename Tag> requires std::same_as<Tag, One>
        Related *
        getRelationFromHash(const QString &relation);

        /*! Create lazy store and obtain a relationship from defined method. */
        template<typename Related, typename Result>
        Result getRelationshipFromMethodWithVisitor(const QString &relation);

        /*! Throw exception if correct getRelation/Value() method was not used, to avoid
            std::bad_variant_access. */
        template<typename Result, typename Related, typename T>
        void checkRelationType(const T &relationVariant, const QString &relation,
                               const QString &source) const;

        /*! Guess the relationship name for belongsTo/belongsToMany. */
        template<typename Related>
        QString guessBelongsToRelationInternal() const;

        /* Eager load relation store related */
        /*! Obtain a relationship instance for eager loading. */
        void eagerLoadRelationWithVisitor(
                const WithItem &relation, const TinyBuilder<Derived> &builder,
                QVector<Derived> &models);

        /* Get related table for belongs to many relation store related */
        /*! Get Related model table name if the relation is BelongsToMany, otherwise
            return empty std::optional. */
        std::optional<QString>
        getRelatedTableForBelongsToManyWithVisitor(const QString &relation);

        /* Push relation store related */
        /*! Create push store and call push for every model. */
        bool pushWithVisitor(const QString &relation,
                             RelationsType<AllRelations...> &models);

        /*! On the base of alternative held by m_relations decide, which
            pushVisitied() to execute. */
        template<typename Related>
        void pushVisited();
        /*! Push for Many relation types. */
        template<typename Related, typename Tag> requires std::same_as<Tag, Many>
        void pushVisited();
        /*! Push for One relation type. */
        template<typename Related, typename Tag> requires std::same_as<Tag, One>
        void pushVisited();

        /* Touch owners store related */
        /*! Create 'touch owners relation store' and touch all related models. */
        void touchOwnersWithVisitor(const QString &relation);
        /*! On the base of alternative held by m_relations decide, which
            touchOwnersVisited() to execute. */
        template<typename Related, typename Relation>
        void touchOwnersVisited(Relation &&relation);

        /* QueriesRelationships store related */
        /*! Create 'QueriesRelationships relation store' to obtain relation instance. */
        template<typename Related = void>
        void queriesRelationshipsWithVisitor(
                const QString &relation, Concerns::QueriesRelationships<Derived> &origin,
                const QString &comparison, qint64 count, const QString &condition,
                const std::function<void(
                        Concerns::QueriesRelationshipsCallback<Related> &)> &callback,
                std::optional<std::reference_wrapper<
                        QStringList>> relations = std::nullopt);

        /* Operations on a model instance */
        /*! Obtain all loaded relation names except pivot relations. */
        QVector<WithItem> getLoadedRelationsWithoutPivot();

        /*! Replace relations in the m_relation. */
        void replaceRelations(
#ifdef __GNUG__
                std::map<QString, RelationsType<AllRelations...>> &relations,
#else
                std::unordered_map<QString, RelationsType<AllRelations...>> &relations,
#endif
                const QVector<WithItem> &onlyRelations);

        /* Static cast this to a child's instance type (CRTP) */
        TINY_CRTP_MODEL_WITH_BASE_DECLARATIONS
    };

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related, template<typename> typename Container>
    const Container<Related *> // NOLINT(readability-const-return-type)
    HasRelationships<Derived, AllRelations...>::getRelationValue(const QString &relation)
    {
        /*! If the key already exists in the relationships hash, it just means the
            relationship has already been loaded, so we'll just return it out of
            here because there is no need to query within the relations twice. */
        if (this->relationLoaded(relation))
            return getRelationFromHash<Related, Container>(relation);

        /*! If the relation is defined on the model, then lazy load and return results
            from the query and hydrate the relationship's value on the "relationships"
            data member m_relations. */
        if (basemodel().getUserRelations().contains(relation))
            return getRelationshipFromMethod<Related, Container>(relation);

        return {};
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related, typename Tag> requires std::same_as<Tag, One>
    Related *
    HasRelationships<Derived, AllRelations...>::getRelationValue(const QString &relation)
    {
        /*! If the key already exists in the relationships hash, it just means the
            relationship has already been loaded, so we'll just return it out of
            here because there is no need to query within the relations twice. */
        if (this->relationLoaded(relation))
            return getRelationFromHash<Related, Tag>(relation);

        /*! If the relation is defined on the model, then lazy load and return results
            from the query and hydrate the relationship's value on the "relationships"
            data member m_relations. */
        if (basemodel().getUserRelations().contains(relation))
            return getRelationshipFromMethod<Related, Tag>(relation);

        return nullptr;
    }

    // TODO make getRelation() Container argument compatible with STL containers API silverqx
    // TODO solve different behavior like Eloquent getRelation() silverqx
    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related, template<typename> typename Container>
    const Container<Related *> // NOLINT(readability-const-return-type)
    HasRelationships<Derived, AllRelations...>::getRelation(const QString &relation)
    {
        if (!relationLoaded(relation))
            throw Exceptions::RelationNotLoadedError(
                    Orm::Utils::Type::classPureBasename<Derived>(), relation);

        return getRelationFromHash<Related, Container>(relation);
    }

    // TODO smart pointer for this relation stuffs? silverqx
    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related, typename Tag> requires std::same_as<Tag, One>
    Related *
    HasRelationships<Derived, AllRelations...>::getRelation(const QString &relation)
    {
        if (!relationLoaded(relation))
            throw Exceptions::RelationNotLoadedError(
                    Orm::Utils::Type::classPureBasename<Derived>(), relation);

        return getRelationFromHash<Related, Tag>(relation);
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    bool
    HasRelationships<Derived, AllRelations...>::relationLoaded(
            const QString &relation) const
    {
        return m_relations.contains(relation);
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related>
    Derived &
    HasRelationships<Derived, AllRelations...>::setRelation(
            const QString &relation,
            const QVector<Related> &models)
    {
        m_relations[relation] = models;

        return model();
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related>
    Derived &
    HasRelationships<Derived, AllRelations...>::setRelation(
            const QString &relation,
            QVector<Related> &&models)
    {
        m_relations[relation] = std::move(models);

        return model();
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related>
    Derived &
    HasRelationships<Derived, AllRelations...>::setRelation(
            const QString &relation,
            const std::optional<Related> &model)
    {
        m_relations[relation] = model;

        return this->model();
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related>
    Derived &
    HasRelationships<Derived, AllRelations...>::setRelation(
            const QString &relation,
            std::optional<Related> &&model)
    {
        m_relations[relation] = std::move(model);

        return this->model();
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related>
    requires std::is_base_of_v<Relations::IsPivotModel, Related>
    Derived &
    HasRelationships<Derived, AllRelations...>::setRelation(
            const QString &relation,
            const std::optional<Related> &model)
    {
        m_pivots.insert(relation);

        m_relations[relation] = model;

        return this->model();
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related>
    requires std::is_base_of_v<Relations::IsPivotModel, Related>
    Derived &
    HasRelationships<Derived, AllRelations...>::setRelation(
            const QString &relation,
            std::optional<Related> &&model)
    {
        m_pivots.insert(relation);

        m_relations[relation] = std::move(model);

        return this->model();
    }

    /* Relationships factory methods */

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related>
    std::unique_ptr<Relations::HasOne<Derived, Related>>
    HasRelationships<Derived, AllRelations...>::hasOne(
            QString foreignKey, QString localKey)
    {
        auto instance = newRelatedInstance<Related>();

        if (foreignKey.isEmpty())
            foreignKey = model().getForeignKey();

        if (localKey.isEmpty())
            localKey = basemodel().getKeyName();

        return newHasOne<Related>(std::move(instance), model(),
                                  QStringLiteral("%1.%2").arg(instance->getTable(),
                                                              foreignKey),
                                  localKey);
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related>
    std::unique_ptr<Relations::BelongsTo<Derived, Related>>
    HasRelationships<Derived, AllRelations...>::belongsTo(
            QString foreignKey, QString ownerKey, QString relation)
    {
        /* If no relation name was given, we will use the Related class type to extract
           the name and use that as the relationship name as most of the time this
           will be what we desire to use for the belongsTo relationships. */
        if (relation.isEmpty())
            relation = guessBelongsToRelation<Related>();

        auto instance = newRelatedInstance<Related>();

        const auto &relatedKeyName = instance->getKeyName();

        /* If no foreign key was supplied, we can guess the proper foreign key name
           by using the snake case name of the relationship, which when combined
           with an "_id" should conventionally match the columns. */
        if (foreignKey.isEmpty())
            foreignKey = QStringLiteral("%1_%2").arg(relation, relatedKeyName);

        /* Once we have the foreign key names, we return the relationship instance,
           which will actually be responsible for retrieving and hydrating every
           relations. */
        if (ownerKey.isEmpty())
            ownerKey = relatedKeyName;

        return newBelongsTo<Related>(std::move(instance), model(),
                                     foreignKey, ownerKey, relation);
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related>
    std::unique_ptr<Relations::HasMany<Derived, Related>>
    HasRelationships<Derived, AllRelations...>::hasMany(
            QString foreignKey, QString localKey)
    {
        auto instance = newRelatedInstance<Related>();

        if (foreignKey.isEmpty())
            foreignKey = model().getForeignKey();

        if (localKey.isEmpty())
            localKey = basemodel().getKeyName();

        return newHasMany<Related>(std::move(instance), model(),
                                   QStringLiteral("%1.%2").arg(instance->getTable(),
                                                               foreignKey),
                                   localKey);
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related, typename PivotType>
    std::unique_ptr<Relations::BelongsToMany<Derived, Related, PivotType>>
    HasRelationships<Derived, AllRelations...>::belongsToMany(
            QString table, QString foreignPivotKey, QString relatedPivotKey,
            QString parentKey, QString relatedKey, QString relation)
    {
        /* If no relation name was given, we will use the Related class type to extract
           the name, suffix it with 's' and use that as the relationship name, as most
           of the time this will be what we desire to use for the belongsToMany
           relationships. */
        if (relation.isEmpty())
            relation = guessBelongsToManyRelation<Related>();

        /* First, we'll need to determine the foreign key and "other key"
           for the relationship. Once we have determined the keys, we'll make
           the relationship instance we need for this. */
        auto instance = newRelatedInstance<Related>();

        if (foreignPivotKey.isEmpty())
            foreignPivotKey = model().getForeignKey();

        if (relatedPivotKey.isEmpty())
            relatedPivotKey = instance->getForeignKey();

        /* If no table name was provided, we can guess it by concatenating the two
           models using underscores in alphabetical order. The two model names
           are transformed to snake case from their default CamelCase also. */
        if (table.isEmpty())
            table = pivotTableName<Related>();

        if (parentKey.isEmpty())
            parentKey = basemodel().getKeyName();

        if (relatedKey.isEmpty())
            relatedKey = instance->getKeyName();

        return newBelongsToMany<Related, PivotType>(
                    std::move(instance), model(), table, foreignPivotKey,
                    relatedPivotKey, parentKey, relatedKey, relation);
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    void HasRelationships<Derived, AllRelations...>::touchOwners()
    {
        for (const auto &relation : getTouchedRelations())
            touchOwnersWithVisitor(relation);
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    const QStringList &
    HasRelationships<Derived, AllRelations...>::getTouchedRelations() const
    {
        return basemodel().getUserTouches();
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    bool
    HasRelationships<Derived, AllRelations...>::touches(const QString &relation) const
    {
        return getTouchedRelations().contains(relation);
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
#ifdef __GNUG__
    const std::map<QString, RelationsType<AllRelations...>> &
#else
    const std::unordered_map<QString, RelationsType<AllRelations...>> &
#endif
    HasRelationships<Derived, AllRelations...>::getRelations() const
    {
        return m_relations;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
#ifdef __GNUG__
    std::map<QString, RelationsType<AllRelations...>> &
#else
    std::unordered_map<QString, RelationsType<AllRelations...>> &
#endif
    HasRelationships<Derived, AllRelations...>::getRelations()
    {
        return m_relations;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    Derived &HasRelationships<Derived, AllRelations...>::unsetRelations()
    {
        m_relations.clear();

        return model();
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    Derived &
    HasRelationships<Derived, AllRelations...>::unsetRelation(const QString &relation)
    {
        m_relations.erase(relation);

        return model();
    }

    /* protected */

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<class Related, template<typename> typename Container>
    Container<Related *>
    HasRelationships<Derived, AllRelations...>::getRelationshipFromMethod(
            const QString &relation)
    {
        // Obtain related models
        auto relatedModels =
                getRelationshipFromMethodWithVisitor<Related,
                                                     QVector<Related>>(relation);

        setRelation(relation, std::move(relatedModels));

        return getRelationFromHash<Related, Container>(relation);
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<class Related, typename Tag> requires std::same_as<Tag, One>
    Related *
    HasRelationships<Derived, AllRelations...>::getRelationshipFromMethod(
            const QString &relation)
    {
        // Obtain related model
        auto relatedModel =
                getRelationshipFromMethodWithVisitor<Related,
                                                     std::optional<Related>>(relation);

        setRelation(relation, std::move(relatedModel));

        return getRelationFromHash<Related, Tag>(relation);
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    Derived &
    HasRelationships<Derived, AllRelations...>::setRelations(
#ifdef __GNUG__
            const std::map<QString, RelationsType<AllRelations...>> &relations)
#else
            const std::unordered_map<QString, RelationsType<AllRelations...>> &relations)
#endif
    {
        m_relations = relations;

        return model();
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    Derived &
    HasRelationships<Derived, AllRelations...>::setRelations(
#ifdef __GNUG__
            std::map<QString, RelationsType<AllRelations...>> &&relations)
#else
            std::unordered_map<QString, RelationsType<AllRelations...>> &&relations)
#endif
    {
        m_relations = std::move(relations);

        return model();
    }

    /* Relationships factory methods */

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related>
    std::unique_ptr<Relations::HasOne<Derived, Related>>
    HasRelationships<Derived, AllRelations...>::newHasOne(
            std::unique_ptr<Related> &&related, Derived &parent,
            const QString &foreignKey, const QString &localKey) const
    {
        return Relations::HasOne<Derived, Related>::instance(
                    std::move(related), parent, foreignKey, localKey);
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related>
    std::unique_ptr<Relations::BelongsTo<Derived, Related>>
    HasRelationships<Derived, AllRelations...>::newBelongsTo(
            std::unique_ptr<Related> &&related, Derived &child,
            const QString &foreignKey, const QString &ownerKey,
            const QString &relation) const
    {
        return Relations::BelongsTo<Derived, Related>::instance(
                    std::move(related), child, foreignKey, ownerKey, relation);
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related>
    std::unique_ptr<Relations::HasMany<Derived, Related>>
    HasRelationships<Derived, AllRelations...>::newHasMany(
            std::unique_ptr<Related> &&related, Derived &parent,
            const QString &foreignKey, const QString &localKey) const
    {
        return Relations::HasMany<Derived, Related>::instance(
                    std::move(related), parent, foreignKey, localKey);
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related, typename PivotType>
    std::unique_ptr<Relations::BelongsToMany<Derived, Related, PivotType>>
    HasRelationships<Derived, AllRelations...>::newBelongsToMany(
            std::unique_ptr<Related> &&related, Derived &parent, const QString &table,
            const QString &foreignPivotKey, const QString &relatedPivotKey,
            const QString &parentKey, const QString &relatedKey,
            const QString &relation) const
    {
        return Relations::BelongsToMany<Derived, Related, PivotType>::instance(
                    std::move(related), parent, table, foreignPivotKey,
                    relatedPivotKey, parentKey, relatedKey, relation);
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related>
    std::unique_ptr<Related>
    HasRelationships<Derived, AllRelations...>::newRelatedInstance() const
    {
        auto instance = std::make_unique<Related>();

        if (instance->getConnectionName().isEmpty())
            instance->setConnection(basemodel().getConnectionName());

        return instance;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related>
    QString
    HasRelationships<Derived, AllRelations...>::guessBelongsToRelation() const
    {
        static const QString relation = guessBelongsToRelationInternal<Related>();

        /* validateUserRelation() method call can not be cached, has to be called
           every time, to correctly inform the user about invalid relation name. */

        // Validate if the guessed relation name exists in the u_relations
        validateUserRelation(relation, RelationFrom::BELONGS_TO);

        return relation;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related>
    QString
    HasRelationships<Derived, AllRelations...>::guessBelongsToManyRelation() const
    {
        static const QString relation =
                QStringLiteral("%1s").arg(guessBelongsToRelationInternal<Related>());

        /* validateUserRelation() method call can not be cached, has to be called
           every time, to correctly inform the user about invalid relation name. */

        // Validate if the guessed relation name exists in the u_relations
        validateUserRelation(relation, RelationFrom::BELONGS_TO_MANY);

        return relation;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related>
    QString HasRelationships<Derived, AllRelations...>::pivotTableName() const
    {
        /* The joining table name, by convention, is simply the snake cased, models
           sorted alphabetically and concatenated with an underscore, so we can
           just sort the models and join them together to get the table name. */
        QStringList segments {
            // The table name of the current model instance
            Orm::Utils::Type::classPureBasename<Derived>(),
            // The table name of the related model instance
            Orm::Utils::Type::classPureBasename<Related>(),
        };

        /* Now that we have the model names in the vector, we can just sort them and
           use the join function to join them together with an underscore,
           which is typically used by convention within the database system. */
        segments.sort(Qt::CaseInsensitive);

        return segments.join(UNDERSCORE).toLower();
    }

    /* private */

    template<typename Derived, AllRelationsConcept ...AllRelations>
    void
    HasRelationships<Derived, AllRelations...>::validateUserRelation(
            const QString &name, const RelationFrom from) const
    {
        if (!basemodel().getUserRelations().contains(name))
            throw Exceptions::RelationNotFoundError(
                    Orm::Utils::Type::classPureBasename<Derived>(), name, from);
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<class Related, template<typename> typename Container>
    Container<Related *>
    HasRelationships<Derived, AllRelations...>::getRelationFromHash(
            const QString &relation)
    {
        auto &relationVariant = m_relations.find(relation)->second;

        // Check relation type to avoid std::bad_variant_access
        checkRelationType<QVector<Related>, Related>(
                    relationVariant, relation, "getRelation");

        /* Obtain related models from data member hash as QVector, it is internal
           format and transform it into a Container of pointers to related models,
           so a user can directly modify these models and push or save them
           afterward. */
        namespace views = ranges::views;
        return std::get<QVector<Related>>(relationVariant)
                | views::transform([](Related &model) -> Related * { return &model; })
                | ranges::to<Container<Related *>>();
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<class Related, typename Tag> requires std::same_as<Tag, One>
    Related *
    HasRelationships<Derived, AllRelations...>::getRelationFromHash(
            const QString &relation)
    {
        auto &relationVariant = m_relations.find(relation)->second;

        // Check relation type to avoid std::bad_variant_access
        checkRelationType<std::optional<Related>, Related>(
                    relationVariant, relation, "getRelation");

        /* Obtain related model from data member hash and return it as a pointer or
           nullptr if no model is associated, so a user can directly modify this
           model and push or save it afterward. */

        auto &relatedModel = std::get<std::optional<Related>>(relationVariant);

        return relatedModel ? &*relatedModel : nullptr;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related, typename Result>
    Result
    HasRelationships<Derived, AllRelations...>::getRelationshipFromMethodWithVisitor(
            const QString &relation)
    {
        // Throw excpetion if a relation is not defined
        validateUserRelation(relation);

        // Save model/s to the store to avoid passing variables to the visitor
        this->template createLazyStore<Related>().visit(relation);

        // Obtain result, related model/s
        const auto lazyResult = this->template lazyStore<Related>().m_result;

        // Releases the ownership and destroy the top relation store on the stack
        this->resetRelationStore();

        // Check relation type to avoid std::bad_variant_access
        checkRelationType<Result, Related>(lazyResult, relation, "getRelationValue");

        return std::get<Result>(lazyResult);
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Result, typename Related, typename T>
    void HasRelationships<Derived, AllRelations...>::checkRelationType(
            const T &relationVariant, const QString &relation,
            const QString &source) const
    {
        if constexpr (std::is_same_v<Result, std::optional<Related>>) {
            if (!std::holds_alternative<Result>(relationVariant))
                throw Orm::Exceptions::RuntimeError(
                        QStringLiteral(
                            "The relation '%1' is many type relation, use "
                            "%2<%3>() method overload without an 'Orm::One' tag.")
                        .arg(relation, source,
                             Orm::Utils::Type::classPureBasename<Related>()));
        } else if constexpr (std::is_same_v<Result, QVector<Related>>) {
            if (!std::holds_alternative<Result>(relationVariant))
                throw Orm::Exceptions::RuntimeError(
                        QStringLiteral(
                            "The relation '%1' is one type relation, use "
                            "%2<%3, Orm::One>() method overload "
                            "with an 'Orm::One' tag.")
                        .arg(relation, source,
                             Orm::Utils::Type::classPureBasename<Related>()));
        } else
            throw Orm::Exceptions::InvalidArgumentError(
                    "Unexpected 'Result' template argument.");
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related>
    QString
    HasRelationships<Derived, AllRelations...>::guessBelongsToRelationInternal() const
    {
        // TODO reliability, also add Orm::Tiny::Utils::String::studly silverqx
        auto relation = Orm::Utils::Type::classPureBasename<Related>();

        relation[0] = relation[0].toLower();

        return relation;
    }

    /* Eager load relation store related */

    template<typename Derived, AllRelationsConcept ...AllRelations>
    void HasRelationships<Derived, AllRelations...>::eagerLoadRelationWithVisitor(
            const WithItem &relation, const TinyBuilder<Derived> &builder,
            QVector<Derived> &models)
    {
        // Throw excpetion if a relation is not defined
        validateUserRelation(relation.name);

        /* Save the needed variables to the store to avoid passing variables
           to the visitor. */
        this->createEagerStore(builder, models, relation).visit(relation.name);

        // Releases the ownership and destroy the top relation store on the stack
        this->resetRelationStore();
    }

    /* Get related table for belongs to many relation store related */

    template<typename Derived, AllRelationsConcept ...AllRelations>
    std::optional<QString>
    HasRelationships<Derived, AllRelations...>
    ::getRelatedTableForBelongsToManyWithVisitor(const QString &relation)
    {
        // Throw excpetion if a relation is not defined
        validateUserRelation(relation);

        // Create the store and visit relation
        this->createBelongsToManyRelatedTableStore().visit(relation);

        // NRVO kicks in
        auto relatedTable = this->belongsToManyRelatedTableStore().m_result;

        // Releases the ownership and destroy the top relation store on the stack
        this->resetRelationStore();

        return relatedTable;
    }

    /* Push relation store related */

    template<typename Derived, AllRelationsConcept ...AllRelations>
    bool HasRelationships<Derived, AllRelations...>::pushWithVisitor(
            const QString &relation, RelationsType<AllRelations...> &models)
    {
        // TODO prod remove, I don't exactly know if this can really happen silverqx
        /* Check for empty variant, the std::monostate is at zero index and
           consider it as success to continue 'pushing'. */
        const auto variantIndex = models.index();
        Q_ASSERT(variantIndex > 0);
        if (variantIndex == 0)
            return true;

        // Throw excpetion if a relation is not defined
        validateUserRelation(relation);

        // Save model/s to the store to avoid passing variables to the visitor
        this->createPushStore(models).visit(relation);

        const auto pushResult = this->pushStore().m_result;

        // Releases the ownership and destroy the top relation store on the stack
        this->resetRelationStore();

        return pushResult;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related>
    void HasRelationships<Derived, AllRelations...>::pushVisited()
    {
        const RelationsType<AllRelations...> &models = this->pushStore().m_models;

        // Invoke pushVisited() on the base of hold alternative in the models
        if (std::holds_alternative<QVector<Related>>(models))
            pushVisited<Related, Many>();
        else if (std::holds_alternative<std::optional<Related>>(models))
            pushVisited<Related, One>();
        else
            throw Orm::Exceptions::RuntimeError(
                    "this->pushStore().models holds unexpected alternative.");
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related, typename Tag> requires std::same_as<Tag, Many>
    void HasRelationships<Derived, AllRelations...>::pushVisited()
    {
        auto &pushStore = this->pushStore();

        for (auto &model : std::get<QVector<Related>>(pushStore.m_models))
            if (!model.push()) {
                pushStore.m_result = false;
                return;
            }

        pushStore.m_result = true;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related, typename Tag> requires std::same_as<Tag, One>
    void HasRelationships<Derived, AllRelations...>::pushVisited()
    {
        auto &pushStore = this->pushStore();

        auto &model = std::get<std::optional<Related>>(pushStore.m_models);
        Q_ASSERT(model);

        // Skip a null model, consider it as success
        if (!model) {
            pushStore.m_result = true;
            return;
        }

        pushStore.m_result = model->push();
    }

    /* Touch owners store related */

    template<typename Derived, AllRelationsConcept ...AllRelations>
    void
    HasRelationships<Derived, AllRelations...>::touchOwnersWithVisitor(
            const QString &relation)
    {
        // Throw excpetion if a relation is not defined
        validateUserRelation(relation);

        // Save model/s to the store to avoid passing variables to the visitor
        this->createTouchOwnersStore(relation).visit(relation);

        // Releases the ownership and destroy the top relation store on the stack
        this->resetRelationStore();
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related, typename Relation>
    void HasRelationships<Derived, AllRelations...>::touchOwnersVisited(
            Relation &&relation)
    {
        const auto &relationName = this->touchOwnersStore().m_relation;

        relation->touch();

        // Many type relation
        if constexpr (std::is_base_of_v<Relations::ManyRelation,
                                        typename Relation::element_type>)
        {
            for (auto *const relatedModel : getRelationValue<Related>(relationName))
                // WARNING check and add note after, if many type relation QVector can contain nullptr silverqx
                if (relatedModel)
                    relatedModel->touchOwners();
        }

        // One type relation
        else if constexpr (std::is_base_of_v<Relations::OneRelation,
                                             typename Relation::element_type>)
        {
            if (auto *const relatedModel = getRelationValue<Related, One>(relationName);
                relatedModel
            )
                relatedModel->touchOwners();
        } else
            throw Orm::Exceptions::RuntimeError(
                    "Bad relation type passed to the Model::touchOwnersVisited().");
    }

    /* QueriesRelationships store related */

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename Related>
    void HasRelationships<Derived, AllRelations...>::queriesRelationshipsWithVisitor(
            const QString &relation, Concerns::QueriesRelationships<Derived> &origin,
            const QString &comparison, const qint64 count, const QString &condition,
            const std::function<void(
                Concerns::QueriesRelationshipsCallback<Related> &)> &callback,
            const std::optional<std::reference_wrapper<QStringList>> relations)
    {
        // Throw excpetion if a relation is not defined
        validateUserRelation(relation);

        // Save model/s to the store to avoid passing variables to the visitor
        this->template createQueriesRelationshipsStore<Related>(
                    origin, comparison, count, condition, callback, relations)
                .visit(relation);

        // Releases the ownership and destroy the top relation store on the stack
        this->resetRelationStore();
    }

    /* Operations on a model instance */

    template<typename Derived, AllRelationsConcept ...AllRelations>
    QVector<WithItem>
    HasRelationships<Derived, AllRelations...>::getLoadedRelationsWithoutPivot()
    {
        QVector<WithItem> relations;

        /* Current model (this) contains a pivot relation alternative
           in the m_relations std::variant. */
        auto hasPivotRelation = std::disjunction_v<std::is_base_of<
                                Relations::IsPivotModel, AllRelations>...>;

        /* Get all currently loaded relation names except pivot relations. We need
           to check for the pivot models, but only if the std::variant which holds
           relations also holds a pivot model alternative, otherwise it is useless. */
        for (const auto &relation : m_relations) {
            const auto &relationName = relation.first;

            // Skip pivot relations
            if (hasPivotRelation && m_pivots.contains(relationName))
                    continue;

            relations.append({relationName});
        }

        return relations;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    void HasRelationships<Derived, AllRelations...>::replaceRelations(
#ifdef __GNUG__
            std::map<QString, RelationsType<AllRelations...>> &relations,
#else
            std::unordered_map<QString, RelationsType<AllRelations...>> &relations,
#endif
            const QVector<WithItem> &onlyRelations)
    {
        /* Replace only relations which was passed to this method, leave other
           relations untouched. */
        for (auto itRelation = relations.begin(); itRelation != relations.end();
             ++itRelation)
        {
            const auto &key = itRelation->first;

            const auto relationsContainKey =
                    ranges::contains(onlyRelations, true, [&key](const auto &relation)
            {
                if (!relation.name.contains(COLON))
                    return relation.name == key;

                // Support for select constraints
                return relation.name.split(COLON).at(0).trimmed() == key;
            });

            if (!relationsContainKey)
                continue;

            m_relations[key] = std::move(itRelation->second);
        }
    }

    /* Static cast this to a child's instance type (CRTP) */
    TINY_CRTP_MODEL_WITH_BASE_DEFINITIONS(HasRelationships)

} // namespace Concerns
} // namespace Orm::Tiny

TINYORM_END_COMMON_NAMESPACE

#endif // ORM_TINY_CONCERNS_HASRELATIONSHIPS_HPP