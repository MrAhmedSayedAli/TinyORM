#include "tom/migrator.hpp"

#include <typeinfo>

#include <orm/databaseconnection.hpp>
#include <orm/schema/schemabuilder.hpp>
#include <orm/utils/query.hpp>
#include <orm/utils/type.hpp>

#include <range/v3/algorithm/contains.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/move.hpp>
#include <range/v3/view/remove_if.hpp>
#include <range/v3/view/reverse.hpp>

#include "tom/exceptions/invalidtemplateargumenterror.hpp"
#include "tom/migration.hpp"
#include "tom/migrationrepository.hpp"

using Orm::DatabaseConnection;

using Orm::Constants::DESC;

using QueryUtils = Orm::Utils::Query;
using TypeUtils = Orm::Utils::Type;

TINYORM_BEGIN_COMMON_NAMESPACE

namespace Tom
{

/* public */

Migrator::Migrator(
        std::shared_ptr<MigrationRepository> &&repository,
        std::shared_ptr<ConnectionResolverInterface> &&resolver,
        const std::vector<std::shared_ptr<Migration>> &migrations,
        const QCommandLineParser &parser
)
    : Concerns::InteractsWithIO(parser)
    , m_repository(std::move(repository))
    , m_resolver(std::move(resolver))
    , m_migrations(migrations)
{
    /* Initialize these containers in the constructor as every command that uses
       the Migrator also needs a migrations map or names list. */
    createMigrationNamesMap();
}

/* Main migrate operations */

std::vector<std::shared_ptr<Migration>> Migrator::run(const MigrateOptions options) const
{
    auto migrations = pendingMigrations(m_repository->getRanSimple());

    /* First we will just make sure that there are any migrations to run. If there
       aren't, we will just make a note of it to the developer so they're aware
       that all of the migrations have been run against this database system. */
    if (migrations.empty()) {
        info(QLatin1String("Nothing to migrate."));

        return migrations;
    }

    /* Next, we will get the next batch number for the migrations so we can insert
       correct batch number in the database migrations repository when we store
       each migration's execution. We will also extract a few of the options. */
    auto batch = m_repository->getNextBatchNumber();

    const auto &[pretend, step, _] = options;

    /* Once we have the vector of migrations, we will spin through them and run the
       migrations "up" so the changes are made to the databases. We'll then log
       that the migration was run so we don't repeat it next time we execute. */
    for (const auto &migration : migrations) {
        runUp(*migration, batch, pretend);

        if (step)
            ++batch;
    }

    return migrations;
}

std::vector<RollbackItem>
Migrator::rollback(const MigrateOptions options) const
{
    /* We want to pull in the last batch of migrations that ran on the previous
       migration operation. We'll then reverse those migrations and run each
       of them "down" to reverse the last migration "operation" which ran. */
    return rollbackMigrations(getMigrationsForRollback(options), options.pretend);
}

std::vector<RollbackItem> Migrator::reset(const bool pretend) const
{
    /* Reverse all the ran migrations list to reset this database. This will allow us
       to get the database back into its "empty" state, ready for the migrations. */
    return rollbackMigrations(getMigrationsForRollback(m_repository->getRan(DESC)),
                              pretend);
}

/* Database connection related */

int Migrator::usingConnection(QString &&name, const bool debugSql,
                              std::function<int()> &&callback)
{
    auto previousConnection = m_resolver->getDefaultConnection();
    /* Default connection can also be "" empty string, eg. auto tests are using empty
       string as the default connection. */
    auto previousDebugSql = getConnectionDebugSql(previousConnection);

    setConnection(std::move(name), debugSql);

    auto exitCode = std::invoke(std::move(callback));

    setConnection(std::move(previousConnection), std::move(previousDebugSql));

    return exitCode;
}

/* Proxies to MigrationRepository */

bool Migrator::repositoryExists() const
{
    return m_repository->repositoryExists();
}

bool Migrator::hasRunAnyMigrations() const
{
    return repositoryExists() && !m_repository->getRanSimple().isEmpty();
}

/* Getters / Setters */

void Migrator::setConnection(QString &&name, std::optional<bool> &&debugSql)
{
    // It indicates "" empty string for the default connection, eg. in auto tests
    if (!name.isEmpty())
        m_resolver->setDefaultConnection(name);

    m_repository->setConnection(name, std::move(debugSql));

    m_connection = std::move(name);
}

/* protected */

/* Database connection related */

DatabaseConnection &Migrator::resolveConnection(const QString &name) const
{
    return m_resolver->connection(name.isEmpty() ? m_connection : name);
}

std::optional<bool> Migrator::getConnectionDebugSql(const QString &name) const
{
    return name.isEmpty() ? std::nullopt
                          : std::make_optional(m_resolver->connection(name).debugSql());
}

/* Migration instances lists and hashes */

void Migrator::createMigrationNamesMap()
{
    QString previousMigrationName;

    for (const auto &migration : m_migrations) {
        // To avoid expression with side effects in the typeid ()
        const auto &migrationRef = *migration;

        // mid(1) tp remove the '_' at beginning
        auto migrationName = TypeUtils::classPureBasename(migrationRef, false).mid(1);

        // Verify alphabetical sorting
        throwIfMigrationsNotSorted(previousMigrationName, migrationName);
        previousMigrationName = migrationName;

        m_migrationNamesMap.emplace(std::type_index(typeid (migrationRef)),
                                    migrationName);

        m_migrationInstancesMap.emplace(migrationName, migration);

        m_migrationNames.emplace(std::move(migrationName));
    }
}

QString Migrator::getMigrationName(const Migration &migration) const
{
    // To avoid expression with side effects in the typeid ()
    const auto &migrationRef = migration;
    const auto &migrationId = typeid (migrationRef);

    Q_ASSERT(m_migrationNamesMap.contains(migrationId));

    return m_migrationNamesMap.at(migrationId);
}

/* Migrate */

std::vector<std::shared_ptr<Migration>>
Migrator::pendingMigrations(const QVector<QVariant> &ran) const
{
    return m_migrations
            | ranges::views::remove_if([this, &ran](const auto &migration)
    {
        return ran.contains(getMigrationName(*migration));
    })
            | ranges::to<std::vector<std::shared_ptr<Migration>>>();
}

void Migrator::runUp(const Migration &migration, const int batch,
                     const bool pretend) const
{
    if (pretend) {
        pretendToRun(migration, MigrateMethod::Up);
        return;
    }

    auto migrationName = getMigrationName(migration);

    comment(QLatin1String("Migrating: "), false).note(migrationName);

    QElapsedTimer timer;
    timer.start();

    runMigration(migration, MigrateMethod::Up);

    const auto elapsedTime = timer.elapsed();

    /* Once we have run a migrations class, we will log that it was run in this
       repository so that we don't try to run it next time we do a migration
       in the application. A migration repository keeps the migrate order. */
    m_repository->log(migrationName, batch);

    info(QLatin1String("Migrated: "), false);
    note(QStringLiteral("%1 (%2ms)").arg(std::move(migrationName)).arg(elapsedTime));
}

/* Rollback */

std::vector<RollbackItem>
Migrator::getMigrationsForRollback(const MigrateOptions options) const
{
    auto migrationsDb = options.stepValue > 0
                        ? m_repository->getMigrations(options.stepValue)
                        : m_repository->getLast();

    return m_migrations
            | ranges::views::reverse
            | ranges::views::filter([this, &migrationsDb](const auto &migration)
    {
        return ranges::contains(migrationsDb, getMigrationName(*migration),
                                [](const auto &m) { return m.migration; });
    })
            | ranges::views::transform([this, &migrationsDb](const auto &migration)
                                       -> RollbackItem
    {
        // Can not happen that it doesn't find, checked in previous lambda by 'contains'
        auto &&[id, migrationName, _] =
                *std::ranges::find(migrationsDb, getMigrationName(*migration),
                                   [](const auto &m) { return m.migration; });

        return {std::move(id), std::move(migrationName), migration};
    })
            | ranges::to<std::vector<RollbackItem>>();
}

std::vector<RollbackItem>
Migrator::getMigrationsForRollback(std::vector<MigrationItem> &&ran) const
{
    return ranges::views::move(ran)
            | ranges::views::transform([this](auto &&migrationItem) -> RollbackItem
    {
        auto &&[id, migrationName, _] = migrationItem;

        auto migration = m_migrationInstancesMap.at(migrationName);

        return {std::move(id), std::move(migrationName), std::move(migration)};
    })
            | ranges::to<std::vector<RollbackItem>>();
}

std::vector<RollbackItem>
Migrator::rollbackMigrations(std::vector<RollbackItem> &&migrations,
                             const bool pretend) const
{
    if (migrations.empty()) {
        info(QLatin1String("Nothing to rollback"));

        return std::move(migrations);
    }

    for (const auto &migration : migrations)
        runDown(migration, pretend);

    return std::move(migrations);
}

void Migrator::runDown(const RollbackItem &migrationToRollback, const bool pretend) const
{
    const auto &[id, migrationName, migration] = migrationToRollback;

    if (pretend) {
        pretendToRun(*migration, MigrateMethod::Down);
        return;
    }

    comment(QLatin1String("Rolling back: "), false).note(migrationName);

    QElapsedTimer timer;
    timer.start();

    runMigration(*migration, MigrateMethod::Down);

    const auto elapsedTime = timer.elapsed();

    /* Once we have successfully run the migration "down" we will remove it from
       the migration repository so it will be considered to have not been run
       by the application then will be able to fire by any later operation. */
    m_repository->deleteMigration(id);

    info(QLatin1String("Rolled back: "), false);
    note(QStringLiteral("%1 (%2ms)").arg(migrationName).arg(elapsedTime));
}

/* Pretend */

void Migrator::pretendToRun(const Migration &migration, const MigrateMethod method) const
{
    for (auto &&query : getQueries(migration, method)) {
        info(QStringLiteral("%1:").arg(getMigrationName(migration)));

        note(QueryUtils::parseExecutedQueryForPretend(query.query,
                                                      query.boundValues));
    }
}

QVector<Migrator::Log>
Migrator::getQueries(const Migration &migration, const MigrateMethod method) const
{
    /* Now that we have the connections we can resolve it and pretend to run the
       queries against the database returning the array of raw SQL statements
       that would get fired against the database system for this migration. */
    return resolveConnection(migration.connection)
            .pretend([this, &migration, method]()
    {
        migrateByMethod(migration, method);
    });
}

/* Migrate up/down common */

void Migrator::runMigration(const Migration &migration, const MigrateMethod method) const
{
    auto &connection = resolveConnection(migration.connection);

    // Invoke migration in the transaction if a database driver supports it
    const auto withinTransaction =
            connection.getSchemaGrammar().supportsSchemaTransactions() &&
            migration.withinTransaction;

    // Without transaction
    if (!withinTransaction) {
        migrateByMethod(migration, method);
        return;
    }

    // Transactional migration
    connection.beginTransaction();

    try {
        migrateByMethod(migration, method);

    }  catch (const std::exception &/*unused*/) {

        connection.rollBack();
        // Re-throw
        throw;
    }

    connection.commit();
}

void Migrator::migrateByMethod(const Migration &migration,
                               const MigrateMethod method) const
{
    switch (method) {
    case MigrateMethod::Up:
        migration.up();
        return;

    case MigrateMethod::Down:
        migration.down();
        return;
    }

    Q_UNREACHABLE();
}

void Migrator::throwIfMigrationsNotSorted(const QString &previousMigrationName,
                                          const QString &migrationName) const
{
    if (previousMigrationName < migrationName)
        return;

    throw Exceptions::InvalidTemplateArgumentError(
            QLatin1String(
                "The template arguments passed to the TomApplication::migrations() "
                "must always be sorted alphabetically."));
}

} // namespace Tom

TINYORM_END_COMMON_NAMESPACE