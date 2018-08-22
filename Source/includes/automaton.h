#ifndef __AUTOMATON_H_
#define __AUTOMATON_H_

#include "global.h"
using namespace std;

// Automaton definition
class automaton
{
  public:
    // General info
    int numAcceptingStates;        // Number of end states
    int numIllegalStates;          // Number of illegal states

    // Containers
    state_set        stateSet;        // All possible states
    signal_set       signalSet;       // All possible signals - the alphabet
    transition_set   transitionSet;   // All transitions
    table            transitionTable; // Table form of automaton behavior

    state_set       illegalStates;   // All illegal states

		// Print flag
		bool printFlag;		// true if functions should print as debug


    // Constructor given set of states, signals, and transitions
    automaton(state_set states, signal_set signals, transition_set transitions);

    // Constructor given two existing automata
    automaton(automaton A, automaton B);

    // Destructor
    ~automaton() {};

    // Build the transition table
    void buildTransitionTable(state_set stateSet,
                              signal_set signalSet,
                              transition_set *transitionSet,
                              table *transTable);

    // Print the transition table
    void printTransitionTable(signal_set signalSet,
                              state_set stateSet,
                              table transitionTable);

    // Generate VHDL
    stringstream generateAutomata_VHDL(int index, int* illegalStateIndex, signal_set referenceSignalSet);

    // Generate SystemC
    stringstream generateAutomata_SystemC(int index, signal_set referenceSignalSet);

    // Returns whether this automaton is composable with the passed automaton
    bool canComposeWith(automaton A);

    // // Appends the set of signals (the alphabet) with new signals
    // void appendAlphabet(signal_set signals, signal_set *signalAlphabet);

    // Removes the unreachable states from the automaton
    void removeUnreachableStates();

    // Resolves the automata's signals against a set with complete type information
    void resolveSignals(signal_set referenceSignals);

    // Merge signals of two signal_sets into this automata's signal set
    void mergeSignals(automaton* A, automaton* B, signal_set *sharedSginals);

    // Merge transitions of two automata into this automata's transition set and populates composed state set
    void mergeTransitions(automaton A, automaton B, signal_set sharedSignals);

    // Add a new signal to the set if it does not already exist
    void addSignal(componentSignal newSignal);

    // Create a new state or use existing and return the index of the state
    int addState(state A, state B, signal_set sharedSignals);

    // Create a new state or use existing and return the index of the state
    int addState(state A, state B, componentSignal enabledSignal, signal_set sharedSignals);

    // Determines if the two passed parent states will compose an illegal state
    bool isComposedStateIllegal(state A, state B, signal_set sharedSignals);

    // Create a new transition and add to this transition set
    transition addTransition(state currentState, state nextState, transition transition);



    // Deep copy for = operator
    automaton& operator=(const automaton object);

};

typedef vector<automaton> automatonSet;


#endif
