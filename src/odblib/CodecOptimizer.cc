/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// \file CodecOptimizer.cc
///
/// @author Piotr Kuchta, Jan 2010


#include "odblib/oda.h"
#include "odblib/DataStream.h"
#include "odblib/Codec.h"
#include "odblib/Column.h"
#include "odblib/CodecOptimizer.h"
#include "eclib/Resource.h"
#include "eclib/StringTools.h"

//

namespace odb {
namespace codec {

CodecOptimizer::CodecOptimizer()
: defaultCodec_()
{
	defaultCodec_[REAL] = "short_real2";
	defaultCodec_[DOUBLE] = "long_real";
	defaultCodec_[STRING] = "chars";
	defaultCodec_[INTEGER] = "int32";
	defaultCodec_[BITFIELD] = "int32";

	typedef eclib::StringTools S;
	vector<std::string> mappings (S::split(",", eclib::Resource<string>("$ODB_DEFAULT_CODEC", "")));

	for (size_t i = 0; i < mappings.size(); ++i)
	{
		vector<string> a(S::split(":", mappings[i]));
		ASSERT("Wrong format of $ODB_DEFAULT_CODEC" && a.size() == 2);
		defaultCodec_[Column::type(S::trim(a[0]))] = S::trim(a[1]);
	}
}

} // namespace codec 
} // namespace odb 
