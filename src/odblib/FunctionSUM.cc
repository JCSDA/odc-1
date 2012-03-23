/*
 * © Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "odblib/FunctionSUM.h"

namespace odb {
namespace sql {
namespace expression {
namespace function {

FunctionSUM::FunctionSUM(const string& name,const expression::Expressions& args)
: FunctionExpression(name,args),
  value_(0),
  resultNULL_(true)
{}

FunctionSUM::FunctionSUM(const FunctionSUM& other)
: FunctionExpression(other.name_, other.args_),
  value_(other.value_),
  resultNULL_(other.resultNULL_)
{}

FunctionSUM::~FunctionSUM() {}

SQLExpression* FunctionSUM::clone() const { return new FunctionSUM(*this); }

double FunctionSUM::eval(bool& missing) const
{
	if (resultNULL_)
		missing = true;
	return value_;
}

void FunctionSUM::prepare(SQLSelect& sql)
{
	FunctionExpression::prepare(sql);
	value_ = 0;
}

void FunctionSUM::cleanup(SQLSelect& sql)
{
	FunctionExpression::cleanup(sql);
	value_ = 0;
}

void FunctionSUM::partialResult() 
{
	bool missing = false;
	double value = args_[0]->eval(missing);
	if(! missing)
	{
		value_ += value;
		resultNULL_ = false;
	}
}

static FunctionMaker<FunctionSUM> make_SUM("sum",1);

} // namespace function
} // namespace expression
} // namespace sqol
} // namespace odb 