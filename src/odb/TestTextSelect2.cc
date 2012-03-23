/*
 * © Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// \file TestTextSelect2.h
///
// @author Piotr Kuchta, ECMWF, Oct 2010

#include <iostream>

using namespace std;

#include "odblib/oda.h"
#include "Tool.h"
#include "TestCase.h"
#include "TestTextSelect2.h"
#include "ToolFactory.h"
#include "odblib/SimpleFilterIterator.h"
#include "eclib/TmpFile.h"
#define SRC __FILE__, __LINE__

namespace odb {
namespace tool {
namespace test {

ToolFactory<TestTextSelect2> _TestTextSelect2("TestTextSelect2");

TestTextSelect2::TestTextSelect2(int argc, char **argv)
: TestCase(argc, argv)
{}

TestTextSelect2::~TestTextSelect2() { }

void TestTextSelect2::setUp() { }

/// Tests syntax 'select lat, lon' (no file name)
///
void TestTextSelect2::test()
{
	selectStarOneColumn();
	selectSumOneColumn();
}

void TestTextSelect2::selectStarOneColumn()
{
	string sql = "select * where a > 4;";
	const string fileName = "TestTextSelect2.txt";
	ifstream fs(fileName.c_str());
	
	odb::Select oda(sql, fs);
	
	Log::info(SRC) << "TestTextSelect2::selectStarOneColumn: Execute '" << sql << "'" << endl;
	odb::Select::iterator it = oda.begin();
	odb::Select::iterator end = oda.end();

	ASSERT((*it).columns().size() == 2);

	double v = 5;
	unsigned long n = 0;
	for( ; it != end; ++it, ++n)
	{
		ASSERT((*it).columns().size() == 2);
		ASSERT(v++ == (*it)[0]);
	}

	ASSERT(n == 6);
}

void TestTextSelect2::selectSumOneColumn()
{
	string sql = "select sum(a), sum(b)";
	const string fileName = "TestTextSelect2.txt";
	ifstream fs(fileName.c_str());
	
	odb::Select oda(sql, fs);
	
	Log::info(SRC) << "TestTextSelect2::selectSumOneColumn: Execute '" << sql << "'" << endl;
	odb::Select::iterator it = oda.begin();
	odb::Select::iterator end = oda.end();

	ASSERT((*it).columns().size() == 2);

	++it;
	ASSERT(! (it != end));
	ASSERT((*it)[0] == 55);
	ASSERT((*it)[1] == 55);
}


void TestTextSelect2::tearDown() {}

} // namespace test
} // namespace tool 
} // namespace odb 
