#include "global.h"
#include "automaton.h"
using namespace std;

typedef vector<automaton> automatonSet;


// Print the usage instructions for the program
void printUsage()
{
	cout << "-------------------------------------------------------------------------------" << endl;
	cout << "Usage: ./capsl [CONFIG DIRECTORY]" << endl;
	cout << "\n Notes: " << endl;
	cout << "        * config directories should be stored in Source/config/<CONFIG DIRECTORY>" << endl;
	cout << "        * .ia and .sere files must be present in the chosen directory" << endl;
	cout << "        * BasicRSA is default when no directory is provided" << endl;
	cout << "\n Coming soon: " << endl;
	cout << "              * simple .config file for configuration" << endl;
	cout << "              * use --systemc for SystemC output - VHDL is default." << endl;
	cout << "-------------------------------------------------------------------------------" << endl;
}


// Adds automaton to a set of automaton
void addAutomaton(automaton toAdd, automatonSet *set)
{
  set->push_back(toAdd);
  cout << "   Adding automata to set without trying composition." << endl;
  return;
}

// Adds automaton to a set of automaton and attempts to compose with existing
void addAndComposeAutomaton(automaton toAdd, automatonSet *set)
{
  // Check compatibility with all others in the set
  for(int setIter = 0; setIter < set->size(); setIter++)
  {
    if(toAdd.canComposeWith((*set)[setIter]))
    {
      cout << "   Composing with existing automata..." << endl;

      // Compose the two automata and produce a single output
      automaton newAutomaton(toAdd, (*set)[setIter]);

      // Erase automata we composed with
      set->erase(set->begin() + setIter);

      // Replace with the new one
      set->push_back(newAutomaton);
      return;
    }
  }

  // If we didn't find any we could compose with, we just add it to the set
  set->push_back(toAdd);
  cout << "   No compatible automata in set, adding to set." << endl;
  return;
}

int main(int argc, char **argv)
{
  // TODO - get this from argument list
  //  But not sure since I need to grab about three files
  //  so perhaps I could just have the manual state to create
  //  these files in the config folder...
  //  Well, hardcoded for now

  // char *configFileName_SERE = "config/emptySERE.config";

  // char *configFileName_IA_Component = "config/serial.config";
  // char *configFileName_IA_System = "config/serial.config";
  // char *configFileName_SERE = "config/SERE.config";

  // char *configFileName_IA_Component = "config/easyIA_Component.config";
  // char *configFileName_IA_System = "config/easyIA_System.config";
  // char *configFileName_SERE = "config/easySERE.config";

  // These are IA specifications from the interface automata paper with empty SERE
  // char *configFileName_IA_Component = "config/messageIA.config";
  // char *configFileName_IA_Component = "config/userIA.config";
  // char *configFileName_IA_Component2 = "config/userIA.config";
  // char *configFileName_IA_System = "config/channelIA.config";
  // char *configFileName_IA_System = "config/messageIA.config";
  // char *configFileName_SERE = "config/message_userSERE.config";

  // Adder IA and SERE
  // char *configFileName_IA_Component = "config/adderIA_Component.config";
  // char *configFileName_IA_Component2 = "config/subtractorIA_Component.config";
  // char *configFileName_IA_System = "config/adderIA_System.config";
  // char *configFileName_SERE = "config/adderSERE.config";

  // UART IA and SERE
  // char *configFileName_IA_Component = "config/UART_IA_Component.config";
  // char *configFileName_IA_System = "config/UART_IA_System.config";
  // char *configFileName_SERE = "config/UART_SERE.config";

  // UserDeviceRelay
  // char *configFileName_IA_Component = "config/UserDeviceRelay/device.ia";
  // char *configFileName_IA_Component2 = "config/UserDeviceRelay/relay.ia";
  // char *configFileName_IA_Component3 = "config/UserDeviceRelay/user.ia";
  // char *configFileName_IA_System = "config/UserDeviceRelay/system.ia";
  // char *configFileName_SERE = "config/UserDeviceRelay/rules.sere";
  // char *configFileName_SERE = "config/empty.SERE";


  // BasicRSA-T100
  // char *configFileName_IA_Component = "config/BasicRSA/T100/T100.ia";
  // char *configFileName_IA_System = "config/BasicRSA/T100/system.ia";
  // char *configFileName_SERE = "config/BasicRSA/T100/rules.sere";

  // BasicRSA-T200
  // char *configFileName_IA_Component = "config/BasicRSA/T200.ia";
  // char *configFileName_IA_System = "config/BasicRSA/T200/system.ia";
  // char *configFileName_SERE = "config/BasicRSA/T200/rules.sere";

  // RS232-T100
  // char *configFileName_IA_Component = "config/RS232/T100/T100.ia";
  // char *configFileName_IA_System = "config/RS232/T100/system.ia";
  // char *configFileName_SERE = "config/RS232/T100/rules.sere";

  // RS232-T300
  // char *configFileName_IA_Component = "config/RS232/T300/T300.ia";
  // char *configFileName_IA_System = "config/RS232/T300/system.ia";
  // char *configFileName_SERE = "config/RS232/T300/rules.sere";

  // RS232 T100 - T901
  // char *configFileName_IA_Component = "config/RS232/RS232.ia";
  // char *configFileName_SERE = "config/RS232/RS232.sere";

  // AES T100 - T1000
  // char *configFileName_IA_Component = "config/AES/AES.ia";
  // char *configFileName_SERE = "config/AES/AES.sere";

  // BasicRSA T100 - T400
  // char *configFileName_IA_Component = "config/BasicRSA/BasicRSA.ia";
  // char *configFileName_SERE = "config/BasicRSA/BasicRSA.sere";

	char *configFileName_IA_Component;
	char *configFileName_SERE;

	// Ensure correct usage
  if (argc < 2)		// no extra args included
  {
    printUsage();

		configFileName_IA_Component = "config/BasicRSA/BasicRSA.ia";
	  configFileName_SERE = "config/BasicRSA/BasicRSA.sere";
  }
	else
	{
		char *directory = argv[1];
	}



  // TODO - get the final design type from arg list
  outputType finalDesignType = VHDL;

  // Define our set of automata
  automatonSet allAutomata;


  cout << endl << "## CAPSL ##" << endl << endl;


  //*****************
  //       IA
  //*****************


  // Component IA configuration will be stored in these containers
  //  to be passed to automaton
  state_set stateSet_IA_Component;
  signal_set signalSet_IA_Component;
  transition_set transitionSet_IA_Component;

  processIAConfiguration(configFileName_IA_Component,
                         &stateSet_IA_Component,
                         &signalSet_IA_Component,
                         &transitionSet_IA_Component);

  cout << "Building Component1 Interface..." << endl;
  automaton componentIA(stateSet_IA_Component, signalSet_IA_Component, transitionSet_IA_Component);

  // Add to the automata set
  cout << "Adding Component1 automata to set..." << endl;
  addAndComposeAutomaton(componentIA, &allAutomata);
  cout << endl;


  // // Component IA configuration will be stored in these containers
  // //  to be passed to automaton
  // state_set stateSet_IA_Component2;
  // signal_set signalSet_IA_Component2;
  // transition_set transitionSet_IA_Component2;
  //
  // processIAConfiguration(configFileName_IA_Component2,
  //                        &stateSet_IA_Component2,
  //                        &signalSet_IA_Component2,
  //                        &transitionSet_IA_Component2);
  //
  // cout << "Building Component2 Interface..." << endl;
  // automaton componentIA2(stateSet_IA_Component2, signalSet_IA_Component2, transitionSet_IA_Component2);
  //
  // // Add to the automata set
  // cout << "Adding Component2 automata to set..." << endl;
  // addAndComposeAutomaton(componentIA2, &allAutomata);
  // cout << endl;


  // // Component IA configuration will be stored in these containers
  // //  to be passed to automaton
  // state_set stateSet_IA_Component3;
  // signal_set signalSet_IA_Component3;
  // transition_set transitionSet_IA_Component3;
  //
  // processIAConfiguration(configFileName_IA_Component3,
  //                        &stateSet_IA_Component3,
  //                        &signalSet_IA_Component3,
  //                        &transitionSet_IA_Component3);
  //
  // cout << "Building Component3 Interface..." << endl;
  // automaton componentIA3(stateSet_IA_Component3, signalSet_IA_Component3, transitionSet_IA_Component3);
  //
  // // Add to the automata set
  // cout << "Adding Component3 automata to set..." << endl;
  // addAndComposeAutomaton(componentIA3, &allAutomata);
  // cout << endl;



  // // System IA configuration will stored in these containers
  // //  to be passed to automaton
  // state_set stateSet_IA_System;
  // signal_set signalSet_IA_System;
  // transition_set transitionSet_IA_System;
  //
  // processIAConfiguration(configFileName_IA_System,
  //                        &stateSet_IA_System,
  //                        &signalSet_IA_System,
  //                        &transitionSet_IA_System);
  //
  // cout << "Building System Interface..." << endl;
  // automaton systemIA(stateSet_IA_System, signalSet_IA_System, transitionSet_IA_System);
  //
  // // Add to the automata set
  // cout << "Adding System automata to set..." << endl;
  // addAndComposeAutomaton(systemIA, &allAutomata);
  // cout << endl;




  // NOTE
  //  The complete IA automata will contain all of the signals refereced
  //    in the SERE configurations. Thus we need to give them a reference for
  //    resolving the signals that are not given a type
  //  These are also used for setting the input and output signals for the checkers
  signal_set referenceSignalSet = allAutomata[0].signalSet;
  // for(int i = 0; i < referenceSignalSet.size(); i++)
  // {
  //   printSignalInfo(referenceSignalSet[i]);
  // }

  //*****************
  //      SERE
  //*****************

  // Component SERE configuration will stored in these containers
  //  to be passed to automaton encompassing all rules
  vector<state_set> stateSets_SERE;
  vector<signal_set> signalSets_SERE;
  vector<transition_set> transitionSets_SERE;
  //
  // Get the SERE configuration and pack into containers
  processSEREConfiguration(configFileName_SERE,
                           &stateSets_SERE,
                           &signalSets_SERE,
                           &transitionSets_SERE);

  // Go through all sets that were taken from the SERE specification and add them
  int numRules = stateSets_SERE.size(); // any of those sets would be fine, all same size

  for(int ruleIter = 0; ruleIter < numRules; ruleIter++)
  {
    cout << "Building SERE rule " << ruleIter << "..." << endl;
    automaton newSEREAutomata(stateSets_SERE[ruleIter], signalSets_SERE[ruleIter], transitionSets_SERE[ruleIter]);
    newSEREAutomata.resolveSignals(referenceSignalSet);

    cout << "Adding rule automata to set..." << endl;
    addAutomaton(newSEREAutomata, &allAutomata);
    cout << endl;

    cout << "AUTOMATA SET SIZE: " << allAutomata.size() << endl;
		cout << endl << "********************" << endl << endl;

  }


  // TODO
  // Build the VHDL checker from the set of automatas


  //*********************
  // Automata Generation
  //*********************

  // The process of implementing the checker requires all of the governing
  //  automatas to be working in parallel. The automata will be combined into
  //  a single checker module file. The automata all act upon a single set
  //  of input and output signals, while the more complex SERE automata add a
  //  bit of logic as required to act upon the composite transition signals.
  // Determine the correct generation procedure based on the final type of design
  switch(finalDesignType)
  {
    // If sandbox is to be generated in VHDL
    case VHDL:
      generateSandbox_VHDL(referenceSignalSet, allAutomata);
      break;

  	// TODO add systemC
    // If sandbox is to be generated in SystemC
    // case SystemC:
    //   generateSandbox_SystemC(referenceSignalSet, allAutomata);
    //   break;

    // Default sandbox generation is to VHDL modules
    default:
      generateSandbox_VHDL(referenceSignalSet, allAutomata);
      break;
  }

  // The steps in creating this checker in VHDL are:
  //  1. Generate the header to the file
  //     - inputs, outputs, declarations etc
  //  2. Step through automata set and generate each in VHDL
  //  3. Add final bits of syntax to the file


  // Generate the module for checker module
  //  Checker is governed by the automaton but
  //  type is determined by finalDesignType
  // componentIA.generateAutomatonModule(finalDesignType);
}
