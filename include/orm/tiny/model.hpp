#pragma once
#ifndef ORM_TINY_MODEL_HPP
#define ORM_TINY_MODEL_HPP

#include "orm/macros/systemheader.hpp"
TINY_SYSTEM_HEADER

#include "orm/concerns/hasconnectionresolver.hpp"
#include "orm/connectionresolverinterface.hpp"
#include "orm/tiny/concerns/guardsattributes.hpp"
#include "orm/tiny/concerns/hasattributes.hpp"
#include "orm/tiny/concerns/hasrelationships.hpp"
#include "orm/tiny/concerns/hastimestamps.hpp"
#include "orm/tiny/exceptions/massassignmenterror.hpp"
#include "orm/tiny/macros/crtpmodel.hpp"
#include "orm/tiny/modelproxies.hpp"
#include "orm/tiny/tinybuilder.hpp"
#ifdef TINYORM_TESTS_CODE
#  include "orm/tiny/types/connectionoverride.hpp"
#endif

TINYORM_BEGIN_COMMON_NAMESPACE

namespace Orm::Tiny
{

    /*! Alias for the GuardedModel. */
    using GuardedModel = Concerns::GuardedModel;

    // TODO model missing methods Soft Deleting, Model::trashed()/restore()/withTrashed()/forceDelete()/onlyTrashed(), check this methods also on EloquentBuilder and SoftDeletes trait silverqx
    // TODO model missing methods Model::replicate() silverqx
    // TODO model missing methods Comparing Models silverqx
    // TODO model missing methods Model::firstOr() silverqx
    // TODO model missing methods Model::updateOrInsert() silverqx
    // TODO model missing methods Model::loadMissing() silverqx
    // TODO model missing methods Model::whereExists() silverqx
    // TODO model missing methods Model::whereBetween() silverqx
    // TODO model missing methods Model::exists()/notExists() silverqx
    // TODO model missing methods Model::saveOrFail() silverqx
    // TODO model missing methods EloquentCollection::toQuery() silverqx
    // TODO model missing methods Model::whereRow() silverqx
    /*! Base model class. */
    template<typename Derived, AllRelationsConcept ...AllRelations>
    class Model :
            public Orm::Concerns::HasConnectionResolver,
            public Tiny::Concerns::HasAttributes<Derived, AllRelations...>,
            public Tiny::Concerns::GuardsAttributes<Derived, AllRelations...>,
            public Tiny::Concerns::HasRelationships<Derived, AllRelations...>,
            public Tiny::Concerns::HasTimestamps<Derived, AllRelations...>,
            public ModelProxies<Derived, AllRelations...>,
            public IsModel
    {
        // To access getUserXx() methods
        friend Concerns::GuardsAttributes<Derived, AllRelations...>;
        // To access getUserXx() methods
        friend Concerns::HasAttributes<Derived, AllRelations...>;
        // To access getUserXx() methods
        friend Concerns::HasTimestamps<Derived, AllRelations...>;
        // Used by BaseRelationStore::visit() and also by visted methods
        friend Concerns::HasRelationStore<Derived, AllRelations...>;
        // To access getUserXx() methods
        friend Concerns::HasRelationships<Derived, AllRelations...>;
        // Used by QueriesRelationships::has()
        friend Concerns::QueriesRelationships<Derived>;
        // FUTURE try to solve problem with forward declarations for friend methods, to allow only relevant methods from TinyBuilder silverqx
        // Used by TinyBuilder::eagerLoadRelations()
        friend TinyBuilder<Derived>;

        /*! Alias for the attribute utils. */
        using AttributeUtils = Orm::Tiny::Utils::Attribute;
        /*! Alias for the string utils. */
        using StringUtils = Orm::Tiny::Utils::String;
        /*! Apply all the Model's template parameters to the passed T template
            argument. */
        template<template<typename ...> typename T>
        using ModelTypeApply = T<Derived, AllRelations...>;

    public:
        /*! The "type" of the primary key ID. */
        using KeyType = quint64;

        /* Constructors */
        /*! Create a new TinORM model instance. */
        Model();

        /*! Model's copy constructor. */
        inline Model(const Model &) = default;
        /*! Model's move constructor. */
        inline Model(Model &&) noexcept = default;

        /*! Model's copy assignment operator. */
        inline Model &operator=(const Model &) = default;
        /*! Model's move assignment operator. */
        inline Model &operator=(Model &&) noexcept = default;

        /*! Create a new TinORM model instance from attributes
            (converting constructor). */
        explicit Model(const QVector<AttributeItem> &attributes);
        /*! Create a new TinORM model instance from attributes
            (converting constructor). */
        explicit Model(QVector<AttributeItem> &&attributes);

        /*! Create a new TinORM model instance from attributes
            (list initialization). */
        Model(std::initializer_list<AttributeItem> attributes);

        /* Static operations on a model instance */
        /*! Create a new TinyORM model instance with given attributes. */
        static Derived instance(const QVector<AttributeItem> &attributes);
        /*! Create a new TinyORM model instance with given attributes. */
        static Derived instance(QVector<AttributeItem> &&attributes);

        /*! Begin querying the model. */
        static std::unique_ptr<TinyBuilder<Derived>> query();
        /*! Begin querying the model on a given connection. */
        static std::unique_ptr<TinyBuilder<Derived>> on(const QString &connection = "");

        /*! Get all of the models from the database. */
        static QVector<Derived> all(const QVector<Column> &columns = {ASTERISK});

        /* Operations on a model instance */
        /*! Save the model to the database. */
        bool save(SaveOptions options = {});
        /*! Save the model and all of its relationships. */
        bool push();

        /*! Update records in the database. */
        bool update(const QVector<AttributeItem> &attributes, SaveOptions options = {});

        /*! Delete the model from the database. */
        bool remove();
        /*! Delete the model from the database (alias). */
        bool deleteModel();

        /*! Reload a fresh model instance from the database. */
        std::optional<Derived> fresh(const QVector<WithItem> &relations = {});
        /*! Reload a fresh model instance from the database. */
        std::optional<Derived> fresh(const QString &relation);
        /*! Reload the current model instance with fresh attributes from the database. */
        Derived &refresh();

        /*! Eager load relations on the model. */
        Derived &load(const QVector<WithItem> &relations);
        /*! Eager load relations on the model. */
        Derived &load(const QString &relation);

        /*! Determine if two models have the same ID and belong to the same table. */
        template<typename ModelToCompare>
        bool is(const std::optional<ModelToCompare> &model) const;
        /*! Determine if two models are not the same. */
        template<typename ModelToCompare>
        bool isNot(const std::optional<ModelToCompare> &model) const;

        /*! Fill the model with a vector of attributes. */
        Derived &fill(const QVector<AttributeItem> &attributes);
        /*! Fill the model with a vector of attributes. */
        Derived &fill(QVector<AttributeItem> &&attributes);
        /*! Fill the model with a vector of attributes. Force mass assignment. */
        Derived &forceFill(const QVector<AttributeItem> &attributes);

        /* Model Instance methods */
        /*! Get a new query builder for the model's table. */
        inline std::unique_ptr<TinyBuilder<Derived>> newQuery();
        /*! Get a new query builder that doesn't have any global scopes. */
        std::unique_ptr<TinyBuilder<Derived>> newQueryWithoutScopes();
        /*! Get a new query builder that doesn't have any global scopes or
            eager loading. */
        std::unique_ptr<TinyBuilder<Derived>> newModelQuery();
        /*! Get a new query builder with no relationships loaded. */
        std::unique_ptr<TinyBuilder<Derived>> newQueryWithoutRelationships();
        /*! Create a new Tiny query builder for the model. */
        std::unique_ptr<TinyBuilder<Derived>>
        newTinyBuilder(const QSharedPointer<QueryBuilder> &query);

        /*! Create a new model instance that is existing. */
        Derived newFromBuilder(const QVector<AttributeItem> &attributes = {},
                               const std::optional<QString> &connection = std::nullopt);
        /*! Create a new instance of the given model. */
        inline Derived newInstance();
        /*! Create a new instance of the given model. */
        Derived newInstance(const QVector<AttributeItem> &attributes,
                            bool exists = false);
        /*! Create a new instance of the given model. */
        Derived newInstance(QVector<AttributeItem> &&attributes,
                            bool exists = false);

        /*! Create a new pivot model instance. */
        template<typename PivotType = Relations::Pivot, typename Parent>
        PivotType
        newPivot(const Parent &parent, const QVector<AttributeItem> &attributes,
                 const QString &table, bool exists) const;

        /* Static cast this to a child's instance type (CRTP) */
        TINY_CRTP_MODEL_DECLARATIONS

        /* Getters / Setters */
        /*! Get the current connection name for the model. */
        const QString &getConnectionName() const;
        /*! Get the database connection for the model. */
        inline DatabaseConnection &getConnection() const;
        /*! Set the connection associated with the model. */
        inline Derived &setConnection(const QString &name);
        /*! Set the table associated with the model. */
        inline Derived &setTable(const QString &value);
        /*! Get the table associated with the model. */
        const QString &getTable() const;
        /*! Get the primary key for the model. */
        inline const QString &getKeyName() const;
        /*! Get the table qualified key name. */
        inline QString getQualifiedKeyName() const;
        /*! Get the value of the model's primary key. */
        inline QVariant getKey() const;
        /*! Get the value indicating whether the IDs are incrementing. */
        inline bool getIncrementing() const;
        /*! Set whether IDs are incrementing. */
        inline Derived &setIncrementing(bool value);
        /*! Get the default foreign key name for the model. */
        QString getForeignKey() const;

        /* Others */
        /*! Qualify the given column name by the model's table. */
        QString qualifyColumn(const QString &column) const;

        /* Data members */
        /*! Indicates if the model exists. */
        bool exists = false;

    protected:
        /* Model Instance methods */
        /*! Get a new query builder instance for the connection. */
        QSharedPointer<QueryBuilder> newBaseQueryBuilder() const;

        /* Operations on a model instance */
        /*! Perform the actual delete query on this model instance. */
        void performDeleteOnModel();

        /*! Set the keys for a save update query. */
        TinyBuilder<Derived> &
        setKeysForSaveQuery(TinyBuilder<Derived> &query);
        /*! Get the primary key value for a save query. */
        QVariant getKeyForSaveQuery() const;
        /*! Set the keys for a select query. */
        TinyBuilder<Derived> &
        setKeysForSelectQuery(TinyBuilder<Derived> &query);
        /*! Get the primary key value for a select query. */
        inline QVariant getKeyForSelectQuery() const;

        /*! Perform a model insert operation. */
        bool performInsert(const TinyBuilder<Derived> &query);
        /*! Perform a model insert operation. */
        bool performUpdate(TinyBuilder<Derived> &query);
        /*! Perform any actions that are necessary after the model is saved. */
        void finishSave(SaveOptions options = {});

        /*! Insert the given attributes and set the ID on the model. */
        quint64 insertAndSetId(const TinyBuilder<Derived> &query,
                               const QVector<AttributeItem> &attributes);

        /* Data members */
        /*! The table associated with the model. */
        QString u_table;
        /*! The connection name for the model. */
        QString u_connection;
        /*! Indicates if the model's ID is auto-incrementing. */
        bool u_incrementing = true;
        /*! The primary key associated with the table. */
        QString u_primaryKey = ID;

        // TODO detect (best at compile time) circular eager relation problem, the exception which happens during this problem is stackoverflow in QRegularExpression silverqx
        /*! The relations to eager load on every query. */
        QVector<QString> u_with;
        /*! The relationship counts that should be eager loaded on every query. */
//        QVector<WithItem> u_withCount;

        /* HasTimestamps */
        /*! The name of the "created at" column. */
        inline static const QString &CREATED_AT = Constants::CREATED_AT; // NOLINT(cppcoreguidelines-interfaces-global-init)
        /*! The name of the "updated at" column. */
        inline static const QString &UPDATED_AT = Constants::UPDATED_AT; // NOLINT(cppcoreguidelines-interfaces-global-init)

    private:
        /* HasAttributes */
        /*! Throw InvalidArgumentError if attributes passed to the constructor contain
            some value, which will cause access of some data member in a derived
            instance. */
        inline static void
        throwIfCRTPctorProblem(const QVector<AttributeItem> &attributes);
        /*! The QDateTime attribute detected, causes CRTP ctor problem. */
        static void throwIfQDateTimeAttribute(const QVector<AttributeItem> &attributes);
        /*! Throw if an attempt to fill a guarded attribute is detected (mass assign.). */
        static void throwIfTotallyGuarded(QString &&key);

        /* HasAttributes */
        /*! Get the u_dateFormat attribute from the Derived model. */
        inline QString &getUserDateFormat();
        /*! Get the u_dateFormat attribute from the Derived model. */
        inline const QString &getUserDateFormat() const;
        /*! Get the u_dates attribute from the Derived model. */
        inline static const QStringList &getUserDates();

        /* GuardsAttributes */
        /*! Get the u_fillable attributes from the Derived model. */
        inline QStringList &getUserFillable();
        /*! Get the u_fillable attributes from the Derived model. */
        inline const QStringList &getUserFillable() const;
        /*! Get the u_guarded attributes from the Derived model. */
        inline QStringList &getUserGuarded();
        /*! Get the u_guarded attributes from the Derived model. */
        inline const QStringList &getUserGuarded() const;

        /* HasRelationships */
        /*! Relation visitor lambda type (an alias for shorter declarations). */
        using RelationVisitorAlias =
                typename Concerns::HasRelationships<Derived, AllRelations...>
                                 ::RelationVisitor;

        /*! Get the u_fillable attributes from the Derived model. */
        inline const QHash<QString, RelationVisitorAlias> &getUserRelations() const;
        /*! Get the u_fillable attributes from the Derived model. */
        inline const QStringList &getUserTouches() const;

        /* HasTimestamps */
        /*! Get the u_timestamps attribute from the Derived model. */
        inline bool &getUserTimestamps();
        /*! Get the u_timestamps attribute from the Derived model. */
        inline bool getUserTimestamps() const;
        /*! Get the CREATED_AT attribute from the Derived model. */
        inline static const QString &getUserCreatedAtColumn();
        /*! Get the UPDATED_AT attribute from the Derived model. */
        inline static const QString &getUserUpdatedAtColumn();
    };

    /* public */

    /* Constructors */

    template<typename Derived, AllRelationsConcept ...AllRelations>
    Model<Derived, AllRelations...>::Model()
    {
        // Compile time check if a primary key type is supported by a QVariant
        qMetaTypeId<typename Derived::KeyType>();

        // Default Attribute Values
        fill(Derived::u_attributes);

        this->syncOriginal();
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    Model<Derived, AllRelations...>::Model(const QVector<AttributeItem> &attributes)
        : Model()
    {
        throwIfCRTPctorProblem(attributes);

        fill(attributes);
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    Model<Derived, AllRelations...>::Model(QVector<AttributeItem> &&attributes)
        : Model()
    {
        throwIfCRTPctorProblem(attributes);

        fill(std::move(attributes));
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    Model<Derived, AllRelations...>::Model(
            std::initializer_list<AttributeItem> attributes
    )
        : Model(QVector<AttributeItem>(attributes.begin(), attributes.end()))
    {}

    /* Static operations on a model instance */

    template<typename Derived, AllRelationsConcept ...AllRelations>
    Derived
    Model<Derived, AllRelations...>::instance(const QVector<AttributeItem> &attributes)
    {
        Derived model;

        model.fill(attributes);

        return model;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    Derived
    Model<Derived, AllRelations...>::instance(QVector<AttributeItem> &&attributes)
    {
        Derived model;

        model.fill(std::move(attributes));

        return model;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    std::unique_ptr<TinyBuilder<Derived>>
    Model<Derived, AllRelations...>::query()
    {
        return Derived().newQuery();
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    std::unique_ptr<TinyBuilder<Derived>>
    Model<Derived, AllRelations...>::on(const QString &connection)
    {
        /* First we will just create a fresh instance of this model, and then we can
           set the connection on the model so that it is used for the queries we
           execute, as well as being set on every relation we retrieve without
           a custom connection name. */
        Derived instance;

        instance.setConnection(connection);

        return instance.newQuery();
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    QVector<Derived>
    Model<Derived, AllRelations...>::all(const QVector<Column> &columns)
    {
        return query()->get(columns);
    }

    /* Operations on a model instance */

    template<typename Derived, AllRelationsConcept ...AllRelations>
    bool Model<Derived, AllRelations...>::save(const SaveOptions options)
    {
//        mergeAttributesFromClassCasts();

        // Ownership of a unique_ptr()
        auto query = newModelQuery();

        auto saved = false;

        /* If the "saving" event returns false we'll bail out of the save and return
           false, indicating that the save failed. This provides a chance for any
           listeners to cancel save operations if validations fail or whatever. */
//        if (fireModelEvent('saving') === false) {
//            return false;
//        }

        /* If the model already exists in the database we can just update our record
           that is already in this database using the current IDs in this "where"
           clause to only update this model. Otherwise, we'll just insert them. */
        if (exists)
            saved = this->isDirty() ? performUpdate(*query) : true;

        // If the model is brand new, we'll insert it into our database and set the
        // ID attribute on the model to the value of the newly inserted row's ID
        // which is typically an auto-increment value managed by the database.
        else {
            saved = performInsert(*query);

            if (const auto &connection = query->getConnection();
                getConnectionName().isEmpty()
            )
                setConnection(connection.getName());
        }

        /* If the model is successfully saved, we need to do a few more things once
           that is done. We will call the "saved" method here to run any actions
           we need to happen after a model gets successfully saved right here. */
        if (saved)
            finishSave(options);

        return saved;
    }

    // FUTURE support for SaveOptions parameter, Eloquent doesn't have this parameter, maybe there's a reason for that, but I didn't find anything on github issues silverqx
    template<typename Derived, AllRelationsConcept ...AllRelations>
    bool Model<Derived, AllRelations...>::push()
    {
        if (!save())
            return false;

        /* To sync all of the relationships to the database, we will simply spin through
           the relationships and save each model via this "push" method, which allows
           us to recurse into all of these nested relations for the model instance. */
        for (auto &[relation, models] : this->m_relations)
            // FUTURE Eloquent uses array_filter on models, investigate when this happens, null value (model) in many relations? silverqx
            /* Following Eloquent API, if any push failed, then quit, remaining push-es
               will not be processed. */
            if (!this->pushWithVisitor(relation, models))
                return false;

        return true;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    bool Model<Derived, AllRelations...>::update(
            const QVector<AttributeItem> &attributes,
            const SaveOptions options)
    {
        if (!exists)
            return false;

        return fill(attributes).save(options);
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    bool Model<Derived, AllRelations...>::remove()
    {
        // FEATURE castable silverqx
//        mergeAttributesFromClassCasts();

        if (getKeyName().isEmpty())
            throw Orm::Exceptions::RuntimeError("No primary key defined on model.");

        /* If the model doesn't exist, there is nothing to delete so we'll just return
           immediately and not do anything else. Otherwise, we will continue with a
           deletion process on the model, firing the proper events, and so forth. */
        if (!exists)
            // NOTE api different silverqx
            return false;

        // FUTURE add support for model events silverqx
//        if ($this->fireModelEvent('deleting') === false) {
//            return false;
//        }

        /* Here, we'll touch the owning models, verifying these timestamps get updated
           for the models. This will allow any caching to get broken on the parents
           by the timestamp. Then we will go ahead and delete the model instance. */
        this->touchOwners();

        // FUTURE performDeleteOnModel() and return value, check logic here, eg what happens when no model is delete and combinations silverqx
        // FUTURE inconsistent return values save(), update(), remove(), ... silverqx
        performDeleteOnModel();

        /* Once the model has been deleted, we will fire off the deleted event so that
           the developers may hook into post-delete operations. We will then return
           a boolean true as the delete is presumably successful on the database. */
//        $this->fireModelEvent('deleted', false);

        return true;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    bool Model<Derived, AllRelations...>::deleteModel()
    {
        return model().remove();
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    std::optional<Derived>
    Model<Derived, AllRelations...>::fresh(
            const QVector<WithItem> &relations)
    {
        if (!exists)
            return std::nullopt;

        return model().setKeysForSelectQuery(*newQueryWithoutScopes())
                .with(relations)
                .first();
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    std::optional<Derived>
    Model<Derived, AllRelations...>::fresh(const QString &relation)
    {
        return fresh(QVector<WithItem> {{relation}});
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    Derived &Model<Derived, AllRelations...>::refresh()
    {
        if (!exists)
            return model();

        this->setRawAttributes(model().setKeysForSelectQuery(*newQueryWithoutScopes())
                               .firstOrFail().getRawAttributes());

        // And reload them again, refresh relations
        load(this->getLoadedRelationsWithoutPivot());

        this->syncOriginal();

        return model();
    }

    // FUTURE LoadItem for Model::load() even it will have the same implmentation, or common parent and inherit silverqx
    template<typename Derived, AllRelationsConcept ...AllRelations>
    Derived &
    Model<Derived, AllRelations...>::load(
            const QVector<WithItem> &relations)
    {
        // Ownership of a unique_ptr()
        auto builder = newQueryWithoutRelationships();

        builder->with(relations);

        // FUTURE make possible to pass single model to eagerLoadRelations() and whole relation flow, I indicative counted how many methods would have to rewrite and it is around 12 methods silverqx
        /* I have to make a copy here of this, because of eagerLoadRelations(),
           the solution would be to add a whole new chain for eager load relations,
           which will be able to work only on one Model &, but it is around
           refactoring of 10-15 methods, or add a variant which can process
           QVector<std::reference_wrapper<Derived>>.
           For now, I have made a copy here and save it into the QVector and after
           that move relations from this copy to the real instance. */
        QVector<Derived> models {model()};

        builder->eagerLoadRelations(models);

        /* Replace only relations which was passed to this method, leave other
           relations untouched.
           They do not need to be removed before 'eagerLoadRelations(models)'
           call, because only the relations passed to the 'with' at the beginning
           will be loaded anyway. */
        this->replaceRelations(models.first().getRelations(), relations);

        return model();
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    Derived &Model<Derived, AllRelations...>::load(const QString &relation)
    {
        return load(QVector<WithItem> {{relation}});
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename ModelToCompare>
    bool Model<Derived, AllRelations...>::is(
            const std::optional<ModelToCompare> &model) const
    {
        return model
                && getKey() == model->getKey()
                && this->model().getTable() == model->getTable()
                && getConnectionName() == model->getConnectionName();
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename ModelToCompare>
    bool Model<Derived, AllRelations...>::isNot(
            const std::optional<ModelToCompare> &model) const
    {
        return !is(model);
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    Derived &
    Model<Derived, AllRelations...>::fill(const QVector<AttributeItem> &attributes)
    {
        const auto totallyGuarded = this->totallyGuarded();

        for (auto &attribute : this->fillableFromArray(attributes))
            /* The developers may choose to place some attributes in the "fillable" vector
               which means only those attributes may be set through mass assignment to
               the model, and all others will just get ignored for security reasons. */
            if (auto &key = attribute.key;
                this->isFillable(key)
            )
                this->setAttribute(key, std::move(attribute.value));

            else if (totallyGuarded)
                throwIfTotallyGuarded(std::move(key));

        return model();
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    Derived &
    Model<Derived, AllRelations...>::fill(QVector<AttributeItem> &&attributes)
    {
        const auto totallyGuarded = this->totallyGuarded();

        for (auto &attribute : this->fillableFromArray(std::move(attributes))) {
            /* The developers may choose to place some attributes in the "fillable" vector
               which means only those attributes may be set through mass assignment to
               the model, and all others will just get ignored for security reasons. */
            if (auto &key = attribute.key;
                this->isFillable(key)
            )
                this->setAttribute(key, std::move(attribute.value));

            else if (totallyGuarded)
                throwIfTotallyGuarded(std::move(key));
        }

        return model();
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    Derived &
    Model<Derived, AllRelations...>::forceFill(
            const QVector<AttributeItem> &attributes)
    {
        // Prevent unnecessary unguard
        if (attributes.isEmpty())
            return model();

        GuardedModel::unguarded([this, &attributes]
        {
            fill(attributes);
        });

        return model();
    }

    /* Model Instance methods */

    template<typename Derived, AllRelationsConcept ...AllRelations>
    std::unique_ptr<TinyBuilder<Derived>>
    Model<Derived, AllRelations...>::newQuery()
    {
        return newQueryWithoutScopes();
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    std::unique_ptr<TinyBuilder<Derived>>
    Model<Derived, AllRelations...>::newQueryWithoutScopes()
    {
        // Transform the QString vector to the WithItem vector
        const auto &relations = model().u_with;

        QVector<WithItem> relationsConverted;
        relationsConverted.reserve(relations.size());

        for (const auto &relation : relations)
            relationsConverted.append({relation});

        // Ownership of a unique_ptr()
        auto tinyBuilder = newModelQuery();

        tinyBuilder->with(relationsConverted);

        return tinyBuilder;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    std::unique_ptr<TinyBuilder<Derived>>
    Model<Derived, AllRelations...>::newModelQuery()
    {
        // Ownership of the QSharedPointer<QueryBuilder>
        const auto query = newBaseQueryBuilder();

        /* Model is passed to the TinyBuilder ctor, because of that setModel()
           isn't used here. Can't be const because of passed non-const model
           to the TinyBuilder. */
        return newTinyBuilder(query);
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    std::unique_ptr<TinyBuilder<Derived>>
    Model<Derived, AllRelations...>::newQueryWithoutRelationships()
    {
        return newModelQuery();
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    std::unique_ptr<TinyBuilder<Derived>>
    Model<Derived, AllRelations...>::newTinyBuilder(
            const QSharedPointer<QueryBuilder> &query)
    {
        return std::make_unique<TinyBuilder<Derived>>(query, model());
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    Derived
    Model<Derived, AllRelations...>::newFromBuilder(
            const QVector<AttributeItem> &attributes,
            const std::optional<QString> &connection)
    {
        auto model = newInstance({}, true);

        model.setRawAttributes(attributes, true);

        model.setConnection(connection ? *connection : getConnectionName());

        return model;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    Derived
    Model<Derived, AllRelations...>::newInstance()
    {
        return newInstance({});
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    Derived
    Model<Derived, AllRelations...>::newInstance(
            const QVector<AttributeItem> &attributes, const bool exists_)
    {
        /* This method just provides a convenient way for us to generate fresh model
           instances of this current model. It is particularly useful during the
           hydration of new objects via the QueryBuilder instances. */
        Derived model;

        /* setAttribute() can call getDateFormat() inside and it tries to obtain
           the date format from grammar which is obtained from the connection, so
           the connection have to be set before fill(). */
        model.setConnection(getConnectionName());

        model.fill(attributes);

        model.exists = exists_;
        model.setTable(this->model().getTable());

        return model;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    Derived
    Model<Derived, AllRelations...>::newInstance(
            QVector<AttributeItem> &&attributes, const bool exists_)
    {
        /* This method just provides a convenient way for us to generate fresh model
           instances of this current model. It is particularly useful during the
           hydration of new objects via the QueryBuilder instances. */
        Derived model;

        /* setAttribute() can call getDateFormat() inside and it tries to obtain
           the date format from grammar which is obtained from the connection, so
           the connection have to be set before fill(). */
        model.setConnection(getConnectionName());

        model.fill(std::move(attributes));

        model.exists = exists_;
        model.setTable(this->model().getTable());

        return model;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    template<typename PivotType, typename Parent>
    PivotType
    Model<Derived, AllRelations...>::newPivot(
            const Parent &parent, const QVector<AttributeItem> &attributes,
            const QString &table, const bool exists_) const
    {
        if constexpr (std::is_same_v<PivotType, Relations::Pivot>)
            return PivotType::template fromAttributes<Parent>(
                        parent, attributes, table, exists_);
        else
            return PivotType::template fromRawAttributes<Parent>(
                        parent, attributes, table, exists_);
    }

    /* Static cast this to a child's instance type (CRTP) */
    TINY_CRTP_MODEL_DEFINITIONS(Model)

    /* Getters / Setters */

    template<typename Derived, AllRelationsConcept ...AllRelations>
    const QString &
    Model<Derived, AllRelations...>::getConnectionName() const
    {
#ifdef TINYORM_TESTS_CODE
        // Used from tests to override connection
        if (const auto &connection = ConnectionOverride::connection;
            !connection.isEmpty()
        )
            return connection;
#endif

        return model().u_connection;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    DatabaseConnection &
    Model<Derived, AllRelations...>::getConnection() const
    {
        return m_resolver->connection(getConnectionName());
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    Derived &
    Model<Derived, AllRelations...>::setConnection(const QString &name)
    {
        model().u_connection = name;

        return model();
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    Derived &
    Model<Derived, AllRelations...>::setTable(const QString &value)
    {
        model().u_table = value;

        return model();
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    const QString &
    Model<Derived, AllRelations...>::getTable() const
    {
        const auto &table = model().u_table;

        // Get pluralized snake-case table name
        if (table.isEmpty())
            const_cast<QString &>(model().u_table) =
                QStringLiteral("%1s").arg(
                    StringUtils::snake(Orm::Utils::Type::classPureBasename<Derived>()));

        return table;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    const QString &
    Model<Derived, AllRelations...>::getKeyName() const
    {
        return model().u_primaryKey;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    QString
    Model<Derived, AllRelations...>::getQualifiedKeyName() const
    {
        return qualifyColumn(getKeyName());
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    QVariant
    Model<Derived, AllRelations...>::getKey() const
    {
        return this->getAttribute(getKeyName());
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    bool
    Model<Derived, AllRelations...>::getIncrementing() const
    {
        return model().u_incrementing;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    Derived &
    Model<Derived, AllRelations...>::setIncrementing(const bool value)
    {
        model().u_incrementing = value;

        return model();
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    QString Model<Derived, AllRelations...>::getForeignKey() const
    {
        return QStringLiteral("%1_%2").arg(
                    StringUtils::snake(Orm::Utils::Type::classPureBasename<Derived>()),
                    getKeyName());
    }

    /* Others */

    template<typename Derived, AllRelationsConcept ...AllRelations>
    QString
    Model<Derived, AllRelations...>::qualifyColumn(const QString &column) const
    {
        if (column.contains(DOT))
            return column;

        return QStringLiteral("%1.%2").arg(model().getTable(), column);
    }

    /* protected */

    /* Model Instance methods */

    template<typename Derived, AllRelationsConcept ...AllRelations>
    QSharedPointer<QueryBuilder>
    Model<Derived, AllRelations...>::newBaseQueryBuilder() const
    {
        return getConnection().query();
    }

    /* Operations on a model instance */

    template<typename Derived, AllRelationsConcept ...AllRelations>
    void Model<Derived, AllRelations...>::performDeleteOnModel()
    {
        /* Ownership of a unique_ptr(), if right passed down, then the
           will be destroyed right after this command. */
        model().setKeysForSaveQuery(*newModelQuery()).remove();

        this->exists = false;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    TinyBuilder<Derived> &
    Model<Derived, AllRelations...>::setKeysForSaveQuery(TinyBuilder<Derived> &query)
    {
        return query.where(getKeyName(), EQ, getKeyForSaveQuery());
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    QVariant Model<Derived, AllRelations...>::getKeyForSaveQuery() const
    {
        // Found
        if (const auto keyName = getKeyName();
            this->m_originalHash.contains(keyName)
        )
            return this->m_original.at(this->m_originalHash.at(keyName)).value;

        // Not found, return the primary key value
        return getKey();
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    TinyBuilder<Derived> &
    Model<Derived, AllRelations...>::setKeysForSelectQuery(TinyBuilder<Derived> &query)
    {
        return query.where(getKeyName(), EQ, getKeyForSelectQuery());
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    QVariant Model<Derived, AllRelations...>::getKeyForSelectQuery() const
    {
        // Currently is the implementation exactly the same, so I can call it
        return getKeyForSaveQuery();
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    bool Model<Derived, AllRelations...>::performInsert(
            const TinyBuilder<Derived> &query)
    {
//        if (!fireModelEvent("creating"))
//            return false;

        /* First we'll need to create a fresh query instance and touch the creation and
           update timestamps on this model, which are maintained by us for developer
           convenience. After, we will just continue saving these model instances. */
        if (this->usesTimestamps())
            this->updateTimestamps();

        /* If the model has an incrementing key, we can use the "insertGetId" method on
           the query builder, which will give us back the final inserted ID for this
           table from the database. Not all tables have to be incrementing though. */
        const auto &attributes = this->getAttributes();

        if (getIncrementing())
            insertAndSetId(query, attributes);

        /* If the table isn't incrementing we'll simply insert these attributes as they
           are. These attribute vectors must contain an "id" column previously placed
           there by the developer as the manually determined key for these models. */
        else
            if (attributes.isEmpty())
                return true;
            else
                query.insert(attributes);

        /* We will go ahead and set the exists property to true, so that it is set when
           the created event is fired, just in case the developer tries to update it
           during the event. This will allow them to do so and run an update here. */
        this->exists = true;

//        fireModelEvent("created", false);

        return true;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    bool Model<Derived, AllRelations...>::performUpdate(TinyBuilder<Derived> &query)
    {
        /* If the updating event returns false, we will cancel the update operation so
           developers can hook Validation systems into their models and cancel this
           operation if the model does not pass validation. Otherwise, we update. */
//        if (!fireModelEvent("updating"))
//            return false;

        /* First we need to create a fresh query instance and touch the creation and
           update timestamp on the model which are maintained by us for developer
           convenience. Then we will just continue saving the model instances. */
        if (this->usesTimestamps())
            this->updateTimestamps();

        /* Once we have run the update operation, we will fire the "updated" event for
           this model instance. This will allow developers to hook into these after
           models are updated, giving them a chance to do any special processing. */
        const auto dirty = this->getDirty();

        if (!dirty.isEmpty()) {
            model().setKeysForSaveQuery(query).update(
                        AttributeUtils::convertVectorToUpdateItem(dirty));

            this->syncChanges();

//            fireModelEvent("updated", false);
        }

        return true;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    void Model<Derived, AllRelations...>::finishSave(const SaveOptions options)
    {
//        fireModelEvent('saved', false);

        if (this->isDirty() && options.touch)
            this->touchOwners();

        this->syncOriginal();
    }

    // FEATURE dilemma primarykey, add support for Derived::KeyType silverqx
    template<typename Derived, AllRelationsConcept ...AllRelations>
    quint64 Model<Derived, AllRelations...>::insertAndSetId(
            const TinyBuilder<Derived> &query,
            const QVector<AttributeItem> &attributes)
    {
        const auto &keyName = getKeyName();

        const auto id = query.insertGetId(attributes, keyName);

        // NOTE api different, Eloquent doesn't check like below and returns void instead silverqx
        // When insert was successful
        if (id != 0)
            this->setAttribute(keyName, id);

        /* QSqlQuery returns an invalid QVariant if can't obtain last inserted id,
           which is converted to 0. */
        return id;
    }

    /* private */

    /* HasAttributes */

    template<typename Derived, AllRelationsConcept ...AllRelations>
    void Model<Derived, AllRelations...>::throwIfCRTPctorProblem(
            const QVector<AttributeItem> &attributes)
    {
        throwIfQDateTimeAttribute(attributes);
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    void Model<Derived, AllRelations...>::throwIfQDateTimeAttribute(
            const QVector<AttributeItem> &attributes)
    {
        static const auto message = QStringLiteral(
            "Attributes passed to the '%1' model's constructor can't contain the "
            "QDateTime attribute, to create a '%1' model instance with attributes that "
            "contain the QDateTime attribute use the %1::instance() method instead "
            "or convert the '%2' QDateTime attribute to QString.");

        for (const auto &[key, value] : attributes)
            if (value.isValid() && !value.isNull() &&
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                value.typeId() == QMetaType::QDateTime
#else
                value.userType() == QMetaType::QDateTime
#endif
            )
                throw Orm::Exceptions::InvalidArgumentError(
                        message.arg(Orm::Utils::Type::classPureBasename<Derived>(), key));
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    void Model<Derived, AllRelations...>::throwIfTotallyGuarded(QString &&key)
    {
        throw Exceptions::MassAssignmentError(
                    QStringLiteral("Add '%1' to u_fillable data member to allow "
                                   "mass assignment on the '%2' model.")
                    .arg(std::move(key),
                         Orm::Utils::Type::classPureBasename<Derived>()));
    }

    /* Getters for u_ data members defined in the Derived models, helps to avoid
       'friend GuardsAttributes/HasTimestamps' declarations in models when a u_ data
       members are private/protected. */

    /* HasAttributes */

    template<typename Derived, AllRelationsConcept ...AllRelations>
    QString &
    Model<Derived, AllRelations...>::getUserDateFormat()
    {
        return Derived::u_dateFormat;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    const QString &
    Model<Derived, AllRelations...>::getUserDateFormat() const
    {
        return Derived::u_dateFormat;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    const QStringList &
    Model<Derived, AllRelations...>::getUserDates()
    {
        return Derived::u_dates;
    }

    /* GuardsAttributes */

    template<typename Derived, AllRelationsConcept ...AllRelations>
    QStringList &
    Model<Derived, AllRelations...>::getUserFillable()
    {
        return Derived::u_fillable;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    const QStringList &
    Model<Derived, AllRelations...>::getUserFillable() const
    {
        return Derived::u_fillable;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    QStringList &
    Model<Derived, AllRelations...>::getUserGuarded()
    {
        return Derived::u_guarded;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    const QStringList &
    Model<Derived, AllRelations...>::getUserGuarded() const
    {
        return Derived::u_guarded;
    }

    /* HasRelationships */

    template<typename Derived, AllRelationsConcept ...AllRelations>
    const QHash<QString, typename Model<Derived, AllRelations...>::RelationVisitorAlias> &
    Model<Derived, AllRelations...>::getUserRelations() const
    {
        return model().u_relations;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    const QStringList &
    Model<Derived, AllRelations...>::getUserTouches() const
    {
        return model().u_touches;
    }

    /* HasTimestamps */

    template<typename Derived, AllRelationsConcept ...AllRelations>
    bool &
    Model<Derived, AllRelations...>::getUserTimestamps()
    {
        return model().u_timestamps;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    bool Model<Derived, AllRelations...>::getUserTimestamps() const
    {
        return model().u_timestamps;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    const QString &
    Model<Derived, AllRelations...>::getUserCreatedAtColumn()
    {
        return Derived::CREATED_AT;
    }

    template<typename Derived, AllRelationsConcept ...AllRelations>
    const QString &
    Model<Derived, AllRelations...>::getUserUpdatedAtColumn()
    {
        return Derived::UPDATED_AT;
    }

} // namespace Orm::Tiny

TINYORM_END_COMMON_NAMESPACE

#endif // ORM_TINY_MODEL_HPP

// TODO release, recompile without PCH silverqx
// TODO release, open cmake generated Visual Studio 2019 solution and fix all errors/warnings silverqx
// TODO release, clang's UndefinedBehaviorSanitizer at https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html silverqx
// TODO repeat, merge changes from latest Eloquent silverqx
// TODO decide/unify when to use class/typename keywords for templates silverqx
// FEATURE EloquentCollection, solve how to implement, also look at Eloquent's Custom Collections silverqx
// FUTURE try to compile every header file by itself and catch up missing dependencies and forward declaration, every header file should be compilable by itself silverqx
// FUTURE include every stl dependency in header files silverqx
// FEATURE logging, add support for custom logging, logging to the defined stream?, I don't exactly know how I will solve this issue, design it 🤔 silverqx
// FEATURE code coverage silverqx
// TODO dilemma, function params. like direction asc/desc for orderBy, operators for where are QStrings, but they should be flags for performance reasons, how to solve this and preserve nice clean api? that is the question 🤔 silverqx
// CUR1 connection SSL support silverqx
// BUG Qt sql drivers do not work with mysql json columns silverqx
// CUR1 add Relations::Concerns::ComparesRelatedModels silverqx
// FEATURE build systems, add docs on how to make a production build of the TinyORM library silverqx
// FEATURE build systems, add docs on how to set up dev. environment and how to run auto tests silverqx
// FEATURE build systems, libuv example how it could look like https://github.com/libuv/libuv silverqx
// FUTURE cmake can generate export header file by GenerateExportHeader module, find way to use it, because I have own export header file, how to unify this? I don't know now silverqx
// CUR try clang-include-fixer at https://clang.llvm.org/extra/clang-include-fixer.html silverqx
// CUR try iwyu at https://include-what-you-use.org/ silverqx
// CUR generate pkg-config file on unix silverqx
// CUR cmake use gold linker option, https://github.com/frobware/c-hacks/blob/master/cmake/use-gold-linker.cmake silverqx
// BUG mingw64, TinyOrmPlayground seg. fault at the end, but everything runs/passes correctly, but only when invoked from mintty terminal, when I run it from QtCreator that uses cmd I don't see any seg. fault silverqx
// BUG mingw64, seg fault in some tests eg. tst_model, and couldn't execute tests again, mingw64 shell works silverqx
// FUTURE mingw64, find out better solution for .text section exhausted in debug build, -Wa,-mbig-obj didn't help, -flto helps, but again it can not find reference to WinMain, so I had to compile with -O1, then it is ok silverqx
// FUTURE tests, QtCreator Qt AutoTests how to pass -maxwarnings silverqx
// FUTURE constexpr, version header file, good example https://googleapis.dev/cpp/google-cloud-common/0.6.1/version_8h_source.html silverqx
// BUG qmake impossible to add d postfix for dlls silverqx
// BUG SemVer version cmake/qmake silverqx
// CUR cmake/qmake SOVERSION silverqx
// CUR 0.1.0 vs 0.1.0.0 Product/FileVersion, investigate, also check versions in pc, prl, ... silverqx
// CUR enable QT_ASCII_CAST_WARNINGS silverqx
// CUR enable QT_NO_CAST_FROM_ASCII silverqx
// BUG qmake MinGW UCRT64 clang static build duplicit symbols, this is MinGW bug silverqx
// BUG qmake MinGW UCRT64 clang shared build with inline_constants cause crashes of 50% of tests, this will be MinGW clang or clang bug, on unix it works without problems silverqx
// BUG cmake MinGW UCRT64 clang static build builds, but cause problem with inline_constants ; shared build with inline_constants cause crashes of 50% of tests, like bug above, this will be MinGW clang or clang bug, on unix it works without problems silverqx
// FUTURE linux, add linker version script https://github.com/sailfishos/qtbase/commit/72ba0079c3967bdfa26acdce78ce6cb98b30c27b?view=parallel https://www.gnu.org/software/gnulib/manual/html_node/Exported-Symbols-of-Shared-Libraries.html https://stackoverflow.com/questions/41061220/where-do-object-file-version-references-come-from silverqx
// TODO Visual Studio memory analyzer https://docs.microsoft.com/en-us/visualstudio/profiling/memory-usage-without-debugging2?view=vs-2019 silverqx
// CUR fix all modernize-pass-by-value silverqx
// CUR use using inside classes where appropriate silverqx
// CUR I'm still not comfortable with using Orm::Constants, inside/outside namespace in cpp/hpp files silverqx
// CUR using namespace Orm::Constants; in ormtypes.hpp silverqx
// CUR make all * to *const silverqx
// CUR merge changes from latest Eloquent silverqx
// CUR cmake/qmake run tests in parallel for tests that allow it silverqx
// CUR study how to use acquire/release memory order for m_queryLogId atomic silverqx
// FUTURE divide Query Builder and TinyOrm to own packages (dlls)? think about it 🤔 silverqx
// BUG clang on mingw inline static initialization with another static in the same class defined line before, all other compilers (on linux too) works silverqx
// CUR unify default/non-default ctors comment silverqx
// CUR cmake when MYSQL_PING is on and QMYSQL driver is not build ctest fails, fail configure? I don't knwo how I will solve this for now, also fix qmake silverqx
// BUG docs many-to-many examples contain queries with created_at in pivot table, but pivot doesn't contain timestamp columns silverqx
// CUR docs mdx syntax highlight prism Treeview https://prismjs.com/plugins/treeview/ silverqx
// CUR docs IdealImage silverqx
// CUR docs, add all the implemented features summary somewhere silverqx
// CUR export dll paths in Invoke- ps1 scripts, also check dotenv and source if needed and add dotenv example to tools/ silverqx
// TODO cache static_cast<>(*this) in the model()/basemodel() CRTP as a class data member std::optional<std::reference_wrapper<Derived>> m_model = std::nullopt, but I will have to create copy ctor to set m_model {std::nullopt}, the same for other similar model() methods like Model::model(), then I can to check if (m_model) and return right away and I will call static_cast or dynamic_cast only once for every instance, it is casted everytime now 😟 silverqx
// CUR rename USING_PCH to TINY_USING_PCH or TINYORM_USING_PCH/TINYUTILS_USING_PCH? silverqx
// CUR docs exceptions, which methods throw which exceptions, so user know which exception classes to define in a catch blocks silverqx
// CUR Belongs To Many firstOrNew, firstOrCreate, and updateOrCreate Methods, revisit first and second argument silverqx
// CUR add attribute to touch() method silverqx
// SEC fix LoadLibrary() before qsql plugin loads? https://www.qt.io/blog/security-advisory-qlockfile-qauthenticator-windows-platform-plugin silverqx
// CUR1 connection, control disable/enable logging at runtime silverqx
// CUR cmake, make TinyUtils_target in TinyTestCommon.cmake optional, not always needed to link to silverqx
// CUR docs, info about unsupported json column by qtsql drivers and link to schema.mdx Default Expressions silverqx
// CUR model, add whereBelongsTo, whereRelation, orWhereRelation silverqx
// CUR schema, add tests for enum and set; and json and jsonb, storedAs / virtualAs silverqx
// CUR propagation, https://ben.balter.com/2017/11/10/twelve-tips-for-growing-communities-around-your-open-source-project/ silverqx
// CUR optimization, use Q_UNREACHABLE in all switch statements, of course where appropriate silverqx
// TODO vcpkg, solve how to build tom (when solving vcpkg builds again), currently I have hardly added tabulate to the vcpkg.json port and also manifest file; it will have to be conditional base of the TINYORM_DISABLE_TOM macro silverqx
// CUR tom docs, disable_tom and TINYORM_DISABLE_TOM to build.mdx, don't forget to add features and update dependencies (tabulate) in vcpkg.json silverqx
// CUR schema, add tests for enum and set; and json and jsonb, storedAs / virtualAs silverqx
// CUR compiler, enable /sdl on msvc https://docs.microsoft.com/en-us/cpp/build/reference/sdl-enable-additional-security-checks?view=msvc-170 silverqx
// CUR cmake, update max. policy to 3.23 silverqx
// CUR tom, tom/conf.pri is used by who silverqx
// CUR cmake, add messages about Building tom example, tests and ORM silverqx
// CUR docs, remove target_link_libs() for transitive dependencies silverqx
// CUR tom docs, write documentation silverqx
// CUR tom, build on mingw, linux, build without pch and all common tasks that should run from time to time silverqx
// CUR tom, don't modify migrate:status command, rather extend it and add possibility to only call it through Application::runWithArguments() (this secure to not be able to call it from the cmd. line), do not show it in the list or help command output silverqx
// CUR tom, think about remove TINYTOM_NO/_DEBUG and TINYTOM_TESTS_CODE and use TINYORM_ defines instead silverqx
// TODO github actions, update llvm analyzer tools to llvm 14 (clang-tidy/clazy), also qa-lint-tinyorm-qtX.ps1 tools silverqx
// CUR check all unique_ptr() of base clases, if base classes have virtual dtors, or find unique_ptrs of aggregates to base classes silverqx
// CUR use EMPTY constant everywhere silverqx
// CUR tom, squashing migrations silverqx
// CUR tom, create icon silverqx
