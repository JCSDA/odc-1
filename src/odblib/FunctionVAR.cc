/*
 * © Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "odblib/FunctionVAR.h"

namespace odb {
namespace sql {
namespace expression {
namespace function {

FunctionVAR::FunctionVAR(const string& name,const expression::Expressions& args)
: FunctionExpression(name,args),
  count_(0),
  value_(0),
  squares_(0)
{}

FunctionVAR::FunctionVAR(const FunctionVAR& other)
: FunctionExpression(other.name_, other.args_),
  count_(other.count_),
  value_(other.value_),
  squares_(other.squares_)
{}

FunctionVAR::~FunctionVAR() {}

SQLExpression* FunctionVAR::clone() const { return new FunctionVAR(*this); }

double FunctionVAR::eval(bool& missing) const
{
	double x,y;

	if(!count_) {
		missing = true;
		return 0;
	}

	x = value_   / count_;
	y = squares_ / count_ ;

	return y - x*x;
}

void FunctionVAR::prepare(SQLSelect& sql)
{
	FunctionExpression::prepare(sql);
	value_ = 0;
	count_ = 0;
	squares_ = 0;
}

void FunctionVAR::cleanup(SQLSelect& sql)
{
//cout << "Cleanup  FunctionVAR " << count_ << " " << value_ << endl;
	FunctionExpression::cleanup(sql);
	squares_ = 0;
	value_ = 0;
	count_ = 0;
}

void FunctionVAR::partialResult() 
{
	bool missing = false;
	double value = args_[0]->eval(missing);
	if(!missing)
	{
		value_   += value;
		squares_ += value * value;
		count_++;
	}
//	else cout << "missing" << endl;
}

static FunctionMaker<FunctionVAR> make_VAR("var",1);
static FunctionMaker<FunctionVAR> make_VARP("varp",1);

} // namespace function
} // namespace expression
} // namespace sql
} // namespace odb
