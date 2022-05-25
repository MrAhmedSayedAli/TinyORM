#include "tom/commands/migrations/migratecommand.hpp"

#include <QCommandLineParser>

#include <orm/constants.hpp>

#include "tom/migrator.hpp"

using Orm::Constants::database_;

using Tom::Constants::database_up;
using Tom::Constants::force;
using Tom::Constants::pretend;
using Tom::Constants::seed;
using Tom::Constants::step_;
using Tom::Constants::DbSeed;
using Tom::Constants::MigrateInstall;

TINYORM_BEGIN_COMMON_NAMESPACE

namespace Tom::Commands::Migrations
{

/* public */

MigrateCommand::MigrateCommand(
        Application &application, QCommandLineParser &parser,
        std::shared_ptr<Migrator> migrator
)
    : Command(application, parser)
    , Concerns::Confirmable(*this, 0)
    , Concerns::UsingConnection(resolver())
    , m_migrator(std::move(migrator))
{}

QList<QCommandLineOption> MigrateCommand::optionsSignature() const
{
    return {
        {database_,     QStringLiteral("The database connection to use"), database_up}, // Value
        {force,         QStringLiteral("Force the operation to run when in production")},
        {pretend,       QStringLiteral("Dump the SQL queries that would be run")},
//        {"schema-path", QStringLiteral("The path to a schema dump file")}, // Value
        {seed,          QStringLiteral("Indicates if the seed task should be re-run")},
        {step_,         QStringLiteral("Force the migrations to be run so they can be "
                                       "rolled back individually")},
    };
}

int MigrateCommand::run()
{
    Command::run();

    // Ask for confirmation in the production environment
    if (!confirmToProceed())
        return EXIT_FAILURE;

    // Database connection to use
    return usingConnection(value(database_), isDebugVerbosity(), m_migrator->repository(),
                           [this]
    {
        // Install db repository and load schema state
        prepareDatabase();

        /* Next, we will check to see if a path option has been defined. If it has
           we will use the path relative to the root of this installation folder
           so that migrations may be run for any path within the applications. */
        m_migrator->run({isSet(pretend), isSet(step_)});

        info("Database migaration completed successfully.");

        /* Finally, if the "seed" option has been given, we will re-run the database
           seed task to re-populate the database, which is convenient when adding
           a migration and a seed at the same time, as it is only this command. */
        if (needsSeeding())
            runSeeder();

        return EXIT_SUCCESS;
    });
}

/* protected */

void MigrateCommand::prepareDatabase() const
{
    if (!m_migrator->repositoryExists())
        call(MigrateInstall, {value(database_)});

    if (!m_migrator->hasRunAnyMigrations() && !isSet(pretend))
        loadSchemaState();
}

void MigrateCommand::loadSchemaState() const
{
    // CUR tom, finish load schema silverqx
}

// CUR tom, remove silverqx
bool MigrateCommand::needsSeeding() const
{
    return !isSet(pretend) && isSet(seed);
}

void MigrateCommand::runSeeder() const
{
    call(DbSeed, {valueCmd(database_), longOption(force)});
}

} // namespace Tom::Commands::Migrations

TINYORM_END_COMMON_NAMESPACE
