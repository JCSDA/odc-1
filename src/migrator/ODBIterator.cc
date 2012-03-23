/*
 * © Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// \file ODBIterator.cc
///
/// @author Piotr Kuchta, Feb 2009

#include "oda.h"

#define SRC __FILE__,__LINE__

#include "SQLDatabase.h"
#include "ODBIterator.h"
#include "Tool.h"
#include "SQLSelectFactory.h"

namespace odb { namespace sql { class SQLInteractiveSession; } }

extern "C" {
#include "odbdump.h"
}

namespace odb {
namespace tool {

ODBIterator::ODBIterator(const PathName& db, const std::string& sql)
: db_(db),
  odbHandle_(0),
  ci_(0),
  columns_(0),
  data_(0),
  schemaParsed_(false),
  noMore_(true)
{
	Log::info() << "ODBIterator::ODBIterator: @" << this << " db=" << db << endl;

	const char *odbDirectory = db.asString().c_str();
	Log::info(SRC) << "Opening ODB in " << odbDirectory << endl;
	ASSERT(PathName(odbDirectory).exists());

	std::string select = sql != "" ? sql : defaultSQL(db);

	const char *db_path = db.asString().c_str();
	const char *sql_select = select.c_str();

	Log::info() << "ODBIterator::ODBIterator: Calling odbdump_open(\"" << db_path << "\",\"" << sql_select << "\", NULL, NULL, NULL, &" << noOfColumns_ << ")" << endl;

	odbHandle_ = odbdump_open(db_path, sql_select, NULL, NULL, NULL, &noOfColumns_);
	ASSERT("odbdump_open returned NULL" && odbHandle_);
	ASSERT("odbdump_open returned noOfColumns_ <= 0" && noOfColumns_ > 0);

	data_ = new double[noOfColumns_];

	next();
	if (noMore_)
	{
		Log::warning() << "ODBIterator::ODBIterator: result set empty, no data." << endl;
	}
}

bool ODBIterator::next()
{
	newDataset_ = false;
	int nd = odbdump_nextrow(odbHandle_, data_, noOfColumns_, &newDataset_);
	if (nd == 0)
	{
		noOfColumns_ = nd;
		return !(noMore_ = true);
	}

	if (newDataset_)
	{
		Log::info() << "ODBIterator::readRow: new data set" << endl;
		createColumns();
	}

	ASSERT(nd == noOfColumns_);
	// This is because sometime ODB has MISSING_VALUE_REAL in INTEGER columns...
	// for example station_type@hdr in ECMA.conv
	for (int i = 0; i < noOfColumns_; ++i)
		if ((*columns_)[i]->type() == odb::INTEGER && data_[i] == odb::MISSING_VALUE_REAL)
			data_[i] = odb::MISSING_VALUE_INT;

	return !(noMore_ = false);
}

void ODBIterator::createColumns()
{
	Log::info() << " => ODBIterator::createColumns: " << endl;

	delete columns_;
	columns_ = new odb::MetaData(noOfColumns_, (odb::Column *) 0);

        bool preservePrecision = Resource<bool>("$ODB2ODA_PRESERVE_PRECISION", false);
	
	ci_ = (colinfo_t *) odbdump_destroy_colinfo( (colinfo_t *) ci_, noOfColumns_); 
	int nci = 0;
	ci_ = (colinfo_t *) odbdump_create_colinfo(odbHandle_, &nci); 
	ASSERT(nci == noOfColumns_);
	for (int i = 0; i < noOfColumns_; i++)
	{
		colinfo_t *pci = &((colinfo_t *) ci_)[i];
		std::string name = pci->nickname ? pci->nickname : pci->name;
		odb::ColumnType type = odb::REAL;
		double missing = odb::MISSING_VALUE_INT; 

		switch(pci->dtnum)
		{
			case DATATYPE_REAL4:
				type = odb::REAL;
				missing = odb::MISSING_VALUE_REAL; 
				break;

			case DATATYPE_REAL8:
                                if (preservePrecision)
                                    type = odb::DOUBLE;
                                else
                                    type = odb::REAL;
				missing = odb::MISSING_VALUE_REAL; 
				break;

			case DATATYPE_STRING:
				type = odb::STRING;
				break;

			case DATATYPE_INT4:
			case DATATYPE_YYYYMMDD:
			case DATATYPE_HHMMSS:
				type = odb::INTEGER;
				break;

			case DATATYPE_BITFIELD:
				type = odb::BITFIELD;
				break;

			default:
				Log::error() << "Unsupported type: [" << pci->type_name << "] " << name
							<< endl;
				break;
		}
		setColumn(i, name, type, missing);
	}
	getSchema(db_).updateBitfieldsDefs(columns());

	Log::info() << " <= ODBIterator::createColumns: " << endl;
}

void ODBIterator::destroy()
{
	Log::info() << "ODBIterator::destroy: @" << this << endl;
	odbdump_destroy_colinfo( (colinfo_t *) ci_, noOfColumns_); 
	odbdump_close(odbHandle_);
	delete columns_;
	delete [] data_;
}

ODBIterator::~ODBIterator ()
{
	Log::info() << "ODBIterator::~ODBIterator: @" << this << endl;
	destroy();
}

odb::MetaData& ODBIterator::columns() { return *columns_; }

double* ODBIterator::data() { return data_; }

bool ODBIterator::isNewDataset() { return newDataset_; }

int ODBIterator::setColumn(unsigned long index, std::string& name, odb::ColumnType type, double missingValue)
{
	//Log::debug() << "ODBIterator::setColumn: " << index << ", " << name << ", " << columnTypeName(type) << ", " << missingValue << endl;

	ASSERT(int(index) < noOfColumns_);
	ASSERT(columns_);
	odb::Column* col = (*columns_)[index];
	delete col;
	col = new odb::Column(*columns_);
	ASSERT(col);

	col->name(name); 
	col->type<DataStream<SameByteOrder, DataHandle> >(type, false);
	col->missingValue(missingValue);

	(*columns_)[index] = col;
	return 0;
}


//const char* defaultSQL = "select {!/LINK/} from hdr,body,errstat";
PathName ODBIterator::schemaFile(const PathName db)
{
	std::string d = db;

	if (d[d.size() - 1] != '/')
		d += "//";

	std::string s = Tool::split(".", Tool::split("//", d).back())[0];

	return d + s + ".sch";
}

const odb::sql::SchemaAnalyzer& ODBIterator::getSchema(const PathName db)
{
	//odb::sql::SQLSelectFactory::instance().config(); //odb::sql::SQLOutputConfig());
	// FIXME: this is not good cause when you call the method several times with different db it will fail...
	// It's not used like that at the moment, anyway. So perhaps I should make db a member (db_).
	if (!schemaParsed_)
	{
		PathName schemaFile = this->schemaFile(db);
		Log::info() << "ImportODBTool::getSchema: parsing '" << schemaFile << "'" << endl;
	
		odb::sql::SQLParser p;
		p.parseFile(schemaFile, static_cast<DataHandle*>(0));
		schemaParsed_ = true;
	}

	return odb::sql::SQLSession::current()
			.currentDatabase()
			.schemaAnalyzer();
}

std::string ODBIterator::defaultSQL(const PathName db)
{
	return getSchema(db).generateSELECT();
}

} // namespace tool 
} //namespace odb 
