#include "global.h"
#include "automaton.h"
using namespace std;

// Methods involved in VHDL generation


// Goes through the steps to prepare the sandbox in VHDL
void generateSandbox_VHDL(signal_set referenceSignalSet, automatonSet allAutomata)
{
  // Checker
  generateChecker(referenceSignalSet, allAutomata);

  // TODO
  // Other modules

}


// Goes through the steps the generate the checker in VHDL
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
  vhdl_out << "-- Component Checker Module" << endl;
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
  for(int signalIter_reference = 0; signalIter_reference < referenceSignalSet.size(); signalIter_reference++)
  {
    vhdl_out << "   -- " << signalIter_reference << " - " << referenceSignalSet[signalIter_reference].ID << endl;
  }
  vhdl_out << endl;

  // Create the checker entity
  vhdl_out << "-- Define the automaton checking entity" << endl;
  vhdl_out << "entity Checker is" << endl;
  vhdl_out << "\tport(" << endl;
  vhdl_out << "\t\tControlClock            : in  std_logic;\t-- Control clock assertions will allow transition to occur" << endl;

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
