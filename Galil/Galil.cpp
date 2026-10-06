#include "Galil.h"
#include "EmbeddedFunctions.h"
#include <iomanip>
#include <sstream>
#include <stdexcept>

// Constants
const std::string DEFAULT_GALIL_ADDRESS = "192.168.0.120 -d";

// Helper Functions
// Protected Helper Functions

void Galil::CommandDirectInputs(bool direct) {
	if (direct) {
		CommandSend("IQ 65535;");
	}
	else {
		CommandSend("IQ 0;");
	}
}

// Private Helper Functions
void Galil::CommandSend(const std::string& command) {
	ReadBuffer[0] = '\0';
	GReturn rc = Functions->GCommand(g, command.c_str(), ReadBuffer, sizeof(ReadBuffer), &BytesRead);
	if (rc != G_NO_ERROR) {
		throw std::runtime_error("CommandSend failed for '" + command + "', error code " + std::to_string(rc));
	}
}

std::string Galil::OPCommandString(const std::string& n0, const std::string& n1) {
	std::string command;
	std::string comma = ",";
	if (n0.empty() && n1.empty()) {
		return "";
	}
	else if (n1.empty()) {
		comma = "";
	} 
	command = "OP " + n0 + comma + n1 + ";";
	return command;
}

// Default constructor. Initialize variables, open Galil connection and allocate memory.
// Should assign a default embedded functions that works with physical hardware and a 
// default Galil address as described in the assignment spec.

Galil::Galil() : Functions(new EmbeddedFunctions()), setPoint(0), ControlParameters{ 0, 0, 0 },
GalilAddress(DEFAULT_GALIL_ADDRESS), NeedToBeCleared(true), ReadBuffer{0}, BytesRead(0)
{
	GReturn rc = Functions->GOpen(GalilAddress.c_str(), &g);
	if (rc != G_NO_ERROR) {
		throw std::runtime_error("Constructor failed, error code " + std::to_string(rc));
	}
}										
 
// Constructor with EmbeddedFunciton pre-initialised and passed in.
Galil::Galil(EmbeddedFunctions* Funcs, GCStringIn address) : Functions(Funcs), setPoint(0), ControlParameters{ 0, 0, 0 },
GalilAddress(address), NeedToBeCleared(false), ReadBuffer{0}, BytesRead(0)
{
	GReturn rc = Functions->GOpen(GalilAddress.c_str(), &g);
	if (rc != G_NO_ERROR) {
		throw std::runtime_error("Constructor failed, error code " + std::to_string(rc));
	}
}

// Copy constructor to copy the state of all elements within the object other.
// It should construct a new EmbeddedFunctions object and open a separate connection
// (i.e., each class will have a unique value of the GCon g). All other data members
// should be transferred.
Galil::Galil(const Galil& other) : Functions(new EmbeddedFunctions()), setPoint(other.setPoint),
ControlParameters{ other.ControlParameters[0], other.ControlParameters[1], other.ControlParameters[2] },
GalilAddress(other.GalilAddress), NeedToBeCleared(true), ReadBuffer{0}, BytesRead(0) {
	Functions->GOpen(other.GalilAddress.c_str(), &g);
}

// Default destructor. Deallocate memory and close Galil connection.
Galil::~Galil() {
	Functions->GClose(g);
	if (this->NeedToBeCleared) {
		delete Functions;
	}
}										

// DIGITAL OUTPUTS
// TODO: complete this function.
// Write to all 16 bits of digital output, 1 command to the Galil
void Galil::DigitalOutput(uint16_t value) {
	uint8_t n0 = value & 0xFF;
	uint8_t n1 = value >> 8;
	std::string command = OPCommandString(std::to_string(n0), std::to_string(n1));
	if (!command.empty()) {
		CommandSend(command);
	}
};						

// Write to one byte, either high or low byte, as specified by user in 'bank'
// 0 = low, 1 = high
void Galil::DigitalByteOutput(bool bank, uint8_t value) {
	std::string strVal = std::to_string(value);
	std::string n0 = bank ? "" : strVal;
	std::string n1 = bank ? strVal : "";
	
	CommandSend(OPCommandString(n0, n1));
}

// Write single bit to digital outputs. 'bit' specifies which bit
void Galil::DigitalBitOutput(bool val, uint8_t bit) {
	if (bit > 15) return;
	std::string bitStr = std::to_string(bit);
	std::string commandVal = val ? "SB" : "CB";
	std::string command = commandVal + " " + bitStr + ";";
	CommandSend(command);
}


// DIGITAL INPUTS
// Query the digital inputs of the GALIL, See Galil command library @IN
// Return the 16 bits of input data
uint16_t Galil::DigitalInput() {
	uint16_t res = 0;
	for (int i = 15; i >= 0; i--) {
		int DIPin = DigitalBitInput(i);
		res |= (DIPin << i);
	}
	return res;
}		

// Read either high or low byte, as specified by user in 'bank'
// 0 = low, 1 = high
// A bank is one byte (8 bits). The low bank (0) is the first 8
// bits (DI0-DI7) and the high bank (1) is the upper 8 bits
// (DI8-DI15).

uint8_t Galil::DigitalByteInput(bool bank) {
	uint16_t  buffer = DigitalInput();
	uint8_t res = bank ? buffer >> 8 : buffer;
	return res;
}			

// Read single bit from current digital inputs. Above functions
// may use this function
// TODO: Error Handling
bool Galil::DigitalBitInput(uint8_t bit) {
	if (bit > 15) return false;
	std::string command = "MG @IN[" + std::to_string(bit) + "];";
	CommandSend(command);
	int res = std::stoi(std::string(ReadBuffer));
	return res;
}

// command executed correctly. 1 = succesful.
// A successful write indicates that a write command (sending a 
// message to the Galil that does not have a response -- e.g., 
// digitalOutput, analogOutput) has completed without errors.
// This should validate some part of the Galil's response (it is 
// up to you how this is completed) but should validly
// differentiate when a write command has been completed 
// successfully.
// This will be called from your main function (do not call it
// within your implementation functions of this Galil class.
bool Galil::CheckSuccessfulWrite() {
	std::string reply(ReadBuffer);
	bool hasColon = reply.find(':') != std::string::npos;
	bool hasQM = reply.find('?') != std::string::npos;
	bool res = hasColon && !hasQM;
	return res;
}							
// Check the string response from the Galil to check that the last

// ANALOG FUNCITONS

// Read Analog channel and return voltage	
// TODO: Error Handling
float Galil::AnalogInput(uint8_t channel) {
	if (channel > 7) return std::numeric_limits<float>::quiet_NaN();
	std::string command = "MG @AN[" + std::to_string(channel) + "];";
	CommandSend(command);
	return std::stof(ReadBuffer);
}						


// Write to any channel of the Galil, send voltages as
// 2 decimal place in the command string
// TODO: Error Handling bound V to +-9.99998
void Galil::AnalogOutput(uint8_t channel, double voltage) {
	if (channel > 7) return;

	std::ostringstream out;
	out << "AO " << static_cast<int>(channel) << ","
		<< std::fixed << std::setprecision(2) << voltage << ";";
	std::string command = out.str();
	CommandSend(command);
}
// TODO : Check the n1 argument for specified PLC and error handling
// Configure the range of the input channel with
// the desired range code
void Galil::AnalogInputRange(uint8_t channel, uint8_t range) {
	if (channel > 7 || range > 4 || range < 1) return;
	std::string command = "AQ " + std::to_string(channel) + "," + std::to_string(range) + ";";
	CommandSend(command);
}	

// ENCODER
// TODO: Error handling.
void Galil::WriteEncoder() {
	CommandSend("WE 0;");
}									// Manually Set the motor encoder value to zero (encoder channel 0)
// TODO: Error handling
// Read from motor Encoder (encoder channel 0)
int Galil::ReadEncoder() {
	CommandSend("QE 0;");
	int res =  std::stoi(ReadBuffer);
	return res;
};										

// CONTROL FUNCTIONS
// Set the desired setpoint for control loops, counts or counts/sec
void Galil::setSetPoint(int s) { setPoint = s; }	

// This should set it within the class not on the actual Galil.
// Gets the current setpoint stored in the class
double Galil::getSetPoint() { return setPoint; }

// Set the proportional gain of the controller used in controlLoop() of Position/SpeedControl
void Galil::setKp(double gain) { ControlParameters[0] = gain; }	

// This should set it within the class not on the actual Galil.
// Gets the current proportional gain stored in the class
double Galil::getKp() { return ControlParameters[0]; }

// Set the integral gain of the controller used in controlLoop()  of Position/SpeedControl
void Galil::setKi(double gain) { ControlParameters[1] = gain; }	

// This should set it within the class not on the actual Galil.
// Gets the current integral gain stored in the class
double Galil::getKi() { return ControlParameters[1]; }											

// Set the derivative gain of the controller used in controlLoop()  of Position/SpeedControl
void Galil::setKd(double gain) { ControlParameters[2] = gain; }								
// This should set it within the class not on the actual Galil.

// Gets the current derivative gain stored in the class
double Galil::getKd() { return ControlParameters[2]; };										

// Run the control loop. ReadEncoder() is the input to the loop. The motor is the output.
void PositionControl(bool debug, int Motorchannel);		

// The loop will run using the PID values specified in the data of this object, and has an 
// automatic timeout of 10s. You do NOT need to implement this function, it is defined in
// GalilControl.lib
// same as above. Setpoint interpreted as counts per second
void SpeedControl(bool debug, int Motorchannel);		


// OPERATOR OVERLOADS

std::ostream& operator<<(std::ostream& output, Galil& galil) {
	char infoBuf[1024] = { 0 };
	char verBuf[1024] = { 0 };
	galil.Functions->GInfo(galil.g, infoBuf, static_cast<GSize>(sizeof(infoBuf)));
	output << infoBuf << "\n\n";

	galil.Functions->GVersion(verBuf, static_cast<GSize>(sizeof(verBuf)));
	output << verBuf << "\n\n";

	return output;
};	

// Operator overload for '<<' operator. So the user can say cout << Galil;
// This function should print out the output of GInfo and GVersion, with
// two newLines after each.


// Copy assignment operator. This acts in the same way as the copy constructor
Galil& Galil::operator=(const Galil& other) {
	if (this == &other) {
		return *this; 
	}

	Functions->GClose(g);
	if (NeedToBeCleared) delete Functions;

	Functions = new EmbeddedFunctions();
	NeedToBeCleared = true;
	setPoint = other.setPoint;
	ControlParameters[0] = other.ControlParameters[0];
	ControlParameters[1] = other.ControlParameters[1];
	ControlParameters[2] = other.ControlParameters[2];
	GalilAddress = other.GalilAddress;

	Functions->GOpen(GalilAddress.c_str(), &g);
	ReadBuffer[0] = '\0';                 
	BytesRead = 0;
	return *this;
}									




