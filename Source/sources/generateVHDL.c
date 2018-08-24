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
	// Local containers
	vector<string> inputNames;
	vector<string> outputNames;
	vector<string> signalNames;

	string inputTag = "in_";
	string outputTag = "out_";
	string signalTag = "s_";

	unsigned long width = 12;			// default amount of space until (:) or (<=, =>)
	unsigned long buffer = 3;			// minimum amount of space between end of signal and next symbol

	// remove internal signals from the signal set (VRM does not need them)
	int signalIter = 0;
  for (auto &signalRef : referenceSignalSet)
  {
    if (signalRef.type == signalType::internal)
    {
			referenceSignalSet.erase(referenceSignalSet.begin() + signalIter);
    }
	  signalIter++;
  }

  // determine width of each statement for vhdl
	for (auto &signalRef : referenceSignalSet) {
		if (signalRef.type == signalType::input || signalRef.type == signalType::output)
		{
			if (signalRef.ID.length() > width - buffer)
			{
				width = signalRef.ID.length() + buffer;
			}
		}
	}

	//! Begin writing to vhdl file
  // Set the output file
  ofstream vhdl_out("outputs/vhdl/manager.vhd");

  // Print header
  vhdl_out << "library IEEE;" << endl;
  vhdl_out << "use IEEE.std_logic_1164.all;" << endl;
  vhdl_out << endl;

  // Print description
  vhdl_out << "-- Virtual Resource Manager Module" << endl;
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

	// Create the manager entity
	vhdl_out << "-- Define the resource manager entity" << endl;
	vhdl_out << "entity Manager is" << endl;
	vhdl_out << "\tport(" << endl;
  vhdl_out << "\t\t" << setw(width) << left << "Enable" << " : in  std_logic;\t-- This signal will be used to control whether IP will be connected through VRM unit or not. Enable is high when connected." << endl;

	vhdl_out << endl;
	vhdl_out << "\t\t-- Checker Ports - in" << endl;
	// Loop through reference signals and declare inputs
	for(int signalIter = 0; signalIter < referenceSignalSet.size(); signalIter++)
	{
		inputNames.push_back(inputTag + referenceSignalSet[signalIter].ID);
		vhdl_out << "\t\t" << setw(width + inputTag.length()) << left
						 << inputNames[signalIter] << ": in  std_logic;\t-- Checker "
						 << (referenceSignalSet[signalIter].type == input ? "input" : "output") << endl;
	}

	vhdl_out << endl;
	vhdl_out << "\t\t-- Checker Ports - out" << endl;
	// Loop through reference signals again and declare outputs
	for(int signalIter = 0; signalIter < referenceSignalSet.size(); signalIter++)
	{
		outputNames.push_back(outputTag + referenceSignalSet[signalIter].ID);
		vhdl_out << "\t\t" << setw(width + outputTag.length()) << left << outputNames[signalIter] << ": out  std_logic;\t-- Checker "
						 << (referenceSignalSet[signalIter].type == input ? "input" : "output") << endl;
	}

  // Finish entity declaration
  vhdl_out << "\t);" << endl;
  vhdl_out << "end Manager;" << endl;
  vhdl_out << endl;

  // Set VHDL file type
  vhdl_out << "architecture Behavioral of Manager is" << endl;

	// TODO implement vector functionality for input/output signals here

  // Loop through signals
	vhdl_out << endl;
	vhdl_out << "\t--------------------------" << endl;
	vhdl_out << "\t-- Signals                " << endl;
	vhdl_out << "\t--------------------------" << endl;
	for(int signalIter = 0; signalIter < referenceSignalSet.size(); signalIter++)
	{
		signalNames.push_back(signalTag + referenceSignalSet[signalIter].ID);
		vhdl_out << "\tsignal " << setw(width + signalTag.length()) << left
						 << signalNames[signalIter] << ": std_logic;" << endl;
	}

  // Add begin statement
	vhdl_out << endl;
  vhdl_out << "begin" << endl;
  vhdl_out << endl;

  // Populate signals?
	vhdl_out << "\t--------------------------" << endl;
	vhdl_out << "\t-- Read Checker ports     " << endl;
	vhdl_out << "\t--------------------------" << endl;

	for(int signalIter = 0; signalIter < referenceSignalSet.size(); signalIter++)
	{
		vhdl_out << "\t" << setw(width + signalTag.length()) << left
						 << signalNames[signalIter] << "<= " << inputNames[signalIter] << ";" << endl;
	}

	// Define behavior based on Controller signal
	vhdl_out << endl;
	vhdl_out << "\t--------------------------" << endl;
	vhdl_out << "\t-- Controller Process     " << endl;
	vhdl_out << "\t--------------------------" << endl;

  vhdl_out << "\tprocess(Enable)" << endl;
  vhdl_out << "\tbegin" << endl;

  vhdl_out << "\t\tif Enable = '1' then " << endl;
	for(int signalIter = 0; signalIter < referenceSignalSet.size(); signalIter++)
	{
		vhdl_out << "\t\t\t" << setw(width + outputTag.length()) << left << outputNames[signalIter] << "<= " << signalNames[signalIter] << ";" << endl;
	}
  vhdl_out << "\t\telse " << endl;
  for(int signalIter = 0; signalIter < referenceSignalSet.size(); signalIter++)
	{
		vhdl_out << "\t\t\t" << setw(width + outputTag.length()) << left << outputNames[signalIter] << "<= " << "\'X\'" << ";" << endl;
	}
  vhdl_out << "\t\tend if;" << endl;

  vhdl_out << "\tend process;" << endl;

  // Close the architecture declaration
  vhdl_out << "end Behavioral;" << endl;
}



// --------------------------------------
// Sandbox.vhd
// --------------------------------------
void generateHWSandbox(signal_set referenceSignalSet, automatonSet allAutomata)
{
	string inputTag = "in_";
	string outputTag = "out_";
	string signalTag = "s_";

	unsigned long width = 12;			// default amount of space until (:) or (<=, =>)
	unsigned long buffer = 3;			// minimum amount of space between end of signal and next symbol

	// remove internal signals from the signal set (VRM does not need them)
	int signalIter = 0;
	for (auto &signalRef : referenceSignalSet)
	{
		if (signalRef.type == signalType::internal)
		{
			referenceSignalSet.erase(referenceSignalSet.begin() + signalIter);
		}
		signalIter++;
	}

	// determine width of each statement for vhdl
	for (auto &signalRef : referenceSignalSet)
	{
		if (signalRef.type == signalType::input || signalRef.type == signalType::output)
		{
			if (signalRef.ID.length() > width - buffer)
			{
				width = signalRef.ID.length() + buffer;
			}
		}
	}
	// add the length of the longest tag
	width += outputTag.length();

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
	vhdl_out << "\t\t" << setw(width) << left << "clk" << ": in  std_logic;" << endl;
	vhdl_out << "\t\t" << setw(width) << left << "SignalSet" << ": in  std_logic_vector(" << referenceSignalSet.size()-1 << " downto 0);" << endl;
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
	vhdl_out << "\t\t\tControlClock            : in  std_logic;" << endl;
	// SignalSet port
	vhdl_out << "\t\t\tSignalSet               : in  std_logic_vector(" << referenceSignalSet.size()-1 << " downto 0);" << endl;
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
		vhdl_out << "\t\t\tEndStateDetections      : out std_logic_vector(" << numAcceptingStates-1 << " downto 0);" << endl;
	}
	// IllegalStateDetections port
	if(numIllegalStates != 0)
	{
		vhdl_out << "\t\t\tIllegalStateDetections  : out std_logic_vector(" << numIllegalStates-1 << " downto 0)" << endl;
	}
	vhdl_out << "\t\t);" << endl;
	vhdl_out << "\tend component;" << endl;
	vhdl_out << endl;

	// Manager component
	vhdl_out << "\tcomponent Manager" << endl;
	vhdl_out << "\t\tport(" << endl;
	vhdl_out << "\t\t\t" << setw(width) << left << "Enable" << ": in  std_logic;" << endl;
	// input ports, then output ports
	for (int signalIter = 0; signalIter < referenceSignalSet.size(); signalIter++)
	{
		vhdl_out << "\t\t\t" << setw(width) << left << inputTag + referenceSignalSet[signalIter].ID
		         << ": in  std_logic;" << endl;
	}
	for (int signalIter = 0; signalIter < referenceSignalSet.size(); signalIter++)
	{
		// last signal, omit the semicolon
		if (signalIter == referenceSignalSet.size() - 1)
			vhdl_out << "\t\t\t" << setw(width) << left << outputTag + referenceSignalSet[signalIter].ID
			         << ": out  std_logic" << endl;
		else
			vhdl_out << "\t\t\t" << setw(width) << left << outputTag + referenceSignalSet[signalIter].ID
			         << ": out  std_logic;" << endl;
	}
	vhdl_out << "\t\t);" << endl;
	vhdl_out << "\tend component;" << endl;
	vhdl_out << endl;


	//! Write signals
	vhdl_out << "\t--------------------------" << endl;
	vhdl_out << "\t-- Signals                " << endl;
	vhdl_out << "\t--------------------------" << endl;
	vhdl_out << "\t" << setw(width + 3) << left << "signal enable: std_logic;" << endl;
	if(numAcceptingStates != 0)
	{
		vhdl_out << "\t" << setw(width + 3) << left << "signal accept: std_logic_vector(" << numAcceptingStates << " downto 0);\t--Set of accepting state detections" << endl;
	}
	if(numIllegalStates != 0)
	{
		vhdl_out << "\t" << setw(width + 3) << left << "signal illegal: std_logic_vector(" << numIllegalStates << " downto 0);\t--Set of illegal state detections" << endl;
	}
	vhdl_out << endl;
	// NOTE not sure what to do with these yet, essentially they are the final outputs
	vhdl_out << "\t-- All signals of the automata that are output from VRM" << endl;
	for(int signalIter = 0; signalIter < referenceSignalSet.size(); signalIter++)
	{
		vhdl_out << "\t\t" << setw(width + 3) << left << "signal " + referenceSignalSet[signalIter].ID << ": std_logic;" << endl;
	}
	vhdl_out << endl;

	// Add begin statement
	vhdl_out << "begin" << endl;
	vhdl_out << endl;


	//! Add component initialization
	//   This section is written weirdly to avoid writing too many commas in the port maps

	// Checker port map
	vhdl_out << "\tchecker:   Checker port map(" << endl;
	vhdl_out << "\t\t" << setw(17) << left << "ControlClock" << "=>  clk," << endl;
	vhdl_out << "\t\t" << setw(17) << left << "SignalSet" << "=>  SignalSet";
	// acceptingStates if they exist
	if (numAcceptingStates != 0)
	{
		vhdl_out << "," << endl;
		vhdl_out << "\t\t" << setw(27) << left << "AcceptingStateDetections" << "=>  accept";
	}
	// illegalStates if they exist
	if (numIllegalStates != 0)
	{
		vhdl_out << "," << endl;
		vhdl_out << "\t\t" << setw(27) << left << "IllegalStateDetections" << "=>  illegal" << endl;
	}
	vhdl_out << "\t);" << endl;
	vhdl_out << endl;

	// Manager port map
	vhdl_out << "\tmanager:   Manager port map(" << endl;
	vhdl_out << "\t\t" << setw(width) << left << "Enable" << "=>  enable," << endl;
	for (int signalIter = 0; signalIter < referenceSignalSet.size(); signalIter++)
	{
		vhdl_out << "\t\t" << setw(width) << left << inputTag + referenceSignalSet[signalIter].ID << "=>  SignalSet(" << signalIter << ")," << endl;
	}
	for (int signalIter = 0; signalIter < referenceSignalSet.size(); signalIter++)
	{
		if (signalIter == referenceSignalSet.size() - 1)    // no comma
			vhdl_out << "\t\t" << setw(width) << left << outputTag + referenceSignalSet[signalIter].ID << "=>  " << referenceSignalSet[signalIter].ID << endl;
		else
			vhdl_out << "\t\t" << setw(width) << left << outputTag + referenceSignalSet[signalIter].ID << "=>  " << referenceSignalSet[signalIter].ID << "," << endl;
	}
	vhdl_out << "\t);" << endl;
	vhdl_out << endl;


	// If no illegal states, not sure what else to do, end here
	if (numIllegalStates == 0)
	{
		// Close the architecture declaration
		vhdl_out << "end Behavioral;" << endl;
		return;
	}


	//! Define behavior based on IllegalStateDetections signal
	vhdl_out << endl;
	vhdl_out << "\t----------------------------------" << endl;
	vhdl_out << "\t-- Illegal State Detection Process     " << endl;
	vhdl_out << "\t----------------------------------" << endl;

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
//	vhdl_out << "--to do: create process. When any IllegalState is reached, toggle the Controller signal to disable the connection." << endl;

	// Close the architecture declaration
	vhdl_out << "end Behavioral;" << endl;
}