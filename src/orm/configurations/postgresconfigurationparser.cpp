#include "orm/configurations/postgresconfigurationparser.hpp"

#include "orm/constants.hpp"
#include "orm/exceptions/invalidargumenterror.hpp"
#include "orm/utils/helpers.hpp"
#include "orm/utils/type.hpp"

TINYORM_BEGIN_COMMON_NAMESPACE

using Orm::Constants::NAME;
using Orm::Constants::dont_drop;
using Orm::Constants::schema_;
using Orm::Constants::search_path;
using Orm::Constants::spatial_ref_sys;
using Orm::Constants::sslmode_;
using Orm::Constants::sslcert;
using Orm::Constants::sslkey;
using Orm::Constants::sslrootcert;

using Orm::Utils::Helpers;

namespace Orm::Configurations
{

/* protected */

void PostgresConfigurationParser::parseDriverSpecificOptions() const
{
    if (!config().contains(dont_drop))
        // The spatial_ref_sys table is used by the PostGIS
        config().insert(dont_drop, QStringList {spatial_ref_sys});

    // Validations
    throwIfContainsSchemaOption();
    throwIfSearchPathHasWrongType();
    throwIfDontDropHasWrongType();
}

void PostgresConfigurationParser::parseDriverSpecificOptionsOption(
        QVariantHash &options) const
{
    // Copy all SSL-related options from the top-level config. to the 'options' hash
    addSslOptions(options);
}

/* private */

void PostgresConfigurationParser::addSslOptions(QVariantHash &options) const
{
    /* Copy all SSL-related connection options from the top-level configuration level
       to the 'options' hash. If the options hash already contains the same option, then
       it will be overwritten. */
    copyOptionsFromTopLevel(options, {sslmode_, sslcert, sslkey, sslrootcert});
}

void PostgresConfigurationParser::throwIfContainsSchemaOption() const
{
    if (!config().contains(schema_))
        return;

    throw Exceptions::InvalidArgumentError(
                QStringLiteral(
                    "The PostgreSQL '%1' configuration contains the 'schema' option, "
                    "please change the option name to the 'search_path', in %2().")
                .arg(config()[NAME].value<QString>(), __tiny_func__));
}

void PostgresConfigurationParser::throwIfSearchPathHasWrongType() const
{
    // Nothing to validate
    if (!config().contains(search_path))
        return;

    if (const auto type = Helpers::qVariantTypeId(config()[search_path]);
        type == QMetaType::QString || type == QMetaType::QStringList
    )
        return;

    throw Exceptions::InvalidArgumentError(
                QStringLiteral(
                    "The PostgreSQL 'search_path' configuration option must be "
                    "the QString or QStringList type in the '%1' connection "
                    "configuration, in %2().")
                .arg(config()[NAME].value<QString>(), __tiny_func__));
}

void PostgresConfigurationParser::throwIfDontDropHasWrongType() const
{
    // Nothing to validate
    if (!config().contains(dont_drop))
        return;

    if (const auto type = Helpers::qVariantTypeId(config()[dont_drop]);
        type == QMetaType::QString || type == QMetaType::QStringList
    )
        return;

    throw Exceptions::InvalidArgumentError(
                QStringLiteral(
                    "The PostgreSQL 'dont_drop' configuration option must be "
                    "the QString or QStringList type in the '%1' connection "
                    "configuration, in %2().")
                .arg(config()[NAME].value<QString>(), __tiny_func__));
}

} // namespace Orm::Configurations

TINYORM_END_COMMON_NAMESPACE
