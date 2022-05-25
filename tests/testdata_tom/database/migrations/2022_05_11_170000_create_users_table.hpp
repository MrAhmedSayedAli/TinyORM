#pragma once

#include <tom/migration.hpp>

namespace Migrations
{

    struct CreateUsersTable : Migration
    {
        T_MIGRATION

        /*! Run the migrations. */
        void up() const override
        {
            Schema::create("users", [](Blueprint &table)
            {
                table.id();

                table.string(NAME).unique();
                table.string("note").nullable();
            });
        }

        /*! Reverse the migrations. */
        void down() const override
        {
            Schema::dropIfExists("users");
        }
    };

} // namespace Migrations
