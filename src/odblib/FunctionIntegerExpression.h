/*
 * © Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File FunctionIntegerExpression.h
// ECMWF July 2010

#ifndef FUNCTION_INTEGER_EXPRESSION_H
#define FUNCTION_INTEGER_EXPRESSION_H

#include "odblib/FunctionExpression.h"

namespace odb {
namespace sql {
namespace expression {
namespace function {

class FunctionIntegerExpression : public FunctionExpression {
public:
	FunctionIntegerExpression(const string&,const expression::Expressions&);
	~FunctionIntegerExpression(); // Change to virtual if base class

// -- Overridden methods
	virtual const odb::sql::type::SQLType* type() const;
	virtual void output(ostream& s) const;

private:
// No copy allowed
	FunctionIntegerExpression(const FunctionIntegerExpression&);
	FunctionIntegerExpression& operator=(const FunctionIntegerExpression&);

// -- Friends
	//friend ostream& operator<<(ostream& s,const FunctionIntegerExpression& p)
	//	{ p.print(s); return s; }
};

} // namespace function
} // namespace expression 
} // namespace sql
} // namespace odb

#endif