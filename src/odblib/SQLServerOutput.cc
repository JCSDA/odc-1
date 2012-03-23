/*
 * © Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eclib/AutoAlarm.h"
#include "eclib/Exceptions.h"
#include "eclib/Log.h"

#include "odblib/Endian.h"
#include "odblib/SQLAST.h"
#include "odblib/SQLBitfield.h"
#include "odblib/SQLServerOutput.h"
#include "odblib/SQLServerSession.h"
#include "odblib/SchemaAnalyzer.h"


namespace odb {
namespace sql {

SQLServerOutput::SQLServerOutput(SQLServerSession& owner,bool swapBytes):
	owner_(owner),
	swapBytes_(swapBytes),
	count_(0)
{}

SQLServerOutput::~SQLServerOutput() {}

void SQLServerOutput::print(ostream& s) const
{
	s << "SQLServerOutput";
}

void SQLServerOutput::size(int)
{
}

void SQLServerOutput::reset()
{
	count_ = 0;
}

void SQLServerOutput::flush() {}

bool SQLServerOutput::output(const expression::Expressions& results)
{
	Swap<double> swap;
	
    for(size_t i = 0; i < results.size(); i++)
	{
		bool missing = false;
		double x = results[i]->eval(missing);
		if(swapBytes_) x = swap(x);
		ASSERT(data_.write(&x,sizeof(x)) == sizeof(x));
	}

	count_++;
	return true;
}

void SQLServerOutput::prepare(SQLSelect& sql)
{
	AutoAlarm alarm(10,false);
	Log::info() << "SQLServerOutput::prepare" << endl;
	data_ = owner_.accept();
	ASSERT(!alarm.caught());
}

void SQLServerOutput::cleanup(SQLSelect& sql)
{
	Log::info() << "SQLServerOutput::cleanup" << endl;
	data_.close();
}

unsigned long long SQLServerOutput::count()
{
	return count_;
}

} // namespace sql 
} // namespace odb 