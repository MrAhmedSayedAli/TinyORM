#include "tom/concerns/printsoptions.hpp"

#include <orm/constants.hpp>
#include <orm/tiny/utils/string.hpp>

#include "tom/application.hpp"
#include "tom/commands/command.hpp"
#include "tom/concerns/interactswithio.hpp"
#include "tom/exceptions/runtimeerror.hpp"
#include "tom/tomconstants.hpp"

using Orm::Constants::COMMA;
using Orm::Constants::SPACE;

using StringUtils = Orm::Tiny::Utils::String;

using Tom::Constants::ShortOption;
using Tom::Constants::LongOption;
using Tom::Constants::LongOptionOnly;
using Tom::Constants::LongOptionValue;

TINYORM_BEGIN_COMMON_NAMESPACE

namespace Tom::Concerns
{

/* public */

PrintsOptions::PrintsOptions(const QList<QCommandLineOption> &options,
                             const Concerns::InteractsWithIO &io)
    : m_options(options)
    , m_io(io)
{}

int PrintsOptions::printOptionsSection(const bool commonOptions) const
{
    io().newLine();
    io().comment(commonOptions ? QStringLiteral("Common options:")
                               : QStringLiteral("Options:"));

    // Get max. option size in all options
    int optionsMaxSize = this->optionsMaxSize();

    // Print options to the console
    printOptions(optionsMaxSize);

    return optionsMaxSize;
}

/* protected */

QString PrintsOptions::defaultValueText(const QString &value)
{
    // Quote the string type
    auto defaultValue = StringUtils::isNumber(value, true)
                        ? value
                        : QStringLiteral("\"%1\"").arg(value);

    return QStringLiteral(" [default: %1]").arg(std::move(defaultValue));
}

/* private */

QStringList PrintsOptions::createOptionNamesList(const QCommandLineOption &option)
{
    QStringList options;

    for (auto names = option.names();
         auto &&name : names
    )
        // Short option
        if (name.size() == 1)
            options << ShortOption
                       // Custom logic for the verbose option, good enough 😎
                       .arg(name == QChar('v') ? QStringLiteral("v|vv|vvv")
                                               : std::move(name));

        // Long option
        else
            // Short and long options passed
            if (auto valueName = option.valueName();
                names.size() == 2
            ) {
                // W/o the value
                if (valueName.isEmpty())
                    options << LongOption.arg(std::move(name));
                // With the value
                else
                    options << LongOptionValue.arg(std::move(name),
                                                   std::move(valueName));
            }
            // Only a long option passed
            else {
                // W/o a value
                if (valueName.isEmpty())
                    options << LongOptionOnly.arg(LongOption.arg(std::move(name)));
                // With a value
                else
                    options << LongOptionOnly.arg(
                                   LongOptionValue.arg(std::move(name),
                                                       std::move(valueName)));
            }

    return options;
}

int PrintsOptions::optionsMaxSize() const
{
    int optionsMaxSize = 0;

    for (const auto &option : options()) {

        validateOption(option);

        optionsMaxSize = std::max<int>(optionsMaxSize,
                                       static_cast<int>(createOptionNamesList(option)
                                                        .join(COMMA).size()));
    }

    return optionsMaxSize;
}

void PrintsOptions::printOptions(const int optionsMaxSize) const
{
    for (const auto &option : options()) {

        auto joinedOptions = createOptionNamesList(option).join(COMMA);

        auto indent = QString(optionsMaxSize - joinedOptions.size(), SPACE);

        io().info(QStringLiteral("  %1%2  ").arg(std::move(joinedOptions),
                                                 std::move(indent)),
                  false);

        io().note(option.description(), false);

        // Print option's default value
        printOptionDefaultValue(option);
    }
}

void PrintsOptions::printOptionDefaultValue(const QCommandLineOption &option) const
{
    const auto defaultValues = option.defaultValues();

    // More default values is not supported
    Q_ASSERT(defaultValues.size() <= 1);

    if (defaultValues.isEmpty())
        io().newLine();
    else
        io().comment(defaultValueText(defaultValues[0]));
}

void PrintsOptions::validateOption(const QCommandLineOption &option) const
{
    const auto optionNames = option.names();
    const auto optionsSize = optionNames.size();

    // This is may be too much, but whatever after 14 hours of coding 😎
    if (optionsSize > 2)
        throw Exceptions::RuntimeError(
                QStringLiteral("Only two option names per option are allowed "
                               "(options: %1).")
                .arg(optionNames.join(COMMA)));

    static const auto longOptionTmpl =
            QStringLiteral("Long option size has to be >1 (option: %1).");

    // One option name was passed
    if (optionsSize == 1) {
        if (const auto &longOption = optionNames[0]; longOption.size() <= 1)
            throw Exceptions::RuntimeError(longOptionTmpl.arg(longOption));
    }
    // Two option names were passed
    else if (optionsSize == 2) {
        if (const auto &shortOption = optionNames[0]; shortOption.size() != 1)
            throw Exceptions::RuntimeError(
                    QStringLiteral("Short option size has to be 1 (option: %1).")
                    .arg(shortOption));

        if (const auto &longOption = optionNames[1]; longOption.size() <= 1)
            throw Exceptions::RuntimeError(longOptionTmpl.arg(longOption));
    }
}

} // namespace Tom::Concerns

TINYORM_END_COMMON_NAMESPACE
