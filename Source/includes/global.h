#ifndef __global_H_
#define __global_H_

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;


#define   MAX_FILE_NAME     50
#define   MAX_SERE_RULES    100


// Type names
typedef vector<int> vector_int;
typedef vector<string> vector_string;
typedef vector< vector<vector_int> > table;

// Possible program outputs
enum outputType{ VHDL, SystemC };\

// Automata types
enum AutomataType{ IA, SERE };

// Config types
enum ConfigFormat{ ia_and_sere, config };

// Config parameters
struct configInfo
{
	// Containers for the information received from IA configuration
  vector_string initState;
  vector_string acceptingStates;
  vector_string allStates;
  vector_string inputSignals;
  vector_string outputSignals;
  vector_string internalSignals;
  vector_string transitions;

	// Containers for the information received from SERE configuration
	vector_string rules;
};


// NOTE: Let's talk signals. At the time, IA config takes the
//  signals as input, output, hidden (which I assume will be
//  produced exclusively by IA composition, which is that
//  similar to composition of the Bobda's typical joining
//  of automata to be used for SERE rules composition). These
//  are intuitive. Composite signals are produced via the SPOT
//  translations we are doing from SERE rules to Buchi Automata.
//  The edge transitions are logical expressions of the signals
//  we first pull from the IA. Unset signals are pulled from
//  SERE transition edges that aren't logical expressions. These
//  may need to be set for composition, but idk, if so, I'll compare
//  names with existing signals from IA config.

// Signal types
enum signalType{ input, output, internal, composite, unset, alwaysTrue};

// Signal events
enum signalEventType{ Low, High };

// Logical operator types
enum logicType{ AND, OR };

// Signal data struct
struct componentSignal
{
  int         index;  // Index is set when placed in the signalSet
  string      ID;     // Signal ID as set in component configuration
  signalType  type;   // Type of component signal

  logicType   logicOperator;                           // SPOT composite signals use either AND or OR, not both
  vector<componentSignal> compositeSignalSet;     // Set of signals required for transition
  vector<signalEventType> compositeSignalEvents;  // Set of event types for each signal in the logic of the composite signal


  // NOTE:
  // When comparing signals, we need to have different procdures
  //  for when dealing with two IA automata, two SERE-derived
  //  automata, and finally for the mix between the two.
  //  IA - need only to match ID, and type
  //  SERE - need only match ID, and type
  //       - composite signals are tricky and need to match the
  //        ID, type, and eventType for each signal in the expression

  // For comparing signals
  bool operator==(const componentSignal& other)
  {
    // For comparing composite signals
    if(type == composite && other.type == composite)
    {
      // Loop through all signals of this expression
      for(int signalIter_this = 0; signalIter_this < compositeSignalSet.size(); signalIter_this++)
      {
        // Flag to determine if there was a matching singal in the other composite expression
        bool foundMatch = false;

        // Loop through all signals of the other expression
        for(int signalIter_other = 0; signalIter_other < other.compositeSignalSet.size(); signalIter_other++)
        {
          // Check ID, type, and event types, if we find a match, set the flag
          if(compositeSignalSet[signalIter_this].ID == other.compositeSignalSet[signalIter_other].ID &&
             compositeSignalSet[signalIter_this].type == other.compositeSignalSet[signalIter_other].type &&
             compositeSignalEvents[signalIter_this] == other.compositeSignalEvents[signalIter_other])
          {
            foundMatch = true;
          }
        }

        // Every signal in expression needs to match, otherwise the composite signals are not equal
        if(!foundMatch)
          return false;
      }

      // If we did not get kicked out, we must have a match for every signal in the expression
      return true;
    }

    // For comparing typical signals
    else
    {
      // Warn if we are trying to compare signals with unset types
      if(type == unset)
      {
        cout << "Cannot compare signals without set types" << endl;
        return false;
      }

      // For most signals, this is all it takes
      return ID       == other.ID    &&
             type     == other.type;
    }
  }

  // For comparing signals to add to composed automata
  //  The type match is waived for noncomposite signals because we assume signals with
  //  the same name and types being input/internal or output/internal are equivalent
  //  considering sharedSignals are added to the signalSet with types set to
  //  internal.
  bool isEquivalent(const componentSignal& other)
  {
    // For comparing composite signals
    if(type == composite && other.type == composite)
    {
      // Loop through all signals of this expression
      for(int signalIter_this = 0; signalIter_this < compositeSignalSet.size(); signalIter_this++)
      {
        // Flag to determine if there was a matching singal in the other composite expression
        bool foundMatch = false;



        // cout << "This Signal: " << ID << endl;
        // cout << "Other signal: " << other.ID << endl;
        //
        // cout << "COMPARING THIS COMPOSITE SIGNAL ELEMENT" << endl;
        // cout << "Signal: " << compositeSignalSet[signalIter_this].ID << endl;
        // // cout << "  numID: " << signal.numID << endl;
        // cout << "  index in signalSet: " << compositeSignalSet[signalIter_this].index << endl;
        //
        // if(compositeSignalSet[signalIter_this].type == signalType::input) cout << "  type: input\n";
        // else if(compositeSignalSet[signalIter_this].type == signalType::output) cout << "  type: output\n";
        // else if(compositeSignalSet[signalIter_this].type == signalType::internal) cout << "  type: internal\n";
        // else if(compositeSignalSet[signalIter_this].type == signalType::composite) cout << "  type: composite\n";
        // else if(compositeSignalSet[signalIter_this].type == signalType::unset) cout << "  type: unset\n";
        //
        // cout << "  event type: " << compositeSignalEvents[signalIter_this] << endl;
        // cout << endl;
        //


        // Loop through all signals of the other expression
        for(int signalIter_other = 0; signalIter_other < other.compositeSignalSet.size(); signalIter_other++)
        {


          //
          // cout << "AGAINST OTHER" << endl;
          //
          // cout << "Signal: " << other.compositeSignalSet[signalIter_other].ID << endl;
          // // cout << "  numID: " << signal.numID << endl;
          // cout << "  index in signalSet: " << other.compositeSignalSet[signalIter_other].index << endl;
          //
          // if(other.compositeSignalSet[signalIter_other].type == signalType::input) cout << "  type: input\n";
          // else if(other.compositeSignalSet[signalIter_other].type == signalType::output) cout << "  type: output\n";
          // else if(other.compositeSignalSet[signalIter_other].type == signalType::internal) cout << "  type: internal\n";
          // else if(other.compositeSignalSet[signalIter_other].type == signalType::composite) cout << "  type: composite\n";
          // else if(other.compositeSignalSet[signalIter_other].type == signalType::unset) cout << "  type: unset\n";
          //
          // cout << "  event type: " << other.compositeSignalEvents[signalIter_other] << endl;
          // cout << endl;



          // Check ID, type, and event types, if we find a match, set the flag
          if(compositeSignalSet[signalIter_this].ID == other.compositeSignalSet[signalIter_other].ID &&
             compositeSignalSet[signalIter_this].type == other.compositeSignalSet[signalIter_other].type &&
             compositeSignalEvents[signalIter_this] == other.compositeSignalEvents[signalIter_other])
          {
            foundMatch = true;

            // cout << "MATCH" << endl;
          }
        }

        // Every signal in expression needs to match, otherwise the composite signals are not equal
        if(!foundMatch)
          return false;
      }

      // If we did not get kicked out, we must have a match for every signal in the expression
      return true;
    }

    // For comparing typical signals
    else
    {
      // Warn if we are trying to compare signals with unset types
      if(type == unset)
      {
        cout << "Cannot compare signals without set types" << endl;
        return false;
      }

      // If the IDs match
      if(ID == other.ID)
      {
        // cout << "ID MATCH" << endl;
        // These account for comparisons of signals when types may not
        //  match but are assumed to be the same as well as types that may
        //  actually be the same
        if((type == signalType::internal && other.type == input) ||
          (type == signalType::internal && other.type == output) ||
          (type == input && other.type == signalType::internal) ||
          (type == output && other.type == signalType::internal) ||
          (type == output && other.type == input) ||
          (type == input && other.type == output) ||
          (type == other.type))
        {
          return true;
        }

        else
        {
          return false;
        }
      }
      else{
        return false;
      }
    }
  }
};

typedef vector<componentSignal> signal_set;


// State data struct
struct state
{
  int     index;            // Index is set when placed in the stateSet
  string  ID;               // State name as set in component configration
  int     initial;          // Initial state? 0:1
  int     accepting;        // Accepting state? 0:1

  int     illegal;          // Illegal state? 0:1

  // signal_set enabledInputSignals;    // Set of input signals enabled at this state
  // signal_set enabledOutputSignals;   // Set of output signals enabled at this state
  // signal_set enabledInternalSignals; // Set of internal signals enabled at this state
  signal_set enabledSignals;         // Set of all signals enabled at this state
};

typedef vector<state> state_set;


// Transition data struct
struct transition
{
  int index;                              // Index is set when placed in the transitionSet
  int row;                                // Row ID for finding this transition in the transition table
  int col;                                // Column ID for finding this transition in the transition table
  state currentState;                     // State to transition from
  state nextState;                        // State to trantition to
  componentSignal transitionSignal;       // Signal on which transition occurs
  signalEventType onSignalEvent;          // Signal event that triggers transition? Low:High

  // Only for composite signals
  // int compositeSignal;                            // Transition requires multiple signals? 0:1
  // signal_set compositeTransitionSignal;           // Set of signals required for transition
  // string compositeSignalLogic;                    // A string of the required logic to combine the composite signal
  // vector<signalEventType> compositeSignalEvents;  // Set of event types for each signal in the logic of the composite signal
};

typedef vector<transition> transition_set;



// extern void processComponentConfiguration(char *filename,
//                                           state_set *stateSet,
//                                           signal_set *signalSet,
//                                           transition_set *transitionSet);



//**********************
//  Read Config
//**********************

extern void readConfig(const char *filename_IA,
											 const char *filename_SERE,
											 const char *filename_Config,
											 ConfigFormat configFormat,
										 	 configInfo &config_info);

extern void readIAFile(const char *filename, configInfo &config_info);

extern void readSEREFile(const char *filename, configInfo &config_info);

extern void readConfigFile(const char *filename, configInfo &config_info);


//**********************
//  IA Configuration
//**********************

extern void processIAConfiguration(state_set *stateSet,
																	 signal_set *signalSet,
																 	 transition_set *transitionSet,
																 	 configInfo config_info);

// extern void readIAConfig(char *filename,
//                          vector_string *initState,
//                          vector_string *acceptingStates,
//                          vector_string *allStates,
//                          vector_string *inputSignals,
//                          vector_string *outputSignals,
//                          vector_string *interalSignals,
//                          vector_string *transitions);

extern void processStates(vector_string initState,
                          vector_string acceptingStates,
                          vector_string allStates,
                          state_set *stateSet);

extern void processSignals(vector_string inputSignals,
                           vector_string outputSignals,
                           vector_string interalSignals,
                           signal_set *signalSet);

extern string removeComment(string element);
extern string extractUntil(string element, string delimiter);

void printStateInfo(state state);
void printSignalInfo(componentSignal signal);
void printTransitionInfo(transition transition);


//**********************
//  SERE Configuration
//**********************
#include "automaton.h"

extern void processSEREConfiguration(vector<state_set> *stateSets,
																		 vector<signal_set> *signalSets,
																	 	 vector<transition_set> *transitionSets,
																	 	 configInfo config_info);

// extern void readSEREConfig(char *filename, vector_string *rules);

extern void processSERE(vector_string rules,
                        vector_string *hoaRuleOutputFiles,
                        vector_string *dotRuleOutputFiles);

extern void parseHOA(string filename,
                     vector_string *initialState,
                     vector_string *states,
                     vector_string *signals,
                     vector_string *transitions);

extern void processStates(vector_string initState,
                          vector_string states,
                          state_set *stateSet);

extern void processSignals(vector_string signals,
                           signal_set *signalSet);


//**********************
//  IA Configuration &
//  SERE Configuration
//**********************

extern void processTransitions(vector_string transitions,
                               state_set *stateSet,
                               signal_set *signalSet,
                               transition_set *transitionSet,
                               AutomataType automataType);


//**********************
//  Sandbox generation
//**********************
// extern void generateSandbox_VHDL(signal_set referenceSignalSet, vector<automaton> allAutomata);
//
// extern void generateSandbox_SystemC(signal_set referenceSignalSet, vector<automaton> allAutomata);

// extern void addAutomaton(automaton toAdd, automatonSet *set);

#endif
