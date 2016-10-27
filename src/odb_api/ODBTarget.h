/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file ODBTarget.h
/// @author Piotr Kuchta

#ifndef odbapi_ODBTarget_h
#define odbapi_ODBTarget_h

#include <utility>

#include "eckit/log/WrapperTarget.h"

namespace odb {

//----------------------------------------------------------------------------------------------------------------------

class ODBTarget : public eckit::WrapperTarget {
public:

    ODBTarget(const char* tag = "", eckit::LogTarget* target = 0);

private:

    virtual void writePrefix();
    virtual void writeSuffix();

private:

    const char* tag_;

};

} // namespace odb

#endif