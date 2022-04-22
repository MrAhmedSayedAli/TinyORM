#include "tom/commands/command.hpp"

#include <QCommandLineParser>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/zip_with.hpp>

#include <orm/databasemanager.hpp>

#include "tom/application.hpp"
#include "tom/version.hpp"

TINYORM_BEGIN_COMMON_NAMESPACE

namespace Tom::Commands
{

/* public */

Command::Command(Application &application, QCommandLineParser &parser)
    : Concerns::InteractsWithIO(parser)
    , m_application(application)
    , m_parser(parser)
{}

QList<QCommandLineOption> Command::optionsSignature() const
{
    return {};
}

int Command::run()
{
    initializePositionalArguments();

    auto &parser = this->parser();

    parser.clearPositionalArguments();

    parser.addOptions(optionsSignature());

    if (!parser.parse(passedArguments()))
        showParserError(parser);

    // Show help if --help argument was passed
    checkHelpArgument();

    return EXIT_SUCCESS;
}

int Command::runWithArguments(QStringList &&arguments)
{
    m_arguments = std::move(arguments);

    return run();
}

/* Getters */

bool Command::hasPositionalArguments() const
{
    return !positionalArguments().empty();
}

bool Command::hasOptions() const
{
    return !optionsSignature().isEmpty();
}

/* protected */

/* Getters */

QStringList Command::passedArguments() const
{
    if (!m_arguments.isEmpty())
        return m_arguments;

    // Never obtain arguments from the QCoreApplication instance in tests
    return application().hasQtApplication ? QCoreApplication::arguments()
                                          : application().prepareArguments();
}

/* Parser helpers */

bool Command::isSet(const QString &name) const
{
    return parser().isSet(name);
}

QString Command::value(const QString &name) const
{
    return parser().value(name);
}

QString Command::valueCmd(const QString &name, const QString &key) const
{
    if (auto value = parser().value(name);
        !value.isEmpty()
    )
        return QStringLiteral("--%1=%2").arg(key.isEmpty() ? name : key,
                                             std::move(value));

    return {};
}

QString Command::boolCmd(const QString &name, const QString &key) const
{
    if (!parser().isSet(name))
        return {};

    return QStringLiteral("--%1").arg(key.isEmpty() ? name : key);
}

bool Command::hasArgument(const ArgumentsSizeType index) const
{
    /* Has to be isNull(), an argument passed on the command line still can be an empty
       value, like "", in this case it has to return a true value. */
    return !argument(index).isNull();
}

QStringList Command::arguments() const
{
    return parser().positionalArguments();
}

QString Command::argument(const ArgumentsSizeType index) const
{
    const auto &positionalArgumentsRef = positionalArguments();


    using ArgumentsStdSizeType =
            std::remove_cvref_t<decltype (positionalArgumentsRef)>::size_type;

    // Default value supported
    return parser().positionalArguments()
            .value(index,
                   positionalArgumentsRef.at(
                       static_cast<ArgumentsStdSizeType>(index) - 1).defaultValue);
}

QString Command::argument(const QString &name) const
{
    // Default value supported
    return argument(m_positionalArguments.at(name));
}

Orm::DatabaseConnection &Command::connection(const QString &name) const
{
    return application().db().connection(name);
}

QCommandLineParser &Command::parser() const noexcept
{
    return m_parser;
}

/* private */

void Command::initializePositionalArguments()
{
    const auto &arguments = positionalArguments();

    if (arguments.empty())
        return;

    m_positionalArguments =
            ranges::views::zip_with([](const auto &argument, auto &&index)
                                    -> std::pair<QString, ArgumentsSizeType>
    {
        return {argument.name, index};
    },
        arguments,
        ranges::views::closed_iota(static_cast<ArgumentsSizeType>(1),
                                   static_cast<ArgumentsSizeType>(arguments.size()))
    )
            | ranges::to<decltype (m_positionalArguments)>();

    // The same as above, I leave above as I want to have one example with zip_with()
//    for (OptionsSizeType index = 0; const auto &argument : positionalArguments())
//        m_positionalArguments.emplace(argument.name, ++index);
}

void Command::checkHelpArgument() const
{
    if (!isSet("help"))
        return;

    call("help", {name()});

    application().exitApplication(EXIT_SUCCESS);
}

void Command::showParserError(const QCommandLineParser &parser) const
{
    errorWall(parser.errorText());

    application().exitApplication(EXIT_FAILURE);
}

} // namespace Tom::Commands

TINYORM_END_COMMON_NAMESPACE