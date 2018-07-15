#include <algorithm>
#include <iostream>
#include <sstream>

#include "options.h"

namespace expenses {
	
	const std::vector<std::string> Options::optLabels
	{"detail", "summary", "sep", "orderedby", "code"};

	Options::Options(int argc, const char* argv[])
	{
		parse(argc, argv);
	}

	void Options::processOption(const std::string& key,
								const std::string& value)
	{
		auto it = std::find(optLabels.begin(), optLabels.end(), key);
		if (it != optLabels.end()) {
		
			// which one is it?
			int index = it - optLabels.begin();
		
			// process the value before turning the option on:
			ColumnList elements = parseValue(value);

			// if it is well-formed, turn it on:
			if (!elements.empty()) {
				optValues[index] = elements;
				options.set(index, 1);
			}
		}
	}

	void Options::parse(int argc, const char* argv[]) {

		auto readString = [](const char*& ch) {
			std::string s;		
			// read until a whitespace is found:
			while(*ch && !isspace(*ch)) {
				s += *ch++;
			}
			return s;
		};
	
		// actually argc is always >= 1;
		if (argc == 1) {
			return;
		}

		// treat argv as a linear sequence of characters:
		// start with the second string
		int i = 1;
		const char* ch = argv[i];
		std::string key;
		while(i < argc) {
		
			// if we have reached the end of the current
			// string we move to the next one if there are more:
			while (!*ch && ++i < argc) {
				ch = argv[i];
			}

			if (!*ch)
				break; // we are done; 

			switch(*ch) {
			case '-': { // found an option;
				if (*++ch != '-') {
					throw std::runtime_error{"Invalid option"};
				}
			
				key = "";
				while(*++ch && !isspace(*ch) && !ispunct(*ch)) {
					key += *ch;
				}
				break;
			}
			case '=': { // found a value:
				// skip any leading blanks:
				while(*++ch && isspace(*ch));
				processOption(key, readString(ch));
				break;
			}
			default:
				// skip leading blanks;
				while(*ch && isspace(*ch)) {
					++ch;
				}
				filename = readString(ch);
				break;
			}
		}
	}

	Options::ColumnList Options::parseValue(const std::string& value)
	{
		std::istringstream is{value};
		std::string member;
		ColumnList members;
		while(std::getline(is, member, ','))
			members.push_back(member);
		return members;
	}

	void Options::print() const
	{
		for(int i=0; i < OptionEnd; ++i) {
			if (!options.test(i)) {
				continue;
			}

			std::cout << optLabels[i] << ": ";
			for(auto val : optValues[i]) {
				std::cout << val << ' ';
			}
			std::cout << '\n';
		}
	}
	
	void Options::printSupportedOptions()
	{
		std::cout << "Expenses processor version 0.1\n\n"
			"The following are the currently supported options:\n\n";
		std::cout << "--sep=column_separator \n"
			"\tconsider the specified character as the column\n"
			"\tseparator for the csv file. Please do not use comma\n"
			"\tif your data contains commas; however, if no separator\n"
			"\tis given, ',' is used.\n";
	
		std::cout << "--detail=column_1[, column_2, ..., column_n]\n"
			"\tPrint a detailed list of all transactions showing\n"
			"\tonly the specified columns.\n";

		std::cout << "--orderedby=column_1[, column_2, ..., column_n]\n"
		    "\tThis is only used for a detailed transaction printing. \n"
			"\tTransactions will be ordered by the columns provided.\n";
	
		std::cout << "--summary=column_1[, column_2, ..., column_n]\n"
			"\tPrint a summary of all the transactions showing\n"
			"\tonly the specified columns. Please note that only numeric\n"
			"\tcolumns should be given here. The transactions are grouped\n"
			"\tthe value of code, see below for more on code.\n";

		std::cout << "--code=column_for_code\n"
			"\tTransactions are summarized by financial codes\n"
			"\tif this value is set, transactions will be grouped by\n"
			"\tthis code; otherwise, the default value is 'Code'.\n";

		std::cout << "Here is an example:\n\n"
			"./ex --detail=FinCode,Date,Amount,HST13%,HST5%/TVQ,Total --orderedBy=Date,Entry# --summary=Amount,HST13%,HST5%/TVQ,Total --code=FinCode --sep='|' ~/expenses.csv\n";

		std::cout << "\nThe preceeding command will print the detailed of all the"
			" transactions in the file ordered by date and entry#  and then print a summary of the transactions grouped by FinCode.\n";
	}
} // namespace expenses
