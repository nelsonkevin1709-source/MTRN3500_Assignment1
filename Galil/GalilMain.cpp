#include "Galil.h"
#include <iostream>
#include <string>

// Runs one check. Note the & after Galil: it uses YOUR object, not a copy.
static void check(Galil& g, const std::string& label, bool expected) {
	bool result = g.CheckSuccessfulWrite();
	std::cout << (result == expected ? "PASS  " : "FAIL  ")
		<< label << "  -> got " << result << ", expected " << expected << "\n";
}

int main(void) {
	EmbeddedFunctions funcs(true);                     // true = simulator
	Galil myGalil(&funcs, "192.168.0.120 -d");         // the simulator is OFF, so this connection fails

	try {
		myGalil.DigitalOutput(5);
		std::cout << "DigitalOutput finished with no error\n";
	}
	catch (const std::exception& e) {
		std::cout << "Error: " << e.what() << "\n";
	}

	std::cout << "Press Enter.\n";
	std::cin.get();
	return 0;
}