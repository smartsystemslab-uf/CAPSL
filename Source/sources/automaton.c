#include "global.h"
#include "automaton.h"
using namespace std;


// Constructor given set of states, signals, and transitions
automaton::automaton(state_set states, signal_set signals, transition_set transitions)
{
  // Set local data
  this->stateSet       = states;
  this->signalSet      = signals;
  this->transitionSet  = transitions;

  // Get the number of accepting and illegal states
  for(int stateIter = 0; stateIter < states.size(); stateIter++)
  {
    if(states[stateIter].accepting)
      this->numAcceptingStates++;
    if(states[stateIter].illegal)
      this->numIllegalStates++;
  }

  // Build the transtion table
  //  Transition set will be modified to hold coordinates of transition objects in the table
  buildTransitionTable(this->stateSet, this->signalSet, &this->transitionSet, &this->transitionTable);
}


// Constructor given two existing automata
automaton::automaton(automaton A, automaton B)
{
  // NOTE
  // This constructor is called for composition of two automata
  // These methods are defined in the Alfaro Interface Automata paper.
  // Steps:
  //  Merge signals
  //    Handle shared actions
  //    Add all others to this signalSet
  //  Step through all transitions
  //    Generates a new set of states and transitions for composed automata
  //  Build transition table

  // Ensure the automata are compatible
  if(!A.canComposeWith(B))
  {
    cerr << "\nAutomata compatibility failure";
    return;
  }

  // Initialize these to 0
  this->numAcceptingStates = 0;
  this->numIllegalStates = 0;

  // Set of shared signals
  signal_set sharedSignals;

  // Merge the signals into this signalSet
  //  This also update enabled signals types for all states in their stateSet
  //  And modify the transition signal types for all transition in ther transitionSet
  this->mergeSignals(&A, &B, &sharedSignals);

  // for(int i = 0; i < sharedSignals.size(); i++)
  //   printSignalInfo(sharedSignals[i]);

  // for(int i = 0; i < this->signalSet.size(); i++)
  //   printSignalInfo(this->signalSet[i]);

  // Merge the transitions into this transitionSet
  //  Also creates the new state set from transition rules
  // NOTE
  // de Alfaro (page 8) for transitions of composed automata is what is modeled below
  this->mergeTransitions(A, B, sharedSignals);

  // Remove the unreachable states, they are unnecessary
  //  Also removes any transitions using these states
  this->removeUnreachableStates();

  // Get the number of accepting and illegal states
  for(int stateIter = 0; stateIter < this->stateSet.size(); stateIter++)
  {
    if(this->stateSet[stateIter].accepting)
      this->numAcceptingStates++;
    if(this->stateSet[stateIter].illegal)
      this->numIllegalStates++;
  }

  // Now that we have merged the signals, states, and transitions, we can proceed with building
  //  transition table
  this->buildTransitionTable(this->stateSet, this->signalSet, &this->transitionSet, &this->transitionTable);
}


// Build transition table
void automaton::buildTransitionTable(state_set stateSet, signal_set signalSet, transition_set *transitionSet, table *transTable)
{
  // Column height determined by size of state set
  for(int stateIndex = 0; stateIndex < stateSet.size(); stateIndex++)
  {
    vector_int tableCell;
    vector<vector_int> row;

    // Set all to -1 before placing transition entries
    tableCell.push_back(-1);

    // Row length determined by size of componentSignal set, our alphabet
    //  Each signal receives two columns - low and high signal events
    for(int signalIndex = 0; signalIndex < (2 * signalSet.size()); signalIndex++)
    {
      row.push_back(tableCell);
    }

    transTable->push_back(row);
  }

  // printTransitionTable(signalSet, stateSet, (*transTable));

  // Fill in the table transitions
  for(int transIndex = 0; transIndex < transitionSet->size(); transIndex++)
  {
    // printTransitionInfo(this->transitionSet[transIndex]);

    // Get row placement in the table
    int stateIndex = 0; // (*transitionSet)[transIndex].currentState.index;
    for(int stateIter_all = 0; stateIter_all < this->stateSet.size(); stateIter_all++)
    {
      if((*transitionSet)[transIndex].currentState.ID == this->stateSet[stateIter_all].ID)
      {
        stateIndex = this->stateSet[stateIter_all].index;
        (*transitionSet)[transIndex].currentState.index = this->stateSet[stateIter_all].index;
      }
    }

    // The signal index must be adjusted since we have two columns for each signal
    int signalIndex = (*transitionSet)[transIndex].transitionSignal.index;

    // Low signal index fix
    if((*transitionSet)[transIndex].onSignalEvent == Low)
    {
      signalIndex = 2 * signalIndex;
    }

    // High signal index fix
    else if((*transitionSet)[transIndex].onSignalEvent == High)
    {
      signalIndex = 2 * signalIndex + 1;
    }

    // Get next state in order to place entry into table
    int tableEntry = -1; //(*transitionSet)[transIndex].nextState.index;
    for(int stateIter_all = 0; stateIter_all < this->stateSet.size(); stateIter_all++)
    {
      if((*transitionSet)[transIndex].nextState.ID == this->stateSet[stateIter_all].ID)
      {
        tableEntry = this->stateSet[stateIter_all].index;
        (*transitionSet)[transIndex].nextState.index = this->stateSet[stateIter_all].index;
      }
    }

    // Ensure transition is associated with entry in the transition table
    (*transitionSet)[transIndex].row = stateIndex;
    (*transitionSet)[transIndex].col = signalIndex;

    // if(tableEntry == -1)
    // {
    //   cout << "There was an issue with a transition." << endl;
    //   printTransitionInfo((*transitionSet)[transIndex]);
    //   printStateInfo((*transitionSet)[transIndex].currentState);
    //   printStateInfo((*transitionSet)[transIndex].nextState);
    // }

    // Place entry in the table but check if something has been added already
    if((*transTable)[stateIndex][signalIndex][0] == -1)
    {
      (*transTable)[stateIndex][signalIndex][0] = tableEntry;// = to_string(tableEntry);
    }
    else
    {
      bool previouslyAdded = false;
      // Add only if the next state has not been added to the cell
      for(int cellIndex = 0; cellIndex < (*transTable)[stateIndex][signalIndex].size(); cellIndex++)
      {
        if((*transTable)[stateIndex][signalIndex][cellIndex] == tableEntry)
        {
          previouslyAdded = true;
        }
      }
      if(!previouslyAdded)
      {
        (*transTable)[stateIndex][signalIndex].push_back(tableEntry);
      }
    }


    // // Place entry in the table but check if something has been added already
    // if((*transTable)[stateIndex][signalIndex] == "")
    // {
    //   (*transTable)[stateIndex][signalIndex] = to_string(tableEntry);
    // }
    // // Add only if the next state string is not in
    // else if((*transTable)[stateIndex][signalIndex] != to_string(tableEntry))
    // // else if((*transTable)[stateIndex][signalIndex].find(to_string(tableEntry)) == std::string::npos)
    // {
    //   // NOTE This is a duplicate transition
    //   // Currently doing nothing, but may need to fix for VHDL generation
    //   // }
    //   // else
    //   // {
    //
    //   // Add another next state to table
    //   (*transTable)[stateIndex][signalIndex] = (*transTable)[stateIndex][signalIndex] + "," + to_string(tableEntry);
    // }

    // printTransitionInfo(this->transitionSet[transIndex]);

  }

  printTransitionTable(signalSet, stateSet, (*transTable));
}


// Prints the transition table
void automaton::printTransitionTable(signal_set signalSet, state_set stateSet, table transitionTable)
{
  cout << "\nTransition table\n";

  // Print a legend for signal/state name and indexes
  cout << "  Legend:" << endl;
  for(int signalIndex = 0; signalIndex < signalSet.size(); signalIndex++)
  {
    cout << "   " << signalIndex << " - " << signalSet[signalIndex].ID;

    if(signalSet[signalIndex].type == signalType::input) cout << " - input\n";
    else if(signalSet[signalIndex].type == signalType::output) cout << " - output\n";
    else if(signalSet[signalIndex].type == signalType::internal) cout << " - internal\n";
    else if(signalSet[signalIndex].type == signalType::composite) cout << " - composite\n";
    else if(signalSet[signalIndex].type == signalType::unset) cout << " - unset\n";
    else if(signalSet[signalIndex].type == signalType::alwaysTrue) cout << " - always true\n";
  }

  for(int stateIndex = 0; stateIndex < stateSet.size(); stateIndex++)
  {
    cout << "   state" << stateIndex << " - " << stateSet[stateIndex].ID << endl;

    cout << "          Enabled: ";
    for(int signalIndex = 0; signalIndex < stateSet[stateIndex].enabledSignals.size(); signalIndex++)
    {
      cout << stateSet[stateIndex].enabledSignals[signalIndex].ID << ", ";
    }
    cout << endl;
  }

  // Print signals along the top
  cout << endl << "            ";
  for(int signalIndex = 0; signalIndex < signalSet.size(); signalIndex++)
  {
    // Even indexes will be the not signals
    cout << "~(" << signalSet[signalIndex].ID << ")    ";

    cout << signalSet[signalIndex].ID << "    ";
  }
  cout << endl;

  // Print table
  for(int stateIndex = 0; stateIndex < stateSet.size(); stateIndex++)
  {
    // Add state info to table
    if(stateSet[stateIndex].initial)
      cout << "> ";
    else if(stateSet[stateIndex].accepting)
      cout << "* ";
    else if(stateSet[stateIndex].illegal)
      cout << "x ";
    else
      cout << "  ";

    cout << "state" << stateSet[stateIndex].index << "    ";

    // Show both low and high columns for each signal
    for(int signalIndex = 0; signalIndex < (2 * signalSet.size()); signalIndex++)
    {
      // -1 means no cell entries, add some filler for printing
      if(transitionTable[stateIndex][signalIndex][0] == -1)
      {
        // cout << "-       ";
        cout << "-";
      }
      else
      {
        for(int cellIndex = 0; cellIndex < transitionTable[stateIndex][signalIndex].size(); cellIndex++)
        {
          // cout << transitionTable[stateIndex][signalIndex] << "       ";
          cout << to_string(transitionTable[stateIndex][signalIndex][cellIndex]) + ",";
        }
      }

      // Style the table a bit and fix some spacing
      int numSpaces = 0;

      // If signal index is even (the not signals)
      if(signalIndex % 2 == 0)
      {
        // ID size plus parentheses, ~ flag, and four spaces between signal names
        numSpaces = signalSet[signalIndex/2].ID.size() + 3 + 4 - 1;
      }
      else
      {
        // ID size plus four spaces between signal names
        numSpaces = signalSet[signalIndex/2].ID.size() + 4 - 1;
      }

      for(int i = 0; i < numSpaces; i++)
      {
        cout << " ";
      }

    }
    cout << endl;
  }
  cout << endl << endl;
}


// Generates a VHDL module of the automata
//  Passed an index for unique states and the signal array for proper referencing in VHDL
stringstream automaton::generateAutomata_VHDL(int index, int* illegalStateIndex, signal_set referenceSignalSet)
{
  stringstream vhdl_out;

  // Add the transition process
  vhdl_out << "\t-- Transition process" << endl;
  vhdl_out << "\tprocess(ControlClock)" << endl;
  vhdl_out << "\tbegin" << endl;
  vhdl_out << "\t\tif ControlClock'event and ControlClock = '1' then" << endl;

  // Add all of the transition statements
  cout << "NUM TRANSITIONS FOR VHDL: " << this->transitionSet.size() << endl;

  for(int transIndex = 0; transIndex < this->transitionSet.size(); transIndex++)
  {

    // printTransitionInfo(this->transitionSet[transIndex]);

    // If the signal is composite
    if(this->transitionSet[transIndex].transitionSignal.type == composite)
    {
      int nextStateIndex = this->transitionSet[transIndex].nextState.index;
      int currentStateIndex = this->transitionSet[transIndex].currentState.index;

      signal_set compositeSignals = this->transitionSet[transIndex].transitionSignal.compositeSignalSet;


      // Write the first portion of the statement
      // Signal low logic
      if(this->transitionSet[transIndex].onSignalEvent == Low)
      {
        // The addition of the not statement allows the state to the asserted even
        //  for transitions that require the signal to be low (0)
        vhdl_out << "\t\t\tS" << index << "_" << nextStateIndex << " <= (not(";
      }

      // Signal high logic
      else
      {
        vhdl_out << "\t\t\tS" << index << "_" << nextStateIndex << " <= ((";
      }



      // Loop through each of the signals in the composite transition signal
      for(int signalIter_comp = 0; signalIter_comp < compositeSignals.size(); signalIter_comp++)
      {
        // Signals are universally used and thus may not reflect the index used for the
        //  VHDL ports, we need to reference the correct signal
        int signalIndex = -1;
        for(int signalIter_reference = 0; signalIter_reference < referenceSignalSet.size(); signalIter_reference++)
        {
          if(compositeSignals[signalIter_comp].ID == referenceSignalSet[signalIter_reference].ID)
          {
            signalIndex = referenceSignalSet[signalIter_reference].index;
          }
        }

        // Add the signal to the current statement

        // Signal low logic
        if(this->transitionSet[transIndex].transitionSignal.compositeSignalEvents[signalIter_comp] == Low)
        {
          // The addition of the not statement allows the state to the asserted even
          //  for transitions that require the signal to be low (0)
          vhdl_out << "not SignalSet(" << signalIndex << ")"; // and S" << index << "_" << currentStateIndex << ");" << endl;
        }

        // Signal high logic
        else
        {
          vhdl_out << "SignalSet(" << signalIndex << ")"; // and S" << index << "_" << currentStateIndex << ");" << endl;
        }


        // TODO
        //  The reset signals that are added to reset the SERE automata will mix these operators
        //  How do I fix this?

        // All signals are combined with some logical operator
        //  If this is not the last statement in the composite set
        //  of signals then add the appropriate type. Composite signals
        //  are only output from spot with a single operator
        if(signalIter_comp < compositeSignals.size() - 1)
        {
          // AND
          if(this->transitionSet[transIndex].transitionSignal.logicOperator == AND)
          {
            vhdl_out << " and ";
          }

          // OR
          if(this->transitionSet[transIndex].transitionSignal.logicOperator == OR)
          {
            vhdl_out << " or ";
          }
        }
      }

      // Write the final portion of the statement that specifies the current state
      vhdl_out << ") and S" << index << "_" << currentStateIndex << ");" << endl;
    }

    // If the signal is an always true
    else if(this->transitionSet[transIndex].transitionSignal.type == alwaysTrue)
    {
      int nextStateIndex = this->transitionSet[transIndex].nextState.index;
      int currentStateIndex = this->transitionSet[transIndex].currentState.index;



      // Write the statement with use of boolean value true
      vhdl_out << "\t\t\tS" << index << "_" << nextStateIndex << " <= ((";

      // All variations of the signals mentioned in this automata will be given a transition
      //  that includes all of these variations 'or'ed together. Any trigger should assert.
      for(int signalIter_this = 0; signalIter_this < this->signalSet.size(); signalIter_this++)
      {
        // cout << "NUM SIGNALS: " << this->signalSet.size() << endl;

        for(int signalIter_reference = 0; signalIter_reference < referenceSignalSet.size(); signalIter_reference++)
        {
          // printSignalInfo(signalSet[signalIter_this]);

          // If this signal is in the reference set, then utilize its index
          if(this->signalSet[signalIter_this].ID == referenceSignalSet[signalIter_reference].ID)
          {
            // All signals are combined with the or logic operator
            if(signalIter_this > 0)
            {
              vhdl_out << " or ";
            }

            vhdl_out << "SignalSet(" << signalIter_reference << ") or not SignalSet(" << signalIter_reference << ")";
          }
        }
      }

      // Finish the transition statement
       vhdl_out << ") and S" << index << "_" << currentStateIndex << ");" << endl;

      // signal_set compositeSignals = this->transitionSet[transIndex].transitionSignal.compositeSignalSet;
      //
      // // Write the statement with use of boolean value true
      // vhdl_out << "\t\t\tS" << index << "_" << nextStateIndex << " <= (true and S" << index << "_" << currentStateIndex << ");" << endl;
    }

    // If the signal is typical
    else
    {
      // printTransitionInfo(this->transitionSet[transIndex]);

      int signalIndex = this->transitionSet[transIndex].transitionSignal.index;

      // Signals are universally used and thus may not reflect the index used for the
      //  VHDL ports, we need to reference the correct signal
      // int signalIndex = -1;
      // for(int signalIter_reference = 0; signalIter_reference < referenceSignalSet.size(); signalIter_reference++)
      // {
      //   // cout << "Index: " << signalIter_reference << endl;
      //   // cout << "RefSignal.index: " << referenceSignalSet[signalIter_reference].index << endl;
      //   // cout << "RefSignal.ID: " << referenceSignalSet[signalIter_reference].ID << endl;
      //   // cout << "TransSignal.index: " << this->transitionSet[transIndex].transitionSignal.index << endl;
      //   // cout << "TransSignal.ID: " << this->transitionSet[transIndex].transitionSignal.ID << endl;
      //
      //   if(this->transitionSet[transIndex].transitionSignal.ID == referenceSignalSet[signalIter_reference].ID)
      //   {
      //     // printSignalInfo(this->signalSet[referenceSignalSet[signalIter_reference].index]);
      //     // signalIndex = referenceSignalSet[signalIter_reference].index;
      //
      //     signalIndex = signalIter_reference;
      //     this->transitionSet[transIndex].transitionSignal.index = signalIndex;
      //     cout << "Match at index: " << signalIter_reference << endl;
      //
      //     cout << "Index: " << signalIter_reference << endl;
      //     cout << "RefSignal.index: " << referenceSignalSet[signalIter_reference].index << endl;
      //     cout << "RefSignal.ID: " << referenceSignalSet[signalIter_reference].ID << endl;
      //     cout << "TransSignal.index: " << this->transitionSet[transIndex].transitionSignal.index << endl;
      //     cout << "TransSignal.ID: " << this->transitionSet[transIndex].transitionSignal.ID << endl;
      //
      //   }
      //
      //   // cout << endl << endl;
      //
      // }


      // cout << "SignalINDEX " << signalIndex << endl;

      // printSignalInfo(this->signalSet[signalIndex]);
      // printSignalInfo(this->transitionSet[transIndex].transitionSignal);

      int nextStateIndex = -1; // this->transitionSet[transIndex].nextState.index;
      int currentStateIndex = -1; // this->transitionSet[transIndex].currentState.index;

      // int stateIndex = -1;
      for(int stateIter = 0; stateIter < this->stateSet.size(); stateIter++)
      {
        // cout << "Index: " << signalIter_reference << endl;
        // cout << "RefSignal.index: " << referenceSignalSet[signalIter_reference].index << endl;
        // cout << "RefSignal.ID: " << referenceSignalSet[signalIter_reference].ID << endl;
        // cout << "TransSignal.index: " << this->transitionSet[transIndex].transitionSignal.index << endl;
        // cout << "TransSignal.ID: " << this->transitionSet[transIndex].transitionSignal.ID << endl;

        if(this->stateSet[stateIter].ID == this->transitionSet[transIndex].currentState.ID)
        {
          currentStateIndex = this->stateSet[stateIter].index;
        }
        if(this->stateSet[stateIter].ID == this->transitionSet[transIndex].nextState.ID)
        {
          nextStateIndex = this->stateSet[stateIter].index;
        }

        // cout << endl << endl;

      }

      // printTransitionInfo(this->transitionSet[transIndex]);

      // Signal low logic
      if(this->transitionSet[transIndex].onSignalEvent == Low)
      {
        // The addition of the not statement allows the state to the asserted even
        //  for transitions that require the signal to be low (0)
        vhdl_out << "\t\t\tS" << index << "_" << nextStateIndex << " <= (not SignalSet(" << signalIndex << ") and S" << index << "_" << currentStateIndex << ");" << endl;
      }

      // Signal high logic
      else
      {
        vhdl_out << "\t\t\tS" << index << "_" << nextStateIndex << " <= (SignalSet(" << signalIndex << ") and S" << index << "_" << currentStateIndex << ");" << endl;
      }
    }
  }

  vhdl_out << endl;

  // Handle Illegal states
  //  Use the index passed and iterate it
  for(int stateIter = 0; stateIter < this->stateSet.size(); stateIter++)
  {
    // Make catches for illegal states
    if(this->stateSet[stateIter].illegal)
    {
      vhdl_out << "\t\t\tIllegalStateDetections(" << *illegalStateIndex << ") <= S" << index << "_" << stateIter << ";" << endl;
      *illegalStateIndex++;
    }
  }

  // Close the if and process statements
  vhdl_out << "\t\tend if;" << endl;
  vhdl_out << "\tend process;" << endl;
  vhdl_out << endl;

  return vhdl_out;
}


// TODO
// Generates a SystemC module of the automata
//  Passed an index for unique states
stringstream automaton::generateAutomata_SystemC(int index, signal_set referenceSignalSet)
{}


// Returns whether this automata is composable with the argument automata
bool automaton::canComposeWith(automaton A)
{
  // Working in terms of Interface automata, automata can be composed iff:
  //  - Hidden actions of A intersected with all actions of B must be empty.
  //  - Input actions of A intersected with input actions of B must be empty.
  //  - Output actions of A intersected with output actions of B must be empty.
  //  - Hidden actions of B intersected with all actions of A must be empty.

  // A - automata passed as argument
  // B - this automata

  // Check compatibility
  for(int signalIter_this = 0; signalIter_this < this->signalSet.size(); signalIter_this++)
  {
    // cout << "Comparing this signal " << this->signalSet[signalIter_this].ID << endl;

    for(int signalIter_A = 0; signalIter_A < A.signalSet.size(); signalIter_A++)
    {
      // cout << "  to A's signal " << A.signalSet[signalIter_A].ID << endl;

      // Name match
      if(this->signalSet[signalIter_this].ID == A.signalSet[signalIter_A].ID)
      {
        // cout << "    Found matching signal..." << endl;

        // printSignalInfo(this->signalSet[signalIter_this]);
        // printSignalInfo(A.signalSet[signalIter_A]);

        // Failed condition 1
        if(this->signalSet[signalIter_this].type == signalType::internal)
        {
          cout << "      Failed condition 1" << endl;
          return false;
        }

        // Failed condition 2
        if(this->signalSet[signalIter_this].type == input && A.signalSet[signalIter_A].type == input)
        {
          cout << "      Failed condition 2" << endl;
          return false;
        }

        // Failed condition 3
        if(this->signalSet[signalIter_this].type == output && A.signalSet[signalIter_A].type == output)
        {
          cout << "      Failed condition 3" << endl;
          return false;
        }

        // Failed condition 4
        if(A.signalSet[signalIter_A].type == signalType::internal)
        {
          cout << "      Failed condition 4" << endl;
          return false;
        }
      }
    }
  }

  // If we weren't kicked out, then there were no violations
  //  The two automata are considered composable
  return true;
  // return false;

}


// Resolves the automata's signals against a set with complete type information
void automaton::resolveSignals(signal_set referenceSignals)
{
  // Loop through this signal set
  for(int signalIter_this = 0; signalIter_this < this->signalSet.size(); signalIter_this++)
  {
    // Loop through reference signals
    for(int signalIter_ref = 0; signalIter_ref < referenceSignals.size(); signalIter_ref++)
    {
      // Signals are identified as the same if their IDs match
      if(this->signalSet[signalIter_this].ID == referenceSignals[signalIter_ref].ID)
      {
        // Resolve signals of unset type, we pull the type from a signal
        //  with a mathing name.
        // Resolutions:
        //  Any unset of this that is an input of reference
        //  Any unset of this that is an output of reference
        //  Any unset of this that is an internal of reference
        if(this->signalSet[signalIter_this].type == unset && referenceSignals[signalIter_ref].type == input ||
           this->signalSet[signalIter_this].type == unset && referenceSignals[signalIter_ref].type == output ||
           this->signalSet[signalIter_this].type == unset && referenceSignals[signalIter_ref].type == signalType::internal)
        {
          // Set the type of the unset signal
          this->signalSet[signalIter_this].type = referenceSignals[signalIter_ref].type;

          // cout << "TYPE RESOLUTION#######" << endl;
          // printSignalInfo(this->signalSet[signalIter_this]);
        }
      }
    }
  }

  // Print the new trans table
  cout << "Resolved transition table:" << endl;
  this->printTransitionTable(this->signalSet, this->stateSet, this->transitionTable);
}


// Remove unreachable states from the automata, we do not care about these
void automaton::removeUnreachableStates()
{
  // All states that are reachable
  state_set reachableStates;

  // Flag to specify all reachable states found
  bool finished = false;

  // Index containers for deletions
  vector_int stateSetIndexesToDelete;
  vector_int transitionSetIndexesToDelete;

  // Add the initial state to the reachableStates
  for(int stateIter = 0; stateIter < this->stateSet.size(); stateIter++)
  {
    if(this->stateSet[stateIter].initial)
    {
      reachableStates.push_back(this->stateSet[stateIter]);
    }
  }
  // Num states we determine to be reachable
  int statesAdded = 1;


  // Determine all reachable states from initial
  while(statesAdded != 0)
  {
    // Start fresh, I want to know when we stop adding, which means we've
    //  found the end of a cycle
    statesAdded = 0;

    // cout << "Reachable states: " << endl;

    // Determine states reachable by one transition
    for(int stateIter = 0; stateIter < reachableStates.size(); stateIter++)
    {
      // cout << "REACHABLE SIZE: " << reachableStates.size() << endl;
      // cout << "REACHABLE INDEX: " << stateIter << endl;
      //
      // printStateInfo(reachableStates[stateIter]);

      // Loop through all of the transitions
      //  If there is a transition with this reachable state as the current index, we add the next state
      for(int transIter = 0; transIter < this->transitionSet.size(); transIter++)
      {
        // cout << "Trans SIZE: " << this->transitionSet.size() << endl;
        // cout << "Trans INDEX: " << transIter << endl;
        //
        // cout << "Transition current state: " << endl;
        // printStateInfo(this->transitionSet[transIter].currentState);
        // cout << "Transition next state: " << endl;
        // printStateInfo(this->transitionSet[transIter].nextState);

        // Transition with this reachable state as the current index
        if(reachableStates[stateIter].ID == this->transitionSet[transIter].currentState.ID)
        {
          bool alreadyAdded = false;

          // Ensure we don't keep adding states in a cycle
          for(int stateIter_check = 0; stateIter_check < reachableStates.size(); stateIter_check++)
          {
            if(this->transitionSet[transIter].nextState.ID == reachableStates[stateIter_check].ID)
            {
              // cout << "WAS ALREADY ADDED" << endl;
              alreadyAdded = true;
            }
          }

          if(!alreadyAdded)
          {
            statesAdded++;
            reachableStates.push_back(this->transitionSet[transIter].nextState);
            // cout << "ADDING: " << endl;
            // printStateInfo(this->transitionSet[transIter].nextState);
          }
        }
      }
    }

    // cout << "STATES ADDED" << statesAdded << endl;
  }

  int numStatesLeft = this->stateSet.size();
  int numStatesRemoved = 0;

  state_set removedStates;

  // cout << "REACHABLE STATES: " << reachableStates.size() << endl;
  // cout << "STATE SET: " << this->stateSet.size() << endl;

  // While all unreachable states are not removed
  while(numStatesLeft > reachableStates.size())
  {

    // Did we find a state in this stateSet to remove?
    int indexToRemove;

    // Loop through all states
    for(int stateIter_all = 0; stateIter_all < this->stateSet.size(); stateIter_all++)
    {
      bool reachable = false;

      // cout << "THIS STATE" << endl;
      // printStateInfo(this->stateSet[stateIter_all]);
      for(int stateIter_reachable = 0; stateIter_reachable < reachableStates.size(); stateIter_reachable++)
      {
        // cout << "REACHABLE STATE" << endl;
        // printStateInfo(reachableStates[stateIter_reachable]);
        if(this->stateSet[stateIter_all].ID == reachableStates[stateIter_reachable].ID)
        {
          // cout << "REACHABLE" << endl;
          reachable = true;
        }
      }

      if(!reachable)
      {
        // cout << "NOT REACHABLE" << endl;
        indexToRemove = stateIter_all;

        // If it wasn't reachable
        removedStates.push_back(this->stateSet[indexToRemove]);
        this->stateSet.erase(this->stateSet.begin()+indexToRemove);
        numStatesRemoved++;

        // Break the for loop to avoid seg fault
        stateIter_all = this->stateSet.size();
      }
    }


    numStatesLeft = this->stateSet.size();
    // cout << "REACHABLE STATES: " << reachableStates.size() << endl;
    // cout << "STATE SET: " << this->stateSet.size() << endl;
  }

  // Make index adjustments
  // Loop through all states
  for(int stateIter = 0; stateIter < this->stateSet.size(); stateIter++)
  {
    this->stateSet[stateIter].index = stateIter;
  }

  // Finally, remove all transitions using the unreachabled states
  for(int stateIter = 0; stateIter < removedStates.size(); stateIter++)
  {
    // Loop through all transitions to remove transitions with current states that were removed
    for(int transIter = 0; transIter < this->transitionSet.size(); transIter++)
    {
      if(this->transitionSet[transIter].currentState.ID == removedStates[stateIter].ID ||
         this->transitionSet[transIter].nextState.ID == removedStates[stateIter].ID)
      {
        // Remove the transitionLogic
        this->transitionSet.erase(this->transitionSet.begin() + transIter);

        // // Break the trans loop to go to next removed states
        // transIter = this->transitionSet.size();
      // }
    // }
    //
    // // Loop through all transitions to remove transitions with next states that were removed
    // for(int transIter = 0; transIter < this->transitionSet.size(); transIter++)
    // {
      // if(this->transitionSet[transIter].nextState.ID == removedStates[stateIter].ID)
      // {
        // Remove the transitionLogic
        // this->transitionSet.erase(this->transitionSet.begin() + transIter);

        // // Break the trans loop to go to next removed states
        // transIter = this->transitionSet.size();
      }
    }
  }

  // Make transition current and next state index adjustments
  // Loop through all transitions
  for(int transIter = 0; transIter < this->transitionSet.size(); transIter++)
  {
    // printTransitionInfo(this->transitionSet[transIter]);

    for(int stateIter = 0; stateIter < this->stateSet.size(); stateIter++)
    {
      // If the current state matches
      if(this->transitionSet[transIter].currentState.ID == this->stateSet[stateIter].ID)
      {
        // Adjust the index reference
        this->transitionSet[transIter].currentState.index = stateIter;
      }

      // If the next state matches
      if(this->transitionSet[transIter].nextState.ID == this->stateSet[stateIter].ID)
      {
        // Adjust the index reference
        this->transitionSet[transIter].nextState.index = stateIter;
      }
    }
  }
}


// Merge the signals of two automata into this automata's signalSet
//  Simply appends the alphabets of each automata and adds any signals shared by both
//  automata into a signalset. These are required for transition merging.
void automaton::mergeSignals(automaton* A, automaton* B, signal_set *sharedSignals)
{
  // Loop through A's signals
  for(int signalIter_A = 0; signalIter_A < A->signalSet.size(); signalIter_A++)
  {
    // Is this A signal shared?
    bool shared = false;

    // Loop through B's signals
    for(int signalIter_B = 0; signalIter_B < B->signalSet.size(); signalIter_B++)
    {
      // Signals are identified as the same if their IDs match
      if(A->signalSet[signalIter_A].ID == B->signalSet[signalIter_B].ID)
      {
        // NOTE
        //  We use this loop to take advantage of being able to compare
        //  signals of A and B against each other for:
        //    Discovering shared signals - we need to consolidate signals
        //      that are equivalent

        // // Resolve signals of unset type, we pull the type from a signal
        // //  with a mathing name.
        // // Resolutions:
        // //  Any unset of A that is an input of B
        // //  Any unset of A that is an output of B
        // //  Any unset of A that is an internal of B
        // //  Any input of A that is an unset of B
        // //  Any output of A that is an unset of B
        // //  Any internal of A that is an unset of B
        // if(A[signalIter_A].type == input && B[signalIter_B].type == unset ||
        //    A[signalIter_A].type == output && B[signalIter_B].type == unset ||
        //    A[signalIter_A].type == signalType::internal && B[signalIter_B].type == unset ||
        //    A[signalIter_A].type == unset && B[signalIter_B].type == input ||
        //    A[signalIter_A].type == unset && B[signalIter_B].type == output ||
        //    A[signalIter_A].type == unset && B[signalIter_B].type == signalType::internal)
        // {
        //   // Set the type of the unset signal
        //   if(A[signalIter_A].type == unset)
        //   {
        //     // Set A to the type of B
        //     A[signalIter_A].type = B[signalIter_B].type;
        //
        //     // cout << "TYPE RESOLUTION#######" << endl;
        //     // printSignalInfo(A[signalIter_A]);
        //
        //   }
        //   else
        //   {
        //     // Set B to the type of A
        //     B[signalIter_B].type = A[signalIter_A].type;
        //
        //     // cout << "TYPE RESOLUTION#######" << endl;
        //     // printSignalInfo(A[signalIter_A]);
        //   }
        // }

        // Shared signals:
        //  Any input of A that is an output of B
        //  Any output of A that is an input of B
        if(A->signalSet[signalIter_A].type == input && B->signalSet[signalIter_B].type == output ||
           A->signalSet[signalIter_A].type == output && B->signalSet[signalIter_B].type == input)
        {
          // Set details of the shared signal
          componentSignal sharedSignal;
          sharedSignal.ID = A->signalSet[signalIter_A].ID;
          sharedSignal.type = signalType::internal; // Shared signals means they can be handled internally away from the composed interface
          // Index is set when added to this signalSet

          // Add to the shared signal set and this signalSet
          sharedSignals->push_back(sharedSignal);
          this->addSignal(sharedSignal);

          // We need to update the enabled signals for all states in A and B because
          //  they will be used later and an internal signal type will be expected
          // We also update the transition signal types for all of their transitions
          // for(int stateIter_A = 0; stateIter_A < A->stateSet.size(); stateIter_A++)
          // {
          //   // Loop through enabled signals
          //   for(int signalIter_enabled = 0; signalIter_enabled < A->stateSet[stateIter_A].enabledSignals.size(); signalIter_enabled++)
          //   {
          //     // If IDs match
          //     if(sharedSignal.ID == A->stateSet[stateIter_A].enabledSignals[signalIter_enabled].ID)
          //     {
          //       // Update the type
          //       A->stateSet[stateIter_A].enabledSignals[signalIter_enabled].type = signalType::internal;
          //     }
          //   }
          // }
          // for(int stateIter_B = 0; stateIter_B < B->stateSet.size(); stateIter_B++)
          // {
          //   // Loop through enabled signals
          //   for(int signalIter_enabled = 0; signalIter_enabled < B->stateSet[stateIter_B].enabledSignals.size(); signalIter_enabled++)
          //   {
          //     // If IDs match
          //     if(sharedSignal.ID == B->stateSet[stateIter_B].enabledSignals[signalIter_enabled].ID)
          //     {
          //       // Update the type
          //       B->stateSet[stateIter_B].enabledSignals[signalIter_enabled].type = signalType::internal;
          //     }
          //   }
          // }
          for(int transIter_A = 0; transIter_A < A->stateSet.size(); transIter_A++)
          {
              // If IDs match
              if(sharedSignal.ID == A->transitionSet[transIter_A].transitionSignal.ID)
              {
                // Update the type
                A->transitionSet[transIter_A].transitionSignal.type = signalType::internal;
              }
          }
          for(int stateIter_B = 0; stateIter_B < B->stateSet.size(); stateIter_B++)
          {
            // If IDs match
            if(sharedSignal.ID == B->transitionSet[stateIter_B].transitionSignal.ID)
            {
              // Update the type
              B->transitionSet[stateIter_B].transitionSignal.type = signalType::internal;
            }
          }

          shared = true;
        }
      }
    }

    // If this signal of A wasn't shared with B, then go ahead and add to the signalSet
    if(!shared)
    {
      this->addSignal(A->signalSet[signalIter_A]);
    }
  }

  // Add the signals from B to this automata
  for(int signalIter_B = 0; signalIter_B < B->signalSet.size(); signalIter_B++)
  {
    // Add the signal to this signalSet, addSignal won't add if it exists already
    this->addSignal(B->signalSet[signalIter_B]);
  }
}


// Merge the transitions of two automata into this automata's transitionSet
//  Populates the stateSet according to the transition rules of de Alfaro - Interface Automata page 8
void automaton::mergeTransitions(automaton A, automaton B, signal_set sharedSignals)
{
  // Loop through all transitions of automata A
  for(int transIter_A = 0; transIter_A < A.transitionSet.size(); transIter_A++)
  {

    // cout << "Merging A transition:" << endl;
    // printTransitionInfo(A.transitionSet[transIter_A]);
    //
    //
    // cout << "Looking for shared signal" << endl;

    // Flag to hold whether or no the current transition uses a signal shared by both A and B
    bool usesSharedSignal = false;

    // Check if the transition action is in shared signal set
    for(int transIter_shared = 0; transIter_shared < sharedSignals.size(); transIter_shared++)
    {

      // cout << "********\n Checking if signals are identical" << endl;
      //
      // printSignalInfo(A.transitionSet[transIter_A].transitionSignal);
      // printSignalInfo(sharedSignals[transIter_shared]);
      //
      // cout << "********" << endl;

      if(A.transitionSet[transIter_A].transitionSignal.isEquivalent(sharedSignals[transIter_shared]))
      {
        // usesSharedSignal = true;
        // if(usesSharedSignal)
        // {
          // cout << "Found shared signal" << endl;

          // Flag to be set if B has a transition with the shared signal
          bool foundSharedSignalTransition = false;
          int sharedSignalBIndex;

          // Loop through B transitions
          for(int transIter_B = 0; transIter_B < B.transitionSet.size(); transIter_B++)
          {
            // cout << "Looking for shared signal in the transitions of B" << endl;
            // printTransitionInfo(B.transitionSet[transIter_B]);

            // Shared signals must match including the transition's onSignalEvent
            if(B.transitionSet[transIter_B].transitionSignal.isEquivalent(A.transitionSet[transIter_A].transitionSignal) &&
               B.transitionSet[transIter_B].onSignalEvent == A.transitionSet[transIter_A].onSignalEvent)
            {

              usesSharedSignal = true;

              sharedSignalBIndex = transIter_B;
              // foundSharedSignalTransition = true;

              // cout << "FOUND SHARED SIGNAL IN TRANS" << endl;

              // The two parent states are used to create or reference an existing state
              // int newCurrentStateIndex = this->addState(A.transitionSet[transIter_A].currentState, B.transitionSet[sharedSignalBIndex].currentState, A.transitionSet[transIter_A].transitionSignal, sharedSignals); // A and B's signal is the same, its shared

              state correctCurrentState_A;
              state correctCurrentState_B;
              for(int stateIter_A = 0; stateIter_A < A.stateSet.size(); stateIter_A++)
              {
                if(A.transitionSet[transIter_A].currentState.ID == A.stateSet[stateIter_A].ID)
                {
                  correctCurrentState_A = A.stateSet[stateIter_A];
                }
              }
              for(int stateIter_B = 0; stateIter_B < B.stateSet.size(); stateIter_B++)
              {
                if(B.transitionSet[sharedSignalBIndex].currentState.ID == B.stateSet[stateIter_B].ID)
                {
                  correctCurrentState_B = B.stateSet[stateIter_B];
                }
              }

              // Now add the new current state
              int newCurrentStateIndex = this->addState(correctCurrentState_A, correctCurrentState_B, A.transitionSet[transIter_A].transitionSignal, sharedSignals); // We don't know the enabled signals of the nextState


              // The below ensures we reference the state set states, these are certain to container
              //  all correct information. We don't use the transition bc the next states were not given
              //  updated enabled signal information. This is just a reprecussion of the transition processing
              state correctNextState_A;
              state correctNextState_B;
              for(int stateIter_A = 0; stateIter_A < A.stateSet.size(); stateIter_A++)
              {
                if(A.transitionSet[transIter_A].nextState.ID == A.stateSet[stateIter_A].ID)
                {
                  correctNextState_A = A.stateSet[stateIter_A];
                }
              }
              for(int stateIter_B = 0; stateIter_B < B.stateSet.size(); stateIter_B++)
              {
                if(B.transitionSet[sharedSignalBIndex].nextState.ID == B.stateSet[stateIter_B].ID)
                {
                  correctNextState_B = B.stateSet[stateIter_B];
                }
              }

              // Now add the new next state
              int newNextStateIndex = this->addState(correctNextState_A, correctNextState_B, sharedSignals); // We don't know the enabled signals of the nextState

              // All thats needed to create the new transition are the two reference states and the signal
              transition newTransition = this->addTransition(this->stateSet[newCurrentStateIndex], this->stateSet[newNextStateIndex], A.transitionSet[transIter_A]);

              // The signal indexes must be updated according to the shared set of signals
              for(int signalIter_this = 0; signalIter_this < sharedSignals.size(); signalIter_this++)
              {
                if(this->transitionSet[this->transitionSet.size()-1].transitionSignal.ID == sharedSignals[signalIter_this].ID)
                {
                  this->transitionSet[this->transitionSet.size()-1].transitionSignal.index = signalIter_this;
                }
              }

              // cout << "Resulting Transition:" << endl;
              // printTransitionInfo(this->transitionSet[this->transitionSet.size()-1]);

            }
          }

                    // Now we need to generate the new current and next states
                    //  if they do not already exist
                    // if(foundSharedSignalTransition)
                    // {
                    //   cout << "FOUND SHARED SIGNAL IN TRANS" << endl;
                    //
                    //   // The two parent states are used to create or reference an existing state
                    //   // int newCurrentStateIndex = this->addState(A.transitionSet[transIter_A].currentState, B.transitionSet[sharedSignalBIndex].currentState, A.transitionSet[transIter_A].transitionSignal, sharedSignals); // A and B's signal is the same, its shared
                    //
                    //   state correctCurrentState_A;
                    //   state correctCurrentState_B;
                    //   for(int stateIter_A = 0; stateIter_A < A.stateSet.size(); stateIter_A++)
                    //   {
                    //     if(A.transitionSet[transIter_A].currentState.ID == A.stateSet[stateIter_A].ID)
                    //     {
                    //       correctCurrentState_A = A.stateSet[stateIter_A];
                    //     }
                    //   }
                    //   for(int stateIter_B = 0; stateIter_B < A.stateSet.size(); stateIter_B++)
                    //   {
                    //     if(B.transitionSet[sharedSignalBIndex].currentState.ID == B.stateSet[stateIter_B].ID)
                    //     {
                    //       correctCurrentState_B = B.stateSet[stateIter_B];
                    //     }
                    //   }
                    //
                    //   // Now add the new current state
                    //   int newCurrentStateIndex = this->addState(correctCurrentState_A, correctCurrentState_B, A.transitionSet[transIter_A].transitionSignal, sharedSignals); // We don't know the enabled signals of the nextState
                    //
                    //
                    //   // The below ensures we reference the state set states, these are certain to container
                    //   //  all correct information. We don't use the transition bc the next states were not given
                    //   //  updated enabled signal information. This is just a reprecussion of the transition processing
                    //   state correctNextState_A;
                    //   state correctNextState_B;
                    //   for(int stateIter_A = 0; stateIter_A < A.stateSet.size(); stateIter_A++)
                    //   {
                    //     if(A.transitionSet[transIter_A].nextState.ID == A.stateSet[stateIter_A].ID)
                    //     {
                    //       correctNextState_A = A.stateSet[stateIter_A];
                    //     }
                    //   }
                    //   for(int stateIter_B = 0; stateIter_B < A.stateSet.size(); stateIter_B++)
                    //   {
                    //     if(B.transitionSet[sharedSignalBIndex].nextState.ID == B.stateSet[stateIter_B].ID)
                    //     {
                    //       correctNextState_B = B.stateSet[stateIter_B];
                    //     }
                    //   }
                    //
                    //   // Now add the new next state
                    //   int newNextStateIndex = this->addState(correctNextState_A, correctNextState_B, sharedSignals); // We don't know the enabled signals of the nextState
                    //
                    //   // All thats needed to create the new transition are the two reference states and the signal
                    //   transition newTransition = this->addTransition(this->stateSet[newCurrentStateIndex], this->stateSet[newNextStateIndex], A.transitionSet[transIter_A]);
                    // }
                  // }

      }
    }


    if(!usesSharedSignal)
    {

                    // {
                    //   // cout << "Found shared signal" << endl;
                    //
                    //   // Flag to be set if B has a transition with the shared signal
                    //   bool foundSharedSignalTransition = false;
                    //   int sharedSignalBIndex;
                    //
                    //   // Loop through B transitions
                    //   for(int transIter_B = 0; transIter_B < B.transitionSet.size(); transIter_B++)
                    //   {
                    //     // cout << "Looking for shared signal in the transitions of B" << endl;
                    //     // printTransitionInfo(B.transitionSet[transIter_B]);
                    //
                    //     // Shared signals must match including the transition's onSignalEvent
                    //     if(B.transitionSet[transIter_B].transitionSignal.isEquivalent(A.transitionSet[transIter_A].transitionSignal) &&
                    //        B.transitionSet[transIter_B].onSignalEvent == A.transitionSet[transIter_A].onSignalEvent)
                    //     {
                    //       sharedSignalBIndex = transIter_B;
                    //       // foundSharedSignalTransition = true;
                    //
                    //       cout << "FOUND SHARED SIGNAL IN TRANS" << endl;
                    //
                    //       // The two parent states are used to create or reference an existing state
                    //       // int newCurrentStateIndex = this->addState(A.transitionSet[transIter_A].currentState, B.transitionSet[sharedSignalBIndex].currentState, A.transitionSet[transIter_A].transitionSignal, sharedSignals); // A and B's signal is the same, its shared
                    //
                    //       state correctCurrentState_A;
                    //       state correctCurrentState_B;
                    //       for(int stateIter_A = 0; stateIter_A < A.stateSet.size(); stateIter_A++)
                    //       {
                    //         if(A.transitionSet[transIter_A].currentState.ID == A.stateSet[stateIter_A].ID)
                    //         {
                    //           correctCurrentState_A = A.stateSet[stateIter_A];
                    //         }
                    //       }
                    //       for(int stateIter_B = 0; stateIter_B < A.stateSet.size(); stateIter_B++)
                    //       {
                    //         if(B.transitionSet[sharedSignalBIndex].currentState.ID == B.stateSet[stateIter_B].ID)
                    //         {
                    //           correctCurrentState_B = B.stateSet[stateIter_B];
                    //         }
                    //       }
                    //
                    //       // Now add the new current state
                    //       int newCurrentStateIndex = this->addState(correctCurrentState_A, correctCurrentState_B, A.transitionSet[transIter_A].transitionSignal, sharedSignals); // We don't know the enabled signals of the nextState
                    //
                    //
                    //       // The below ensures we reference the state set states, these are certain to container
                    //       //  all correct information. We don't use the transition bc the next states were not given
                    //       //  updated enabled signal information. This is just a reprecussion of the transition processing
                    //       state correctNextState_A;
                    //       state correctNextState_B;
                    //       for(int stateIter_A = 0; stateIter_A < A.stateSet.size(); stateIter_A++)
                    //       {
                    //         if(A.transitionSet[transIter_A].nextState.ID == A.stateSet[stateIter_A].ID)
                    //         {
                    //           correctNextState_A = A.stateSet[stateIter_A];
                    //         }
                    //       }
                    //       for(int stateIter_B = 0; stateIter_B < A.stateSet.size(); stateIter_B++)
                    //       {
                    //         if(B.transitionSet[sharedSignalBIndex].nextState.ID == B.stateSet[stateIter_B].ID)
                    //         {
                    //           correctNextState_B = B.stateSet[stateIter_B];
                    //         }
                    //       }
                    //
                    //       // Now add the new next state
                    //       int newNextStateIndex = this->addState(correctNextState_A, correctNextState_B, sharedSignals); // We don't know the enabled signals of the nextState
                    //
                    //       // All thats needed to create the new transition are the two reference states and the signal
                    //       transition newTransition = this->addTransition(this->stateSet[newCurrentStateIndex], this->stateSet[newNextStateIndex], A.transitionSet[transIter_A]);
                    //
                    //     }
                    //   }
                    //
                    //   // Now we need to generate the new current and next states
                    //   //  if they do not already exist
                    //   // if(foundSharedSignalTransition)
                    //   // {
                    //   //   cout << "FOUND SHARED SIGNAL IN TRANS" << endl;
                    //   //
                    //   //   // The two parent states are used to create or reference an existing state
                    //   //   // int newCurrentStateIndex = this->addState(A.transitionSet[transIter_A].currentState, B.transitionSet[sharedSignalBIndex].currentState, A.transitionSet[transIter_A].transitionSignal, sharedSignals); // A and B's signal is the same, its shared
                    //   //
                    //   //   state correctCurrentState_A;
                    //   //   state correctCurrentState_B;
                    //   //   for(int stateIter_A = 0; stateIter_A < A.stateSet.size(); stateIter_A++)
                    //   //   {
                    //   //     if(A.transitionSet[transIter_A].currentState.ID == A.stateSet[stateIter_A].ID)
                    //   //     {
                    //   //       correctCurrentState_A = A.stateSet[stateIter_A];
                    //   //     }
                    //   //   }
                    //   //   for(int stateIter_B = 0; stateIter_B < A.stateSet.size(); stateIter_B++)
                    //   //   {
                    //   //     if(B.transitionSet[sharedSignalBIndex].currentState.ID == B.stateSet[stateIter_B].ID)
                    //   //     {
                    //   //       correctCurrentState_B = B.stateSet[stateIter_B];
                    //   //     }
                    //   //   }
                    //   //
                    //   //   // Now add the new current state
                    //   //   int newCurrentStateIndex = this->addState(correctCurrentState_A, correctCurrentState_B, A.transitionSet[transIter_A].transitionSignal, sharedSignals); // We don't know the enabled signals of the nextState
                    //   //
                    //   //
                    //   //   // The below ensures we reference the state set states, these are certain to container
                    //   //   //  all correct information. We don't use the transition bc the next states were not given
                    //   //   //  updated enabled signal information. This is just a reprecussion of the transition processing
                    //   //   state correctNextState_A;
                    //   //   state correctNextState_B;
                    //   //   for(int stateIter_A = 0; stateIter_A < A.stateSet.size(); stateIter_A++)
                    //   //   {
                    //   //     if(A.transitionSet[transIter_A].nextState.ID == A.stateSet[stateIter_A].ID)
                    //   //     {
                    //   //       correctNextState_A = A.stateSet[stateIter_A];
                    //   //     }
                    //   //   }
                    //   //   for(int stateIter_B = 0; stateIter_B < A.stateSet.size(); stateIter_B++)
                    //   //   {
                    //   //     if(B.transitionSet[sharedSignalBIndex].nextState.ID == B.stateSet[stateIter_B].ID)
                    //   //     {
                    //   //       correctNextState_B = B.stateSet[stateIter_B];
                    //   //     }
                    //   //   }
                    //   //
                    //   //   // Now add the new next state
                    //   //   int newNextStateIndex = this->addState(correctNextState_A, correctNextState_B, sharedSignals); // We don't know the enabled signals of the nextState
                    //   //
                    //   //   // All thats needed to create the new transition are the two reference states and the signal
                    //   //   transition newTransition = this->addTransition(this->stateSet[newCurrentStateIndex], this->stateSet[newNextStateIndex], A.transitionSet[transIter_A]);
                    //   // }
                    // }

                    // else
                    // {
                      // cout << "A TRANS with B states" << endl;

      // If the transitionSignal is not shared, we need to make new transition with this A transition's
      // current and next states composed with all states of B
      for(int stateIter_B = 0; stateIter_B < B.stateSet.size(); stateIter_B++)
      {
        // cout << "ABTrans" << endl;
        // The two parent states are used to create or reference an existing state
        int newCurrentStateIndex = this->addState(A.transitionSet[transIter_A].currentState, B.stateSet[stateIter_B], A.transitionSet[transIter_A].transitionSignal, sharedSignals);
        int newNextStateIndex; // = this->addState(A.transitionSet[transIter_A].nextState, B.stateSet[stateIter_B], sharedSignals); // We don't know the enabled signals of the nextState

        // The below ensures we reference the state set states, these are certain to container
        //  all correct information. We don't use the transition bc the next states were not given
        //  updated enabled signal information. This is just a reprecussion of the transition processing
        for(int stateIter_A = 0; stateIter_A < A.stateSet.size(); stateIter_A++)
        {
          if(A.transitionSet[transIter_A].nextState.ID == A.stateSet[stateIter_A].ID)
          {
            newNextStateIndex = this->addState(A.stateSet[stateIter_A], B.stateSet[stateIter_B], sharedSignals); // We don't know the enabled signals of the nextState
          }
        }

        // All thats needed to create the new transition are the two reference states and the A transition
        //  The signal event is taken from A's transition
        transition newTransition = this->addTransition(this->stateSet[newCurrentStateIndex], this->stateSet[newNextStateIndex], A.transitionSet[transIter_A]);

        // cout << "Resulting Transition:" << endl;
        // printTransitionInfo(this->transitionSet[this->transitionSet.size()-1]);
      }

      // The signal indexes must be updated according to the shared set of signals
      for(int signalIter_this = 0; signalIter_this < this->signalSet.size(); signalIter_this++)
      {
        if(this->transitionSet[this->transitionSet.size()-1].transitionSignal.ID == this->signalSet[signalIter_this].ID)
        {
          this->transitionSet[this->transitionSet.size()-1].transitionSignal.index = signalIter_this;
        }
      }
    }
  }

  // for(int transIter_AB = 0; transIter_AB < this->transitionSet.size(); transIter_AB++)
  // {
  //   printTransitionInfo(this->transitionSet[transIter_AB]);
  // }

  // Loop through all transitions of automata B
  //  We need to get the rest of B's transitions that were not added from the shared signal handling
  //  in the above loop through A's transitions
  for(int transIter_B = 0; transIter_B < B.transitionSet.size(); transIter_B++)
  {
    // cout << "Merging B transition:" << endl;
    // printTransitionInfo(B.transitionSet[transIter_B]);

    // Flag to hold whether or no the current transition uses a signal shared by both A and B
    bool usesSharedSignal = false;

    // Check if the transition action is in shared signal set
    for(int transIter_shared = 0; transIter_shared < sharedSignals.size(); transIter_shared++)
    {

      // cout << "********\n Checking if signals are identical" << endl;
      //
      // printSignalInfo(B.transitionSet[transIter_B].transitionSignal);
      // printSignalInfo(sharedSignals[transIter_shared]);
      //
      // cout << "********" << endl;

      if(B.transitionSet[transIter_B].transitionSignal.isEquivalent(sharedSignals[transIter_shared]))
        // usesSharedSignal = true;
      {
        // Flag to be set if A has a transition with the shared signal
        bool foundSharedSignalTransition = false;
        int sharedSignalAIndex;

        // Loop through A transitions
        for(int transIter_A = 0; transIter_A < A.transitionSet.size(); transIter_A++)
        {
          // cout << "Looking for shared signal in the transitions of B" << endl;
          // printTransitionInfo(B.transitionSet[transIter_B]);

          // Shared signals must match including the transition's onSignalEvent
          if(B.transitionSet[transIter_B].transitionSignal.isEquivalent(A.transitionSet[transIter_A].transitionSignal) &&
             B.transitionSet[transIter_B].onSignalEvent == A.transitionSet[transIter_A].onSignalEvent)
          {
            usesSharedSignal = true;
            sharedSignalAIndex = transIter_A;

            // this->transitionSet[]
            // foundSharedSignalTransition = true;

            // cout << "FOUND SHARED SIGNAL IN TRANS" << endl;

            // The two parent states are used to create or reference an existing state
            // int newCurrentStateIndex = this->addState(A.transitionSet[transIter_A].currentState, B.transitionSet[sharedSignalAIndex].currentState, A.transitionSet[transIter_A].transitionSignal, sharedSignals); // A and B's signal is the same, its shared

            state correctCurrentState_A;
            state correctCurrentState_B;
            for(int stateIter_A = 0; stateIter_A < A.stateSet.size(); stateIter_A++)
            {
              if(A.transitionSet[transIter_A].currentState.ID == A.stateSet[stateIter_A].ID)
              {
                correctCurrentState_A = A.stateSet[stateIter_A];
              }
            }
            for(int stateIter_B = 0; stateIter_B < B.stateSet.size(); stateIter_B++)
            {
              if(B.transitionSet[sharedSignalAIndex].currentState.ID == B.stateSet[stateIter_B].ID)
              {
                correctCurrentState_B = B.stateSet[stateIter_B];
              }
            }

            // Now add the new current state
            int newCurrentStateIndex = this->addState(correctCurrentState_A, correctCurrentState_B, B.transitionSet[transIter_B].transitionSignal, sharedSignals);

            // The below ensures we reference the state set states, these are certain to contain
            //  all correct information. We don't use the transition bc the next states were not given
            //  updated enabled signal information. This is just a reprecussion of the transition processing
            state correctNextState_A;
            state correctNextState_B;
            for(int stateIter_A = 0; stateIter_A < A.stateSet.size(); stateIter_A++)
            {
              if(A.transitionSet[transIter_A].nextState.ID == A.stateSet[stateIter_A].ID)
              {
                correctNextState_A = A.stateSet[stateIter_A];
              }
            }
            for(int stateIter_B = 0; stateIter_B < B.stateSet.size(); stateIter_B++)
            {
              if(B.transitionSet[sharedSignalAIndex].nextState.ID == B.stateSet[stateIter_B].ID)
              {
                correctNextState_B = B.stateSet[stateIter_B];
              }
            }
            // printStateInfo(correctNextState_A);
            // printStateInfo(correctNextState_B)

            // Now add the new next state
            int newNextStateIndex = this->addState(correctNextState_A, correctNextState_B, sharedSignals); // We don't know the enabled signals of the nextState

            // All thats needed to create the new transition are the two reference states and the signal
            transition newTransition = this->addTransition(this->stateSet[newCurrentStateIndex], this->stateSet[newNextStateIndex], B.transitionSet[transIter_B]);

            // The signal indexes must be updated according to the shared set of signals
            for(int signalIter_this = 0; signalIter_this < sharedSignals.size(); signalIter_this++)
            {
              if(this->transitionSet[this->transitionSet.size()-1].transitionSignal.ID == sharedSignals[signalIter_this].ID)
              {
                this->transitionSet[this->transitionSet.size()-1].transitionSignal.index = signalIter_this;
              }
            }

            // cout << "Resulting Transition:" << endl;
            // printTransitionInfo(this->transitionSet[this->transitionSet.size()-1]);

          }
        }
      }
    }

    if(!usesSharedSignal)
    {

      // cout << "B TRANS with A states" << endl;
      // printTransitionInfo(B.transitionSet[transIter_B]);

      // If the transitionSignal is not shared, we need to make new transitions with this B transition's
      // current and next states composed with all states of A
      for(int stateIter_A = 0; stateIter_A < A.stateSet.size(); stateIter_A++)
      {
        // cout << " A state" << endl;
        // printStateInfo(A.stateSet[stateIter_A]);

        // cout << " B trans" << endl;
        // printTransitionInfo(B.transitionSet[transIter_B]);
        // cout << "Current state" << endl;
        // printStateInfo(B.transitionSet[transIter_B].currentState);
        // cout << "Next state" << endl;
        // printStateInfo(B.transitionSet[transIter_B].nextState);
        //
        // cout << "MAKE CURRENT" << endl;
        // The two parent states are used to create or reference an existing state
        int newCurrentStateIndex = this->addState(A.stateSet[stateIter_A], B.transitionSet[transIter_B].currentState, B.transitionSet[transIter_B].transitionSignal, sharedSignals);
        int newNextStateIndex;// = this->addState(A.stateSet[stateIter_A], B.transitionSet[transIter_B].nextState, sharedSignals); // We don't know the enabled signals of the nextState

        // The below ensures we reference the state set states, these are certain to container
        //  all correct information. We don't use the transition bc the next states were not given
        //  updated enabled signal information. This is just a reprecussion of the transition processing
        for(int stateIter_B = 0; stateIter_B < B.stateSet.size(); stateIter_B++)
        {
          if(B.transitionSet[transIter_B].nextState.ID == B.stateSet[stateIter_B].ID)
          {
            // cout << "A Next state" << endl;
            // printStateInfo(A.stateSet[stateIter_A]);
            // cout << "B Next state" << endl;
            // printStateInfo(B.stateSet[stateIter_B]);
            // cout << "MAKE NEXT" << endl;

            newNextStateIndex = this->addState(A.stateSet[stateIter_A], B.stateSet[stateIter_B], sharedSignals); // We don't know the enabled signals of the nextState
          }
        }

        // All thats needed to create the new transition are the two reference states and the A transition
        //  The signal event is taken from A's transition
        transition newTransition = this->addTransition(this->stateSet[newCurrentStateIndex], this->stateSet[newNextStateIndex], B.transitionSet[transIter_B]);

        // The signal indexes must be updated according to the shared set of signals
        for(int signalIter_this = 0; signalIter_this < this->signalSet.size(); signalIter_this++)
        {
          if(this->transitionSet[this->transitionSet.size()-1].transitionSignal.ID == this->signalSet[signalIter_this].ID)
          {
            this->transitionSet[this->transitionSet.size()-1].transitionSignal.index = signalIter_this;
          }
        }

        // cout << "Resulting Transition:" << endl;
        // printTransitionInfo(this->transitionSet[this->transitionSet.size()-1]);
      }
    }
  }

  // for(int transIter_AB = 0; transIter_AB < this->transitionSet.size(); transIter_AB++)
  // {
  //   printTransitionInfo(this->transitionSet[transIter_AB]);
  // }
}


// Determines if the composed state is illegal
bool automaton::isComposedStateIllegal(state A, state B, signal_set sharedSignals)
{
  // Illegal states are determined by looping through the set of shared signals and
  //  determining if any of the signals violate the below conditions
  //  - The signal is an output signal enabled at state A and the signal is not an enabled input signal at state B
  //      or
  //  - The signal is an output signal enabled at state B and the signal is not an  enabledinput signal at state A

  // cout << "********\n Checking if parents form illegal" << endl;

  // cout << "NUM shared signals: " << sharedSignals.size() << endl;
  // printStateInfo(A);
  // printStateInfo(B);

  bool violation = false;

  // Loop through all shared signals
  for(int signalIter_shared = 0; signalIter_shared < sharedSignals.size(); signalIter_shared++)
  {
    // cout << "********\n Shared signal: ";
    // printSignalInfo(sharedSignals[signalIter_shared]);

    // Loop through the enabled signals of A to find shared signals
    for(int signalIter_enabledA = 0; signalIter_enabledA < A.enabledSignals.size(); signalIter_enabledA++)
    {
      // cout << "Enabled signal A" << endl;
      // printSignalInfo(A.enabledSignals[signalIter_enabledA]);
      // cout << "********" << endl;


      // Can we find the shared signal in the enabled signals
      if(sharedSignals[signalIter_shared].isEquivalent(A.enabledSignals[signalIter_enabledA]))
      {
        // cout << "Enabled signal A is equivalent to shared signal" << endl;

        // We are looking for output signals in A that are not enabled as inputs in B
        if(A.enabledSignals[signalIter_enabledA].type == output)
        {
          // cout << "Enabled signal A is OUTPUT" << endl;

          // The violation exists if we cannot find an enabled input
          //  signal in B's enabled signals to satisfy A's output
          bool satisfyingSignalFound = false;

          // Loop through the enabled signals of B to find input signal
          for(int signalIter_enabledB = 0; signalIter_enabledB < B.enabledSignals.size(); signalIter_enabledB++)
          {
            // If we find a signal with matching name
            if(A.enabledSignals[signalIter_enabledA].ID == B.enabledSignals[signalIter_enabledB].ID)
            {
              // It must be an input
              if(B.enabledSignals[signalIter_enabledB].type == input)
              {
                // cout << "Found an input in B" << endl;
                satisfyingSignalFound = true;
              }
            }
          }

          // Illegal
          if(!satisfyingSignalFound)
          {
            cout << "No suitable input found in B" << endl;
            violation = true;
          }

          // Otherwise, we go to the next enabled signal of A
        }
      }
    }

    // Loop through the enabled signals of B to find shared signals
    for(int signalIter_enabledB = 0; signalIter_enabledB < B.enabledSignals.size(); signalIter_enabledB++)
    {
      // cout << "Enabled signal B" << endl;
      // printSignalInfo(B.enabledSignals[signalIter_enabledB]);
      // cout << "********" << endl;


      // Can we find the shared signal in the enabled signals
      if(sharedSignals[signalIter_shared].isEquivalent(B.enabledSignals[signalIter_enabledB]))
      {
        // cout << "Enabled signal B is equivalent to shared signal" << endl;

        // We are looking for output signals in B that are not enabled as inputs in A
        if(B.enabledSignals[signalIter_enabledB].type == output)
        {
          // cout << "Enabled signal B is OUTPUT" << endl;

          // The violation exists if we cannot find an enabled input
          //  signal in B's enabled signals to satisfy B's output
          bool satisfyingSignalFound = false;

          // Loop through the enabled signals of B to find input signal
          for(int signalIter_enabledA = 0; signalIter_enabledA < A.enabledSignals.size(); signalIter_enabledA++)
          {
            // If we find a signal with matching name
            if(B.enabledSignals[signalIter_enabledB].ID == A.enabledSignals[signalIter_enabledA].ID)
            {
              // If that signal is an input
              if(A.enabledSignals[signalIter_enabledA].type == input)
              {
                // cout << "Found an input in A" << endl;
                satisfyingSignalFound = true;
              }
            }
          }

          // Illegal
          if(!satisfyingSignalFound)
          {
            cout << "No suitable input found in A" << endl;
            violation = true;
            // return true;
          }

          // Otherwise, we go to the next enabled signal of A
        }
      }
    }

  }

  // If we found no vilations, then the state produced will not be illegal
  if(!violation)
  {
    // cout << "No violations, state permissable" << endl;
    return false;
  }
  else
  {
    // cout << "Illegal state" << endl;
    return true;
  }
}


// Add a new signal to the set if it does not already exist
//  ONLY USED FOR AUTOMATA COMPOSURE - see isEquivalent function of the signal struct
void automaton::addSignal(componentSignal newSignal)
{
  // cout << "COMAPRING>>>>>>>>" << endl;
  // printSignalInfo(newSignal);

  // Loop through this stateSet to determine if the new signal already exists
  for(int signalIter_this = 0; signalIter_this < this->signalSet.size(); signalIter_this++)
  {
    // printSignalInfo(this->signalSet[signalIter_this]);

    if(newSignal.isEquivalent(this->signalSet[signalIter_this]))
    {
      // cout << "ALREADY EXISTS" << endl;
      return;
    }
  }
  // cout << "ADDING" << endl;

  // If we were not kicked out of the function, add the signal
  newSignal.index = this->signalSet.size();
  this->signalSet.push_back(newSignal);
}


// Create a new state or return existing state from this.stateSet with ID
//  This state is generated from two other states during composition
//  Overloaded for use without supplying enabled signal (mostly for making nextStates where
//  we don't know their enabled signals)
int automaton::addState(state A, state B, signal_set sharedSignals)
{
  // TODO
  // I need to handle accepting states and possibly illegal states
  //  They may be differnet than initial, but not sure

  // cout << "Adding new composed state from these parents (No enabled signal):" << endl;
  // printStateInfo(A);
  // printStateInfo(B);

  // New state ID
  string ID = A.ID + B.ID;

  // New state to be added
  state newState;

  // Flags showing whether or not the proposed new
  //  state already exists in this stateSet
  bool foundState;

  // Proposed new index for the new state
  //  If doesn't exist, we are just going to push_back the stateSet
  int newStateIndex = this->stateSet.size();

  // Ensure they don't already exist
  for(int stateIter_this = 0; stateIter_this < this->stateSet.size(); stateIter_this++)
  {
    if(ID == this->stateSet[stateIter_this].ID)
    {
      foundState = true;
      newStateIndex = stateIter_this;
    }
  }

  // If we couldn't find new state in stateSet
  if(!foundState)
  {
    // Populate fields of the new state
    newState.ID = ID;
    newState.index = newStateIndex;

    // TODO
    //  For formality we shouldn't just assume these, but I'm not sure we are
    //  going to need to know about accepting states and illegal states may be
    //  introduced sometime before this step.
    newState.accepting = 0;
    newState.illegal = 0;

    // Check if the state is an initial
    if(A.initial && B.initial)
      newState.initial = 1;
    else
      newState.initial = 0;

    // cout << "Added new composed state";
    // printStateInfo(newState);

    // Now the newly created state is in the newState object, just add to this stateSet
    this->stateSet.push_back(newState);
  }

  // If we found an existing match
  else
  {
    // cout << "State already exists" << endl;
    // Reference the modified state
    newState = this->stateSet[newStateIndex];

    // We dont add newState to the stateSet bc its already there, but
    //  we still return the object for index reference
  }

  // Determine if the new state is illegal
  //  Pass the original states of the composure, and the set of shared signals
  if(isComposedStateIllegal(A, B, sharedSignals))
  {
    this->stateSet[newStateIndex].illegal = 1;
    this->illegalStates.push_back(this->stateSet[newStateIndex]);
  }

  // Return the state index
  return newState.index;
}


// Create a new state or use existing and return the index of the state
//  Overloaded to accept a signal to add to the set of enabled signals of the state
//  This state is generated from two other states during composition
int automaton::addState(state A, state B, componentSignal enabledSignal, signal_set sharedSignals)
{
  // TODO
  // I need to handle accepting states and possibly illegal states
  //  They may be differnet than initial, but not sure

  // cout << "Adding new composed state from these parents:" << endl;
  // cout << "With enabled signal:" << endl;
  // printSignalInfo(enabledSignal);
  // printStateInfo(A);
  // printStateInfo(B);

  // New state ID
  string ID = A.ID + B.ID;

  // New state to be added if they don't already exist
  state newState;

  // Flags showing whether or not the proposed new
  //  state already exists in this stateSet
  bool foundState;

  // Proposed new index for the new state
  //  If doesn't exist, we are just going to push_back the stateSet
  int newStateIndex = this->stateSet.size();

  // Ensure they don't already exist
  for(int stateIter_this = 0; stateIter_this < this->stateSet.size(); stateIter_this++)
  {
    if(ID == this->stateSet[stateIter_this].ID)
    {
      foundState = true;
      newStateIndex = stateIter_this;
    }
  }

  // If we couldn't find new state in stateSet
  if(!foundState)
  {

    // cout << "New ID: " << ID << endl;

    // Populate fields of the new state
    newState.ID = ID;
    newState.index = newStateIndex;

    // TODO
    //  For formality we shouldn't just assume these, but I'm not sure we are
    //  going to need to know about accepting states and illegal states may be
    //  introduced sometime before this step.
    newState.accepting = 0;
    newState.illegal = 0;

    // Check if the state is an initial
    if(A.initial && B.initial)
      newState.initial = 1;
    else
      newState.initial = 0;

    // This adds the supplied signal to the state
    newState.enabledSignals.push_back(enabledSignal);

    // cout << "Added new composed state";
    // printStateInfo(newState);

    // Now the newly created state is in the newState object, just add to this stateSet
    this->stateSet.push_back(newState);
  }

  // If we found an existing match
  else
  {

    // cout << "Existing state" << endl;
    // Does the enabled already exist?
    bool alreadyExists = false;

    // Ensure the enabled signal already exists or is added
    for(int signalIter = 0; signalIter < this->stateSet[newStateIndex].enabledSignals.size(); signalIter++)
    {
      if(enabledSignal.isEquivalent(this->stateSet[newStateIndex].enabledSignals[signalIter]))
      {
        alreadyExists = true;
      }
    }

    // If the enabledSignal is not already added
    if(!alreadyExists)
    {
      // Modify the existing state to update the enabled signals
      this->stateSet[newStateIndex].enabledSignals.push_back(enabledSignal);

      // cout << "Added an enabled signal to an existing composed state";
      // printStateInfo(this->stateSet[newStateIndex]);
    }


    // Reference the modified state
    newState = this->stateSet[newStateIndex];

    // We dont add newState to the stateSet bc its already there, but
    //  we still return the object for index reference
  }

  // Determine if the new state is illegal
  //  Pass the original states of the composure, and the set of shared signals
  if(isComposedStateIllegal(A, B, sharedSignals))
  {
    this->stateSet[newStateIndex].illegal = 1;
    this->illegalStates.push_back(this->stateSet[newStateIndex]);
  }

  // Return the state index
  return newState.index;
}


// Create a new transition and add to this transition set
transition automaton::addTransition(state currentState, state nextState, transition transition)
{
  // Set all attributes
  struct transition newTrans;
  newTrans.index = this->transitionSet.size();
  newTrans.currentState = currentState;
  newTrans.nextState = nextState;
  newTrans.transitionSignal = transition.transitionSignal;

  // Take care of the onSignalEvent using the supplied transition for reference
  newTrans.onSignalEvent = transition.onSignalEvent;

  // Set the row and col to -1 signifying transition has not been placed
  //  in the transition table as of yet
  newTrans.row = -1;
  newTrans.col = -1;

  // Finally, we add the new transition with references to the appropriate signal
  //   and states to this automata's transitionSet
  this->transitionSet.push_back(newTrans);

  // Return the transition
  return newTrans;
}


// Overload the = operator
automaton& automaton::operator=(const automaton object)
{
  // Deep copy
  this->stateSet = object.stateSet;
  this->signalSet = object.signalSet;
  this->transitionSet = object.transitionSet;
  this->transitionTable = object.transitionTable;

  return(*this);
}
