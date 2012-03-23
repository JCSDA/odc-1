/*
 * © Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File SQLReal.h
// Baudouin Raoult - ECMWF Dec 03

#ifndef SQLReal_H
#define SQLReal_H

#include "odblib/SQLType.h"

namespace odb {
namespace sql {

class SQLOutput;

namespace type {

class SQLReal : public SQLType {
public:

// -- Exceptions
	// None

// -- Contructors

	SQLReal(const string& );

// -- Destructor

	~SQLReal(); // Change to virtual if base class

// -- Convertors
	// None

// -- Operators
	// None

// -- Methods
	// None

// -- Overridden methods
	virtual void output(SQLOutput&, double, bool) const;

// -- Class members
	// None

// -- Class methods
	// None

protected:

// -- Members
	// None

// -- Methods
	
	// void print(ostream&) const; // Change to virtual if base class	

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

private:

// No copy allowed

	SQLReal(const SQLReal&);
	SQLReal& operator=(const SQLReal&);

// -- Members
	// None

// -- Methods
	// None

// -- Overridden methods
	// None

	virtual size_t size() const;
	virtual int getKind() const { return realType; }

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	//friend ostream& operator<<(ostream& s,const SQLReal& p)
	//	{ p.print(s); return s; }

};

} // namespace type 
} // namespace sql
} // namespace odb

#endif