#pragma once
#ifndef ORM_UTILS_TYPE_HPP
#define ORM_UTILS_TYPE_HPP

#include "orm/macros/systemheader.hpp"
TINY_SYSTEM_HEADER

#include <QString>

#include <typeindex>
#include <typeinfo>

#ifdef __GNUG__
#  include <cxxabi.h>
#endif

#include "orm/macros/commonnamespace.hpp"
#include "orm/macros/export.hpp"

#ifdef __GNUG__
// NOLINTNEXTLINE(bugprone-reserved-identifier)
#  define __tiny_func__ \
    Orm::Utils::Type::prettyFunction(static_cast<const char *>(__PRETTY_FUNCTION__))
#elif _MSC_VER
// NOLINTNEXTLINE(bugprone-reserved-identifier)
#  define __tiny_func__ \
    Orm::Utils::Type::prettyFunction(static_cast<const char *>(__FUNCTION__))
#else
#  define __tiny_func__ QString {__FUNCTION__}
#endif

TINYORM_BEGIN_COMMON_NAMESPACE

namespace Orm::Utils
{

    /*! Library class for types. */
    class SHAREDLIB_EXPORT Type
    {
        Q_DISABLE_COPY(Type)

    public:
        /*! Deleted default constructor, this is a pure library class. */
        Type() = delete;
        /*! Deleted destructor. */
        ~Type() = delete;

        /*! Class name with or w/o a namespace and w/o template parameters. */
        template<typename T>
        inline static QString classPureBasename(bool withNamespace = false);
        /*! Class name with or w/o a namespace and w/o template parameters. */
        template<typename T>
        inline static QString
        classPureBasename(const T &type, bool withNamespace = false);
        /*! Class name with or w/o a namespace and w/o template parameters. */
        static QString
        classPureBasename(std::type_index typeIndex, bool withNamespace = false);

        /*! Return a pretty function name in the following format: Xyz::function. */
        static QString prettyFunction(const QString &function);

        /*! Determine whether a string is true bool value (false for "", "0", "false"). */
        static bool isTrue(const QString &value);

    private:
        /*! Class name with or w/o a namespace and w/o template parameters, common
            code. */
        static QString
        classPureBasenameInternal(const std::type_info &typeInfo, bool withNamespace);
        /*! Class name with or w/o a namespace and w/o template parameters, common
            code. */
        static QString
        classPureBasenameInternal(const char *typeName, bool withNamespace);
        /*! Class name with or w/o a namespace and w/o template parameters, msvc code. */
        static QString
        classPureBasenameMsvc(const QString &className, bool withNamespace);
        /*! Class name with or w/o a namespace and w/o template parameters, gcc code. */
        static QString
        classPureBasenameGcc(const QString &className, bool withNamespace);
    };

    template<typename T>
    QString Type::classPureBasename(const bool withNamespace)
    {
        return classPureBasenameInternal(typeid (T), withNamespace);
    }

    template<typename T>
    QString Type::classPureBasename(const T &type, const bool withNamespace)
    {
        /* If you want to obtain a name for the polymorphic type, take care to pass
           a glvalue as the 'type' argument, the 'this' pointer is a prvalue!
           Above is whole true, but doesn't make sense as if I passing this or better
           *this to this function then it will be implicitly converted to the reference
           for polymorphic types as the parameter is const &.
           Anyway for polymorphic types pass *this! */
        return classPureBasenameInternal(typeid (type), withNamespace);
    }

} // namespace Orm::Utils

TINYORM_END_COMMON_NAMESPACE

#endif // ORM_UTILS_TYPE_HPP
