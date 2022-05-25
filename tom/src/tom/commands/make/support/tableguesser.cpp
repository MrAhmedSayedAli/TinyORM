#include "tom/commands/make/support/tableguesser.hpp"

#include <QRegularExpression>

TINYORM_BEGIN_COMMON_NAMESPACE

namespace Tom::Commands::Make::Support
{

/* private */

const QString
TableGuesser::CreatePatterns =
        QStringLiteral(
            R"(^create(?:_|-)(\w+?)(?:(?:_|-)table)?$)");
const QString
TableGuesser::ChangePatterns =
        QStringLiteral(
            R"((?:_|-)(?:to|from|in)(?:_|-)(\w+?)(?:(?:_|-)table)?$)");

/* public */

std::tuple<QString, bool> TableGuesser::guess(const QString &migration)
{
    {
        static const QRegularExpression regex(CreatePatterns);

        const auto match = regex.match(migration);

        if (match.hasMatch()) {
            Q_ASSERT(regex.captureCount() == 1);

            return {match.captured(1), true};
        }
    }

    {
        static const QRegularExpression regex(ChangePatterns);

        const auto match = regex.match(migration);

        if (match.hasMatch()) {
            Q_ASSERT(regex.captureCount() == 1);

            return {match.captured(1), false};
        }
    }

    return {};
}

} // namespace Tom::Commands::Make::Support

TINYORM_END_COMMON_NAMESPACE
