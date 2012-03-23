/*
 * © Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "odblib/FunctionAND.h"

namespace odb {
namespace sql {
namespace expression {
namespace function {

FunctionAND::FunctionAND(const string& name,const expression::Expressions& args)
: FunctionExpression(name, args)
{}

FunctionAND::FunctionAND(const FunctionAND& other)
: FunctionExpression(other.name_, other.args_)
{}

SQLExpression* FunctionAND::clone() const { return new FunctionAND(*this); }

FunctionAND::~FunctionAND() {}

double FunctionAND::eval(bool& missing) const
{
	return args_[0]->eval(missing) && args_[1]->eval(missing);
}

bool FunctionAND::andSplit(expression::Expressions& e)
{
	bool ok = false;

	if(!args_[0]->andSplit(e))
	{
		e.push_back(args_[0]);
		ok = true;
	}

	if(!args_[1]->andSplit(e))
	{
		e.push_back(args_[1]);
		ok = true;
	}

	return ok;

}

SQLExpression* FunctionAND::simplify(bool& changed) 
{
	SQLExpression* x = FunctionExpression::simplify(changed);
	if(x) return x;

	
	for(int i = 0; i < 2 ; i++)
	{
		bool missing = false;
		if(args_[i]->isConstant())
		{
			if(args_[i]->eval(missing))
			{
				cout << "SYMPLIFY " << *this << " to ";
				changed = true;

				SQLExpression* x = args_[1-i];

				args_[1-i] = 0; // So we don't delete it
				delete args_[i];
				args_.clear();

				cout << *x << endl;

				return x;
			}
			else
			{
				cout << "SYMPLIFY " << *this << "to 0 " << endl;
				changed = true;
				return SQLExpression::number(0);
			}
		}
	}

	return 0;
}


static FunctionMaker<FunctionAND> make_AND("and",2);

} // namespace function
} // namespace expression
} // namespace sql
} // namespace odb
