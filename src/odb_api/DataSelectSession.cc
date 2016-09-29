/// @file   DataSelectSession.cc
/// @author Tomas Kral

#include "eckit/exception/Exceptions.h"
#include "odb_api/DataSelectOutput.h"
#include "odb_api/DataSelectSession.h"

namespace odb {
namespace internal {

DataSelectSession::DataSelectSession(DataSelectIterator& it)
  : it_(it), sql_(0)
{}

DataSelectSession::~DataSelectSession()
{}

void DataSelectSession::statement(odb::sql::SQLStatement* sql)
{
    ASSERT(sql);    
    sql_ = sql;
    gotSelectAST(false);
}

odb::sql::SQLOutput* DataSelectSession::defaultOutput()
{
    return new DataSelectOutput(it_);
}

odb::sql::SQLStatement* DataSelectSession::statement()
{
    typedef odb::sql::SQLStatement* P;
    if (gotSelectAST())
    {
        gotSelectAST(false);
        sql_ = P(selectFactory().create(*this, selectAST()));
    }
    return sql_;
}

} // namespace internal
} // namespace odb
