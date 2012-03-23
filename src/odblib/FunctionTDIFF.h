/*
 * © Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File FunctionTDIFF.h
// ECMWF July 2010

#ifndef FunctionTDIFF_H
#define FunctionTDIFF_H

#include "odblib/FunctionExpression.h"

namespace odb {
namespace sql {
namespace expression {
namespace function {

class FunctionTDIFF : public FunctionExpression {
public:
	FunctionTDIFF(const string&,const expression::Expressions&);
	FunctionTDIFF(const FunctionTDIFF&);
	~FunctionTDIFF(); // Change to virtual if base class

// -- Overridden methods
	virtual const odb::sql::type::SQLType* type() const;
	virtual void output(ostream& s) const;

	SQLExpression* clone() const;

private:
// No copy allowed
	FunctionTDIFF& operator=(const FunctionTDIFF&);

// -- Overridden methods
	virtual double eval(bool& missing) const;

// -- Friends
	//friend ostream& operator<<(ostream& s,const FunctionTDIFF& p)
	//	{ p.print(s); return s; }
};

} // namespace function
} // namespace expression 
} // namespace sql
} // namespace odb

#endif