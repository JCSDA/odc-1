/*
 * (C) Copyright 1996-2012 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef ODBBehavior_h
#define ODBBehavior_h

#include "eckit/runtime/ContextBehavior.h"

//-----------------------------------------------------------------------------

namespace odb {

class ODBBehavior : public eckit::ContextBehavior {
public:

    /// Contructors

    ODBBehavior();

    /// Destructor

    ~ODBBehavior();

public: // methods

    virtual eckit::LogStream& infoStream();
    virtual eckit::LogStream& warnStream();
    virtual eckit::LogStream& errorStream();
    virtual eckit::LogStream& debugStream();

};

} // namespace odb

//-----------------------------------------------------------------------------

#endif

