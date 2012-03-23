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
/// \file Writer.cc
///
/// @author Piotr Kuchta, Feb 2009


#include <algorithm>
#include <iostream>
#include <sstream>
#include <errno.h>
#include <math.h>
#include <values.h>

using namespace std;

#include "eclib/DataHandle.h"
#include "eclib/Exceptions.h"
#include "eclib/FileHandle.h"
#include "eclib/PathName.h"


#include "odblib/Codec.h"
#include "odblib/Column.h"
#include "odblib/DataStream.h"
#include "odblib/FixedSizeWriterIterator.h"
#include "odblib/HashTable.h"
#include "odblib/Header.h"
#include "odblib/IteratorProxy.h"
#include "odblib/MemoryBlock.h"
#include "odblib/MetaData.h"
#include "odblib/Reader.h"
#include "odblib/ReaderIterator.h"
#include "odblib/RowsIterator.h"
#include "odblib/SQLBitfield.h"
#include "odblib/SQLExpression.h"
#include "odblib/SQLInteractiveSession.h"
#include "odblib/SQLIteratorSession.h"
#include "odblib/SQLParser.h"
#include "odblib/SQLSelect.h"
#include "odblib/SQLTable.h"
#include "odblib/SQLType.h"
#include "odblib/SelectIterator.h"
#include "odblib/Writer.h"
#include "odblib/WriterBufferingIterator.h"
#include "odblib/WriterBufferingIterator.h"

using namespace std;

namespace odb {
#define MEGA(x) (x*1024*1024)

template <typename ITERATOR>
Writer<ITERATOR>::Writer()
: path_(""),
  dataHandle_(0),
  rowsBufferSize_(Resource<long>("$ODB_ROWS_BUFFER_SIZE;-rowsBufferSize;rowsBufferSize", DEFAULT_ROWS_BUFFER_SIZE)),
  openDataHandle_(true),
  deleteDataHandle_(true)
{} 

template <typename ITERATOR>
Writer<ITERATOR>::Writer(const PathName path)
: path_(path),
  dataHandle_(0),
  rowsBufferSize_(Resource<long>("$ODB_ROWS_BUFFER_SIZE;-rowsBufferSize;rowsBufferSize", DEFAULT_ROWS_BUFFER_SIZE)),
  openDataHandle_(true),
  deleteDataHandle_(true)
{} 

template <typename ITERATOR>
Writer<ITERATOR>::Writer(DataHandle *dh, bool openDataHandle, bool deleteDataHandle)
: path_(""),
  dataHandle_(dh),
  rowsBufferSize_(Resource<long>("$ODB_ROWS_BUFFER_SIZE;-rowsBufferSize;rowsBufferSize", DEFAULT_ROWS_BUFFER_SIZE)),
  openDataHandle_(openDataHandle),
  deleteDataHandle_(deleteDataHandle)
{}

template <typename ITERATOR>
Writer<ITERATOR>::Writer(DataHandle &dh, bool openDataHandle)
: path_(""),
  dataHandle_(&dh),
  rowsBufferSize_(Resource<long>("$ODB_ROWS_BUFFER_SIZE;-rowsBufferSize;rowsBufferSize", DEFAULT_ROWS_BUFFER_SIZE)),
  openDataHandle_(openDataHandle),
  deleteDataHandle_(false)
{}

template <typename ITERATOR>
Writer<ITERATOR>::~Writer() { if (deleteDataHandle_) delete dataHandle_; }

template <typename ITERATOR>
ITERATOR* Writer<ITERATOR>::writer(bool fixedSizeRows)
{
	if (string(path_).size())
	{
		FileHandle *fh = new FileHandle(path_);
		return fixedSizeRows ?
			new FixedSizeWriterIterator(*this, fh)
			: new ITERATOR(*this, fh);
	}
		
	ASSERT(dataHandle_);	

	// TODO: check what iterators do with the data handle (closing, deleting).
	return fixedSizeRows ?
		new FixedSizeWriterIterator(*this, dataHandle_)
		: new ITERATOR(*this, dataHandle_);
}

template <typename ITERATOR>
typename Writer<ITERATOR>::iterator Writer<ITERATOR>::begin(bool openDataHandle)
{
	DataHandle *dh = 0;
	if (string(path_).size())
		dh = new FileHandle(path_);
	else
	{
		ASSERT(dataHandle_);
		dh = dataHandle_;
	}
	return typename Writer::iterator(new ITERATOR(*this, dh, openDataHandle));
}

template <typename ITERATOR>
ITERATOR* Writer<ITERATOR>::createWriteIterator(PathName pathName, bool append)
{
	DataHandle *h = new FileHandle(pathName);
	if (append)
	{
		Length estimatedLength = 10*1024*1024;
		h->openForAppend(estimatedLength);
	}
	return new ITERATOR(*this, h, !append);
}

// Explicit templates' instantiations.

template Writer<WriterBufferingIterator>::Writer();
template Writer<WriterBufferingIterator>::Writer(PathName);
template Writer<WriterBufferingIterator>::Writer(DataHandle&,bool);
template Writer<WriterBufferingIterator>::Writer(DataHandle*,bool,bool);

template Writer<WriterBufferingIterator>::~Writer();
template Writer<WriterBufferingIterator>::iterator Writer<WriterBufferingIterator>::begin(bool);
template WriterBufferingIterator * Writer<WriterBufferingIterator>::createWriteIterator(PathName,bool);

} // namespace odb