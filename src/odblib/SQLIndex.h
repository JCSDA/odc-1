/*
 * © Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File SQLIndex.h
// Baudouin Raoult - ECMWF Jan 04

#ifndef SQLIndex_H
#define SQLIndex_H

#include "eclib/machine.h"

namespace odb {
namespace sql {
// Forward declarations

class BerkeleyDB;
class SQLColumn;

class SQLIndex {
public:

// -- Exceptions
	// None

// -- Contructors

	SQLIndex(SQLColumn&);

// -- Destructor

	~SQLIndex(); // Change to virtual if base class

// -- Convertors
	// None

// -- Operators
	// None

// -- Methods

	void update();

	void rewind(double* value);
	bool next(unsigned long&,unsigned long&);

// -- Overridden methods
	// None

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

	SQLIndex(const SQLIndex&);
	SQLIndex& operator=(const SQLIndex&);

// -- Members

	SQLColumn&           owner_;
#if 0
	auto_ptr<BerkeleyDB> db_;
#endif
	double*              value_;

// -- Methods
	// None

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	//friend ostream& operator<<(ostream& s,const SQLIndex& p)
	//	{ p.print(s); return s; }

};

} // namespace sql
} // namespace odb

#endif