#include "orm/query/processors/mysqlprocessor.hpp"

#include <QtSql/QSqlQuery>
#include <QVariant>

TINYORM_BEGIN_COMMON_NAMESPACE

namespace Orm::Query::Processors
{

QStringList MySqlProcessor::processColumnListing(QSqlQuery &query) const
{
    QStringList columns;

    while (query.next())
        columns.append(query.value("column_name").value<QString>());

    return columns;
}

} // namespace Orm::Query::Processors

TINYORM_END_COMMON_NAMESPACE
