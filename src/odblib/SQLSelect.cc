/*
 * © Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eclib/BigNum.h"
#include "eclib/Exceptions.h"
#include "eclib/Types.h"

#include "odblib/ConstantExpression.h"
#include "odblib/SQLAST.h"
#include "odblib/SQLColumn.h"
#include "odblib/SQLDatabase.h"
#include "odblib/SQLSelect.h"
#include "odblib/SQLSession.h"
#include "odblib/SQLTable.h"
#include "odblib/SQLTable.h"
#include "odblib/SQLType.h"
#include "odblib/SchemaAnalyzer.h"

#define SRC __FILE__,__LINE__

namespace odb {
namespace sql {

void SQLSelect::pushFirstFrame() { env.pushFrame(sortedTables_.begin()); }

SQLSelect::SQLSelect(const Expressions& columns, 
	const vector<SQLTable*>& tables,
	SQLExpression* where,
	SQLOutput* output,
	SQLOutputConfig cfg)
: select_(columns),
  tables_(tables),
  where_(where),
  simplifiedWhere_(0),
  output_(output),
  total_(0),
  skips_(0),
  aggregate_(false),
  mixedAggregatedAndScalar_(false),
  outputConfig_(cfg)
{
	output->config(cfg);
}

SQLSelect::~SQLSelect()
{
	//Log::info() << "SQLSelect::~SQLSelect: " << *this << endl;
	for (size_t i = 0; i < select_.size(); ++i)
	{
		//Log::info() << "SQLSelect::~SQLSelect: deleting " << *select_[i] << endl;
		delete select_[i];
	}
}

SQLTable* SQLSelect::findTable(const string& name,
	string *fullName, bool *hasMissingValue, double *missingValue, bool* isBitfield, BitfieldDef* bitfieldDef) const
{
	set<SQLTable*> names;

	for(vector<SQLTable*>::const_iterator t = tables_.begin(); 
		t != tables_.end() ; ++t)
	{
		SQLTable* table = const_cast<SQLTable*>(*t);
		if(table->hasColumn(name, fullName))
		{
			//if(table->hasColumn(name, fullName, hasMissingValue, missingValue, bitfieldDef))
			names.insert(table);
			SQLColumn& column = *table->column(name);
			if (hasMissingValue) *hasMissingValue = column.hasMissingValue();
			if (missingValue) *missingValue = column.missingValue();
			if (isBitfield) *isBitfield = column.isBitfield();
			if (bitfieldDef) *bitfieldDef = column.bitfieldDef();
		}
	}

	if(names.size() == 0)
		throw UserError("Can't find a table for", name);

	if(names.size() != 1)
		throw UserError("Ambiguous column name", name);

	Log::debug() << "SQLSelect::findTable: name='" << name << "', fullName=" << (fullName ? (string("'")+ *fullName+"'") : "") << endl;

	return *names.begin();
		
}


pair<double,bool>* SQLSelect::column(const string& name, SQLTable* table)
{

	if(!table) table = findTable(name);
	SQLColumn* column = table->column(name);

	string full = column->fullName();
	if(values_.find(full) != values_.end())
		return &values_[full];

	allTables_.insert(table);

	// Take care of aligned tables
	SQLTable* master = table->master();

	if(tablesToFetch_.find(master) == tablesToFetch_.end())
		tablesToFetch_[master] = SelectOneTable(master);

	tablesToFetch_[master].fetch_.push_back(column);
	tablesToFetch_[master].values_.push_back(&values_[full]);

	Log::debug() << "Accessing column " << full << endl;

	return &values_[full];

}

const type::SQLType* SQLSelect::typeOf(const string& name, SQLTable* table) const
{
	if(!table) table = findTable(name);
	SQLColumn* column = table->column(name);

	const type::SQLType& type = column->type();
	return type.subType(name); // This should take care of bitfields

}

static bool compareTables(SelectOneTable* a,SelectOneTable *b)
{
//#if 1
//	if(&(a->table_->owner()) != &(b->table_->owner()))
		return a->table_->owner().name() < b->table_->owner().name();
//#else
//	return a->table_->index() < b->table_->index();
//#endif
}

inline bool SQLSelect::resultsOut()
{
	return output_->output(results_);
}

void SQLSelect::prepareExecute() {
	reset();

	for(Expressions::iterator c = select_.begin(); c != select_.end(); ++c)
	{
		if((*c)->isAggregate())
		{
			aggregated_.push_back(*c);
			mixedResultColumnIsAggregated_.push_back(true);
		}
		else
		{
			nonAggregated_.push_back(*c);
			mixedResultColumnIsAggregated_.push_back(false);
		}

		(*c)->prepare(*this);

		Log::debug(SRC) << "SQLSelect::prepareExecute: '" << *(*c) << "'" << endl;
	}
	ASSERT(select_.size() == mixedResultColumnIsAggregated_.size());
	ASSERT(select_.size() == aggregated_.size() + nonAggregated_.size());

	output_->prepare(*this);
	results_ = select_;
	output_->size(results_.size());

	if(aggregated_.size()) {
		aggregate_ = true;
		Log::debug() << "SELECT is aggregated" << endl;

		if(aggregated_.size() != results_.size())
		{
			mixedAggregatedAndScalar_ = true;
			Log::info() << "SELECT has aggregated and non-aggregated results" << endl;
		}
	}


	SQLExpression *where = where_.get();
	if(where) {
		where->prepare(*this);
		
		bool more = true;
		while(more) {
			more = false;
			SQLExpression* w = where->simplify(more);
			if(w) where = w; // Warning, potential memory leak
			simplifiedWhere_ = where;
		}
		
		Log::debug() << "Simplified WHERE " << *where << endl;
		if(where->isConstant()) {
			bool missing = false;
			if(where->eval(missing)) {
				Log::debug() << "WHERE condition always true, ignoring" << endl;
				where = 0;
			} else {
				Log::debug() << "WHERE condition always false" << endl;
				return;
			}
		}
	}
	// Check for links
	for(set<SQLTable*>::iterator j = allTables_.begin(); j != allTables_.end() ; ++j) {
		SQLTable* table1 = *j;
		const string& name1    = table1->name();

		for(set<SQLTable*>::iterator k = allTables_.begin(); k != allTables_.end() ; ++k) {
			SQLTable* table2 = *k;
			SelectOneTable& x      = tablesToFetch_[table2->master()];

			if(table1->hasLinkTo(*table2)) {
				const string& name2    = table2->name();

				// That can happen for 'aligned' tables
				if(x.column_) {
					ASSERT(table2->master() == x.table2_->master());
					Log::warning() << "Ignoring link " << name1             << "->" << name2             << 
					", using "         << x.table1_->fullName() 
						<< "->" << x.table2_->fullName() << endl;
					continue;
				}
				Log::info() << "Using link " << table1->fullName() 
					<< "->" << table2->fullName() << endl;

					//
				string o                  = name2 + ".offset";
				pair<double,bool>* offset = column(o,table1);

				string l                  = name2 + ".length";
				pair<double,bool>* length = column(l,table1);

				// There should not be 2 tables with a link on the same table
				
				ASSERT(x.offset_ == 0);
				ASSERT(x.length_ == 0);
				ASSERT(x.column_ == 0);

				x.offset_ = offset;
				x.length_ = length;
				x.column_ = table1->column(name2 + ".offset");
				x.table1_ = table1;
				x.table2_ = table2;
			}
		}
	}

	// ------------------------------------

	for(TableMap::iterator j = tablesToFetch_.begin() ; j != tablesToFetch_.end(); ++j)
		sortedTables_.push_back(& (*j).second);

	if(where) {
		// Try to analyse where
		
		expression::Expressions e;
		if(!where->andSplit(e))
			e.push_back(where);

		for(size_t i = 0 ; i < e.size() ; ++i) {
			Log::debug() << "WHERE AND split " << *(e[i]) << endl;

			// Get tables accessed
			set<SQLTable*> t;
			e[i]->tables(t);

			for(set<SQLTable*>::iterator j = t.begin(); j != t.end(); ++j)
				Log::debug() << "  tables -> " << (*j)->fullName() << endl;

				
			if(t.size() == 1) {
				SQLTable* table = *(t.begin());

				if(e[i]->useIndex()) {
					tablesToFetch_[table].index_.push_back(e[i]);
					Log::debug() << "INDEX on " << (*e[i]) << endl;
				}
				//else
				{
					tablesToFetch_[table].check_.push_back(e[i]);
					Log::debug() << "WHERE quick check for " << table->fullName() << " " << (*e[i]) << endl;
				}
			}
		}
	}

	// Needed, for example, if we do: select count(*) from "file.oda"
	if (sortedTables_.size() == 0)
		for (vector<SQLTable*>::iterator i = tables_.begin(); i != tables_.end(); ++i)
			sortedTables_.push_back(new SelectOneTable(*i)); // TODO: release the objects!

	std::sort(sortedTables_.begin(), sortedTables_.end(), compareTables);
	Log::debug() << "TABLE order " << endl;
	for(SortedTables::iterator k = sortedTables_.begin(); k != sortedTables_.end(); ++k) {
		Log::debug() << (*k)->table_->fullName() << " " << (*k)->order_ << endl;

		for(size_t i = 0; i < (*k)->check_.size(); i++)
			Log::debug() << "    QUICK CHECK " << *((*k)->check_[i]) << endl;

		for(size_t i = 0; i < (*k)->index_.size(); i++)
			Log::debug() << "    INDEX CHECK " << *((*k)->index_[i]) << endl;
	}


	// Add the multi-table quick checks
	if(where) {
		expression::Expressions e;
		if(!where->andSplit(e))
			e.push_back(where);

		set<const SQLTable*> ordered;
		for(SortedTables::iterator k = sortedTables_.begin(); k != sortedTables_.end(); ++k) {
			const SQLTable* table = (*k)->table_;
			ordered.insert(table);

			for(size_t i = 0; i < e.size(); ++i)
				if(e[i]) {
					// Get tables accessed
					set<SQLTable*> t;
					e[i]->tables(t);

					if(t.size() != 1) {
						bool ok = true;
						for(set<SQLTable*>::iterator j = t.begin() ; j != t.end(); ++j)
							if(ordered.find(*j) == ordered.end())
								ok = false;

						if(ok) {
							(*k)->check_.push_back(e[i]);
							Log::info() << "WHERE multi-table quick check for " << table->fullName() << " " 
								<< (*e[i]) << endl;

							if(e[i]->useIndex())
							{
								(*k)->index_.push_back(e[i]);
								Log::info() << "WHERE multi-table INDEX" << table->fullName() << " " 
									<< (*e[i]) << endl;
							}

							e[i] = 0;
						}
					}
					else e[i] = 0;
				}
		}

	// Add what's left to last table

		for(size_t i = 0 ; i < e.size() ; ++i)
		if(e[i]) {
			sortedTables_.back()->check_.push_back(e[i]);
		}
		where = 0;
	}

	Log::debug() << "SQLSelect:prepareExecute: TABLE order:" << endl;
	for(SortedTables::iterator k = sortedTables_.begin(); k != sortedTables_.end(); ++k)
	{
		Log::debug() << "SQLSelect:prepareExecute: TABLE order " <<  (*k)->table_->fullName() << " " <<
			(*k)->order_ << endl;

		for(size_t i = 0; i < (*k)->check_.size(); i++)
			Log::debug() << "    QUICK CHECK " << *((*k)->check_[i]) << endl;
	}
}

unsigned long long SQLSelect::execute()
{
	prepareExecute();

	unsigned long long n = process(simplifiedWhere_, sortedTables_.begin());

	postExecute();

	return n;
}


void SQLSelect::postExecute()
{
	if (mixedAggregatedAndScalar_)
	{
		for (AggregatedResults::iterator it = aggregatedResults_.begin(); it != aggregatedResults_.end(); ++it)
		{
			const vector<pair<double,bool> >& nonAggregatedValues = it->first;
			const Expressions& aggregated = *(it->second); 

			Expressions results;
			size_t ai = 0, ni = 0;
			for (size_t i = 0; i < mixedResultColumnIsAggregated_.size(); ++i)
			{
				if (mixedResultColumnIsAggregated_[i])
					results.push_back(aggregated[ai++]->clone());
				else
				{
					results.push_back(
						new ConstantExpression(nonAggregatedValues[ni].first,
								nonAggregatedValues[ni].second,
								nonAggregated_[ni]->type()));
					++ni;
				}
			}

			output_->output(results);

			results.release();
		}
	}
	else if (aggregate_)
	{
		resultsOut();
	}

	output_->flush();

	output_->cleanup(*this);

	if(simplifiedWhere_) simplifiedWhere_->cleanup(*this);
	
	for(expression::Expressions::iterator c = results_.begin(); c != results_.end() ; ++c)
		(*c)->cleanup(*this);

	Log::info() << "Matching row(s): " << BigNum(output_->count()) << " out of " << BigNum(total_) << endl;
	Log::info() << "Skips : " << BigNum(skips_) << endl;

	reset();
}

void SQLSelect::reset()
{
	aggregate_ = false;
	mixedAggregatedAndScalar_ = false;

	aggregated_.clear();
	nonAggregated_.clear();

	for (AggregatedResults::iterator it = aggregatedResults_.begin(); it != aggregatedResults_.end(); ++it)
	{
		Expressions* exps = it->second; 
		for (size_t i = 0; i < exps->size(); ++i)
			delete (*exps)[i];
		delete exps; 
	}
	aggregatedResults_.clear();

	mixedResultColumnIsAggregated_.clear();

	values_.clear();
	results_.clear();

	tablesToFetch_.clear();
	allTables_.clear();

	sortedTables_.clear();

	skips_ = total_ = 0;

	output_->reset();
	count_ = 0;
}



bool SQLSelect::output(SQLExpression* where)
{
	//if (where) Log::info() << "SQLSelect::output: where: " << *where << endl;

	bool newRow = false;
	bool missing = false;
	if ( (where == 0) || (where->eval(missing) && !missing) )
	{
		if (! aggregate_)
			newRow = resultsOut();
		else
		{
			size_t n = results_.size();
			if (! mixedAggregatedAndScalar_)
			{
				for(size_t i = 0; i < n; i++)
					results_[i]->partialResult();
			} else {
				vector<pair<double,bool> > nonAggregatedValues;
				for (size_t i = 0; i < nonAggregated_.size(); ++i)
				{
					bool missing = false;
					double v = nonAggregated_[i]->eval(missing);
					nonAggregatedValues.push_back(make_pair(v, missing));
				}
	
				AggregatedResults::iterator results = aggregatedResults_.find(nonAggregatedValues);
				if (results == aggregatedResults_.end())
					aggregatedResults_[nonAggregatedValues] = static_cast<Expressions*>(aggregated_.clone());

				Expressions& aggregated = *aggregatedResults_[nonAggregatedValues];
				for (size_t i = 0; i < aggregated.size(); ++i)
					aggregated[i]->partialResult();
			}
		}
	}
	return newRow;
}


unsigned long long SQLSelect::process(SQLExpression* where, SortedTables::iterator j) {
	simplifiedWhere_ = where;
	env.pushFrame(j);

	unsigned long long n = 0;
	while (processOneRow())
		++n;
	return n;
}


bool SQLSelect::processOneRow() { 
	++count_;
	//Log::info() << "SQLSelect::processOneRow: count = " << count_ << endl;
	bool recursiveCall;
	do
	{
		recursiveCall = false;
		if(env.tablesIterator() == sortedTables_.end())
		{
			bool rowProduced = output(simplifiedWhere_);
			env.popFrame();
			if (rowProduced)
				return true;
		}

		if (&env.table() == 0)
		{
			env.table( *(env.tablesIterator()) );

			size_t n = env.table().fetch_.size();
			for(size_t i = 0; i < n; i++) {
				env.table().fetch_[i]->rewind();
			}
			env.cursor(env.table().table_->iterator(env.table().fetch_));
			env.cursor().rewind();
		}

		while(env.cursor().next())
		{
			total_++;
			size_t n = env.table().fetch_.size();
			for(size_t i = 0; i < n; i++)
			{
				SQLColumn &fetchColumn = *env.table().fetch_[i];
				//Log::info() << "SQLSelect::processOneRow: fetchColumn.name() => " << fetchColumn.name() << endl;
				//Log::info() << "SQLSelect::processOneRow: fetchColumn.type() => " << fetchColumn.type() << endl;
				bool &missing = env.table().values_[i]->second;
				env.table().values_[i]->first = fetchColumn.next(missing);
			}
			bool ok = true;
			n = env.table().check_.size();
			for(size_t i = 0; i < n; i++)
			{
				bool missing = false;
				if(!env.table().check_[i]->eval(missing) || missing) {
					ok = false;
					break;
				} 
			}
			if(!ok) skips_++;
			else {
				SortedTables::iterator k = env.tablesIterator();
				env.pushFrame(++k);
				//processOneRow();
				recursiveCall = true;
				break;
			}
		}
	} while(recursiveCall);
	env.popFrame();
	return false;
}

void SQLSelect::print(ostream& s) const
{
	s << "SELECT"; char sep = ' ';

	//if(distinct_)
	//	s << " DISTINCT";

	for(expression::Expressions::const_iterator c = select_.begin(); c != select_.end() ; ++c)
	{
		s << sep << *(*c);
		sep = ',';
	}

	s << " FROM";
	sep = ' ';
	for(vector<SQLTable*>::const_iterator t = tables_.begin(); t != tables_.end() ; ++t)
	{
		s << sep << (*t)->name();
		sep = ',';
	}

	if(where_.get())
		s << " WHERE " << *where_;

	s << " " << *output_;
}

expression::Expressions SQLSelect::output() const { return select_; }

} // namespace sql 
} // namespace odb 