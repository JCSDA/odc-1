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
/// \file Header.cc
///
/// @author Piotr Kuchta, Feb 2009

#include "odblib/DataStream.h"
#include "odblib/Header.h"
#include "odblib/InMemoryDataHandle.h"
#include "odblib/MD5.h"

namespace odb {

template <typename OWNER>
Header<OWNER>::Header(OWNER& owner)
: owner_(owner),
  dataSize_(0),
  rowsNumber_(0)
{} 

template <typename OWNER>
Header<OWNER>::~Header ()
{}

template <typename OWNER>
void Header<OWNER>::load()
{
	DataStream<SameByteOrder> f(owner_.f);

	uint16_t c;
	f.readUInt16(c);
	ASSERT(c == ODA_MAGIC_NUMBER);

	unsigned char cc;
	f.readUChar(cc); ASSERT(cc == 'O');
	f.readUChar(cc); ASSERT(cc == 'D');
	f.readUChar(cc); ASSERT(cc == 'A');

	loadAfterMagic();
}

template <typename OWNER>
void Header<OWNER>::loadAfterMagic()
{
	DataStream<SameByteOrder> f(owner_.f);

	int32_t byteOrder;
	f.readInt32(byteOrder);

	if (byteOrder != BYTE_ORDER_INDICATOR)
	{
		DataStream<OtherByteOrder> ds(owner_.f);
		load(ds);
	}
	else
	{
		DataStream<SameByteOrder> ds(owner_.f);
		load(ds);
	}
}

template <typename OWNER>
template <typename DATASTREAM>
void Header<OWNER>::load(DATASTREAM &ff)
{
	int32_t formatVersionMajor;
	ff.readInt32(formatVersionMajor);
	ASSERT("File format version not supported" && formatVersionMajor <= FORMAT_VERSION_NUMBER_MAJOR);

	int32_t formatVersionMinor;
	ff.readInt32(formatVersionMinor);
	ASSERT("File format version not supported" && formatVersionMinor <= FORMAT_VERSION_NUMBER_MINOR && formatVersionMinor > 3);

	string headerDigest; 
	ff.readString(headerDigest);

	MemoryBlock buffer(0);
	ff.readBuffer(buffer);

	MD5 md5;
	md5.add(buffer, buffer.size());
	string actualHeaderDigest = md5.digest();

	if (! (headerDigest == actualHeaderDigest))
	{
		Log::info() << "headerDigest(" << headerDigest.size() << "):       '" << headerDigest << "'" << endl;
		Log::info() << "actualHeaderDigest(" << actualHeaderDigest.size() << "): '" << actualHeaderDigest << "'" << endl;
		ASSERT(headerDigest == actualHeaderDigest);
	}
	
	PrettyFastInMemoryDataHandle memoryHandle(buffer);
	DataStream<typename DATASTREAM::ByteOrderType, PrettyFastInMemoryDataHandle> f(memoryHandle);

	// 0 means we don't know offset of next header.
	int64_t nextFrameOffset;
	f.readInt64(nextFrameOffset);
	dataSize_ = nextFrameOffset;
	owner_.columns().dataSize(dataSize_);

	// Reserved, not used yet.
	int64_t prevFrameOffset;
	f.readInt64(prevFrameOffset);
	ASSERT(prevFrameOffset == 0);

	// TODO: increase file format version

	int64_t numberOfRows;
	f.readInt64(numberOfRows);
	rowsNumber_ = numberOfRows;
	owner_.columns().rowsNumber(rowsNumber_);

	Log::debug() << "Header<OWNER>::load: numberOfRows = " << numberOfRows << endl;

	// Flags -> ODAFlags
	Flags flags;
	f.readFlags(flags);

	f.readProperties(owner_.properties_);

	owner_.columns().load(f);
}

template <typename OWNER>
template <typename DATAHANDLE>
void Header<OWNER>::save(DATAHANDLE &dh)
{
	DataStream<SameByteOrder, DATAHANDLE> ff(dh);

	// Header.
	uint16_t c = ODA_MAGIC_NUMBER;
	ff.writeUInt16(c);

	c = 'O'; ff.writeChar(c); 
	c = 'D'; ff.writeChar(c); 
	c = 'A'; ff.writeChar(c);

	int32_t byteOrderIndicator = 1;
	ff.writeInt32(byteOrderIndicator);

	ff.writeInt32(FORMAT_VERSION_NUMBER_MAJOR);
	ff.writeInt32(FORMAT_VERSION_NUMBER_MINOR);
	
	InMemoryDataHandle memoryHandle;
	DataStream<SameByteOrder> f(memoryHandle);

	// Reserved.	
	int64_t nextFrameOffset = dataSize_;
	f.writeInt64(nextFrameOffset);

	// Reserved.	
	int64_t prevFrameOffset = 0;
	f.writeInt64(prevFrameOffset);

	int64_t numberOfRows = rowsNumber_;
	f.writeInt64(numberOfRows);

	Flags flags(10, 0);
	f.writeFlags(flags);

	f.writeProperties(owner_.properties_);

	owner_.columns().save(f);

	Length len = memoryHandle.openForRead();

	MemoryBlock buffer(len);
	Length readBytes = memoryHandle.read(buffer, len);
	ASSERT(len == readBytes);

	MD5 md5;
	md5.add(buffer, len);
	string headerDigest = md5.digest();
	ff.writeString(headerDigest);

	ff.writeBuffer(buffer);
}

} //namespace odb 
