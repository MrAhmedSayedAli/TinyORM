#include "tom/commands/helpcommand.hpp"

#include <QCommandLineOption>

#include <orm/constants.hpp>
#include <orm/tiny/utils/string.hpp>

#include "tom/application.hpp"

using Orm::Constants::SPACE;

using StringUtils = Orm::Tiny::Utils::String;

TINYORM_BEGIN_COMMON_NAMESPACE

namespace Tom::Commands
{

/* public */

HelpCommand::HelpCommand(Application &application, QCommandLineParser &parser)
    : Command(application, parser)
    , Concerns::PrintsOptions(*this, 0)
{}

const std::vector<PositionalArgument> &HelpCommand::positionalArguments() const
{
    static const std::vector<PositionalArgument> cached {
        {"command_name", "The command name", {}, true, "help"},
    };

    return cached;
}

QString HelpCommand::help() const
{
    return QLatin1String(
"  The <info>help</info> command displays help for a given command:\n\n"
"    <info>tom</info> help list\n\n"
"  To display the list of available commands, please use the <info>list</info> "
  "command.");
}

int HelpCommand::run()
{
    Command::run();

    const auto commandNameArg = argument("command_name");

    const auto command = createCommand(commandNameArg);
    const auto &arguments = command->positionalArguments();

    if (!validateRequiredArguments(arguments))
        return EXIT_FAILURE;

    printDescriptionSection(*command);
    printUsageSection(commandNameArg, *command, arguments);

    printArgumentsSection(arguments);
    printOptionsSection(*command);

    printHelpSection(*command);

    return EXIT_SUCCESS;
}

/* protected */

std::unique_ptr<Command> HelpCommand::createCommand(const QString &name) const
{
    auto command = application().createCommand(name, std::nullopt, false);

    if (command)
        return command;

    errorWall(QLatin1String("Command '%1' is not defined.").arg(name));

    application().exitApplication(EXIT_FAILURE);
}

bool HelpCommand::validateRequiredArguments(
        const std::vector<PositionalArgument> &arguments) const
{
    // Fail on required argument after optional argument
    for (std::vector<PositionalArgument>::size_type i = 1;
         i < arguments.size() ; ++i
    ) {
        const auto &left = arguments.at(i - 1);
        const auto &right = arguments.at(i);

        if (left.optional && !right.optional) {
            errorWall(QLatin1String("Cannot add a required argument '%1' after "
                                    "an optional one '%2'.")
                      .arg(right.name, left.name));

            return false;
        }
    }

    // Fail when required argument has a default value
    return std::ranges::none_of(arguments, [this](const auto &argument)
    {
        const auto requiredWithDefault = !argument.optional &&
                                         !argument.defaultValue.isEmpty();

        if (requiredWithDefault)
            errorWall(QLatin1String("The required argument '%1' has a default value "
                                    "'%2'.")
                      .arg(argument.name, argument.defaultValue));

        return requiredWithDefault;
    });
}

void HelpCommand::printDescriptionSection(const Command &command) const
{
    comment(QLatin1String("Description:"));

    note(QStringLiteral("  %1").arg(command.description()));
}

void HelpCommand::printUsageSection(
        const QString &commandNameArg, const Command &command,
        const std::vector<PositionalArgument> &arguments) const
{
    /* Everything after the option -- (double dash) is treated as positional arguments.
       [] means optional, <> means positional argument. If an argument is optional,
       all arguments after have to be optional. */

    newLine();

    comment(QLatin1String("Usage:"));

    QString usage(2, SPACE);
    usage += commandNameArg;

    if (command.hasOptions())
        usage += QLatin1String(" [options]");

    if (command.hasPositionalArguments()) {
        usage += QLatin1String(" [--]");

        auto optionalCounter = 0;

        for (const auto &argument : arguments) {
            auto syntax = argument.syntax.isEmpty() ? argument.name : argument.syntax;

            if (argument.optional) {
                usage += QStringLiteral(" [<%1>").arg(std::move(syntax));
                ++optionalCounter;
            }
            else
                usage += QStringLiteral(" <%1>").arg(std::move(syntax));
        }

        usage += QString(optionalCounter, QChar(']'));
    }

    note(usage);
}

void HelpCommand::printArgumentsSection(
        const std::vector<PositionalArgument> &arguments) const
{
    if (arguments.empty())
        return;

    newLine();

    comment(QLatin1String("Arguments:"));

    for (const auto argumentsMaxSize = this->argumentsMaxSize(arguments);
         const auto &argument : arguments
    ) {
        // Compute indent
        auto indent = QString(argumentsMaxSize - argument.name.size(), SPACE);

        info(QStringLiteral("  %1%2  ").arg(argument.name, std::move(indent)), false);

        note(argument.description, false);

        printArgumentDefaultValue(argument);
    }
}

int HelpCommand::argumentsMaxSize(const std::vector<PositionalArgument> &arguments) const
{
    const auto it = std::ranges::max_element(arguments, std::less {},
                                             [](const auto &argument)
    {
        return argument.name.size();
    });

    return static_cast<int>((*it).name.size());
}

void HelpCommand::printArgumentDefaultValue(const PositionalArgument &argument) const
{
    // Empty default value, don't render
    const auto &defaultValueRef = argument.defaultValue;

    if (defaultValueRef.isEmpty()) {
        newLine();

        return;
    }

    // Quote string type
    auto defaultValue = StringUtils::isNumber(defaultValueRef, true)
                        ? defaultValueRef
                        : QStringLiteral("\"%1\"").arg(defaultValueRef);

    comment(QStringLiteral(" [default: %1]").arg(std::move(defaultValue)));
}

int HelpCommand::printOptionsSection(const Command &command) const
{
    // Prepare the command parser to show command options
    parser().addOptions(application().prependOptions(command.optionsSignature()));

    return PrintsOptions::printOptionsSection(false);
}

void HelpCommand::printHelpSection(const Command &command) const
{
    const auto help = command.help();

    if (help.isEmpty())
        return;

    newLine();

    comment(QLatin1String("Help:"));

    note(help);
}

} // namespace Tom::Commands

TINYORM_END_COMMON_NAMESPACE