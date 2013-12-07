/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// \file TestSelectIterator2.h
///
/// @author Piotr Kuchta, ECMWF, Feb 2009

#include "eckit/utils/Timer.h"
#include "odblib/Select.h"
#include "odblib/ToolFactory.h"
#include "odblib/Writer.h"
#include "odb/TestSelectIterator2.h"

using namespace std;
using namespace eckit;


namespace odb {
namespace tool {
namespace test {

ToolFactory<TestSelectIterator2> _TestSelectIterator2("TestSelectIterator2");

TestSelectIterator2::TestSelectIterator2(int argc, char **argv)
: TestCase(argc, argv)
{}

TestSelectIterator2::~TestSelectIterator2() { }

const string testFile = "TestSelectIterator2.odb";

const double VALUE[] = { 1, 2, 3 };

void TestSelectIterator2::setUp()
{
	Log::debug() << "TestSelectIterator2::setUp" << std::endl;

	Timer t("Writing " + testFile );
	odb::Writer<> oda(testFile);

	odb::Writer<>::iterator writer = oda.begin();
	writer->columns().setSize(1);
	(**writer).setColumn(0, "value", odb::INTEGER);
	(**writer).writeHeader();

	for (size_t i = 0; i < sizeof(VALUE) / sizeof(double); ++i)
	{
		(*writer)[0] = VALUE[i]; // col 0
		++writer;
	}
}

///
/// Tests problem fixed with p4 change 23687
///
void TestSelectIterator2::test()
{
	const string SELECT = "select * from \"" + testFile + "\";";

	odb::Select oda(SELECT);
	size_t i=0;
	for (odb::Select::iterator it = oda.begin();
		it != oda.end() && i < sizeof(VALUE) / sizeof(double);
		++it, ++i) 
	{
		Log::info() << "TestSelectIterator2::testBug01: it[" << i << "]=" << (*it)[0] << ", should be " << VALUE[i] << std::endl;
		ASSERT((*it)[0] == VALUE[i]);
	}
}

void TestSelectIterator2::tearDown() { }

} // namespace test
} // namespace tool 
} // namespace odb 
