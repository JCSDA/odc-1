/*
 * © Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "oda.h"
#include <Application.h>
#include <PathName.h>
#include <FileHandle.h>

#include "Tool.h"
#include "ToolFactory.h"
#include "ToolRunnerApplication.h"
#include "TestRunnerApplication.h"
#include "ReptypeGenIterator.h"
#include "ODBIterator.h"
#include "FakeODBIterator.h"
#include "ODB2ODATool.h"

using namespace std;
using namespace odb::tool;

// Cannot use just string for str because of a clash with a typedef in ODB header...
typedef std::string str;

int gdb(int argc, char *argv[]);
int valgrind(int argc, char *argv[]);

//void test_schemaFile();

int main(int argc, char *argv[])
{
	//test_schemaFile();

	CommandLineParser clp(argc, argv);
	clp.registerOptionWithArgument("-genreptype");
	clp.registerOptionWithArgument("-reptypecfg");
	clp.registerOptionWithArgument("-addcolumns");

	if (clp.parameters().size() < 2 || clp.parameters().size() > 4)
	{
		cerr << "Usage:" << endl
			<< "	" << clp.parameters(0)
			<< " [-genreptype <list-of-columns>]"
			<< " [-reptypecfg <reptype-generation-config-file>]"
			<< " [-addcolumns <list-of-assignments>]"

			<< " <odb_database> [<file-with-select-statement-defining-dump> [<output.odb>]]"
			<< endl;
		return 1;
	}

	cout << clp << endl;

	if (clp.parameters(1) == "g")    return gdb(argc, argv);
	if (clp.parameters(1) == "vg")   return valgrind(argc, argv);
	if (clp.parameters(1) == "test")
	{
		odb::tool::test::TestRunnerApplication(argc, argv).start();
		return 0; // TODO: Retrieve a status from the test runner
	}

	if (clp.parameters(1) == "help") return ToolRunnerApplication(argc, argv).printHelp(cout);

	ToolRunnerApplication runner(argc, argv, false, false);
	//ODB2ODATool odb2oda(&runner);
	ODB2ODATool odb2oda(clp);
	runner.tool(&odb2oda);
	runner.start();
	return 0;
}

int gdb(int argc, char *argv[])
{
	str cmd = argv[0];
	str args;
	str gdbScript = argv[1];
	for (int i = 2; i < argc; i++)
		args += str(" ") + argv[i];

	PathName scriptFile = str(".gdb_") + str(argv[2]);
	if (! scriptFile.exist())
	{
		str s = str("file ") + cmd + "\nbreak main\nrun " + args + "\n";
		FileHandle f(scriptFile);
		f.openForWrite(1024);
		f.write(s.c_str(), s.size());
		f.close();
	}
	str vi = str("vi ") + scriptFile;
	str gdb = str("gdb -x ") + scriptFile;
	cout << "Executing '" << vi << "'" << endl;
	system(vi.c_str());
	cout << "Executing '" << gdb << "'" << endl;
	return system(gdb.c_str());
}

// valgrind --log-file=v.log --show-reachable=yes --leak-check=full ./oda test 
int valgrind(int argc, char *argv[])
{
	str cmd = argv[0];
	str args;
	for (int i = 2; i < argc; i++)
		args += str(" ") + argv[i];


	//str logFile = str("vg.") + argv[2] + ".log";
	str logFile = str("vg.log");
	str vg = str("valgrind --log-file=") + logFile + " --show-reachable=yes --leak-check=full " + cmd + " " + args;
	cout << "Executing '" << vg << "'" << endl;
	return system(vg.c_str());
}

#if 0
#include <assert.h>

void test_schemaFile() {
	assert(ODBIterator::schemaFile("/asdfasd/sdfas/ECMA.tmi") == "/asdfasd/sdfas/ECMA.tmi/ECMA.sch");
	assert(ODBIterator::schemaFile("/asdfasd/sdfas/ODA.tmi") == "/asdfasd/sdfas/ODA.tmi/ODA.sch");
	assert(ODBIterator::schemaFile("/asdfasd/sdfas/ODA.tmi/") == "/asdfasd/sdfas/ODA.tmi/ODA.sch");
	assert(ODBIterator::schemaFile("ODA.tmi/") == "ODA.tmi/ODA.sch");
	assert(ODBIterator::schemaFile("ODA.tmi") == "ODA.tmi/ODA.sch");
	assert(ODBIterator::schemaFile("dupa/ODA.tmi") == "dupa/ODA.tmi/ODA.sch");
}
#endif 

