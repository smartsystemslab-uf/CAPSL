#include <iomanip>

#include "global.h"
#include "automaton.h"

using namespace std;

// Methods involved in VHDL generation


// Goes through the steps to prepare the sandbox in VHDL
void generateSandbox_VHDL(signal_set referenceSignalSet, automatonSet allAutomata)
{
  // Checker
  generateChecker(referenceSignalSet, allAutomata);

	// Virtual Resource Manager
	generateManager(referenceSignalSet, allAutomata);

	// Handware Sandbox Module
	generateHWSandbox(referenceSignalSet, allAutomata);

  // TODO
  // Other modules
}

// --------------------------------------
// Checker.vhd
// --------------------------------------
void generateChecker(signal_set referenceSignalSet, automatonSet allAutomata)
{
  // The steps in creating this checker in VHDL are:
  //  1. Generate the header to the file
  //     - inputs, outputs, declarations etc
  //  2. Step through automata set and generate each in VHDL
  //  3. Add final bits of syntax to the file

  // Set the output file
  ofstream vhdl_out("outputs/vhdl/checker.vhd");

  // Print header
  vhdl_out << "library IEEE;" << endl;
  vhdl_out << "use IEEE.std_logic_1164.all;" << endl;
  vhdl_out << endl;

  // Print description
  vhdl_out << "-- Compenent Checker Module" << endl;
  vhdl_out << "--	This file has been generated from the component configurations provided. The" << endl;
  vhdl_out << "--    checker module simply determines component behavior to be in line with the" << endl;
  vhdl_out << "--    expected behavior or not." << endl;
  vhdl_out << "--  If illegal behavior is detected, the appropriate illegal state will be asserted and" << endl;
  vhdl_out << "--    no further transitions will be allowed. Current state will be locked." << endl;
  vhdl_out << "--	Accepting state detections are asserted upon transitions to end states. Transitions" << endl;
  vhdl_out << "--    can continue according to configured behavior. Current state will not be locked" << endl;
  vhdl_out << "--    so long as transitions exist to progress the automata" << endl;
  vhdl_out << endl;

	// Print signal info
  vhdl_out << "-- The signal set is indexed as listed below:" << endl;
  for(int signalIter = 0; signalIter < referenceSignalSet.size(); signalIter++)
  {
    vhdl_out << "   -- " << signalIter << " - " << referenceSignalSet[signalIter].ID << endl;
  }
  vhdl_out << endl;

	// Create the checker entity
	vhdl_out << "-- Define the automaton checking entity" << endl;
	vhdl_out << "entity Checker is" << endl;
	vhdl_out << "\tport(" << endl;
	vhdl_out << "\t\tControlClock            : in  std_logic;\t-- This may not be needed" << endl;

	// Create SignalSet port
  //  This port size is determined by the number of signals available to the
  //  component to be checked, this classes signalSet
  vhdl_out << "\t\tSignalSet               : in  std_logic_vector(" << referenceSignalSet.size()-1 << " downto 0);\t-- All signals are input here to determine the correct transition" << endl;


  // Loop through the automata set and get the number of accepting and illegal states for each
  int numAcceptingStates = 0;
  int numIllegalStates = 0;

  for(int automataIter = 0; automataIter < allAutomata.size(); automataIter++)
  {
    // Keep count
    numIllegalStates += allAutomata[automataIter].numIllegalStates;
    numAcceptingStates += allAutomata[automataIter].numAcceptingStates;

    // cout << "THIS NUM ILLEGAL: " << allAutomata[automataIter].numIllegalStates << endl;
    // cout << "THIS NUM ACCEPTING: " << allAutomata[automataIter].numAcceptingStates << endl;
    // cout << "NUM ILLEGAL: " << numIllegalStates << endl;
    // cout << "NUM ACCEPTING: " << numAcceptingStates << endl;
  }

  // TODO - Should I make this port optional? If no accepting states, then what?
  // Create EndStateDetections port
  //  This port size is determined by the number of accepting states specified
  //  in the IA configuration
  if(numAcceptingStates != 0)
  {
    vhdl_out << "\t\tEndStateDetections      : out std_logic_vector(" << numAcceptingStates-1 << " downto 0);\t-- End state detections are asserted via this port" << endl;
  }

  if(numIllegalStates != 0)
  {
    // Create IllegalStateDetections port
    //  This port size is determined by the number of illegal states
    vhdl_out << "\t\tIllegalStateDetections  : out std_logic_vector(" << numIllegalStates-1 << " downto 0)\t-- Illegal state detections are asserted via this port" << endl;
  }

  // Finish entity declaration
  vhdl_out << "\t);" << endl;
  vhdl_out << "end Checker;" << endl;
  vhdl_out << endl;

  // Set VHDL file type
  vhdl_out << "architecture STRUCTURAL of Checker is" << endl;
  vhdl_out << endl;

  // Loop through the automata set and get the states for each of the automata
  for(int automataIter = 0; automataIter < allAutomata.size(); automataIter++)
  {
    // Specify the originating automata of the states
    vhdl_out << "\t--------------------------" << endl;
    vhdl_out << "\t-- States - Automata " << automataIter << endl;
    vhdl_out << "\t--------------------------" << endl;

    // Declare the states - keep the signals formatted nicely
    vhdl_out << "\tsignal ";
    int i;
    for(i = 0; i < allAutomata[automataIter].stateSet.size()-1; i++)
    {
      // 20 states per line
      if(i % 20 == 0 && i != 0)
      {
        vhdl_out << "S" << automataIter << "_" << i <<" : std_logic;" << endl;
        vhdl_out <<"\tsignal ";
      }
      else
        vhdl_out << "S" << automataIter << "_" << i <<", ";
    }
    vhdl_out << "S" << automataIter << "_" << i << " : std_logic;\n ";
    vhdl_out << endl << endl;
  }

  // Add begin statement
  vhdl_out << "begin" << endl;
  vhdl_out << endl;

  int illegalStateIter = 0;

  // Loop through the automata set and generate their VHDL
  for(int automataIter = 0; automataIter < allAutomata.size(); automataIter++)
  {
    vhdl_out << "\t--------------------------" << endl;
    vhdl_out << "\t-- Logic - Automata " << automataIter << endl;
    vhdl_out << "\t--------------------------" << endl << endl;


    std::stringstream automataCode;

    // Get the VHDL and write to the file
    automataCode = allAutomata[automataIter].generateAutomata_VHDL(automataIter, &illegalStateIter, referenceSignalSet);
    vhdl_out << automataCode.rdbuf();
    vhdl_out << endl;
  }

  // Close the architecture declaration
  vhdl_out << "end STRUCTURAL;" << endl;
}



// --------------------------------------
// Manager.vhd
// --------------------------------------
void generateManager(signal_set referenceSignalSet, automatonSet allAutomata)
{
	unsigned long buffer = 2;			// buffer space between the two sides of each statement
	unsigned long width = 12 + buffer;			// default amount of space until (:) or (<=, =>) for signals (dynamically determined)
	unsigned long signalWidth = 8 + buffer;			// width for generated signals (dynamic)
	unsigned long setWidth = 16 + buffer;			// width for SignalSet_in(?)

	// remove internal signals from the signal set (VRM does not need them)
	// int signalIter = 0;
	// for (auto &signalRef : referenceSignalSet)
	// {
	// 	if (signalRef.type == signalType::internal)
	// 	{
	// 		referenceSignalSet.erase(referenceSignalSet.begin() + signalIter);
	// 	}
	// 	signalIter++;
	// }

  // determine width of each statement for vhdl
	for (auto &signalRef : referenceSignalSet) {
		if (signalRef.ID.length() > signalWidth - buffer)
			signalWidth = signalRef.ID.length() + buffer;
	}
	// increment setWidth to accommadate double digit signalsets
	if (referenceSignalSet.size() > 10)
		setWidth++;

	//! Begin writing to vhdl file
  // Set the output file
  ofstream vhdl_out("outputs/vhdl/manager.vhd");

  // Print header
  vhdl_out << "library IEEE;" << endl;
  vhdl_out << "use IEEE.std_logic_1164.all;" << endl;
  vhdl_out << endl;

  // Print description
  vhdl_out << "-- Virtual Resource Manager Module (VRM)" << endl;
	// TODO more info
	vhdl_out << "--  Additional info..." << endl;
	vhdl_out << endl;

	// Print signal info
	vhdl_out << "-- The signal set is indexed as listed below:" << endl;
	for(int signalIter = 0; signalIter < referenceSignalSet.size(); signalIter++)
	{
		vhdl_out << "   -- " << signalIter << " - " << referenceSignalSet[signalIter].ID << endl;
	}
	vhdl_out << endl;
  /*
	vhdl_out << "\t\t-- Checker Ports - out" << endl;
	// Loop through reference signals again and declare outputs (both checker inputs and outputs)
	for(int signalIter = 0; signalIter < referenceSignalSet.size(); signalIter++)
	{
    if (signalIter ==  (referenceSignalSet.size() - 1)){
		    outputNames.push_back("out_" + referenceSignalSet[signalIter].ID);
		    vhdl_out << "\t\t" << setw(15) << left << outputNames[signalIter] << " : out  std_logic\t-- "
						 << (referenceSignalSet[signalIter].type == input ? "output to IP" : "output to Interface") << endl;
    }
    else {
      outputNames.push_back("out_" + referenceSignalSet[signalIter].ID);
      vhdl_out << "\t\t" << setw(15) << left << outputNames[signalIter] << " : out  std_logic;\t-- "
           << (referenceSignalSet[signalIter].type == input ? "output to IP" : "output to Interface") << endl;
    }
	}
*/
	// Create the manager entity
	vhdl_out << "-- Define the resource manager entity" << endl;
	vhdl_out << "entity Manager is" << endl;
	vhdl_out << "\tport(" << endl;
	// Enable
  vhdl_out << "\t\t" << setw(width) << left << "Enable" << " : in  std_logic;"
	         << "\t-- This signal will be used to control whether IP will be connected through VRM unit or not. Enable is high when connected." << endl;
	// SignalSet_in
	vhdl_out << "\t\t" << setw(width) << left << "SignalSet_in" << " : in  std_logic_vector(" << referenceSignalSet.size() << " downto 0);"
	         << "\t-- The set of sandboxed signals that are input to the VRM." << endl;
	// SignalSet_out
	vhdl_out << "\t\t" << setw(width) << left << "SignalSet_out" << " : out  std_logic_vector(" << referenceSignalSet.size() << " downto 0)"
	         << "\t-- The set of sandboxed signals that are output from the VRM." << endl;

  // Finish entity declaration
  vhdl_out << "\t);" << endl;
  vhdl_out << "end Manager;" << endl;
  vhdl_out << endl;

  // Set VHDL file type
  vhdl_out << "architecture Behavioral of Manager is" << endl;

  // Create individual signals for each signal in the signal set
	vhdl_out << endl;
	vhdl_out << "\t--------------------------" << endl;
	vhdl_out << "\t-- Signals                " << endl;
	vhdl_out << "\t--------------------------" << endl;

	for (auto &signalRef : referenceSignalSet)
	{
		vhdl_out << "\tsignal " << setw(signalWidth) << left << signalRef.ID << " : std_logic;" << endl;
	}

  // Add begin statement
	vhdl_out << endl;
  vhdl_out << "begin" << endl;
  vhdl_out << endl;

  // Populate signals
	vhdl_out << "\t--------------------------" << endl;
	vhdl_out << "\t-- Initialize Signals     " << endl;
	vhdl_out << "\t--------------------------" << endl;

	for (int signalIter = 0; signalIter < referenceSignalSet.size(); signalIter++)
	{
		vhdl_out << "\tsignal " << setw(signalWidth) << left << referenceSignalSet[signalIter].ID
		         << " <= " << "SignalSet_in(" << signalIter << ");" << endl;
	}

	// Define behavior based on Enable signal
	vhdl_out << endl;
	vhdl_out << "\t--------------------------" << endl;
	vhdl_out << "\t-- Enable Process         " << endl;
	vhdl_out << "\t--------------------------" << endl;

  vhdl_out << "\tprocess(Enable)" << endl;
  vhdl_out << "\tbegin" << endl;

  vhdl_out << "\t\tif Enable = '1' then " << endl;
	for (int signalIter = 0; signalIter < referenceSignalSet.size(); signalIter++)
	{
		vhdl_out << "\t\t\t" << setw(setWidth) << left << "SignalSet_out(" + to_string(signalIter) + ")"
		         << " <= " << referenceSignalSet[signalIter].ID << ";" << endl;
	}
  vhdl_out << "\t\telse " << endl;
  for (int signalIter = 0; signalIter < referenceSignalSet.size(); signalIter++)
	{
		vhdl_out << "\t\t\t" << setw(setWidth) << left << "SignalSet_out(" + to_string(signalIter) + ")"
		         << " <= " << "\'X\'" << ";" << endl;
	}
  vhdl_out << "\t\tend if;" << endl;

  vhdl_out << "\tend process;" << endl;
/*
  vhdl_out << "\t--------------------------" << endl;
	vhdl_out << "\t-- Defining output depending on Controller value     " << endl;
	vhdl_out << "\t--------------------------" << endl;

  vhdl_out << "process(Controller)"<<endl;
  vhdl_out << "begin" <<endl;
  vhdl_out <<"\tif Controller = '1' then "<<endl;
	for(int signalIter = 0; signalIter < referenceSignalSet.size(); signalIter++)
	{
		vhdl_out << "\t\t" << setw(15) << left << outputNames[signalIter] << " <= " << signalNames[signalIter] << ";" << endl;
	}
  vhdl_out <<"else "<<endl;
  for(int signalIter = 0; signalIter < referenceSignalSet.size(); signalIter++)
	{
		vhdl_out << "\t\t" << setw(15) << left << outputNames[signalIter] << " <= " << "\'X\'" << ";" << endl;
	}
  vhdl_out <<"\tend if;"<<endl;
  vhdl_out << "end process;" <<endl;
  */
  // Close the architecture declaration
  vhdl_out << "end Behavioral;" << endl;
}



// --------------------------------------
// Sandbox.vhd
// --------------------------------------
void generateHWSandbox(signal_set referenceSignalSet, automatonSet allAutomata)
{
	unsigned long buffer = 2;			// minimum amount of space between end of signal and next symbol
	unsigned long width = 10 + buffer;			// default amount of space until (:) or (<=, =>)
	unsigned long checkerWidth = 22 + buffer;
	unsigned long managerWidth = 13 + buffer;
	unsigned long signalWidth = 16 + buffer;

	// // remove internal signals from the signal set (VRM does not need them)
	// int signalIter = 0;
	// for (auto &signalRef : referenceSignalSet)
	// {
	// 	if (signalRef.type == signalType::internal)
	// 	{
	// 		referenceSignalSet.erase(referenceSignalSet.begin() + signalIter);
	// 	}
	// 	signalIter++;
	// }

	// determine width of each signal
	for (auto &signalRef : referenceSignalSet)
	{
		if (signalRef.ID.length() > signalWidth - buffer)
			signalWidth = signalRef.ID.length() + buffer;
	}

	//! Begin writing to vhdl file
	// Set the output file
	ofstream vhdl_out("outputs/vhdl/sandbox.vhd");

	// Print header
	vhdl_out << "library IEEE;" << endl;
	vhdl_out << "use IEEE.std_logic_1164.all;" << endl;
	vhdl_out << endl;

	// Print description
	vhdl_out << "-- Hardware Sandbox Module" << endl;
	// TODO more info
	vhdl_out << "--  Additional info..." << endl;
	vhdl_out << endl;

	// Print signal info
	vhdl_out << "-- The signal set is indexed as listed below:" << endl;
	for(int signalIter = 0; signalIter < referenceSignalSet.size(); signalIter++)
	{
		vhdl_out << "   -- " << signalIter << " - " << referenceSignalSet[signalIter].ID << endl;
	}
	vhdl_out << endl;

	//! Create the Sandbox entity
	vhdl_out << "-- Define the hardware sandbox entity" << endl;
	vhdl_out << "entity Sandbox is" << endl;
	vhdl_out << "\tport(" << endl;
	// Sandbox ports
	vhdl_out << "\t\t" << setw(width) << left << "clk" << " : " << setw(4) << "in" << "std_logic;" << endl;
	vhdl_out << "\t\t" << setw(width) << left << "SignalSet" << " : " << setw(4) << "in" << "std_logic_vector(" << referenceSignalSet.size()-1 << " downto 0)" << endl;
	// finish entity declaration
	vhdl_out << "\t);" << endl;
	vhdl_out << "end Sandbox;" << endl;
	vhdl_out << endl;

	//! Define Architecture
	vhdl_out << "architecture Behavioral of Sandbox is" << endl;
	vhdl_out << endl;


	//! Define Components
	vhdl_out << "\t--------------------------" << endl;
	vhdl_out << "\t-- Components             " << endl;
	vhdl_out << "\t--------------------------" << endl;

	// Checker component
	vhdl_out << "\tcomponent Checker" << endl;
	vhdl_out << "\t\tport(" << endl;
	vhdl_out << "\t\t\t" << setw(checkerWidth) << left << "ControlClock" << " : " << setw(4) << "in" << "std_logic;" << endl;
	// SignalSet port
	vhdl_out << "\t\t\t" << setw(checkerWidth) << left << "SignalSet" << " : " << setw(4) << "in" << "std_logic_vector(" << referenceSignalSet.size()-1 << " downto 0)";
	// Loop through the automata set and get the number of accepting and illegal states for each
	int numAcceptingStates = 0;
	int numIllegalStates = 0;
	for(int automataIter = 0; automataIter < allAutomata.size(); automataIter++)
	{
		numIllegalStates += allAutomata[automataIter].numIllegalStates;
		numAcceptingStates += allAutomata[automataIter].numAcceptingStates;
	}
	// EndStateDetections port
	if(numAcceptingStates != 0)
	{
		// compensate for longer port name
		checkerWidth = 18 + buffer;
		// semicolon and newline occur here
		vhdl_out << ";" << endl;
		vhdl_out << "\t\t\t" << setw(checkerWidth) << left << "EndStateDetections" << " : " << setw(4) << "out" << "std_logic_vector(" << numAcceptingStates-1 << " downto 0)";
	}
	// IllegalStateDetections port
	if(numIllegalStates != 0)
	{
		// compensate for longer port name
		checkerWidth = 22 + buffer;
		// semicolon and newline occur here
		vhdl_out << ";" << endl;
		vhdl_out << "\t\t\t" << setw(checkerWidth) << left << "IllegalStateDetections" << " : " << setw(4) << "out" << "std_logic_vector(" << numIllegalStates-1 << " downto 0)";
	}
	vhdl_out << endl;
	vhdl_out << "\t\t);" << endl;
	vhdl_out << "\tend component;" << endl;
	vhdl_out << endl;

	// TODO
	// Manager component
	vhdl_out << "\tcomponent Manager" << endl;
	vhdl_out << "\t\tport(" << endl;
	vhdl_out << "\t\t\t" << setw(managerWidth) << left << "Enable" << " : " << setw(4) << "in" << "std_logic;" << endl;
	// input ports, then output ports
	// for (int signalIter = 0; signalIter < referenceSignalSet.size(); signalIter++)
	// {
	// 	vhdl_out << "\t\t\t" << setw(managerWidth) << left << signalRef.ID << " : in  std_logic;" << endl;
	// }
	// for (int signalIter = 0; signalIter < referenceSignalSet.size(); signalIter++)
	// {
	// 	// last signal, omit the semicolon
	// 	if (signalIter == referenceSignalSet.size() - 1)
	// 		vhdl_out << "\t\t\t" << setw(managerWidth) << left << outputTag + referenceSignalSet[signalIter].ID
	// 		         << " : out  std_logic" << endl;
	// 	else
	// 		vhdl_out << "\t\t\t" << setw(managerWidth) << left << outputTag + referenceSignalSet[signalIter].ID
	// 		         << " : out  std_logic;" << endl;
	// }
	vhdl_out << "\t\t\t" << setw(managerWidth) << left << "SignalSet_in" << " : " << setw(4) << "in" << "std_logic_vector(" << referenceSignalSet.size() << " downto 0);" << endl;
	vhdl_out << "\t\t\t" << setw(managerWidth) << left << "SignalSet_out" << " : " << setw(4) << "out" << "std_logic_vector(" << referenceSignalSet.size() << " downto 0)" << endl;
	vhdl_out << "\t\t);" << endl;
	vhdl_out << "\tend component;" << endl;
	vhdl_out << endl;


	//! Write signals
	vhdl_out << "\t--------------------------" << endl;
	vhdl_out << "\t-- Signals                " << endl;
	vhdl_out << "\t--------------------------" << endl;
	vhdl_out << "\tsignal " << setw(signalWidth) << left << "enable" << " : std_logic;\t--Enable connection" << endl;
	if (numAcceptingStates != 0)
	{
		vhdl_out << "\tsignal " << setw(signalWidth) << left << "accept" << " : std_logic_vector(" << numAcceptingStates << " downto 0);"
		         << "\t--Set of accepting state detections" << endl;
	}
	if (numIllegalStates != 0)
	{
		vhdl_out << "\tsignal " << setw(signalWidth) << left << "illegal" << " : std_logic_vector(" << numIllegalStates << " downto 0);"
		         << "\t--Set of illegal state detections" << endl;
	}

	// TODO NOTE not sure what to do with these yet, essentially they are the final outputs
	vhdl_out << "\tsignal " << setw(signalWidth) << left << "sandboxSignalSet" << " : std_logic_vector(" << referenceSignalSet.size() << " downto 0);"
	         << "\t-- All signals of the automata that are output from VRM" << endl;
	vhdl_out << endl;

	// Add begin statement
	vhdl_out << "begin" << endl;
	vhdl_out << endl;


	//! Add component initialization
	//   This section is written weirdly to avoid writing too many commas in the port maps

	// Checker port map
	vhdl_out << "\tchecker:   Checker port map(" << endl;
	vhdl_out << "\t\t" << setw(checkerWidth) << left << "ControlClock" << " => clk," << endl;
	vhdl_out << "\t\t" << setw(checkerWidth) << left << "SignalSet" << " => SignalSet";
	// acceptingStates if they exist
	if (numAcceptingStates != 0)
	{
		vhdl_out << "," << endl;
		vhdl_out << "\t\t" << setw(checkerWidth) << left << "EndStateDetections" << " => accept";
	}
	// illegalStates if they exist
	if (numIllegalStates != 0)
	{
		vhdl_out << "," << endl;
		vhdl_out << "\t\t" << setw(checkerWidth) << left << "IllegalStateDetections" << " => illegal";
	}
	vhdl_out << endl;
	vhdl_out << "\t);" << endl;
	vhdl_out << endl;

	// Manager port map
	vhdl_out << "\tmanager:   Manager port map(" << endl;
	vhdl_out << "\t\t" << setw(managerWidth) << left << "Enable" << " => enable," << endl;
	vhdl_out << "\t\t" << setw(managerWidth) << left << "SignalSet_in" << " => SignalSet," << endl;
	vhdl_out << "\t\t" << setw(managerWidth) << left << "SignalSet_out" << " => sandboxSignalSet" << endl;
	vhdl_out << "\t);" << endl;
	vhdl_out << endl;


	//! Define behavior based on IllegalStateDetections signal
	vhdl_out << endl;
	vhdl_out << "\t----------------------------------" << endl;
	vhdl_out << "\t-- Illegal State Detection Process     " << endl;
	vhdl_out << "\t----------------------------------" << endl;

	// If no illegal states, not sure what else to do, end here
	if (numIllegalStates == 0)
	{
		vhdl_out << endl;
		// Close the architecture declaration
		vhdl_out << "-- Note: no illegal states" << endl;
		vhdl_out << endl;
		vhdl_out << "end Behavioral;" << endl;
		return;
	}

	vhdl_out << "\tprocess(illegal)" << endl;
	vhdl_out << "\tbegin" << endl;

	vhdl_out << "\t\tif illegal /= \"";
	for (int i = 0; i < numIllegalStates; i++)
	{
		vhdl_out << "0";
	}
	vhdl_out << "\" then" << endl;
	vhdl_out << "\t\t\tenable <= '0';" << endl;
	vhdl_out << "\t\telse " << endl;
	vhdl_out << "\t\t\tenable <= '1';" << endl;
	vhdl_out << "\t\tend if;" << endl;
	vhdl_out << "\tend process;" << endl;

	// Close the architecture declaration
	vhdl_out << "end Behavioral;" << endl;
}
