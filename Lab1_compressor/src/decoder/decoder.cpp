#include "ppmd.h"
#include <iostream>


int main(int argc, char* argv[]) {
	char* input_filename;
	
	if ( argc > 1 ) {
		input_filename = argv[1];
	} else {
		std::cerr << "No arguments provided\n";
	}

	std::string output_filename = input_filename;

	if ( output_filename.find(".ppmd") ) {
		output_filename.erase(output_filename.find(".ppmd"), 5);

		FILE * fpIn = fopen(input_filename,"rb"), * fpOut = fopen(output_filename.c_str(),"wb");
		if (!fpIn || !fpOut) {
			std::cerr << "Unable to open file-stream\n";
			exit(-1);
		}
		ppmd::DecodeSequence(4, fpIn, fpOut);
		fclose(fpIn); fclose(fpOut);
	} else
		std::cerr << "Invalid file extension";

	return 0;
}
