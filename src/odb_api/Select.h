/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Piotr Kuchta
/// @author
/// @date April 2010

#ifndef odb_api_Select_H
#define odb_api_Select_H

#ifdef SWIGPYTHON
#include <Python.h>
#endif

#include "odb_api/IteratorProxy.h"
#include "eckit/sql/SQLSession.h"

namespace eckit { class PathName; }
namespace eckit { class DataHandle; }

namespace odb {

//----------------------------------------------------------------------------------------------------------------------

class SelectIterator;

class Select
{
public:
	typedef IteratorProxy<SelectIterator,Select,const double> iterator;
	typedef iterator::Row row;

    Select(const std::string& selectStatement="", bool manageOwnBuffer=true);
    Select(const std::string& selectStatement, eckit::DataHandle& dh, bool manageOwnBuffer=true);
    Select(const std::string& selectStatement, const std::string& path, bool manageOwnBuffer=true);

	virtual ~Select();

#ifdef SWIGPYTHON
    iterator __iter__() { return iterator(createSelectIterator(selectStatement_)); }
#endif

	iterator begin();
	const iterator end();

    SelectIterator* createSelectIterator(const std::string&);

private:


    friend class odb::IteratorProxy<odb::SelectIterator, odb::Select, const double>;

    std::unique_ptr<eckit::DataHandle> ownDH_;

//	std::istream* istream_;

	std::string selectStatement_;
	std::string delimiter_;

    eckit::sql::SQLSession session_;
};

//----------------------------------------------------------------------------------------------------------------------

} // namespace odb

#endif
