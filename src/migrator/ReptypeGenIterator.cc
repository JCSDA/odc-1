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
/// \file ReptypeGenIterator.cc
///
/// @author Piotr Kuchta, Feb 2009

#include "Types.h"
#include "Tool.h"
#include "Tokenizer.h"
#include "Translator.h"

#include "oda.h"

#define SRC __FILE__,__LINE__

extern "C" {
#include "odbdump.h"
}

#include "ODBIterator.h"
#include "FakeODBIterator.h"
#include "ReptypeGenIterator.h"

#include <strings.h>

namespace odb {
namespace tool {

ostream& operator<<(ostream& s, const ReptypeTable& m)
{
	s << "{";
	for (ReptypeTable::const_iterator it = m.begin(); it != m.end(); ++it)
	{
		s << "[";
		const Values& vals = it->first;
		for (Values::const_iterator i = vals.begin(); i != vals.end(); ++i)
			s << *i;
		s << "]";

		s << " : " << it->second << "," << endl;
	}
	s << "}";
	return s;
}

/*
Hi Peter,

most of the columns given by Manuel do not exist yet.

As a first attempt, you can use
sensor@hdr,
satname_1@hdr,
satname_2@hdr,
satname_3@hdr,
satname_4@hdr,
bufrtype@hdr,
subtype@hdr,
obstype@hdr,
codetype@hdr
(when sat.len > 0 you can add satid@sat).

Anne. 
*/

vector<std::string> ReptypeTableConfig::columns_ = vector<std::string>();
ReptypeTable ReptypeTableConfig::reptypeTable_ = ReptypeTable();

void ReptypeTableConfig::load(const PathName& fileName)
{
	std::string s = Tool::readFile(fileName, false);
	Log::debug() << "ReptypeTableConfig::load(fileName = '" << fileName << "')" << endl;
	Log::debug() << "ReptypeTableConfig::load(fileName = '" << fileName << "')" << "'" << s << "'" << endl;

    vector<std::string> lines = Tool::split("\n", s);

	size_t i = 0;
	while (lines[i] == "")
		++i;

	vector<std::string> firstLine = Tool::split(":", lines[i]);
	ASSERT(firstLine[0] == "reptype");

	vector<std::string> columnNames = Tool::split(",", firstLine[1]);
	std::for_each(columnNames.begin(), columnNames.end(), Tool::trimInPlace);

	columns_.insert(columns_.end(), columnNames.begin(), columnNames.end());
	++i;

    for (; i < lines.size(); ++i)
    {
        vector<std::string> lr;
        Tokenizer(":")(lines[i], lr);

        // Skip empty lines.
        if (lr.size() == 0) continue;

		ASSERT(lr.size() == 2);

		int reptype_value = Translator<std::string, int>()(lr[0]);

		vector<std::string> rvalues = Tool::split(",", lr[1]);
		//Log::debug() << "ReptypeTableConfig::load: rvalues = " << rvalues << endl;

		ASSERT("Number of values must be equal to number of column names (first line)"
			&& columnNames.size() == rvalues.size());

		std::for_each(rvalues.begin(), rvalues.end(), Tool::trimInPlace);
		Values vals;
		Log::debug() << "ReptypeTableConfig::load: " << reptype_value << " = ";
		for (size_t j = 0; j < rvalues.size(); ++j)
		{
			std::string &vs(rvalues[j]);

			Log::debug() << "{" << vs << ":" << Tool::isInQuotes(vs) << "}";
			double v = Tool::isInQuotes(vs)
				? Tool::cast_as_double(Tool::unQuote(vs))
				: Translator<std::string, double>()(vs);
			vals.push_back(v);
			Log::debug() << "[" << rvalues[j] << "] '" << Tool::double_as_string(v) << "', " << endl;
		}
		Log::debug() << endl;
		//at(vals) = reptype_value;
		reptypeTable_[vals] = reptype_value;
	}

	//Log::debug() << "ReptypeTableConfig::load: columns_ = " << columns_ << endl; 
	//Log::debug() << "ReptypeTableConfig::load: reptypeTable_ = " << reptypeTable_ << endl;
}

template<typename ITERATOR, typename CONFIG>
ReptypeGenIterator<ITERATOR, CONFIG>::ReptypeGenIterator(const PathName& db, const std::string& sql)
: iterator_(db, sql),
  data_(0),
  reptypeTable_(CONFIG::reptypeTable())
{
	odb::MetaData &md = iterator_.columns();
	data_ = new double[md.size()];
	reptypeIndex_ = md.columnIndex("reptype");

	const vector<std::string>& columnNames = CONFIG::columns();
	for (vector<std::string>::const_iterator i = columnNames.begin(); i != columnNames.end(); ++i)
	{
		std::string name = *i;

		Log::debug() << "ReptypeGenIterator<ITERATOR>::ctor: " << name << endl;

		indices_.push_back(iterator_.columns().columnIndex(name));
		values_.push_back(0);
	}
	Log::debug() << "ReptypeGenIterator::ReptypeGenIterator: Reptype table:" << endl;
	Log::debug() << "reptypeTable_ = " << reptypeTable_ << endl;
}

template<typename ITERATOR, typename CONFIG>
ReptypeGenIterator<ITERATOR, CONFIG>::~ReptypeGenIterator()
{
	Log::debug() << "ReptypeGenIterator::~ReptypeGenIterator: Reptype table:" << endl;
	Log::debug() << "reptypeTable_.size() = " << reptypeTable_.size() << endl;
	Log::debug() << "reptypeTable_ =" << reptypeTable_ << endl;
	delete [] data_;
}

template<typename ITERATOR, typename CONFIG>
odb::MetaData& ReptypeGenIterator<ITERATOR, CONFIG>::columns()
{
	return iterator_.columns();
}

template<typename ITERATOR, typename CONFIG>
double* ReptypeGenIterator<ITERATOR, CONFIG>::data()
{
	return data_;
}

template<typename ITERATOR, typename CONFIG>
bool ReptypeGenIterator<ITERATOR, CONFIG>::isNewDataset()
{
	return iterator_.isNewDataset();
}

template<typename ITERATOR, typename CONFIG>
bool ReptypeGenIterator<ITERATOR, CONFIG>::next()
{
	bool r = iterator_.next();
	if (r)
	{
		double* trueData = iterator_.data();

		copy(trueData, trueData + iterator_.columns().size(), data_);

		for (size_t i = 0; i < indices_.size(); ++i)
			values_[i] = data_[indices_[i]];

		ReptypeTable::const_iterator it = reptypeTable_.find(values_); 
		if (it != reptypeTable_.end())
			data_[reptypeIndex_] = it->second;
		else
		{
			// TODO: rethink!
			Log::info() << "ReptypeGenIterator::next(): No matching report type, creating new one." << endl;

			size_t newRT = reptypeTable_.size();
			for (ReptypeTable::const_iterator i = reptypeTable_.begin(); i != reptypeTable_.end(); ++i)
			{
				size_t rt = i->second;
				if (newRT <= rt)
					newRT = rt + 1;
			}

			Log::info() << "ReptypeGenIterator::next(): New report type: " << newRT << endl;

			data_[reptypeIndex_] = reptypeTable_[values_] = newRT;
		}
	}
	noMore_ = !r;
	return r;
}

template class ReptypeGenIterator<>;
template class ReptypeGenIterator<FakeODBIterator>;

} // namespace tool 
} // namespace odb 
