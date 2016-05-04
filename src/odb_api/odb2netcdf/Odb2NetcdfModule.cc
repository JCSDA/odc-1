/*
 * (C) Copyright 1996-2013 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file Odb2NetcdfModule.cc
/// @author Piotr Kuchta - (c) ECMWF September 2015

#include <string>

#include "eckit/ecml/parser/Request.h"

#include "eckit/ecml/core/RequestHandler.h"
#include "eckit/ecml/core/ExecutionContext.h"
#include "eckit/ecml/core/Environment.h"
#include "eckit/ecml/core/SpecialFormHandler.h"

#include "odb_api/odb2netcdf/ecml_verbs/Odb2NetcdfHandler.h"

#include "Odb2NetcdfModule.h"

namespace odb {

using namespace std;
using namespace eckit;

Odb2NetcdfModule::Odb2NetcdfModule() {}
Odb2NetcdfModule::~Odb2NetcdfModule() {}

static Request native(const string& name) { return new Cell("_native", name, 0, 0); }
static Request macro(const string& name) { return new Cell("_macro", name, 0, 0); }

void Odb2NetcdfModule::importInto(ExecutionContext& context)
{
    static Odb2NetcdfHandler odb2netcdf("odb.odb2netcdf");
    context.registerHandler("odb2netcdf", odb2netcdf);
}

} // namespace odb 