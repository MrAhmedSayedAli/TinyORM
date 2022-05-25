#include "orm/support/configurationoptionsparser.hpp"

#include "orm/connectors/connector.hpp"
#include "orm/constants.hpp"
#include "orm/exceptions/runtimeerror.hpp"

using Orm::Constants::EQ_C;
using Orm::Constants::options_;
using Orm::Constants::SEMICOLON;

TINYORM_BEGIN_COMMON_NAMESPACE

namespace Orm::Support
{

ConfigurationOptionsParser::ConfigurationOptionsParser(const Connectors::Connector &connector)
    : m_connector(connector)
{}

QString
ConfigurationOptionsParser::parseConfiguration(const QVariantHash &config) const
{
    // Get options from a user configuration
    const auto &configOptions = config.find(options_).value();

    // Validate options type in the connection configuration
    validateConfigOptions(configOptions);

    /* Prepare options for prepareConfigOptions() function, convert to QVariantHash
       if needed. */
    QVariantHash preparedConfigOptions = prepareConfigOptions(configOptions);

    // Parse config connection options, driver specific validation/modification
    m_connector.parseConfigOptions(preparedConfigOptions);

    // Merge TinyORM default connector options with user's provided connection options
    QVariantHash mergedOptions = mergeOptions(m_connector.getConnectorOptions(),
                                              preparedConfigOptions);

    // Return in the format expected by QSqlDatabase
    return joinOptions(mergedOptions);
}

void
ConfigurationOptionsParser::validateConfigOptions(const QVariant &options) const
{
    if (
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        (options.typeId() != QMetaType::QString)
#else
        (options.userType() != QMetaType::QString)
#endif
        && !options.canConvert<QVariantHash>()
    )
        throw Exceptions::RuntimeError(
                "Passed unsupported 'options' type in the connection configuration, "
                "it has to be the QString or QVariantHash type.");
}

QVariantHash
ConfigurationOptionsParser::prepareConfigOptions(const QVariant &options) const
{
    // Input is already validated, so I can be sure that options key is QVariantHash
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (options.typeId() != QMetaType::QString)
#else
    if (options.userType() != QMetaType::QString)
#endif
        return options.value<QVariantHash>();

    // Convert to the QVariantHash
    const auto list = options.value<QString>().split(SEMICOLON,
                                                     Qt::SkipEmptyParts);
    QVariantHash preparedOptions;

    for (const auto &value : list) {
        const auto option = value.split(EQ_C);

        preparedOptions.insert(option[0].trimmed(),
                               option[1].trimmed());
    }

    return preparedOptions;
}

QVariantHash
ConfigurationOptionsParser::mergeOptions(
        const QVariantHash &connectortOptions,
        const QVariantHash &preparedConfigOptions) const
{
    // Make a copy of prepared config options
    QVariantHash merged = preparedConfigOptions;

    /* Insert options from the default connector options hash, when prepared
       config options already doesn't contain it, so user can overwrite
       default connector options. */
    auto itDefault = connectortOptions.constBegin();
    while (itDefault != connectortOptions.constEnd()) {
        const auto &key = itDefault.key();
        const auto &value = itDefault.value();

        if (!merged.contains(key))
            merged.insert(key, value);

        ++itDefault;
    }

    return merged;
}

QString ConfigurationOptionsParser::joinOptions(const QVariantHash &options) const
{
    QStringList joined;

    auto itOption = options.constBegin();
    while (itOption != options.constEnd()) {
        const auto &key = itOption.key();
        const auto &value = itOption.value();

        joined << QStringLiteral("%1=%2").arg(key,
                                              value.value<QString>());
        ++itOption;
    }

    return joined.join(SEMICOLON);
}

} // namespace Orm::Support

TINYORM_END_COMMON_NAMESPACE
