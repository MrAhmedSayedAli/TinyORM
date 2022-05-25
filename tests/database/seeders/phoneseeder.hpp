#pragma once

#ifndef TINYORM_DISABLE_ORM
#  include <orm/tiny/model.hpp>
#endif

#include <tom/seeder.hpp>

#ifndef TINYORM_DISABLE_ORM
namespace Models
{
    class Phone final : public Orm::Tiny::Model<Phone>
    {};
}
#endif

namespace Seeders
{

    struct PhoneSeeder : Seeder
    {
        /*! Run the database seeders. */
        void run() override
        {
#ifdef TINYORM_DISABLE_ORM
            DB::table("phones")->insert({
                {{"name", "1. phone"}},
                {{"name", "2. phone"}},
            });
#else
            // This tests GuardedModel::unguarded()
            Models::Phone::create({{"name", QDateTime::currentDateTime()}});
#endif
        }
    };

} // namespace Seeders
