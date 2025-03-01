extern_constants: \
    sourcesList += \
        $$PWD/tom/tomconstants_extern.cpp

sourcesList += \
    $$PWD/tom/application.cpp \
    $$PWD/tom/commands/command.cpp \
    $$PWD/tom/commands/completecommand.cpp \
    $$PWD/tom/commands/database/seedcommand.cpp \
    $$PWD/tom/commands/database/wipecommand.cpp \
    $$PWD/tom/commands/environmentcommand.cpp \
    $$PWD/tom/commands/helpcommand.cpp \
    $$PWD/tom/commands/inspirecommand.cpp \
    $$PWD/tom/commands/integratecommand.cpp \
    $$PWD/tom/commands/listcommand.cpp \
    $$PWD/tom/commands/make/concerns/prepareoptionvalues.cpp \
    $$PWD/tom/commands/make/makecommand.cpp \
    $$PWD/tom/commands/make/migrationcommand.cpp \
    $$PWD/tom/commands/make/modelcommand.cpp \
#    $$PWD/tom/commands/make/projectcommand.cpp \
    $$PWD/tom/commands/make/seedercommand.cpp \
    $$PWD/tom/commands/make/support/migrationcreator.cpp \
    $$PWD/tom/commands/make/support/modelcreator.cpp \
    $$PWD/tom/commands/make/support/prepareforeignkeyvalues.cpp \
    $$PWD/tom/commands/make/support/seedercreator.cpp \
    $$PWD/tom/commands/make/support/tableguesser.cpp \
    $$PWD/tom/commands/migrations/freshcommand.cpp \
    $$PWD/tom/commands/migrations/installcommand.cpp \
    $$PWD/tom/commands/migrations/migratecommand.cpp \
    $$PWD/tom/commands/migrations/refreshcommand.cpp \
    $$PWD/tom/commands/migrations/resetcommand.cpp \
    $$PWD/tom/commands/migrations/rollbackcommand.cpp \
    $$PWD/tom/commands/migrations/statuscommand.cpp \
    $$PWD/tom/commands/migrations/uninstallcommand.cpp \
    $$PWD/tom/concerns/callscommands.cpp \
    $$PWD/tom/concerns/confirmable.cpp \
    $$PWD/tom/concerns/guesscommandname.cpp \
    $$PWD/tom/concerns/interactswithio.cpp \
    $$PWD/tom/concerns/pretendable.cpp \
    $$PWD/tom/concerns/printsoptions.cpp \
    $$PWD/tom/concerns/usingconnection.cpp \
    $$PWD/tom/exceptions/tomlogicerror.cpp \
    $$PWD/tom/exceptions/tomruntimeerror.cpp \
    $$PWD/tom/migrationrepository.cpp \
    $$PWD/tom/migrator.cpp \
    $$PWD/tom/seeder.cpp \
    $$PWD/tom/terminal.cpp \
    $$PWD/tom/tomutils.cpp \

SOURCES += $$sorted(sourcesList)

unset(sourcesList)
