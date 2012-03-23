/*
 * © Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eclib/Log.h"
#include "eclib/Translator.h"

#include "odblib/Codec.h"
#include "odblib/DataStream.h"
#include "odblib/HashTable.h"
#include "odblib/SQLAST.h"
#include "odblib/SQLBitfield.h"
#include "odblib/SQLCreateTable.h"
#include "odblib/SQLDatabase.h"
#include "odblib/SQLSession.h"
#include "odblib/SQLType.h"
#include "odblib/SchemaAnalyzer.h"

namespace odb {
namespace sql {

SQLCreateTable::SQLCreateTable(string tableName, ColumnDefs &cols)
: tableName_(tableName), cols_(cols)
{}

SQLCreateTable::~SQLCreateTable() {}

void SQLCreateTable::print(ostream& s) const
{
}

unsigned long long SQLCreateTable::execute()
{
	SQLDatabase &db = SQLSession::current().currentDatabase();
	// SQLTable::SQLTable(SQLDatabase& owner,const PathName& path,const string& name,int no_rows,int index)
	// TODO:
	SQLTable *table = NULL; //new SQLTable(db, tableName_ /*?*/, tableName_, /*no_rows (columns, really..)*/ 0, /*index*/ 0);

	int index = 1;
	for (ColumnDefs::const_iterator i = cols_.begin(); i != cols_.end(); i++)
	{
		const string columnName = i->name();
		const string typeName = i->type();

		const long start = i->range().first;
		const long end = i->range().second;
		bool isVector = ! (start == 0 && end == 0); // e.g: colname[1:3] pk1real

		//cout << " ==== columnName: " << columnName << ", typeName: " << typeName;
		//if (isVector) cout << "[" << start << ":" << end << "]";
		//cout << endl;

		//vector<string> bitmap;
		BitfieldDef bitfieldDef;

		if (typeName == "@LINK")
		{
			ASSERT(! isVector);

			db.links()[tableName_].insert(columnName);

			const type::SQLType& intType = type::SQLType::lookup("pk5int");
			table->addColumn(columnName + ".offset", index++, intType, false, MISSING_VALUE_INT, false, bitfieldDef);
			table->addColumn(columnName + ".length", index++, intType, false, MISSING_VALUE_INT, false, bitfieldDef);
		}
		else
		{
			const type::SQLType& typ = type::SQLType::lookup(typeName);
			bool isBitmap = typ.getKind() == type::SQLType::bitmapType;
			if (isBitmap)
				//bitmap = static_cast<const type::SQLBitfield&>(typ).fields();
				bitfieldDef = static_cast<const type::SQLBitfield&>(typ).bitfieldDef();

			if (!isVector)
				// FIXME: no information about missing value in the CREATE TABLE syntax
				// TODO: choose a sensible default based upon type
				table->addColumn(columnName, index++, typ, true, MISSING_VALUE_REAL, isBitmap, bitfieldDef);
			else
			{
				for (int i = start; i <= end; i++)
				{
					string expandedName = columnName + "_" + Translator<int,string>()(i);
					Log::debug() << "  === expandedName: " << expandedName << endl;
					// TODO: choose a sensible default based upon type
					table->addColumn(expandedName, index++, typ, true, MISSING_VALUE_REAL, isBitmap, bitfieldDef);
				}
			}
		}
	}
	db.addTable(table);
	//db.setLinks(links);
	return 0;
}

} // namespace sql
} // namespace odb
