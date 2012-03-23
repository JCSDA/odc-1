/*
 * © Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// \file ToolRunnerApplication.h
///
/// @author Piotr Kuchta, ECMWF, Feb 2009

#include "eclib/Application.h"
#include "eclib/PathName.h"
#include "eclib/FileHandle.h"

#include "Tool.h"
#include "TestCase.h"
#include "ToolFactory.h"
#include "ToolRunnerApplication.h"

namespace odb {
namespace tool {

ToolRunnerApplication::ToolRunnerApplication (int argc, char **argv, bool createCommandLineTool, bool deleteTool)
: Application(argc, argv),
  tool_(!createCommandLineTool ? 0 : AbstractToolFactory::createTool(PathName(argv[1]).baseName(), argc - 1, argv + 1)),
  deleteTool_(deleteTool)
{}

ToolRunnerApplication::ToolRunnerApplication (Tool &tool, int argc, char **argv)
: Application(argc, argv),
  tool_(&tool),
  deleteTool_(false)
{}

ToolRunnerApplication::~ToolRunnerApplication ()
{
	Log::info() << "ToolRunnerApplication::~ToolRunnerApplication" << endl;

	if (deleteTool_) delete tool_;
}

void ToolRunnerApplication::tool(Tool *tool)
{
	tool_ = tool;
}

void ToolRunnerApplication::run()
{
	if (tool_ == 0)
	{
		cerr << argv(0) << ": Unknown command '" << argv(1) << "'" << endl;
		return;
	}

	tool_->run();

	if (deleteTool_)
	{
		delete tool_;
		tool_ = 0;
	}
}


int ToolRunnerApplication::printHelp(ostream &out)
{
	if (tool_ == 0)
	{
		cerr << argv(0) << ": Unknown command '" << argv(1) << "'" << endl;
		return 1;
	}
	//tool_->help(out);
	return 0;
}

} // namespace tool 
} // namespace odb 
