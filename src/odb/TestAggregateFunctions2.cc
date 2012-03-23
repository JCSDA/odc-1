/*
 * © Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// \file TestAggregateFunctions2.h
///
/// @author Piotr Kuchta, ECMWF, September 2010

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;

#include "odblib/oda.h"
#include "Tool.h"
#include "TestCase.h"
#include "TestAggregateFunctions2.h"
#include "ToolFactory.h"

#define SRC __FILE__, __LINE__


#include "eclib/PathName.h"
#include "eclib/DataHandle.h"
#include "odblib/DataStream.h"
#include "odblib/HashTable.h"
#include "odblib/Codec.h"
#include "odblib/Column.h"
#include "odblib/MetaData.h"
#include "eclib/DataHandle.h"
#include "odblib/DataStream.h"
#include "odblib/RowsIterator.h"
#include "odblib/HashTable.h"
#include "eclib/Log.h"
#include "odblib/SQLBitfield.h"
#include "odblib/SQLAST.h"
#include "odblib/SchemaAnalyzer.h"
#include "odblib/SQLIteratorSession.h"
#include "odblib/Header.h"
#include "odblib/Reader.h"
#include "eclib/DataHandle.h"
#include "eclib/FILEHandle.h"
#include "odblib/SelectIterator.h"
#include "odblib/ReaderIterator.h"
#include "odblib/oda.h"

namespace odb {
namespace tool {
namespace test {

ToolFactory<TestAggregateFunctions2> _TestAggregateFunctions2("TestAggregateFunctions2");

TestAggregateFunctions2::TestAggregateFunctions2(int argc, char **argv)
: TestCase(argc, argv)
{}

TestAggregateFunctions2::~TestAggregateFunctions2() { }

///
void TestAggregateFunctions2::test()
{
	string sql = "select count(*) from \"TestAggregateFunctions2.odb\"";

	Log::info() << "Executing: '" << sql << "'" << endl;

	odb::Select oda(sql);
	odb::Select::iterator it = oda.begin();

	ASSERT(it->columns().size() == 1);
	ASSERT((*it)[0] == 10); 

	odb::Select sel(sql);
	odb::Select::iterator it2 = sel.begin();
	odb::Select::iterator end2 = sel.end();

	FILE *fout = fopen("TestAggregateFunctions2_out.odb", "w");
	FILEHandle fhout(fout);
	odb::Writer<> writer(fhout);
	odb::Writer<>::iterator outit = writer.begin();

	//outit->pass1(it2, end2);
	size_t i = 0;
	for (; it2 != end2; ++it2)
	{
		++i;
		ASSERT( (*it2)[0] == 10);
	}
	ASSERT( i == 1);
}

void TestAggregateFunctions2::setUp() {}

void TestAggregateFunctions2::tearDown() {}

} // namespace test 
} // namespace tool 
} // namespace odb 
