#include "EmbeddedFunctions.h"

static const int READ_TIMEOUT = 5000;

EmbeddedFunctions::EmbeddedFunctions() : client(nullptr), stream(nullptr) {}
EmbeddedFunctions::~EmbeddedFunctions() {
	try {
		GClose();
	}
	catch (Exception^ e) {
		 Console::WriteLine("GClose failed in destructor: " + e->Message);
	}

}

void EmbeddedFunctions::GOpen(String^ address, const int port) {
	if (String::IsNullOrWhiteSpace(address)) {
		throw gcnew ArgumentException("GOpen: Address is empty");
	}

	if (client != nullptr) {
		GClose();
	} 

	array<String^>^ parts = address->Trim()->Split(' ');
	String^ ip = parts[0];

	try {
		client = gcnew TcpClient();
		client->Connect(ip, port);
		stream = client->GetStream();
	}
	catch (Exception^ e) {
		if (client != nullptr) {
			client->Close();
		}
		client = nullptr;
		stream = nullptr;
		throw gcnew Exception("GOpen failed for " + ip + ": " + e->Message);
	}
}
void EmbeddedFunctions::GClose() {
	if (client == nullptr && stream == nullptr) {
		return;
	}

	String^ problem = nullptr;

	try {
		if (stream != nullptr) {
			stream->Close();
		}
	}
	catch (Exception^ e) {
		problem = e->Message;
	}

	try {
		if (client != nullptr) {
			client->Close();
		}
	}
	catch (Exception^ e) {
		if (problem == nullptr) {
			problem = e->Message;
		}
	}

	stream = nullptr;
	client = nullptr;

	if (problem != nullptr) {
		throw gcnew Exception("GClose failed: " + problem);
	}
}

String^ EmbeddedFunctions::GCommand(String^ command) {
	if (client == nullptr || stream == nullptr) {
		throw gcnew InvalidOperationException("GCommand: no open connection");
	}

	if (String::IsNullOrWhiteSpace(command)) {
		throw gcnew ArgumentException("GCommand: command is empty");
	}
	command = command->Trim();
	if (!command->EndsWith(";")) {
		command = command + ";";
	}
	try {
		array<Byte>^ outBytes = Text::Encoding::ASCII->GetBytes(command);
		stream->Write(outBytes, 0, outBytes->Length);

		stream->ReadTimeout = READ_TIMEOUT;                      
		array<Byte>^ inBytes = gcnew array<Byte>(1024);
		String^ reply = "";
		while (true) {
			int n = stream->Read(inBytes, 0, inBytes->Length);
			if (n <= 0) {
				throw gcnew Exception("connection closed by the controller");
			}
			reply = reply + Text::Encoding::ASCII->GetString(inBytes, 0, n);
			if (reply->EndsWith(":")) {
				break;
			}
		}
		return reply;
	}
	catch (Exception^ e) {
		throw gcnew Exception("GCommand failed for '" + command + "': " + e->Message);
	}

}

