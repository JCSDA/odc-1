/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <sstream>

#include "eclib/Log.h"
#include "eclib/Translator.h"

#include "odblib/SQLAST.h"
#include "odblib/SQLBitfield.h"
#include "odblib/SchemaAnalyzer.h"
#include "odblib/ShiftedColumnExpression.h"
#include "odblib/SQLSelect.h"
#include "odblib/SQLDatabase.h"
#include "odblib/SQLType.h"
#include "odblib/SQLTable.h"

namespace odb {
namespace sql {
namespace expression {

template <typename T>
void ShiftedColumnExpression<T>::allocateCircularBuffer()
{
	//Log::info() << "allocateCircularBuffer:" << shift_ <<  endl;

// FIXME: we need to retrieve actual value of missing value for this column
	double const MISSING_VALUE_REAL = -2147483647.0;
	static pair<double,bool> missing_(MISSING_VALUE_REAL,true);

	ASSERT(shift_ > 0);

	for (size_t i = 0; i < shift_; ++i)
		oldValues_.push_front(missing_);
}

template <typename T>
ShiftedColumnExpression<T>::ShiftedColumnExpression(const T& o, int shift, int nominalShift)
: T(o),
  shift_(shift),
  nominalShift_(nominalShift),
  oldValues_()
{}

template <typename T>
ShiftedColumnExpression<T>::ShiftedColumnExpression(const string& name, SQLTable* table, int shift, int nominalShift, int begin, int end)
: T(name, table, begin, end),
  shift_(shift),
  nominalShift_(nominalShift),
  oldValues_()
{}

template <typename T>
ShiftedColumnExpression<T>::ShiftedColumnExpression(const string& name, const string& tableReference, int shift, int nominalShift, int begin, int end)
: T(name, tableReference, begin, end),
  shift_(shift),
  nominalShift_(nominalShift),
  oldValues_()
{}

template <typename T>
ShiftedColumnExpression<T>::ShiftedColumnExpression(const string& name, const string& field, SQLTable* table, int shift, int nominalShift)
: T(name, field, table),
  shift_(shift),
  nominalShift_(nominalShift),
  oldValues_()
{}

template <typename T>
ShiftedColumnExpression<T>::ShiftedColumnExpression(const string& name, const string& field, const string& tableReference, int
shift, int nominalShift)
: T(name, field, tableReference),
  shift_(shift),
  nominalShift_(nominalShift),
  oldValues_()
{}

template <typename T>
ShiftedColumnExpression<T>::ShiftedColumnExpression(const ShiftedColumnExpression& e)
: T(e),
  shift_(e.shift_),
  nominalShift_(e.nominalShift_),
  oldValues_(e.oldValues_)
{}

template <typename T>
SQLExpression* ShiftedColumnExpression<T>::clone() const { NOTIMP; return 0; } // return new ShiftedColumnExpression(*this); }

template <typename T>
ShiftedColumnExpression<T>::~ShiftedColumnExpression() {}

template <typename T>
void ShiftedColumnExpression<T>::print(ostream& s) const
{
	s << this->columnName_; 
	if (nominalShift_ != 0)
		s << "#" << nominalShift_;
}

template <typename T>
double ShiftedColumnExpression<T>::eval(bool& missing) const
{
	ShiftedColumnExpression& self(*const_cast<ShiftedColumnExpression<T>*>(this));
	ASSERT(shift_ > 0);

	if (oldValues_.size() == 0)
		self.allocateCircularBuffer();

	pair<double,bool> const& v(self.oldValues_.back());
	double value = v.first;
	bool miss = v.second;

	self.oldValues_.pop_back();

	pair<double,bool> ev;
	ev.first = T::eval(ev.second);
	self.oldValues_.push_front(ev);

	if(miss) missing = true;
	return value;
}

template <typename T>
void ShiftedColumnExpression<T>::cleanup(SQLSelect& sql)
{
	static pair<double,bool> zero_(0,false);
	this->value_ = &zero_;
	this->type_  = 0;
	oldValues_.clear();
}

template <typename T>
void ShiftedColumnExpression<T>::output(SQLOutput& o) const 
{ 
	//Log::info() << "ShiftedColumnExpression::output:" << endl;

	bool missing = false;
	double v = eval(missing);
	this->type_->output(o, v, missing); 
}

} // namespace expression
} // namespace sql
} // namespace odb
