#include "ppmd.h"
#include <iostream>


int main(int argc, char* argv[]) {
	char* input_filename;
	
	if (argc > 1) {
		input_filename = argv[1];
	} else {
		std::cerr << "No arguments provided\n";
		exit(-1);
	}

	std::string output_filename = input_filename;
	output_filename.append(".ppmd");

	FILE * fpIn = fopen(input_filename,"rb"), * fpOut = fopen(output_filename.c_str(),"wb");
    if (!fpIn || !fpOut) {
        std::cerr << "Unable to open file-stream\n";
        exit(-1);
    }
    
	ppmd::EncodeSequence(4, fpOut, fpIn);
	fclose(fpIn); fclose(fpOut);
	
	return 0;
}
