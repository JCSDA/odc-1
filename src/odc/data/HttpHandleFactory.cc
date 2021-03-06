/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <sstream>


#include "eckit/utils/StringTools.h"

#include "odc/data/HttpHandle.h"
#include "odc/data/HttpHandleFactory.h"

using namespace eckit;
using namespace std;

namespace odc {

HttpHandleFactory::HttpHandleFactory()
: DataHandleFactory("http")
{}

eckit::DataHandle* HttpHandleFactory::makeHandle(const std::string& r) const
{
    return new HttpHandle(r);
}

} // namespace odc
