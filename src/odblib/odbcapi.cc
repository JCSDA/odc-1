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
/// \file odbcapi.cc
///
/// @author Piotr Kuchta, March 2009

#include "eclib/Application.h"

#include "odblib/oda.h"
#include "odblib/MetaDataReaderIterator.h"
#include "odblib/MetaDataReader.h"
#include "odblib/FastODA2Request.h"

using namespace odb;

char *dummyAppsArgs[] = { const_cast<char*>("odbcapi"), 0 };

class DummyApplication : public Application {
public:
	DummyApplication() : Application(1, dummyAppsArgs)
	{
		Log::info() << "Eclib initialised." << endl;
	}
	void run() {}
};

#include "odbcapi.h"

extern "C" {

void odb_start()
{
	static DummyApplication * dummyApplication = 0;

	if (dummyApplication == 0)
		dummyApplication = new DummyApplication;
}

double odb_count(const char * filename)
{
	double n = 0;

	PathName path = filename;
	MetaDataReader mdReader(path);
	MetaDataReader::iterator it = mdReader.begin();
	MetaDataReader::iterator end = mdReader.end();
	for (; it != end; ++it)
	{
		MetaData &md = it->columns();
		n += md.rowsNumber();
	}
	return n;
}

int get_blocks_offsets(const char* fileName, size_t* numberOfBlocks,  off64_t** offsets, size_t** sizes)
{
	FastODA2Request<ODA2RequestClientTraits> o;
	o.mergeSimilarBlocks(false);

	OffsetList offs;
	LengthList lengths;
	vector<ODAHandle*> handles;

	o.scanFile(fileName, offs, lengths, handles);

	ASSERT(offs.size() == lengths.size());
	ASSERT(offs.size() == handles.size());

	size_t n = offs.size();

	*numberOfBlocks = n;
	*offsets = new off64_t[n];
	*sizes = new size_t[n];
	
	for (size_t i = 0; i < n; ++i)
	{
		(*offsets)[i] = offs[i];
		(*sizes)[i] = lengths[i];
		delete handles[i];
	}

	return 0;
}

int release_blocks_offsets(off64_t** offsets) { delete [] *offsets; *offsets = 0; return 0; }
int release_blocks_sizes(size_t** sizes) { delete [] *sizes; *sizes = 0; return 0; }

int filter_in_place(char *buffer, size_t bufferLength, size_t* filteredLength, const char *sql)
{
	MemoryBlock mbIn(buffer, bufferLength);
	InMemoryDataHandle dhIn(mbIn);
	odb::Select oda(sql, dhIn);

	InMemoryDataHandle dhOut;
	odb::Writer<> writer(dhOut);
	odb::Writer<>::iterator outit = writer.begin();
	outit->pass1(oda.begin(), oda.end());

	*filteredLength = dhOut.position();
	Length len = dhOut.openForRead();
	if(*filteredLength != len)
		return 99; // should not happen

	if (bufferLength < *filteredLength)
		return 1;

	dhOut.read(buffer, *filteredLength);

	return 0;
}

unsigned int odb_get_headerBufferSize() { return ODBAPISettings::instance().headerBufferSize(); } 
void odb_set_headerBufferSize(unsigned int n) { ODBAPISettings::instance().headerBufferSize(n); }

unsigned int odb_get_setvbufferSize() { return ODBAPISettings::instance().setvbufferSize(); } 
void odb_set_setvbufferSize(unsigned int n) { ODBAPISettings::instance().setvbufferSize(n); }

const char* odb_api_version() { return odb::ODBAPIVersion::version(); }
unsigned int odb_api_format_version_major() { return odb::ODBAPIVersion::formatVersionMajor(); }
unsigned int odb_api_format_version_minor() { return odb::ODBAPIVersion::formatVersionMinor(); }

/// @param config  ignored for now.
oda_ptr odb_create(const char *config, int *err)
{
	Reader* o = new Reader;
	*err = !o;
	return oda_ptr(o);
}

/// @param config  ignored for now.
oda_writer_ptr odb_writer_create(const char *config, int *err)
{
	//PathName path = filename;
	Writer<>* o = new Writer<>; //(path);
	*err = !o;
	return oda_writer_ptr(o);
}

int odb_destroy(oda_ptr o)
{
	delete reinterpret_cast<Reader*>(o);
	return 0;
}

int odb_writer_destroy(oda_writer_ptr o)
{
	delete reinterpret_cast<Writer<> *>(o);
	return 0;
}

oda_read_iterator_ptr odb_create_read_iterator(oda_ptr co, const char *filename, int *err)
{
	Reader *o = reinterpret_cast<Reader*>(co);
	string fileName = filename;
	PathName fn(fileName);
	if (! fn.exists())
	{
		*err = 2; //TODO: define error codes
		return 0;
	}
	
	ReaderIterator* iter = o->createReadIterator(fn);
	*err = !iter;
	return oda_read_iterator_ptr(iter);
	
}

oda_select_iterator_ptr odb_create_select_iterator(oda_ptr co, const char *sql, int *err)
{
	Select *o = reinterpret_cast<Select*>(co);
	
	SelectIterator* iter = o->createSelectIterator(sql);
	*err = !iter;
	return oda_select_iterator_ptr(iter);
	
}

oda_select_iterator_ptr odb_create_select_iterator_from_file(oda_ptr co, const char *sql, const char *filename, int *err)
{
    Select *o = reinterpret_cast<Select*>(co);

    string full_sql = string(sql) + " from \"" + string(filename) + "\"";

    SelectIterator* iter = o->createSelectIterator(full_sql);
    *err = !iter;
    return oda_select_iterator_ptr(iter);

}


int odb_read_iterator_destroy(oda_read_iterator_ptr it)
{
	delete reinterpret_cast<ReaderIterator*>(it);
	return 0;
}

int odb_select_iterator_destroy(oda_select_iterator_ptr it)
{
	delete reinterpret_cast<SelectIterator*>(it);
	return 0;
}

int odb_read_iterator_get_no_of_columns(oda_read_iterator_ptr it, int *numberOfColumns)
{
	ReaderIterator* iter = reinterpret_cast<ReaderIterator*>(it);
	*numberOfColumns = iter->columns().size();
	return 0;
}

int odb_select_iterator_get_no_of_columns(oda_select_iterator_ptr it, int *numberOfColumns)
{
	SelectIterator* iter = reinterpret_cast<SelectIterator*>(it);
	*numberOfColumns = iter->columns().size();
	return 0;
}

int odb_read_iterator_get_column_type(oda_read_iterator_ptr it, int n, int *type)
{
	ReaderIterator* iter = reinterpret_cast<ReaderIterator*>(it);
	*type = iter->columns()[n]->type();
	return 0;
}

int odb_select_iterator_get_column_type(oda_select_iterator_ptr it, int n, int *type)
{
	SelectIterator* iter = reinterpret_cast<SelectIterator*>(it);
	*type = iter->columns()[n]->type();
	return 0;
}

int odb_read_iterator_get_column_name(oda_read_iterator_ptr it, int n, char **name, int *size_name)
{
	ReaderIterator* iter = reinterpret_cast<ReaderIterator*>(it);
	*name = const_cast<char*>(iter->columns()[n]->name().c_str());
	*size_name = iter->columns()[n]->name().length();
    return 0;
}

int odb_select_iterator_get_column_name(oda_select_iterator_ptr it, int n, char **name, int *size_name)
{
	SelectIterator* iter = reinterpret_cast<SelectIterator*>(it);
	*name = const_cast<char*>(iter->columns()[n]->name().c_str());
	*size_name = iter->columns()[n]->name().length();
     return 0;
}

int odb_read_iterator_get_next_row(oda_read_iterator_ptr it, int count, double* data, int *new_dataset)
{
	ReaderIterator* iter = reinterpret_cast<ReaderIterator*>(it);
	if (! iter->next())
		return 1;

	if (iter->isNewDataset())
		*new_dataset = 1;
	else
		*new_dataset = 0;

	if (count != static_cast<int>(iter->columns().size()))
		return 2; // TDOO: define error codes

	for (int i = 0; i < count; ++i)
		data[i] = iter->data()[i];

	return 0;
}

int odb_select_iterator_get_next_row(oda_select_iterator_ptr it, int count, double* data, int *new_dataset)
{
	SelectIterator* iter = reinterpret_cast<SelectIterator*>(it);
	if (! iter->next())
		return 1;

	if (iter->isNewDataset())
		*new_dataset = 1;
	else
		*new_dataset = 0;

	if (count != static_cast<int>(iter->columns().size()))
		return 2; // TDOO: define error codes

	for (int i = 0; i < count; ++i)
		data[i] = iter->data()[i];

	return 0;
}

oda_write_iterator_ptr odb_create_write_iterator(oda_ptr co, const char *filename, int *err)
{
	Writer<> *o = reinterpret_cast<Writer<> *>(co);
	string fileName(filename);
	PathName fn(fileName);
	FileHandle *fh = new FileHandle(fn);

	// TODO: make sure there's no leaks (FileHandle)
	Writer<>::iterator_class* w = new Writer<>::iterator_class(*o, fh);
	*err = !w;
	return oda_write_iterator_ptr(w);
}

int odb_write_iterator_destroy(oda_write_iterator_ptr wi)
{
	delete reinterpret_cast<Writer<>::iterator_class *>(wi);
	return 0;
}

int odb_write_iterator_set_no_of_columns(oda_write_iterator_ptr wi, int n)
{
	Writer<>::iterator_class *w = reinterpret_cast<Writer<>::iterator_class *>(wi);
	w->columns().setSize(n);
	return 0;
}

int odb_write_iterator_set_column(oda_write_iterator_ptr wi, int index, int type, const char *name)
{
	Writer<>::iterator_class * w = reinterpret_cast<Writer<>::iterator_class *>(wi);
	std::string columnName(name);
	
	return w->setColumn(index, columnName, ColumnType(type));
}

int odb_write_iterator_set_bitfield(oda_write_iterator_ptr wi, int index, int type, const char *name, const char* bitfieldNames, const char *bitfieldSizes)
{
	string bnames = bitfieldNames;
    string bsizes = bitfieldSizes;
    odb::FieldNames bitfield_names;
    odb::Sizes      bitfield_sizes;
 
//	cout << " columnName = " << name << " " << bnames << " " << bsizes << endl;
	size_t iprev = 0;
	for (size_t i = 0; i < bnames.size(); i++) {
		if (bnames[i] == ':') {
			string name = bnames.substr(iprev,i-iprev);
			iprev = i+1;    
			bitfield_names.push_back(name);
		}
	}
    iprev = 0;
	for (size_t i = 0; i < bsizes.size(); i++) {
		if (bsizes[i] == ':') {
			string name = bsizes.substr(iprev,i-iprev);
			size_t size = atof(name.c_str()); // bit[0-9]+
			iprev = i+1;    
			bitfield_sizes.push_back(size);
		}
	}

    odb::BitfieldDef bitfieldType;
    bitfieldType = make_pair(bitfield_names,bitfield_sizes);

	Writer<>::iterator_class * w = reinterpret_cast<Writer<>::iterator_class *>(wi);
	std::string columnName(name);
	
	int rc = w->setBitfieldColumn(index, columnName, ColumnType(type), bitfieldType);

	return rc;

}

int odb_write_iterator_set_missing_value(oda_write_iterator_ptr wi, int index, double value)
{
	Writer<>::iterator_class * w = reinterpret_cast<Writer<>::iterator_class *>(wi);
	w->missingValue(index, value);
	return 0;
}

int odb_write_iterator_write_header(oda_write_iterator_ptr wi)
{
	Writer<>::iterator_class * w = reinterpret_cast<Writer<>::iterator_class *>(wi);
	w->writeHeader();
	return 0;
}

int odb_write_iterator_set_next_row(oda_write_iterator_ptr wi, double *data, int count)
{
	return reinterpret_cast<Writer<>::iterator_class *>(wi)->writeRow(data, count);
}

} // extern "C" 
