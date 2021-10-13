#include <iostream>
#include "encoder.h"

bool is_ok() {
	return true;
}

int main(int argc, char* argv[]) {
	std::cout << "Hello, cross-compilation!" << std::endl;
	
	#ifdef __WIN32
	system("PAUSE");
	#endif
	
	return 0;
}
