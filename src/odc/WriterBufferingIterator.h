/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// \file WriterBufferingIterator.h
///
/// @author Piotr Kuchta, August 2009

#ifndef odc_WriterBufferingIterator_H
#define odc_WriterBufferingIterator_H

#include "odc/Array.h"
#include "odc/CodecOptimizer.h"
#include "odc/Header.h"
#include "odc/IteratorProxy.h"
#include "odc/MemoryBlock.h"

namespace eckit { class PathName; }
namespace eckit { class DataHandle; }

namespace odc {

template <typename I> class Writer;
namespace sql { class TableDef; }

class WriterBufferingIterator 
{
public:
	typedef Writer<WriterBufferingIterator> Owner;

	//WriterBufferingIterator (Owner &owner, eckit::DataHandle *, bool openDataHandle=true);
	WriterBufferingIterator (Owner &owner, eckit::DataHandle *, bool openDataHandle, const odc::sql::TableDef* tableDef=0);

	~WriterBufferingIterator();

	int open();

	double* data();
	double& data(size_t i);

	int setColumn(size_t index, std::string name, ColumnType type);
    int setBitfieldColumn(size_t index, std::string name, ColumnType type, eckit::sql::BitfieldDef b);

	void missingValue(size_t i, double); 

	template <typename T> unsigned long pass1(T&, const T&);
	unsigned long gatherStats(const double* values, unsigned long count);

	int close();

    const MetaData& columns() const { return columns_; }
    const MetaData& columns(const MetaData& md) {
        columns_ = md;
        initialisedColumns_ = columns_.allColumnsInitialised();
        return columns_;
    }

    void setNumberOfColumns(size_t n) { columns_.setSize(n); }

	Owner& owner() { return owner_; }

	eckit::DataHandle& dataHandle() { return *f; }

	void property(std::string key, std::string value) { properties_[key] = value; }

    /// The offset of a given column in the doubles[] data array
    size_t dataOffset(size_t i) const { ASSERT(columnOffsets_); return columnOffsets_[i]; }

//protected:

	template <typename DATASTREAM> int setOptimalCodecs();

	void writeHeader();

	int writeRow(const double* values, unsigned long count);

    // Get the number of doubles per row.
    size_t rowDataSizeDoubles() const { return rowDataSizeDoubles_; }

	size_t rowsBufferSize() { return rowsBufferSize_; }
	void rowsBufferSize(size_t n) { rowsBufferSize_ = n; }

	void flush();

    std::vector<eckit::PathName> outputFiles();
	int refCount_;
    bool next();

private:
    size_t rowDataSizeDoublesInternal() const;
protected:
	Owner& owner_;
	MetaData columns_;
	double* lastValues_;
	double* nextRow_;
    size_t* columnOffsets_; // in doubles
    size_t* columnByteSizes_;
	unsigned long long nrows_;

	eckit::DataHandle *f;
	Array<unsigned char> encodedDataBuffer_;
    eckit::PathName path_;

	unsigned char* writeNumberOfRepeatedValues(unsigned char *, uint16_t);

private:
// No copy allowed.
	WriterBufferingIterator(const WriterBufferingIterator&);
	WriterBufferingIterator& operator=(const WriterBufferingIterator&);

	template <typename T> void pass1init(T&, const T&);

	template <typename T> void doWriteHeader(T&, size_t, size_t);


	void allocBuffers();
	void allocRowsBuffer();
	void resetColumnsBuffer();

	int doWriteRow(const double*, unsigned long);

    bool initialisedColumns_;
	Properties properties_;

	Array<unsigned char> blockBuffer_;
	Array<unsigned char> rowsBuffer_;
	unsigned char* nextRowInBuffer_;
	FastInMemoryDataHandle memoryDataHandle_;
	MetaData columnsBuffer_;

	size_t rowsBufferSize_;
    size_t rowDataSizeDoubles_;;
	MemoryBlock setvBuffer_;
	size_t maxAnticipatedHeaderSize_;

	codec::CodecOptimizer codecOptimizer_;

    const odc::sql::TableDef* tableDef_;

private:
    bool openDataHandle_;

	friend class IteratorProxy<WriterBufferingIterator, Owner>;
	friend class Header<WriterBufferingIterator>;
};

template<typename T>
void WriterBufferingIterator::pass1init(T& it, const T& end)
{
	eckit::Log::debug() << "WriterBufferingIterator::pass1init" << std::endl;

	// Copy columns from the input iterator.
	columns(columnsBuffer_ = it->columns());

	columns_.resetStats();
	columnsBuffer_.resetStats();
	
	size_t nCols = it->columns().size();
	ASSERT(nCols > 0);

	allocRowsBuffer();
}

template<typename T>
unsigned long WriterBufferingIterator::pass1(T& it, const T& end)
{
	eckit::Log::debug() << "WriterBufferingIterator::pass1" << std::endl;

	pass1init(it, end);
    writeHeader();

	unsigned long nrows = 0;
	for ( ; it != end; ++it, ++nrows)
	{
		if (it->isNewDataset() && it->columns() != columnsBuffer_)
		{
			eckit::Log::debug() << "WriterBufferingIterator::pass1: Change of input metadata." << std::endl;
			flush();
			pass1init(it, end);
            writeHeader();
        }

		const double *data = it->data();
		size_t nCols = it->columns().size();

		gatherStats(data, nCols);

        std::copy(data, data + nCols, reinterpret_cast<double*>(nextRowInBuffer_ + sizeof(uint16_t)));
		nextRowInBuffer_ += sizeof(uint16_t) + nCols * sizeof(double);

		ASSERT(nextRowInBuffer_ <= rowsBuffer_ + rowsBuffer_.size());
		if (nextRowInBuffer_ == rowsBuffer_ + rowsBuffer_.size())
			flush();
	} 

	eckit::Log::debug() << "Flushing rest of the buffer..." << std::endl;
	flush();

	eckit::Log::debug() << "WriterBufferingIterator::pass1: processed " << nrows << " row(s)." << std::endl;
	ASSERT(close() == 0);
	return nrows;
}

} // namespace odc 

#endif