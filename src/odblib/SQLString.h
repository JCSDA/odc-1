/*
 * © Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File SQLString.h
// Baudouin Raoult - ECMWF Dec 03

#ifndef SQLString_H
#define SQLString_H

#include "odblib/SQLType.h"

namespace odb {
namespace sql {

class SQLOutput;

namespace type {

class SQLString : public SQLType {
public:
	SQLString(const string& );
	~SQLString(); // Change to virtual if base class

private:
// No copy allowed
	SQLString(const SQLString&);
	SQLString& operator=(const SQLString&);

// -- Overridden methods
	virtual size_t size() const;
	virtual void output(SQLOutput&, double, bool) const;
	virtual int getKind() const { return stringType; }

	//friend ostream& operator<<(ostream& s,const SQLString& p)
	//	{ p.print(s); return s; }
};

} // namespace type 
} // namespace sql 
} // namespace odb 

#endif