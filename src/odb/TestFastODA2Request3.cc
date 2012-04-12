/// \file TestFastODA2Request3.h
///
/// @author Piotr Kuchta, ECMWF, Jan 2011

#include <iostream>
#include <vector>
#include <map>

#include <strings.h>

using namespace std;

#include "oda.h"

#include "Tool.h"
#include "ToolFactory.h"
#include "TestCase.h"
#include "FastODA2Request.h"
#include "TestFastODA2Request3.h"

namespace odb {
namespace tool {
namespace test {

ToolFactory<TestFastODA2Request3> _TestFastODA2Request3("TestFastODA2Request3");

TestFastODA2Request3::TestFastODA2Request3(int argc, char **argv)
: TestCase(argc, argv)
{}

TestFastODA2Request3::~TestFastODA2Request3() {}

void TestFastODA2Request3::setUp() {}

void TestFastODA2Request3::test()
{
	const char * configFile = "/tmp/p4/mars/server/dev/oda/mars/marsKeywordToODBColumn";
	const char * config = 
	"DATE: andate\n"
	"TIME: antime\n"
	"REPORTYPE: reportype\n"
	"CLASS: class\n"
	"TYPE: type\n"
	"STREAM: stream\n"
	"OBSGROUP: groupid\n"
	"EXPVER: expver\n"
	;

	FastODA2Request<ODA2RequestClientTraits> o;
	//o.parseConfig(StringTool::readFile(cfgFile));
	o.parseConfig(config);

	OffsetList offsets;
	LengthList lengths;
	vector<ODAHandle*> handles;

	//PathName pathName("mondb_conv.17.16001.odb.fn6x");
	PathName pathName("mondb.1.12.odb");
	bool rc = o.scanFile(pathName, offsets, lengths, handles);
	ASSERT(rc == true);

	for (size_t i = 0; i < handles.size(); ++i)
		Log::info() << "TestFastODA2Request3::test: handles[" << i << "]=" << *handles[i] << endl;

	string r = o.genRequest();
	Log::info() << "TestFastODA2Request3::test: o.genRequest() => " << endl << r << endl;

	unsigned long long n = o.rowsNumber();
	Log::info() << "TestFastODA2Request3::test: rowsNumber == " << n <<  endl;
}


void TestFastODA2Request3::tearDown() { }

} // namespace test 
} // namespace tool 
} // namespace odb 
