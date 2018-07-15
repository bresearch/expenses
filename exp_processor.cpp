#include "exp_processor.h"

#include <iostream>
#include <set>
#include <fstream>
#include <algorithm>

#include "options.h"

namespace expenses
{
	// enable this for arithmetic types only: use
	// this to format numbers
	template<typename T>
	typename std::enable_if<std::is_arithmetic<T>::value, std::ostream&>::type
	operator<<(std::ostream& os, const Binder<T>& binder)
	{
		std::ostringstream oss;
		oss.precision(binder.f.prc);
		oss.width(binder.f.width);
		oss.fill(binder.f.fChar);
		oss.setf(binder.f.fmt, binder.f.ffmt);
		oss << binder.val;
		os << oss.str();
		return os;
	}

	// use this to format string values
	std::ostream& operator<<(std::ostream& os, const Binder<std::string>& binder)
	{
		std::ostringstream oss;
		oss.width(binder.f.width);
		oss.fill(binder.f.fChar);
		oss.setf(binder.f.fmt, std::ios_base::adjustfield);
		oss << binder.val;
		os << oss.str();
		return os;
	}
	
	Processor::Processor(const std::string& filename, char fieldDelimiter) :
		db {}
{
	std::fstream fin{filename};
	if (!fin) {
		throw std::ios_base::failure{filename + " does not exist"};
	}

	for(std::string line; std::getline(fin, line);) {
		Row row = fillRow(line, fieldDelimiter);
		
		// skip any empty row:
		if (row.empty()) {
			continue;
		}
		
		db.push_back(row);
	}
}

	// print the codes; codes can be in any
	// column position
	void Processor::printCodes() const {
		Row codes = getAllCodeValues();
		for(auto code : codes) {
			std::cout << code << "\n";
		}
	}

	Processor::Row
	Processor::getAllCodeValuesByColumn(const std::string& column) const {

		int index = findIndex(column);
		if (index < 0) {
			return Row{};
		}

		return getAllCodeValuesByIndex(index);
	}

	Processor::Row Processor::getAllCodeValuesByIndex(int colIndex) const {
		std::set<std::string> uniqueCodes;
		for(int i = 1, e = db.size(); i != e; ++i) {
			const Row& row = db[i];
			if (row.empty() || static_cast<int>(row.size()) <= colIndex) {
				continue;
			}
		
			uniqueCodes.insert(row[colIndex]);
		}
	
		return Row{uniqueCodes.begin(), uniqueCodes.end()};
	}

	
	int Processor::findIndex(const std::string& column, bool ignoreCase) const
	{
		// the first row contains column heading:
		if (db.empty()) {
			return -1; // column does not exist
		}

		int sz = column.size();
		const Row& headings = db[0];
	
		// ignore case ?
		auto it = ignoreCase ? std::find_if(headings.begin(), headings.end(),
											[sz, &column](const std::string& heading) {
												if (static_cast<int>(heading.size()) != sz) {
													return false;
												}
					 
												for(int i=0; i < sz; ++i) {
													if (toupper(heading[i]) != toupper(column[i])) {
														return false;
													}
												}
												return true;
											})
			: std::find(headings.begin(), headings.end(), column); 

		return it != headings.end() ? it - headings.begin() : -1;
	}

	void Processor::reverse(Row& fields)
	{
		for(int i=0, j=fields.size() - 1; i < j; ++i, --j) {
			std::swap(fields[i], fields[j]);
		}
	}

	void Processor::printLine(int len) const
	{
		for(int i=0; i < len; ++i) {
			std::cout << '=';
		}
		std::cout << '\n';
	}

	Processor::IndexList Processor::getIndicesForColumns(const Row& columns) const
	{
		if (columns.empty()) {
			return {};
		}
	
		IndexList iList(columns.size());
		const Row& headings = getHeadings();
		for(int i = 0, e = columns.size(); i != e; ++i) {
			auto it = std::find(headings.begin(), headings.end(), columns[i]);
			iList[i] = it != headings.end() ? it - headings.begin() : -1;
		}

		return iList;
	}

	bool Processor::isInteger(const std::string& str, int& nbr)
	{
		try {
			size_t n = 0;
			int ival = stoi(str, &n);
			if (n != str.size()) {
				return false;
			}
			
			nbr = ival;
		} catch(const std::invalid_argument& e) {
			return false;
		} catch(const std::out_of_range& e) {
			return false;
		} catch(...) {
			return false;
		}
		
		return true;
	}

	// if the string comprises of 3 integers
	// separated by '-' or '/', this simple test
	// considers the string a date, false otherwise;
	// note that the supported date format is YYYY-MM-DD
	// or YYYY/MM/DD or // YY-MM-DD or YY/MM/DD
	bool Processor::isDate(const std::string& str, int& intVal)
	{
	    const std::string delimiters {"-/"};
		int first = str.find_first_of(delimiters);
		if (first == std::string::npos) {
			return false;
		}

		// we need to find exactly 2 matching delimiters:
		int second = str.find_first_of(delimiters, first + 1);
		if (second == std::string::npos) {
			return false;
		}

		// now, check that they match:
		if (str[first] != str[second]) {
			return false;
		}

		// so far all tests passed, does the string
		// contain 3 integers? do they all use the 
		// same number of digits as the string? If
		// any exception is thrown, then it is not a
		// date
		
		// get the year:
		int ival = 0;
		std::string next = str.substr(0, first);
		if (!isInteger(next, ival)) {
			return false;
		}
		intVal = 12*31*ival;
		
		// get the month:
		next = str.substr(first + 1, second - first - 1);
		if (!isInteger(next, ival)) {
			return false;
		}	
		intVal += 31*ival;

		// get the day:
		next = str.substr(second + 1, str.size() - second - 1);
		if (!isInteger(next, ival)) {
			return false;
		}
		
		intVal += ival;
		return true;
	}
	
	std::string Processor::serializeRow(const Row& row, const IndexList& ordering)
	{
		std::ostringstream os;
		for(auto index : ordering) {
			// check that the index is in the range in row:
			if (0 > index || index >= row.size()) {
				continue; // skip the bad ones:
			}

			const std::string& value = row[index];
			// use a simple test to check whether the
			// cell contains a date value:
			int ival = 0;
			if (isDate(value, ival)) { 
				os << ival; 
			} else {
				os << value;
			}
		}

		return os.str();
	}
	
	void Processor::sortDB(const Row& orderedBy)
	{
		IndexList iList = getIndicesForColumns(orderedBy);
		auto compare = [&iList](const Row& a, const Row& b) {
			std::string serializedRowA = serializeRow(a, iList);
			std::string serializedRowB = serializeRow(b, iList);			
			return serializedRowA < serializedRowB;
		};

		// exclude the headings:
		std::sort(db.begin() + 1, db.end(), compare);
	}
	
	void Processor::printDetailsForColumns(const Row& columns, const Row& orderedBy)
	{	
		// make the format to use to print the data
		Format<std::string> sfmt(10, std::ios_base::right, ' ');
		IndexList iList = getIndicesForColumns(columns);

		// sort the data by the columns provided
		if (!orderedBy.empty()) {
			sortDB(orderedBy);
		}
		
		// now that we have all the indices, we can traverse the DB:
		std::cout << "\n";
		std::string sep{" | "};
		for(auto row : db) {			
			// let's print the row according to the order of the
			// columns given by the user instead of the order in
			// which the columns appear in the input file:
			std::string prefix = "";
			for(int i = 0, e = iList.size(); i != e; ++i) {
				std::cout << prefix << sfmt(row[iList[i]]);
				prefix = sep;
			}
			std::cout << "\n";
		}
	}

	void Processor::printSummaryForColumns(const Row& columns,
										   const std::string& codeHeading) const
	{
		// make the format to use to print the data
		Format<double> fmt{2, 10, std::ios_base::fixed};
		fmt.fill(' ');
		Format<std::string> sfmt(5, std::ios_base::left);
		sfmt.fill(' ');
	
		const std::string sep{" | "};
		std::string finCodeColumn = codeHeading.empty() ? defaultFinCodeColumn :
			codeHeading;

		int len = columns.size() * (fmt.getWidth() + sep.size()) + fmt.getWidth() + 2;
		// print the headings:
		std::cout << "\n";
		printLine(len);
		sfmt.setWidth(fmt.getWidth());
		std::cout << sfmt(finCodeColumn) << sep;
		for(auto heading : columns) {
			std::cout << sfmt(heading) << sep;
		}
		std::cout << "\n";
		printLine(len);

		// print the sums:
		Processor::DBRow codes = getAllCodeValuesByColumn(finCodeColumn);
		for(auto code : codes) {
			if (code.empty()) {
				continue; // skip the xls summary
			}
			std::cout << sfmt(code) << sep;
			for(const auto& column : columns) {
				std::cout << fmt(getColumnTotal(column, code)) << sep;
			}
			std::cout << "\n";
		}

		// print the summary for the selected colums:
		printLine(len);
		std::cout << sfmt("Sum") << sep;
		for(const auto& column : columns) {
			std::cout << fmt(getColumnTotal(column)) << sep;
		}
		std::cout << "\n";
		printLine(len);
	}
	
	void Processor::dump() const
	{
		for(auto row : db) {
			for(const std::string& field : row) {
				std::cout << field << ": ";
			}
			std::cout << "\n";
		}
		std::cout << "No of rows processed: " << db.size() << "\n";
	}

	Processor::Row Processor::fillRow(const std::string& line, char delimiter)
	{
		// empty rows are ignored
		std::istringstream is{line};
		Row row;
		std::string field;
		bool isEmpty = true;
		while(std::getline(is, field, delimiter)) {
			isEmpty = isEmpty && field.empty();
			row.push_back(field);
		}

		// return empty row iff all fields are empty:
		return isEmpty ? Row{} : row;
	}

	double Processor::getColumnTotal(const std::string& column,
									 const std::string& code) const
	{
		int colIndex = findIndex(column);
		if (colIndex < 0) {
			return 0;
		}

		int catCodeIndex = findIndex(defaultFinCodeColumn);
		if (catCodeIndex < 0) {
			return 0;
		}
		
		bool skipRow = !code.empty();
		double total = 0;
		// skip the headings:
		for(int i=1, e = db.size();  i != e; ++i) {
			auto row = db[i];
			if (row.empty() || static_cast<int>(row.size()) <= colIndex) {
				continue; // skip any empty or short row;
			}

			const std::string& catCode = row[catCodeIndex];
			if (skipRow && catCode != code) {
				continue;
			}
		
			const std::string& value = row[colIndex];
			if (value.empty()) {
				continue;
			}
		
			// convert it to a double precision floating point;
			double dVal;
			try {
				dVal = stod(row[colIndex]);
			} catch(std::invalid_argument& e) {				
				continue;
			}
		
			total += dVal;
		}

		return total;
	}

	void Processor::processExpenses(const Options& options)
	{
		Processor pr{options.getFilename(), options.getColumnSeparator()};
		
		if (options.code()) {
			// change the default financial code heading:
			pr.setDefaultFinCodeColumn(options.getFinCodeColumn());
		}
		
		if (options.details()) {
			// print the details for the columns ordered 
			// by the given columns if not empty
			pr.printDetailsForColumns(options.getDetailColumns(),
			                         options.getOrderedByColumns());
		}
		
		if (options.summary()) {
			// print summary by code provided, group by 'Code' if
			// the column exists, otherwise do nothing
			pr.printSummaryForColumns(options.getSummaryColumns());
		}
	}
	
} // namespace expenses
