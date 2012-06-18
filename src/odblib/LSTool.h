/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef LSTool_H
#define LSTool_H

#include "odblib/Tool.h"

namespace odb {
namespace tool {

class LSTool : public Tool {
public:
	LSTool (int argc, char *argv[]); 

	void run(); 

	static void help(ostream &o)
	{ o << "Shows file's contents"; }

	static void usage(const string& name, ostream &o)
	{ o << name << " [-o <output-file>] <file-name>" << endl << endl; }

	unsigned long long printData(const string &db, ostream &out);

private:
// No copy allowed
    LSTool(const LSTool&);
    LSTool& operator=(const LSTool&);

	static const string nullString;
};

} // namespace tool 
} // namespace odb 

#endif 
