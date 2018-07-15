#ifndef OPTIONS_H_
#define OPTIONS_H_

#include <vector>
#include <bitset>

// Command line options parser for exp
namespace expenses {
	class Options
	{
		enum {
			DetailOn,
			SummaryOn,
			SeparatorOn,
			OrderedByOn,
			CodeOn,
			OptionEnd
		};
	public:
		using ColumnList = std::vector<std::string>;
		Options(int argc, const char* argv[]);

		void print() const;
		bool details() const { return options[DetailOn]; }
		bool summary() const { return options[SummaryOn]; }
		bool separator() const { return options[SeparatorOn]; }
		bool orderedBy() const { return options[OrderedByOn]; }
		bool code() const { return options[CodeOn]; }

		char getColumnSeparator() const {
			return separator() ? optValues[SeparatorOn][0][0] : ',';
		}
	
		const std::string& getFilename() const { return filename; }
	
		std::string getFinCodeColumn() const
			{ return code() ? optValues[CodeOn][0] : ""; }

		const ColumnList& getOrderedByColumns() const
		{ return optValues[OrderedByOn]; }

		const ColumnList& getSummaryColumns() const
		{ return optValues[SummaryOn]; }

		const ColumnList& getDetailColumns() const
		{ return optValues[DetailOn]; }
	
		void printSetOptions();
		static void printSupportedOptions();
	private:
		void parse(int argc, const char* argv[]);
		void processOption(const std::string& key,
						   const std::string& value);
		ColumnList parseValue(const std::string& value);

		std::bitset<OptionEnd> options;
		std::string filename;
	
		// to generalize option processing
		ColumnList optValues[OptionEnd];
	
		static const std::vector<std::string> optLabels;
	};

} // namespace expenses

#endif
