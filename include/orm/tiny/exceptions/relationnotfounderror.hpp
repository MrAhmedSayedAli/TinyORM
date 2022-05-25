#pragma once
#ifndef ORM_TINY_EXCEPTIONS_RELATIONNOTFOUNDERROR_HPP
#define ORM_TINY_EXCEPTIONS_RELATIONNOTFOUNDERROR_HPP

#include "orm/macros/systemheader.hpp"
TINY_SYSTEM_HEADER

#include "orm/exceptions/runtimeerror.hpp"

TINYORM_BEGIN_COMMON_NAMESPACE

namespace Orm::Tiny::Exceptions
{

    /*! Relation not found exception, check whether the relation was defined
        in the u_relation data member, called from Model::validateUserRelation(). */
    class SHAREDLIB_EXPORT RelationNotFoundError : public Orm::Exceptions::RuntimeError
    {
    public:
        /*! Exception message will be generated on the base of this enum struct. */
        enum struct From
        {
            UNDEFINED       = -1,
            BELONGS_TO      = 0,
            BELONGS_TO_MANY = 1,
        };

        /*! Constructor. */
        RelationNotFoundError(const QString &model, const QString &relation,
                              From from = From::UNDEFINED);

        /*! Get the affected TinyORM model. */
        inline const QString &getModel() const;
        /*! Get the name of the relation. */
        inline const QString &getRelation() const;

    protected:
        /*! The name of the affected TinyORM model.. */
        QString m_model;
        /*! The name of the relation. */
        QString m_relation;
        /*! Exception message will be generated on the base of this data member. */
        From m_from;

    private:
        /*! Format the error message. */
        QString formatMessage(const QString &model, const QString &relation,
                              From from) const;
    };

    const QString &
    RelationNotFoundError::getModel() const
    {
        return m_model;
    }

    const QString &
    RelationNotFoundError::getRelation() const
    {
        return m_relation;
    }

} // namespace Orm::Tiny::Exceptions

TINYORM_END_COMMON_NAMESPACE

#endif // ORM_TINY_EXCEPTIONS_RELATIONNOTFOUNDERROR_HPP
