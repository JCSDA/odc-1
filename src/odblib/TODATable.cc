/*
 * © Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "odblib/oda.h"

#include "odblib/ODAColumn.h"
#include "odblib/SQLDatabase.h"
#include "odblib/SQLType.h"
#include "odblib/TODATable.h"


#define SRC __FILE__,__LINE__

namespace odb {
namespace sql {

template <typename T>
TODATable<T>::TODATable(SQLDatabase& owner, const PathName& path, const string& name):
	SQLTable(owner, path, name),
	data_(0),
	oda_(path),
	reader_(oda_.begin()),
    end_(oda_.end())
{
	populateMetaData();
}

template <typename T>
TODATable<T>::~TODATable() { delete[] data_; }

static const PathName nullPathName("<>");
static const string inputTable("input");

template <typename T>
TODATable<T>::TODATable(SQLDatabase& owner, DataHandle &dh):
	SQLTable(owner, nullPathName, inputTable),
	data_(0),
	oda_(dh),
	reader_(oda_.begin()),
    end_(oda_.end())
{
	populateMetaData();
}

template <typename T>
TODATable<T>::TODATable(SQLDatabase& owner, istream &is):
	SQLTable(owner, nullPathName, inputTable),
	data_(0),
	oda_(is),
	reader_(oda_.begin()),
    end_(oda_.end())
{
	populateMetaData();
}

template <typename T>
void TODATable<T>::populateMetaData()
{
	Log::debug() << "TODATable::populateMetaData:" << endl;
	size_t count = reader_->columns().size();

	delete[] data_;
	data_ = new double[count];
	ASSERT(data_);

	for(size_t i = 0; i < count; i++)
	{
		Column& column = *reader_->columns()[i];

		const string name = column.name();
		bool hasMissing = column.hasMissing();
		double missing = column.missingValue();
		BitfieldDef bitfieldDef = column.bitfieldDef();
	
		string sqlType;
		switch(column.type())
		{
			case INTEGER: sqlType = "integer"; break;
			case STRING:  sqlType = "string"; break;
			case REAL:    sqlType = "real"; break;
			case DOUBLE:  sqlType = "double"; break;
			case BITFIELD:
				{
					string typeSignature = type::SQLBitfield::make("Bitfield", bitfieldDef.first, bitfieldDef.second, "DummyTypeAlias");
					addColumn(name, i, type::SQLType::lookup(typeSignature), hasMissing, missing, true, bitfieldDef);
					continue;
				}
				break;
			default:
				ASSERT("Unknown type" && 1==0);
				break;
		}
		SQLColumn *c = new ODAColumn(type::SQLType::lookup(sqlType), *this, name, i,
							hasMissing, missing, column.type() == BITFIELD, bitfieldDef, &data_[i]);
		addColumn(c, name, i);
	}
}

template <typename T>
void TODATable<T>::updateMetaData(const vector<SQLColumn*>& selected)
{
	Log::debug() << "ODATableIterator::updateMetaData: " << endl;
	for(size_t i = 0; i < selected.size(); i++)
	{
		ODAColumn *c = dynamic_cast<ODAColumn *>(selected[i]);
		ASSERT(c);
		if (reader_->columns()[c->index()]->name() != c->name()) 
		{
			Log::warning() << "Column '" << c->fullName() << "': index has changed in new dataset." << endl;
			Log::warning() << "Was: " << c->index() << "." << endl;
			bool newIndexFound = false;
			for (size_t j = 0; j < reader_->columns().size(); ++j)
			{
				Column &other = *reader_->columns()[j];
				if (other.name() == c->name() || other.name() == c->fullName())
				{
					newIndexFound = true;
					Log::warning() << "New index: " << j << endl;
					c->index(j);
					break;
				}
			}
			ASSERT("One of selected columns does not exist in new data set." && newIndexFound);
		}
		//c->value(&data_[i]);
	}
}


template <typename T>
SQLColumn* TODATable<T>::createSQLColumn(const type::SQLType& type, const string& name, int index, bool hasMissingValue, double
missingValue, bool isBitfield, const BitfieldDef& bitfieldDef)
{
	return new ODAColumn(type, *this, name, index, hasMissingValue, missingValue, isBitfield, bitfieldDef, &data_[index]);
}


template <typename T>
bool TODATable<T>::hasColumn(const string& name, string* fullName)
{
	if (SQLTable::hasColumn(name))
	{
		if (fullName)
		{
			*fullName = name;
			Log::debug() << "TODATable<T>::hasColumn: name='" << name << "', fullName='" << *fullName << "'" << endl;
		}
		return true;
	}

	string colName = name + "@";

	int n = 0;
	map<string,SQLColumn*>::iterator it = columnsByName_.begin();
	for ( ; it != columnsByName_.end(); ++it)
	{
		string s = it->first;
		if (s.find(colName) == 0)
		{
			n++;
			if (fullName)
			{
				*fullName = s;
				Log::debug() << "TODATable<T>::hasColumn: colName='" << colName << "', fullName='" << *fullName << "'" << endl;
			}
		}
	}

	if (n == 0) return false;
	if (n == 1) return true;

	throw UserError(string("TODATable:hasColumn(\"") + name + "\"): ambiguous name");

	return false;
}

template <typename T>
SQLColumn* TODATable<T>::column(const string& name)
{
	string colName = name + "@";

	SQLColumn * column = 0;
	map<string,SQLColumn*>::iterator it = columnsByName_.begin();
	for ( ; it != columnsByName_.end(); ++it)
	{
		string s = it->first;
		if (s.find(colName) == 0)
		{
			if (column)
				throw UserError(string("TODATable::column: \"") + name + "\": ambiguous name.");
			else 
				column = it->second;
		}
	}
	if (column) return column;

	return SQLTable::column(name);
}

template <typename T>
SQLTableIterator* TODATable<T>::iterator(const vector<SQLColumn*>& x) const
{
	return new TableIterator(const_cast<TODATable&>(*this), reader_, end_, const_cast<double *>(data_), x);
}

} // namespace sql
} // namespace odb