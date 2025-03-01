#include <orm/db.hpp>
#include <orm/utils/configuration.hpp>

#include <tom/application.hpp>

#include "migrations/2022_05_11_170000_create_users_table.hpp"
#include "migrations/2022_05_11_170100_create_roles_table.hpp"
#include "migrations/2022_05_11_170200_create_role_user_table.hpp"
#include "migrations/2022_05_11_170300_create_user_phones_table.hpp"
#include "migrations/2022_05_11_170400_create_settings_table.hpp"
#include "migrations/2022_05_11_170500_create_torrents_table.hpp"
#include "migrations/2022_05_11_170600_create_torrent_peers_table.hpp"
#include "migrations/2022_05_11_170700_create_torrent_previewable_files_table.hpp"
#include "migrations/2022_05_11_170800_create_torrent_previewable_file_properties_table.hpp"
#include "migrations/2022_05_11_170900_create_file_property_properties_table.hpp"
#include "migrations/2022_05_11_171000_create_torrent_tags_table.hpp"
#include "migrations/2022_05_11_171100_create_tag_torrent_table.hpp"
#include "migrations/2022_05_11_171200_create_tag_properties_table.hpp"
#include "migrations/2022_05_11_171300_create_types_table.hpp"
#include "migrations/2022_05_11_171400_create_datetime_table.hpp"

#include "seeders/databaseseeder.hpp"

using Orm::DatabaseManager;
using Orm::DB;

using ConfigUtils = Orm::Utils::Configuration;

using TomApplication = Tom::Application;

using namespace Migrations; // NOLINT(google-build-using-namespace)
using namespace Seeders;    // NOLINT(google-build-using-namespace)

/*! Build the database manager instance and add a database connection. */
std::shared_ptr<DatabaseManager> setupManager();

/*! c++ main function. */
int main(int argc, char *argv[])
{
    try {
        // Ownership of the shared_ptr()
        auto db = setupManager();

        return TomApplication(argc, argv, std::move(db), "TOM_TESTDATA_ENV")
                // TODO tests, add migration for createDatabase if not exists and remove DB creation from the workflows? silverqx
                .migrations<CreateUsersTable,
                            CreateRolesTable,
                            CreateRoleUserTable,
                            CreateUserPhonesTable,
                            CreateSettingsTable,
                            CreateTorrentsTable,
                            CreateTorrentPeersTable,
                            CreateTorrentPreviewableFilesTable,
                            CreateTorrentPreviewableFilePropertiesTable,
                            CreateFilePropertyPropertiesTable,
                            CreateTorrentTagsTable,
                            CreateTagTorrentTable,
                            CreateTagPropertiesTable,
                            CreateTypesTable,
                            CreateDatetimeTable>()
                .seeders<DatabaseSeeder>()
                // Fire it up 🔥🚀✨
                .run();

    } catch (const std::exception &e) {

        TomApplication::logException(e);
    }

    return EXIT_FAILURE;
}

std::shared_ptr<DatabaseManager> setupManager()
{
    using namespace Orm::Constants; // NOLINT(google-build-using-namespace)

    // Ownership of the shared_ptr()
    return DB::create({
        // MySQL connection
        {QStringLiteral("tinyorm_testdata_tom_mysql"), { // shell:connection
            {driver_,         QMYSQL},
            {host_,           qEnvironmentVariable("DB_MYSQL_HOST",      H127001)},
            {port_,           qEnvironmentVariable("DB_MYSQL_PORT",      P3306)},
            {database_,       qEnvironmentVariable("DB_MYSQL_DATABASE",  EMPTY)},
            {username_,       qEnvironmentVariable("DB_MYSQL_USERNAME",  EMPTY)},
            {password_,       qEnvironmentVariable("DB_MYSQL_PASSWORD",  EMPTY)},
            {charset_,        qEnvironmentVariable("DB_MYSQL_CHARSET",   UTF8MB4)},
            {collation_,      qEnvironmentVariable("DB_MYSQL_COLLATION", UTF8MB40900aici)},
            {timezone_,       TZ00},
            // Specifies what time zone all QDateTime-s will have
            {qt_timezone,     QVariant::fromValue(Qt::UTC)},
            {prefix_,         EMPTY},
            {prefix_indexes,  false},
            {strict_,         true},
            {isolation_level, QStringLiteral("REPEATABLE READ")}, // MySQL default is REPEATABLE READ for InnoDB
            {engine_,         InnoDB},
            {Version,         {}}, // Autodetect
            {options_,        ConfigUtils::mysqlSslOptions()},
        }},

        // MariaDB connection
        {QStringLiteral("tinyorm_testdata_tom_maria"), { // shell:connection
            {driver_,         QMYSQL},
            {host_,           qEnvironmentVariable("DB_MARIA_HOST",      H127001)},
            {port_,           qEnvironmentVariable("DB_MARIA_PORT",      P3306)},
            {database_,       qEnvironmentVariable("DB_MARIA_DATABASE",  EMPTY)},
            {username_,       qEnvironmentVariable("DB_MARIA_USERNAME",  EMPTY)},
            {password_,       qEnvironmentVariable("DB_MARIA_PASSWORD",  EMPTY)},
            {charset_,        qEnvironmentVariable("DB_MARIA_CHARSET",   UTF8MB4)},
            {collation_,      qEnvironmentVariable("DB_MARIA_COLLATION",
                                                   UTF8MB4Unicode520ci)},
            // SYSTEM - set the time zone to your local MySQL server time zone
            {timezone_,       TZ00},
            // Specifies what time zone all QDateTime-s will have
            {qt_timezone,     QVariant::fromValue(Qt::UTC)},
            {prefix_,         EMPTY},
            {prefix_indexes,  false},
            {strict_,         true},
            {isolation_level, QStringLiteral("REPEATABLE READ")}, // MySQL default is REPEATABLE READ for InnoDB
            {engine_,         InnoDB},
            {Version,         {}}, // Autodetect
            {options_,        ConfigUtils::mariaSslOptions()},
        }},

        // PostgreSQL connection
        {QStringLiteral("tinyorm_testdata_tom_postgres"), { // shell:connection
            {driver_,            QPSQL},
            {application_name,   QStringLiteral("tom_testdata")},
            {host_,              qEnvironmentVariable("DB_PGSQL_HOST",       H127001)},
            {port_,              qEnvironmentVariable("DB_PGSQL_PORT",       P5432)},
            {database_,          qEnvironmentVariable("DB_PGSQL_DATABASE",   EMPTY)},
            {search_path,        qEnvironmentVariable("DB_PGSQL_SEARCHPATH", PUBLIC)},
            {username_,          qEnvironmentVariable("DB_PGSQL_USERNAME",   postgres_)},
            {password_,          qEnvironmentVariable("DB_PGSQL_PASSWORD",   EMPTY)},
            {charset_,           qEnvironmentVariable("DB_PGSQL_CHARSET",    UTF8)},
            {timezone_,          UTC},
            // Specifies what time zone all QDateTime-s will have
            {qt_timezone,        QVariant::fromValue(Qt::UTC)},
            {prefix_,            EMPTY},
            {prefix_indexes,     false},
//            {isolation_level,    QStringLiteral("REPEATABLE READ")}, // Postgres default is READ COMMITTED
//            {synchronous_commit, QStringLiteral("off")}, // Postgres default is on
            // ConnectionFactory provides a default value for this (for reference only)
//            {dont_drop,          QStringList {spatial_ref_sys}},
            {options_,           ConfigUtils::postgresSslOptions()},
        }},

        // SQLite connection
        {QStringLiteral("tinyorm_testdata_tom_sqlite"), { // shell:connection
            {driver_,                 QSQLITE},
            {database_,               qEnvironmentVariable("DB_SQLITE_DATABASE", {})},
            {foreign_key_constraints, true},
            {check_database_exists,   true},
            // Specifies what time zone all QDateTime-s will have
            {qt_timezone,             QVariant::fromValue(Qt::UTC)},
            /* Return a QDateTime with the correct time zone instead of the QString,
               only works when the qt_timezone isn't set to the DontConvert. */
            {return_qdatetime,        true},
            {prefix_,                 EMPTY},
            {prefix_indexes,          false},
        }}
    },
        /* Because the default connection name is not defined, then will be needed
           to provide the connection name using the --database=xyz argument. */
        {});
}
