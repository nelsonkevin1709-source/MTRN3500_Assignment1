

#include "EmbeddedFunctions.h"

	// Sends one command and prints the reply, with \r and \n made visible.
	static void run(EmbeddedFunctions ^ ef, String ^ label, String ^ cmd) {
		try {
			String^ r = ef->GCommand(cmd);
			Console::WriteLine("{0} -> [{1}]", label,
				r->Replace("\r", "\\r")->Replace("\n", "\\n"));
		}
		catch (Exception^ e) {
			Console::WriteLine("{0} -> ERROR: {1}", label, e->Message);
		}
	}





	//int main(void) {
	//	// Keep the values that worked in your last run.
	//	const int PORT = 26000;                      // <-- type YOUR port here
	//	String^ ADDR = "127.0.0.1";             // <-- type YOUR ip here (keep the " -d")

	//	EmbeddedFunctions^ ef = gcnew EmbeddedFunctions();

	////	// ---------- 1. before opening ----------
	////	Console::WriteLine("--- 1. before opening ---");
	//	run(ef, "command before GOpen", "OP 5,0;");               // expect ERROR: no open connection

	//	// ---------- 2. open ----------
	//	Console::WriteLine("--- 2. open ---");
	//	try {
	//		ef->GOpen(ADDR, PORT);
	//		Console::WriteLine("GOpen OK");
	//	}
	//	catch (Exception^ e) {
	//		Console::WriteLine("GOpen ERROR: " + e->Message);
	//		Console::WriteLine("Press Enter.");
	//		Console::ReadLine();
	//		return 1;                                              // nothing else can work
	//	}

	//	// ---------- 3. basic commands ----------
	//	Console::WriteLine("--- 3. basic commands ---");
	//	run(ef, "write OP 5,0;", "OP 5,0;");                       // expect [\n:]
	//	run(ef, "write without ;", "OP 0,0");                      // expect [\n:]  (';' was appended)
	//	run(ef, "read analog", "MG @AN[0];");                      // expect a number, then \n:
	//	run(ef, "bad command", "XYZ 5;");                          // expect a ? in the reply
	//	run(ef, "empty command", "");                              // expect ERROR: command is empty

	//	// ---------- 4. does a write show up on the inputs? ----------
	//	Console::WriteLine("--- 4. write then read back ---");
	//	run(ef, "write OP 5,0;", "OP 5,0;");
	//	run(ef, "read DI0 straight away (expect 1)", "MG @IN[0];");
	//	run(ef, "read DI1 straight away (expect 0)", "MG @IN[1];");
	//	run(ef, "read DI2 straight away (expect 1)", "MG @IN[2];");

	//	System::Threading::Thread::Sleep(300);                     // give the simulator time to update
	//	Console::WriteLine("(waited 300 ms)");
	//	run(ef, "read DI0 after pause (expect 1)", "MG @IN[0];");
	//	run(ef, "read DI1 after pause (expect 0)", "MG @IN[1];");
	//	run(ef, "read DI2 after pause (expect 1)", "MG @IN[2];");

	//	run(ef, "TI 0; (expect 5)", "TI 0;");                      // the command Part A used
	//	run(ef, "TI 1; (expect 0)", "TI 1;");

	//	// ---------- 5. close ----------
	//	Console::WriteLine("--- 5. close ---");
	//	try {
	//		ef->GClose();
	//		Console::WriteLine("GClose OK");
	//		run(ef, "command after GClose", "OP 5,0;");            // expect ERROR: no open connection
	//		ef->GClose();                                          // second close: no error
	//		Console::WriteLine("double GClose OK");
	//	}
	//	catch (Exception^ e) {
	//		Console::WriteLine("GClose ERROR: " + e->Message);
	//	}

	//	// ---------- 6. reopen ----------
	//	Console::WriteLine("--- 6. reopen ---");
	//	try {
	//		ef->GOpen(ADDR, PORT);
	//		run(ef, "write after reopen", "OP 0,0;");              // expect [\n:]
	//		ef->GClose();
	//	}
	//	catch (Exception^ e) {
	//		Console::WriteLine("Reopen ERROR: " + e->Message);
	//	}

	//	Console::WriteLine("Done. Press Enter.");
	//	Console::ReadLine();
	//	return 0;
	//}

	int main(void) {
		EmbeddedFunctions^ ef = gcnew EmbeddedFunctions();
		const int PORT = 26000;                      // <-- type YOUR port here
		String^ ADDR = "127.0.0.1";
		ef->GOpen(ADDR, PORT);
		Threading::Thread::Sleep(100);
		Console::WriteLine("Sending Command");
		String^ response = ef->GCommand("OP 255, 255;");
		Console::WriteLine("Command is " + response);
		ef->GClose();
		Console::ReadKey();

	}