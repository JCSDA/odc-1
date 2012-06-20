/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File FunctionNOT_IN.h
// Baudouin Raoult - ECMWF Dec 03

#ifndef FunctionNOT_IN_H
#define FunctionNOT_IN_H

#include "odblib/FunctionExpression.h"

namespace odb {
namespace sql {
namespace expression {
namespace function {

class FunctionNOT_IN : public FunctionExpression {
public:
	FunctionNOT_IN(const string&, const expression::Expressions&);
	FunctionNOT_IN(const FunctionNOT_IN&);
	~FunctionNOT_IN();

	SQLExpression* clone() const;

private:
// No copy allowed
	FunctionNOT_IN& operator=(const FunctionNOT_IN&);

	int size_;

// -- Overridden methods
	virtual const odb::sql::type::SQLType* type() const;
	virtual double eval(bool& missing) const;

// -- Friends
	//friend ostream& operator<<(ostream& s,const FunctionNOT_IN& p)
	//	{ p.print(s); return s; }
};

} // namespace function
} // namespace expression 
} // namespace sql
} // namespace odb

#endif
