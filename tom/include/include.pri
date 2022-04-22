INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/tom/application.hpp \
    $$PWD/tom/commands/command.hpp \
    $$PWD/tom/commands/database/wipecommand.hpp \
    $$PWD/tom/commands/environmentcommand.hpp \
    $$PWD/tom/commands/helpcommand.hpp \
    $$PWD/tom/commands/inspirecommand.hpp \
    $$PWD/tom/commands/listcommand.hpp \
    $$PWD/tom/commands/make/migrationcommand.hpp \
#    $$PWD/tom/commands/make/projectcommand.hpp \
    $$PWD/tom/commands/make/stubs/migrationstubs.hpp \
    $$PWD/tom/commands/make/stubs/projectstubs.hpp \
    $$PWD/tom/commands/migrations/freshcommand.hpp \
    $$PWD/tom/commands/migrations/installcommand.hpp \
    $$PWD/tom/commands/migrations/migratecommand.hpp \
    $$PWD/tom/commands/migrations/refreshcommand.hpp \
    $$PWD/tom/commands/migrations/resetcommand.hpp \
    $$PWD/tom/commands/migrations/rollbackcommand.hpp \
    $$PWD/tom/commands/migrations/statuscommand.hpp \
    $$PWD/tom/concerns/callscommands.hpp \
    $$PWD/tom/concerns/confirmable.hpp \
    $$PWD/tom/concerns/interactswithio.hpp \
    $$PWD/tom/concerns/printsoptions.hpp \
    $$PWD/tom/config.hpp \
    $$PWD/tom/exceptions/invalidargumenterror.hpp \
    $$PWD/tom/exceptions/invalidtemplateargumenterror.hpp \
    $$PWD/tom/exceptions/logicerror.hpp \
    $$PWD/tom/exceptions/runtimeerror.hpp \
    $$PWD/tom/exceptions/tomerror.hpp \
    $$PWD/tom/migration.hpp \
    $$PWD/tom/migrationcreator.hpp \
    $$PWD/tom/migrationrepository.hpp \
    $$PWD/tom/migrator.hpp \
    $$PWD/tom/terminal.hpp \
    $$PWD/tom/tomtypes.hpp \
    $$PWD/tom/version.hpp \