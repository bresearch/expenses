#include "options.h"
#include "exp_processor.h"

using namespace expenses;

int main(int argc, const char* argv[]) {
	if (argc < 2) {
		
		Options::printSupportedOptions();
		return 1;
	}

	Options options(argc, argv);
	//	options.print();
	
	Processor::processExpenses(options);

	return 0;
}
