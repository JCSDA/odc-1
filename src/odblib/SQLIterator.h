/*
 * © Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File SQLIterator
// Baudouin Raoult - ECMWF Dec 03

#ifndef SQLIterator_H
#define SQLIterator_H

#include "eclib/machine.h"

#include "eclib/MemoryPool.h"

#include "odblib/SQLType.h"

namespace odb {
namespace sql {

// Forward declarations
class SQLIterator {
public:

	void *operator new(size_t s)          { return MemoryPool::fastAllocate(s);}
	//void *operator new(size_t s,void *p)  { return p;                          }
	void operator delete(void* p)         { MemoryPool::fastDeallocate(p);     }


// -- Exceptions
	// None

// -- Contructors

	SQLIterator(const type::SQLType& type): type_(type) {}

// -- Destructor

	virtual ~SQLIterator() {}; // Change to virtual if base class

// -- Convertors
	// None

// -- Operators
	// None

// -- Methods

	const type::SQLType& type() const { return type_; }

	virtual void rewind()                = 0;
	virtual double next(bool& missing)                = 0;
	virtual void advance(unsigned long) = 0;

	virtual void load()   {}
	virtual void unload() {}

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

protected:

// -- Members

	const type::SQLType& type_;

// -- Methods
	
	virtual void print(ostream&) const = 0; // Change to virtual if base class	

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

private:

// No copy allowed

	SQLIterator(const SQLIterator&);
	SQLIterator& operator=(const SQLIterator&);

// -- Members


// -- Methods
	// None

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	friend ostream& operator<<(ostream& s,const SQLIterator& p)
		{ p.print(s); return s; }

};

} // namespace sql
} // namespace odb

#endif