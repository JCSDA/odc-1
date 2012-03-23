/*
 * © Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File SQLServer.h
// Baudouin Raoult - ECMWF Jun 96

#ifndef SQLServer_H
#define SQLServer_H

#include "eclib/ThreadControler.h"

namespace odb {
namespace sql {

class SQLServer {
public:

// -- Contructors

	SQLServer(int port);

// -- Destructor

	~SQLServer();

private:

// No copy allowed

	SQLServer(const SQLServer&);
	SQLServer& operator=(const SQLServer&);

// -- Members

	ThreadControler odb_;

};

} // namespace sql
} // namespace odb

#endif