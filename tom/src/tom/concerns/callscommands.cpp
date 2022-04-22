#include "tom/concerns/callscommands.hpp"

#include <unordered_set>

#include <range/v3/action/remove_if.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/move.hpp>

#include "tom/application.hpp"
#include "tom/commands/command.hpp"

TINYORM_BEGIN_COMMON_NAMESPACE

namespace Tom::Concerns
{

/* protected */

int CallsCommands::runCommand(const QString &command, QStringList &&arguments) const
{
    QCommandLineParser parser;

    auto &application = this->command().application();

    application.initializeParser(parser);

    /* Parse needed also here because InteractiveIO ctor calls isSet(). Also no error
       handling needed here, it will be handled in the Command::run(), this is only
       pre-parse because of isSet() in the InteractiveIO ctor. */
    auto currentArguments = application.arguments();
    parser.parse(currentArguments);

    return application.createCommand(command, parser)
            ->runWithArguments(
                createCommandLineArguments(command, std::move(arguments),
                                           std::move(currentArguments)));
}

QStringList
CallsCommands::createCommandLineArguments(
            const QString &command, QStringList &&arguments,
            QStringList &&currentArguments) const
{
    // Must contain a command exe name and tom's command name
    Q_ASSERT(currentArguments.size() >= 2);

    /* First create a new arguments list that starts with an executable name followed
       by a command name to execute, then obtain common arguments which were passed
       on the current command line, and as the last thing append passed arguments. */

    // Absolute path of the exe name
    QStringList newArguments {std::move(currentArguments[0])};
    // Command name
    newArguments << std::move(command);

    // Remove a command exe name and tom's command name
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    currentArguments.remove(0, 2);
#else
    currentArguments.removeFirst();
    currentArguments.removeFirst();
#endif

    // Get common command-line arguments from the current command-line arguments
    newArguments << getCommonArguments(std::move(currentArguments));

    // Append passed arguments
    std::ranges::move(ranges::actions::remove_if(std::move(arguments),
                                                 [](auto &&v) { return v.isEmpty(); }),
                      std::back_inserter(newArguments));

    return newArguments;
}

QStringList CallsCommands::getCommonArguments(QStringList &&arguments) const
{
    static const std::unordered_set<QString> allowed {
        "--ansi",
        "--no-ansi",
        "--no-interaction", "-n",
        "--quiet",          "-q",
        "--verbose",        "-v", "-vv", "-vvv",
    };

    return ranges::views::move(arguments)
            | ranges::views::filter([&allowed = allowed](auto &&argument)
    {
        return allowed.contains(argument);
    })
            | ranges::to<QStringList>();
}

/* private */

const Commands::Command &CallsCommands::command() const
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    return static_cast<const Commands::Command &>(*this);
}

} // namespace Tom::Concerns

TINYORM_END_COMMON_NAMESPACE