#ifndef EXP_PROCESSOR_H_
#define EXP_PROCESSOR_H_

#include <vector>

#include "fmt.h"

namespace expenses {
	class Options;
	class Processor {
		using Row = std::vector<std::string>;
		using DB = std::vector<Row>;
		using IndexList = std::vector<int>;
	public:
		using DBRow = Row;

		Processor(const std::string& filename, char fieldDelimiter);
		static void processExpenses(const Options& options);
		void dump() const;
		double getColumnTotal(const std::string& column,
							  const std::string& code = "") const;
		Row getHeadings() const { return db.empty() ? Row() : db[0]; }
	
		std::string getHeading(int i) const
			{ return db.empty() ? "" : db[0].empty() ? "" : db[0][0]; }
	
		Row getAllCodeValues() const { return getAllCodeValuesByIndex(0); }
		Row getAllCodeValuesByIndex(int i) const;
		Row getAllCodeValuesByColumn(const std::string& codeHeading) const;
		void printCodes() const;

		void printSummaryForColumns(const Row& columns,
									const std::string& finCodeHeading="") const;

		void setDefaultFinCodeColumn(const std::string& column)
		{ defaultFinCodeColumn = column; }
	
		IndexList getIndicesForColumns(const Row& columns) const;
		void printDetailsForColumns(const Row& columns,
									const Row& orderBy=Row{});
	private:
		
		Row fillRow(const std::string& line, char fieldDelimiter);
		int findIndex(const std::string& column, bool ignoreCase = false) const;
		void printLine(int len) const;
		void sortDB(const Row& orderedBy);
		
		static void reverse(Row& fields);
		static bool isInteger(const std::string& str, int& nbr);
		static bool isDate(const std::string& str, int& ival);
		static std::string serializeRow(const Row& row, const IndexList& ordering);
		
		DB db;
		std::string defaultFinCodeColumn{"Code"};
	};
} // namespace expenses
 #endif
