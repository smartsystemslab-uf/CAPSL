#include <stdlib.h>
#include <vector>
#include <string.h>
#include <iostream>
#include <fstream>
#include <algorithm>

#include <spot/tl/parse.hh>
#include <spot/tl/print.hh>
#include <spot/twaalgos/dot.hh>
#include <spot/twaalgos/hoa.hh>
#include <spot/twaalgos/translate.hh>

#include "automaton.h"
#include "global.h"

using namespace std;


typedef vector<automaton> automatonSet;


//*****************
//      FILE
//*****************

// Read config info from ia/sere/config files
void readConfig(const char *filename_IA,
								const char *filename_SERE,
								const char *filename_Config,
								ConfigFormat configFormat,
							 	configInfo &config_info)
{
	// configure using .ia and .sere format
	if (configFormat == ia_and_sere)
	{
		readIAFile(filename_IA, config_info);

	 	readSEREFile(filename_SERE, config_info);
	}
	// configure using .config format
	else
	{
		readConfigFile(filename_Config, config_info);
	}
}

// Read from Config file
void readConfigFile(const char *filename, configInfo &config_info)
{
	ifstream configFile(filename);
	if (!configFile)
	{
		cerr << "\nUnable to open config file \n";
		// exit(0);
	}

	// Containers for IA specifications
	vector_string initState;
  vector_string acceptingStates;
  vector_string allStates;
  vector_string inputSignals;
  vector_string outputSignals;
  vector_string internalSignals;
  vector_string transitions;

	// Container for SERE rules
	vector_string rules;

	// Containers for others
	vector_string resources;

	// Get IP Name from filename
	string strFilename(filename);
	// Ex: config/BasicRSA/BasicRSA.config
	//  extract the substr between the '/' characters
	size_t start = strFilename.find_first_of("/");
	size_t end = strFilename.find_last_of("/");
	string IPName = strFilename.substr( start + 1,  end - start - 1);		// extract

	// Tokens
	string IPDefToken           = IPName + ".def";
	string logicDefToken        = "logic.def";
	string systemDefToken       = "system.def";

	string transitionsToken     = "transitions";
	string resourcesToken       = "resources";
	string prohibitedToken      = "prohibited";
	string counterToken         = "counter";

	string inputToken           = "input";
	string outputToken          = "output";
	string vectorToken          = "vector";


	// read lines from config file
	string line;
	while (getline(configFile, line))
	{
		// Skip empty lines
		if (line.empty())
			continue;


		// **************
		// <IP_Name>.def
		// **************
		if (line.find(IPDefToken) != string::npos)
		{
			while (getline(configFile, line))
			{
				// skip empty lines
				if (line.empty())
					continue;

				// } - end of scope
				if (removeComment(line).find("}") == 0)
					break;

				// input signals
				if (line.find(inputToken) != string::npos)
				{
					// extract signal name, add it to container
					inputSignals.push_back(extractUntil(line, ":"));
					cout << "input: " << extractUntil(line, ":") << endl;
				}

				// output signals
				else if (line.find(outputToken) != string::npos)
				{
					// extract signal name, add it to container
					outputSignals.push_back(extractUntil(line, ":"));
					cout << "output: " << extractUntil(line, ":") << endl;
				}

				// transitions{
				else if (line.find(transitionsToken) != string::npos)
				{
					string transition, state;
					bool first_transition = true;

					// get remaining states/transitions
					while (getline(configFile, transition))
					{
						if (transition.empty())
							continue;

						if (transition.find("}") != string::npos)
							break;

						// extract state from beginning of transition, add to set
						state = extractUntil(transition, ":");
						allStates.push_back(state);

						// on the first transition, add state to initState
						if (first_transition)
						{
							initState.push_back(state);
							cout << "INIT STATE: " << state << endl;
							first_transition = false;
						}
						cout << "STATE: " << state << endl;

						// add transition to set
						transition = extractUntil(removeComment(transition), ",");
						transitions.push_back(transition);
						cout << "TRANSITION: " << transition << endl;
					}
				}

				// resources{
				else if (line.find(resourcesToken) != string::npos)
				{
					string resource;
					while (getline(configFile, resource))
					{
						if (resource.empty())
							continue;

						if (resource.find("}") != string::npos)
							break;

						// TODO handle each resource
					}
				}
			}
		}


		// **************
		// system.def
		// **************
		if (line.find(systemDefToken) != string::npos)
		{
			while (getline(configFile, line))
			{
				if (line.empty())
					continue;

				if (line.find("}") != string::npos)
					break;

				// transitions{
				if (line.find(transitionsToken) != string::npos)
				{
					while (getline(configFile, line))
					{
						if (line.empty())
							continue;

						if (line.find("}") != string::npos)
							break;

						// TODO handle transitions
					}
				}

				// TODO handle each signal
			}
		}


		// **************
		// logic.def
		// **************
		if (line.find(logicDefToken) != string::npos)
		{
			while (getline(configFile, line))
			{
				if (line.empty())
					continue;

				if (line.find("}") != string::npos)
					break;

				// counter {
				if (line.find(counterToken) != string::npos)
				{
					while (getline(configFile, line))
					{
						if (line.empty())
							continue;

						if (line.find("}") != string::npos)
							break;

						// TODO handle transitions
					}
				}

				// prohibited{
				if (line.find(prohibitedToken) != string::npos)
				{
					int ruleCount = 0;
					string rule;
					while (getline(configFile, rule))
					{
						if (rule.empty())
							continue;
						// remove all comments
						rule = removeComment(rule);

						// } should be the first character on its own line if not part of a rule
						if (rule.find("}") == 0)
							break;

						rules.push_back(extractUntil(rule, ","));
						cout << "\nRule" << ruleCount << ":"  << endl << "  " << extractUntil(rule, ",") << endl;
						ruleCount++;
					}
				}

				// TODO handle each signal
			}
		}
	}

	// pack up the containers and send them back
	config_info.initState = initState;
	config_info.acceptingStates = acceptingStates;
	config_info.allStates = allStates;
	config_info.inputSignals = inputSignals;
	config_info.outputSignals = outputSignals;
	config_info.internalSignals = internalSignals;
	config_info.transitions = transitions;

	config_info.rules = rules;
}

// Read from IA file
void readIAFile(const char *filename, configInfo &config_info)
{
  // Open the file
  ifstream configFile(filename);
  if(!configFile)
  {
    cerr << "\nUnable to open IA configuration file \n";
    // exit(0);
  }

	// Containers for IA specifications
	vector_string initState;
  vector_string acceptingStates;
  vector_string allStates;
  vector_string inputSignals;
  vector_string outputSignals;
  vector_string internalSignals;
  vector_string transitions;

  // Tokens
  string stateToken             = "DEFINE STATES";
  string initToken              = "DEFINE INITIAL";
  string acceptingToken         = "DEFINE ACCEPTING";
  string inputToken             = "DEFINE INPUTS";
  string outputToken            = "DEFINE OUTPUTS";
  string internalToken          = "DEFINE INTERNALS";
  string transitionsToken       = "DEFINE TRANSITIONS";

  string stateEndToken          = "END STATES";
  string initEndToken           = "END INITIAL";
  string acceptingEndToken      = "END ACCEPTING";
  string inputsEndToken         = "END INPUTS";
  string outputsEndToken        = "END OUTPUTS";
  string internalsEndToken      = "END INTERNALS";
  string signalsEndToken        = "END SIGNALS";
  string transitionsEndToken    = "END TRANSITIONS";

  string line;
  while(getline(configFile, line))
  {
    // NOTE:
    //  string.find(token) == std::string::npos - TOKEN NOT FOUND
    //  string.find(token) != std::string::npos - TOKEN FOUND

    // Get all non empty lines
    if(!line.empty()){

      // Extract the states
      if(line.find(stateToken) != std::string::npos)
      {
        string state;
        while(getline(configFile, state))
        {
          if(state.find(stateEndToken) == std::string::npos)
          {
            if(!state.empty())
            {
              cout << "STATE: " << removeComment(state) << "\n";
              allStates.push_back(removeComment(state));
            }
          }
          else
            break;
        }
      }

      // Extract the initial state
      if(line.find(initToken) != std::string::npos)
      {
        string initialState;
        while(getline(configFile, initialState))
        {
          if(initialState.find(initEndToken) == std::string::npos)
          {
            if(!initialState.empty())
            {
              cout << "INIT STATE: " << removeComment(initialState) << "\n";
              initState.push_back(removeComment(initialState));
            }
          }
          else
            break;
        }
      }

      // Extract the accepting states
      if(line.find(acceptingToken) != std::string::npos)
      {
        string acceptState;
        while(getline(configFile, acceptState))
        {
          if(acceptState.find(acceptingEndToken) == std::string::npos)
          {
            if(!acceptState.empty())
            {
              cout << "ACCEPTING STATE: " << removeComment(acceptState) << "\n";
              acceptingStates.push_back(removeComment(acceptState));
            }
          }
          else
            break;
        }
      }

      // Extract the input signals
      if(line.find(inputToken) != std::string::npos)
      {
        string componentSignal;
        while(getline(configFile, componentSignal))
        {
          if(componentSignal.find(inputsEndToken) == std::string::npos)
          {
            if(!componentSignal.empty())
            {
              cout << "INPUT ACTION: " << removeComment(componentSignal) << "\n";
              inputSignals.push_back(removeComment(componentSignal));
            }
          }
          else
            break;
        }
      }

      // Extract the output signals
      if(line.find(outputToken) != std::string::npos)
      {
        string componentSignal;
        while(getline(configFile, componentSignal))
        {
          if(componentSignal.find(outputsEndToken) == std::string::npos)
          {
            if(!componentSignal.empty())
            {
              cout << "OUTPUT ACTION: " << removeComment(componentSignal) << "\n";
              outputSignals.push_back(removeComment(componentSignal));
            }
          }
          else
            break;
        }
      }

      // Extract the internal signals
      if(line.find(internalToken) != std::string::npos)
      {
        string componentSignal;
        while(getline(configFile, componentSignal))
        {
          if(componentSignal.find(internalsEndToken) == std::string::npos)
          {
            if(!componentSignal.empty())
            {
              cout << "INTERNAL ACTION: " << removeComment(componentSignal) << "\n";
              internalSignals.push_back(removeComment(componentSignal));
            }
          }
          else
            break;
        }
      }

      // Extract the transitions
      if(line.find(transitionsToken) != std::string::npos)
      {
        string transition;
        while(getline(configFile, transition))
        {
          if(transition.find(transitionsEndToken) == std::string::npos)
          {
            if(!transition.empty())
            {
              cout << "TRANSITION: " << removeComment(transition) << "\n";
              transitions.push_back(removeComment(transition));
            }
          }
          else
            break;
        }
      }
    }
  }

  configFile.close();

	// Store containers in configInfo struct
	config_info.initState = initState;
	config_info.acceptingStates = acceptingStates;
	config_info.allStates = allStates;
	config_info.inputSignals = inputSignals;
	config_info.outputSignals = outputSignals;
	config_info.internalSignals = internalSignals;
	config_info.transitions = transitions;
}


// Read from SERE file
void readSEREFile(const char *filename, configInfo &config_info)
{
	// Container for SERE specifications
	vector_string rules;

  // Open the configuration file
  ifstream configFile(filename);
  if(!configFile)
  {
    cerr << "\nUnable to open SERE configuration file \n";
    // exit(0);
  }

  // Read in the SERE expressions
  string line;
  int ruleCount = 0;

  while(getline(configFile, line))
  {
    // Get all non empty lines
    if(!removeComment(line).empty())
    {
      cout << "\nRule" << ruleCount << ":\n  " << removeComment(line) << endl;
      rules.push_back(removeComment(line));

      // Keep count
      ruleCount++;
    }
  }

  // Close the file
  configFile.close();

	// Store container in config_info
	config_info.rules = rules;
}



//*****************
//       IA
//*****************

// Process the IA specification
void processIAConfiguration(state_set *stateSet,
                            signal_set *signalSet,
                            transition_set *transitionSet,
														configInfo config_info)
{
  // Containers for the IA specifications in config_info
  vector_string initState = config_info.initState;
  vector_string acceptingStates = config_info.acceptingStates;
  vector_string allStates = config_info.allStates;
  vector_string inputSignals = config_info.inputSignals;
  vector_string outputSignals = config_info.outputSignals;
  vector_string internalSignals = config_info.internalSignals;
  vector_string transitions = config_info.transitions;

  // Populate stateSet
  processStates(initState, acceptingStates, allStates, stateSet);

  // Populate signalSet
  processSignals(inputSignals, outputSignals, internalSignals, signalSet);

  // Populate transitionSet
  processTransitions(transitions, stateSet, signalSet, transitionSet, AutomataType::IA);
}


// Process the states from IA configuration into state stucts
void processStates(vector_string initState,
                   vector_string acceptingStates,
                   vector_string allStates,
                   state_set *stateSet)
{
  // cout << "In processStates..." << "\n";

  // Loop through all states
  for(int stateIter = 0; stateIter < allStates.size(); stateIter++)
  {
    state newState;
    newState.index = stateIter; // Set index upon adding to container
    // newState.numID = stoi(allStates[stateIter].substr(1, allStates[stateIter].length()));
    newState.ID = allStates[stateIter];
    newState.initial = 0;
    newState.accepting = 0;
    newState.illegal = 0; // Upon building state set, all will be deemed legal at this point

    // Is this the initial state?
    if(newState.ID == initState[0])
      newState.initial = 1;

    // Is this an accepting state?
    for(int acceptingStateIter = 0; acceptingStateIter < acceptingStates.size(); acceptingStateIter++)
    {
      if(newState.ID == acceptingStates[acceptingStateIter])
      {
        newState.accepting = 1;
      }
    }

    // If state wasn't set as accepting, set the parameter
    if(newState.accepting != 1)
      newState.accepting = 0;

    // printStateInfo(newState);

    // Add to the set of states
    stateSet->push_back(newState);
  }
}


// Process the signals from IA configuration into componentSignal structs
void processSignals(vector_string inputSignals,
                    vector_string outputSignals,
                    vector_string interalSignals,
                    signal_set *signalSet)
{
  // cout << "In processSignals..." << "\n";

  // Indexing for all actions
  int signalSetIndex = 0;

  // Loop through all input signals
  for(int inputIter = 0; inputIter < inputSignals.size(); inputIter++)
  {
    componentSignal newInputSignal;
    newInputSignal.index = signalSetIndex; // Set index upon adding to container
    // newInputSignal.numID = stoi(inputSignals[inputIter]);
    newInputSignal.ID = inputSignals[inputIter];
    newInputSignal.type = signalType::input;

    // printSignalInfo(newInputSignal);

    // Add to the set of signals
    signalSet->push_back(newInputSignal);

    // Must increment the comprehensive index of actions
    signalSetIndex++;
  }

  // Loop through all output signals
  for(int outputIter = 0; outputIter < outputSignals.size(); outputIter++)
  {
    componentSignal newOutputSignal;
    newOutputSignal.index = signalSetIndex; // Set index upon adding to container
    // newOutputSignal.numID = stoi(outputSignals[outputIter]);
    newOutputSignal.ID = outputSignals[outputIter];
    newOutputSignal.type = signalType::output;

    // printSignalInfo(newOutputSignal);

    // Add to the set of signals
    signalSet->push_back(newOutputSignal);

    // Must increment the comprehensive index of actions
    signalSetIndex++;
  }

  // Loop through all internal signals
  for(int internalIter = 0; internalIter < interalSignals.size(); internalIter++)
  {
    componentSignal newInternalSignal;
    newInternalSignal.index = signalSetIndex; // Set index upon adding to container
    // newInternalSignal.numID = stoi(interalSignals[internalIter]);
    newInternalSignal.ID = interalSignals[internalIter];
    newInternalSignal.type = signalType::internal;

    // printSignalInfo(newInternalSignal);

    // Add to the set of signals
    signalSet->push_back(newInternalSignal);

    // Must increment the comprehensive index of actions
    signalSetIndex++;
  }
}


// Process the transitions from configuration into transition structs
void processTransitions(vector_string transitions,
                        state_set *stateSet,
                        signal_set *signalSet,
                        transition_set *transitionSet,
                        AutomataType automataType)
{
  // cout << "In processTransitions..." << "\n";

  // Loop through all transitions
  for(int transIter = 0; transIter < transitions.size(); transIter++)
  {
    // Tokenize transition
    string delim1 = ":";
    string delim2 = ">";

    // Signal low
    string notFlag = "!";

    // For logical expressions of signals
    string andOperatorToken = "&";
    string orOperatorToken = "|";

    string currentState, componentSignal, nextState;
    currentState = transitions[transIter].substr(0, transitions[transIter].find(delim1));
    nextState = transitions[transIter].substr(transitions[transIter].find(delim2)+1, transitions[transIter].length());
    componentSignal = transitions[transIter].substr(transitions[transIter].find(delim1)+1, transitions[transIter].length()-(nextState.length()+1)-(currentState.length()+1));

    // cout << "transitions[transIter].find(delim2): " << transitions[transIter].find(delim2) << endl;
    // cout << "transitions[transIter]: " << transitions[transIter] << endl;
    // cout << "COMPONENTSIGNAL: " << componentSignal << endl;
    // cout << "CURRENTSTATE: " << currentState << endl;
    // cout << "NEXTSTATE: " << nextState << endl;
    // cout << "transitions[transIter].length()-nextState.length(): " << transitions[transIter].length()-nextState.length() << endl;

    // The new transition object
    transition newTrans;

    // NOTE
    // Transitions are put into a common format, however, the signals
    //  that can be depicted are numerous. The signal can be the signal name,
    //  signal name with a ! flag, signal index, signal index with a not flag, or
    //  a combination of signal indexes with some boolean logic.
    // The composite signals are currently produced via SPOT which seems
    //  to only produce logical expressions with the 'and'(&) and 'or(|)' operator and
    //  not(!) operator. This is subject to change.

    // If the current transition signal has logical and operators
    if(componentSignal.find(andOperatorToken) != std::string::npos)
    {
      // Add the expression as a signal in order to add to signalSet which will
      //  allow the transition table to reflect this signal
      struct componentSignal newCompositeSignal;

      newCompositeSignal.index = signalSet->size();
      newCompositeSignal.ID = componentSignal; // The ID reflects the actual expression
      newCompositeSignal.type = composite;
      newCompositeSignal.logicOperator = AND;


      // Tokenize the expression with the and delimiter
      vector_string expressionParts;
      int i = 0;
      int found = componentSignal.find(andOperatorToken);
      bool gotAllExpressions = false;
      while (!gotAllExpressions)
      {
        // Get each of the expression parts
        string subExpression = componentSignal.substr(i, found-i);

        // Add to set of expression parts
        expressionParts.push_back(subExpression);

        // Iterate
        i = ++found;
        found = componentSignal.find(andOperatorToken, found);

        // Get the last expression and kill loop
        if(found == std::string::npos)
        {
          string subExpression = componentSignal.substr(i, found-i);

          // Add to set of expression parts
          expressionParts.push_back(subExpression);
          gotAllExpressions = true;
        }
      }

      // Loop through the extracted signals from the expression
      for(int subExpressionIter = 0; subExpressionIter < expressionParts.size(); subExpressionIter++)
      {
        // Loop through signalSet to assign transition componentSignal
        for(int signalIter = 0; signalIter < signalSet->size(); signalIter++)
        {
          // Signal index only - This signal is high
          if(expressionParts[subExpressionIter] == to_string((*signalSet)[signalIter].index))
          {
            newCompositeSignal.compositeSignalSet.push_back((*signalSet)[signalIter]);
            newCompositeSignal.compositeSignalEvents.push_back(High);
          }

          // Signal index with ! flag - This signal is low
          else if(expressionParts[subExpressionIter].substr(1,expressionParts[subExpressionIter].length()) == to_string((*signalSet)[signalIter].index))
          {
            newCompositeSignal.compositeSignalSet.push_back((*signalSet)[signalIter]);
            newCompositeSignal.compositeSignalEvents.push_back(Low);
          }
        }
      }

      // cout << "Adding composite signal..." << endl;
      // printSignalInfo(newCompositeSignal);

      // Add the new signal to the comprehensive set,
      //  ensuring there are no duplicates
      bool alreadyExists = false;
      int existingIndex = -1;
      for(int signalIter_all = 0; signalIter_all < signalSet->size(); signalIter_all++)
      {
        if((*signalSet)[signalIter_all].ID == newCompositeSignal.ID)
        {
          alreadyExists = true;
          existingIndex = signalIter_all;
        }
      }
      if(!alreadyExists)
      {
        signalSet->push_back(newCompositeSignal);

        // Now we add the new signal to the transition
        //  Event type should be high, we only care when expression is asserted
        newTrans.transitionSignal = newCompositeSignal;
        newTrans.onSignalEvent = High;
      }
      else
      {
        // We add the new signal to the transition
        //  Event type should be high, we only care when expression is asserted
        newTrans.transitionSignal = (*signalSet)[existingIndex];
        newTrans.onSignalEvent = High;
      }

    }

    // If the current transition signal has logical or operators
    else if(componentSignal.find(orOperatorToken) != std::string::npos)
    {
      // Add the expression as a signal in order to add to signalSet which will
      //  allow the transition table to reflect this signal
      struct componentSignal newCompositeSignal;

      newCompositeSignal.index = signalSet->size();
      newCompositeSignal.ID = componentSignal; // The ID reflects the actual expression
      newCompositeSignal.type = composite;
      newCompositeSignal.logicOperator = OR;

      // Tokenize the expression with the or delimiter
      vector_string expressionParts;
      int i = 0;
      int found = componentSignal.find(orOperatorToken);
      bool gotAllExpressions = false;
      while (!gotAllExpressions)
      {
        // Get each of the expression parts
        string subExpression = componentSignal.substr(i, found-i);
        subExpression.erase(std::remove(subExpression.begin(),subExpression.end(),' '),subExpression.end());

        // Add to set of expression parts
        expressionParts.push_back(subExpression);

        // Iterate
        i = ++found;
        found = componentSignal.find(orOperatorToken, found);

        // Get the last expression and kill loop
        if(found == std::string::npos)
        {
          string subExpression = componentSignal.substr(i, found-i);

          subExpression.erase(std::remove(subExpression.begin(),subExpression.end(),' '),subExpression.end());
          // Add to set of expression parts
          expressionParts.push_back(subExpression);
          gotAllExpressions = true;
        }
      }

      // Loop through the extracted signals from the expression
      for(int subExpressionIter = 0; subExpressionIter < expressionParts.size(); subExpressionIter++)
      {
        // cout << expressionParts[subExpressionIter] << endl;
        // Loop through signalSet to assign transition componentSignal
        for(int signalIter = 0; signalIter < signalSet->size(); signalIter++)
        {
          // Signal index only - This signal is high
          if(expressionParts[subExpressionIter] == to_string((*signalSet)[signalIter].index))
          {
            newCompositeSignal.compositeSignalSet.push_back((*signalSet)[signalIter]);
            newCompositeSignal.compositeSignalEvents.push_back(High);
          }

          // Signal index with ! flag - This signal is low
          else if(expressionParts[subExpressionIter].substr(1,expressionParts[subExpressionIter].length()) == to_string((*signalSet)[signalIter].index))
          {
            newCompositeSignal.compositeSignalSet.push_back((*signalSet)[signalIter]);
            newCompositeSignal.compositeSignalEvents.push_back(Low);
          }
        }
      }

      // cout << "Adding composite signal..." << endl;
      // printSignalInfo(newCompositeSignal);

      // Add the new signal to the comprehensive set,
      //  ensuring there are no duplicates
      bool alreadyExists = false;
      int existingIndex = -1;
      for(int signalIter_all = 0; signalIter_all < signalSet->size(); signalIter_all++)
      {
        if((*signalSet)[signalIter_all].ID == newCompositeSignal.ID)
        {
          alreadyExists = true;
          existingIndex = signalIter_all;
        }
      }
      if(!alreadyExists)
      {
        signalSet->push_back(newCompositeSignal);

        // Now we add the new signal to the transition
        //  Event type should be high, we only care when expression is asserted
        newTrans.transitionSignal = newCompositeSignal;
        newTrans.onSignalEvent = High;
      }
      else
      {
        // We add the new signal to the transition
        //  Event type should be high, we only care when expression is asserted
        newTrans.transitionSignal = (*signalSet)[existingIndex];
        newTrans.onSignalEvent = High;
      }
    }

    // TRUE or FALSE transition
    else if(componentSignal == "t" || componentSignal == "f")
    {
      if(componentSignal == "t")
      {
        // Add the expression as a signal in order to add to signalSet which will
        //  allow the transition table to reflect this signal
        struct componentSignal alwaysTrueSignal;
        alwaysTrueSignal.index = signalSet->size();
        alwaysTrueSignal.type = alwaysTrue;

        // Set ID
        stringstream ID;
        for(int signalIter_all = 0; signalIter_all < signalSet->size(); signalIter_all++)
        {
          ID << (*signalSet)[signalIter_all].index << "|!" << (*signalSet)[signalIter_all].index;

          if(signalIter_all < signalSet->size()-1)
          {
            ID << "|";
          }
        }
        // cout << ID.str() << endl;
        alwaysTrueSignal.ID = ID.str(); // The ID reflects the actual expression

        // Add the new signal to the comprehensive set,
        //  ensuring there are no duplicates
        bool alreadyExists = false;
        int existingIndex = -1;
        for(int signalIter_all = 0; signalIter_all < signalSet->size(); signalIter_all++)
        {
          if((*signalSet)[signalIter_all].ID == alwaysTrueSignal.ID)
          {
            alreadyExists = true;
            existingIndex = signalIter_all;
          }
        }
        if(!alreadyExists)
        {
          signalSet->push_back(alwaysTrueSignal);

          // Now we add the new signal to the transition
          //  Event type should be high, we only care when expression is asserted
          newTrans.transitionSignal = alwaysTrueSignal;
          newTrans.onSignalEvent = High;
        }
        else
        {
          // We add the new signal to the transition
          //  Event type should be high, we only care when expression is asserted
          newTrans.transitionSignal = (*signalSet)[existingIndex];
          newTrans.onSignalEvent = High;
        }
      }
    }

    // If the current transition signal has no logical operators
    else
    {
      // Loop through signalSet to assign transition componentSignal
      for(int signalIter = 0; signalIter < signalSet->size(); signalIter++)
      {
        string componentSignal_withoutNotFlag = componentSignal.substr(1,componentSignal.length());

        // Signal name only - This is a transition activated when signal high
        // if((*signalSet)[signalIter].ID.find(componentSignal) != std::string::npos)
        if((*signalSet)[signalIter].ID == componentSignal)
        {
          newTrans.transitionSignal = (*signalSet)[signalIter];
          newTrans.onSignalEvent = High;
        }

        // Signal name with ! flag - This is a transition activated when signal low
        //  Also check for the componentSignal being empty after removing not flag - false
        //  match to this conditional if we do not.
        else if((*signalSet)[signalIter].ID == componentSignal_withoutNotFlag &&
                !componentSignal_withoutNotFlag.empty())
        {
          newTrans.transitionSignal = (*signalSet)[signalIter];
          newTrans.onSignalEvent = Low;
        }

        // Here we assume that the signal index is the identifier in the transition
        //  Thus we need to differentiate between the types
        else
        {
          // Signal index only - This is a transition activated when signal is high
          if(componentSignal == to_string((*signalSet)[signalIter].index))
          {
            newTrans.transitionSignal = (*signalSet)[signalIter];
            newTrans.onSignalEvent = High;
          }

          // Signal index with ! flag - This is a transition activated when signal low
          else if(componentSignal.substr(1,componentSignal.length()) == to_string((*signalSet)[signalIter].index))
          {
            newTrans.transitionSignal = (*signalSet)[signalIter];
            newTrans.onSignalEvent = Low;
          }
        }
      }
    }

    // Loop through stateSet to assign current and next state for transition
    for(int stateIter = 0; stateIter < stateSet->size(); stateIter++)
    {

      if(currentState == (*stateSet)[stateIter].ID)
      {
        // (*stateSet)[stateIter].enabledSignals.push_back(newTrans.transitionSignal);
        newTrans.currentState = (*stateSet)[stateIter];

        bool alreadyExists = false;

        // Ensure the enabled signal already exists or is added
        for(int signalIter = 0; signalIter < (*stateSet)[stateIter].enabledSignals.size(); signalIter++)
        {
          if(newTrans.transitionSignal.ID == (*stateSet)[stateIter].enabledSignals[signalIter].ID)
          {
            alreadyExists = true;
          }
        }

        // If the enabledSignal is not already added
        if(!alreadyExists)
        {
          // Modify the existing state to update the enabled signals
          (*stateSet)[stateIter].enabledSignals.push_back(newTrans.transitionSignal);
          // printSignalInfo(newTrans.transitionSignal);
        }
      }

      if(nextState == (*stateSet)[stateIter].ID)
      {
        newTrans.nextState = (*stateSet)[stateIter];
        // printStateInfo((*stateSet)[stateIter]);
      }
    }

    // Add the index
    newTrans.index = transIter;

    // Set the row and col to -1 signifying transition has not been placed
    //  in the transition table as of yet
    newTrans.row = -1;
    newTrans.col = -1;

    // printTransitionInfo(newTrans);

    // Add to the set of transitions
    transitionSet->push_back(newTrans);
  }

  // SERE ONLY
  if(automataType == SERE)
  {
    // NOTE
    // SERE Transitions
    //  Since the SERE rules provided are assumed to be restrictive, we use the
    //  SPOT produced automata as a one time switch, while the IA are intended to
    //  be continuously running machines. SERE automata will fall into an illegal
    //  state if the SERE rule is met. SPOT produces this basic automata. However,
    //  we need to add resets to this simple automata so that SERE rules can be specific
    //  to as many or as few of the cummulative set of signals as needed. The sink state
    //  also needs to be made illegal.


    // Illegal state
    //  The SERE automata products of the SPOT tool do not specify the final state that we
    //  are considering the illegal final state after the sequence of events specified occurs.
    //  If a state contains only a single, always true(see signal types), outgoing transition
    //  that routes to itself, then we consider this to be the sink and thus the illegal final state.

    // Loop through each state, add reset if needed, handle illegal states
    for(int stateIter_this = 0; stateIter_this < stateSet->size(); stateIter_this++)
    {
      // NOTE
      // I am not sure if this logic for illegal states will withold all possible
      //  SERE rules, but this seems to be the basic idea for the simple one's
      //  initially used.

      // Is this our illegal sink

      // Count num outgoing transitions
      int numOutgoingTransitions = 0;
      bool foundPossibleIllegalState = false;

      int illegalStateIndex = 0;

      // Loop through all transitions
      for(int transIter = 0; transIter < transitionSet->size(); transIter++)
      {
        // Outgoing transition
        if((*transitionSet)[transIter].currentState.ID == (*stateSet)[stateIter_this].ID)
        {
          // if(numOutgoingTransitions != 0)
          // {
          //   // Broad assumption that the illegal final state sink will only have
          //   //  a single always true signal transtion
          //   break;
          // }

          // Looping transition
          if((*transitionSet)[transIter].nextState.ID == (*stateSet)[stateIter_this].ID)
          {
            // Always true transition signal
            if((*transitionSet)[transIter].transitionSignal.type == alwaysTrue)
            {
              foundPossibleIllegalState = true;
              illegalStateIndex = stateIter_this;
            }
          }

          // Increment our outgoing trans count
          numOutgoingTransitions++;
        }
      }

      // Determine if this state is the illegal state
      if(numOutgoingTransitions == 1 && foundPossibleIllegalState)
      {
        // Change the illegal flag for the state
        (*stateSet)[illegalStateIndex].illegal = 1;
      }


    }


    // Resets
    //  Add a single transition to every state in the SERE automata, with the exception of
    //  the final illegal state. This transition accounts for the inverse of all other
    //  signals that are used for a transition from some state.
    //  Ex:
    //    a    b
    //  O--->O--->O
    // The above FSM example SERE rule requires a reset transition from the middle
    //  state to the initial state. Since the automata reflects the SERE operation
    //  of concatenation, the high signal b must follow after the signal assertion of a.
    // We need to reset the automata if anything other than the expected transitions happen.
    //  This prevents the rule from being violated over the course of multiple clock cycles
    //  and ahderes to the SERE operation that requires a followed by b in the NEXT clock cycle.

    // Find the initial state, all resets route here
    int initialStateIndex = 0;
    for(int stateIter = 0; stateIter < stateSet->size(); stateIter++)
    {
      if((*stateSet)[stateIter].initial)
      {
        initialStateIndex = stateIter;
      }
    }

    // Loop through each state
    for(int stateIter = 0; stateIter < stateSet->size(); stateIter++)
    {
      // If state is not illegal or initial, all other need resets
      if(!(*stateSet)[stateIter].illegal && !(*stateSet)[stateIter].initial)
      {
        // // Loop through transitions to create new signal
        // for(int transIter = 0; transIter < transitionSet->size(); transIter++)
        // {
        //   // If transition signal is composite we just go ahead and add a transition for it.
        //   //  Since the reset signals OR the existing transition signals, a separate transition
        //   //  gives this OR operation
        //   if((*transitionSet)[transIter].transitionSignal.type == composite)
        //   {
        //     // New trans
        //     transition subResetTrans;
        //     subResetTrans.index = transitionSet->size();
        //     subResetTrans.row = -1;
        //     subResetTrans.col = -1;
        //     subResetTrans.currentState = (*stateSet)[stateIter];
        //     subResetTrans.nextState = (*stateSet)[initialStateIndex]; // Reset to initial
        //
        //     // The signal to be added will be a composite but since we are trying
        //     //  to use the inverse, we make the signal event low
        //     subResetTrans.onSignalEvent = Low;
        //
        //     // Reference the existing
        //     subResetTrans.transitionSignal = (*transitionSet)[transIter].transitionSignal;
        //
        //     // Add new transition to the transition set
        //     transitionSet->push_back(subResetTrans);
        //   }
        // }


        // New transition for basic transition signals
        transition resetTrans;
        resetTrans.index = transitionSet->size();
        resetTrans.row = -1;
        resetTrans.col = -1;
        resetTrans.currentState = (*stateSet)[stateIter];
        resetTrans.nextState = (*stateSet)[initialStateIndex]; // Reset to initial

        // The signal to be added will be a composite but since we are trying
        //  to use the inverse, we make the signal event low
        resetTrans.onSignalEvent = Low;

        // The transition signal is the not of all signals of existing outgoing transitions
        //  that are 'or'ed together. Its the inverse of the defined transitions

        componentSignal newSignal;

        stringstream ID;
        // Loop through transitions to add new signal parts and get an ID
        for(int transIter = 0; transIter < transitionSet->size(); transIter++)
        {
          // If the transition is outgoing for this state
          if((*transitionSet)[transIter].currentState.ID == (*stateSet)[stateIter].ID)
          {
            // Add the signal to the new composite signal
            if((*transitionSet)[transIter].transitionSignal.type != composite)
            {
              newSignal.compositeSignalSet.push_back((*transitionSet)[transIter].transitionSignal);
              newSignal.compositeSignalEvents.push_back((*transitionSet)[transIter].onSignalEvent);

              // Add the logic operator to ID between signals
              if(newSignal.compositeSignalSet.size() > 1)
              {
                ID << "|";
              }

              ID << ((*transitionSet)[transIter].onSignalEvent == Low ? "!" : "" ) << (*transitionSet)[transIter].transitionSignal.index;
              // cout << "HERE" << endl;

            }
            else
            {
              for(int signalIter = 0; signalIter < (*transitionSet)[transIter].transitionSignal.compositeSignalSet.size(); signalIter++)
              {
                newSignal.compositeSignalSet.push_back((*transitionSet)[transIter].transitionSignal.compositeSignalSet[signalIter]);
                newSignal.compositeSignalEvents.push_back((*transitionSet)[transIter].transitionSignal.compositeSignalEvents[signalIter]);
              }


              // Add the logic operator to ID between signals
              if(newSignal.compositeSignalSet.size() > 1)
              {
                ID << "|";
              }

              ID << ((*transitionSet)[transIter].onSignalEvent == Low ? "!" : "" ) << "(" << (*transitionSet)[transIter].transitionSignal.ID << ")";
              // cout << "HERE" << endl;
            }
          }
        }
        // cout << "TRAN SET SIZE: " << transitionSet->size();
        // cout << "HERE" << endl;


        // Only add the new elements if there were signals added to the composite state set
        if(newSignal.compositeSignalSet.size() > 0)
        {
          // cout << "ID: " << ID.str() << endl;

          // If we only have 1 composite signal, its easier to merge signals
          if(newSignal.compositeSignalSet.size() == 1)
          {
            int existingIndex = 0;
            // Loop through the signalSet to find the existing signal
            for(int signalIter = 0; signalIter < signalSet->size(); signalIter++)
            {
              if(newSignal.compositeSignalSet[0].ID == (*signalSet)[signalIter].ID)
              {
                existingIndex = signalIter;
              }
            }

            // Give this signal to the transition
            resetTrans.transitionSignal = (*signalSet)[existingIndex];
          }
          else
          {
            newSignal.ID = ID.str();
            newSignal.index = signalSet->size();
            newSignal.type = composite;
            newSignal.logicOperator = OR;

            // Add new signal to the signalSet
            signalSet->push_back(newSignal);

            // Add the new signal as the transition signal for the rest transition
            resetTrans.transitionSignal = newSignal;
          }

          // Set the ID of the newSignal
          cout << newSignal.ID << endl;
          cout << newSignal.compositeSignalSet[0].ID << endl;
          // cout << "HERE" << endl;

          // Add new transition to the transition set
          transitionSet->push_back(resetTrans);
        }
      }
    }
  }
}



//*****************
//      SERE
//*****************

// Process the SERE specification
void processSEREConfiguration(vector<state_set> *stateSets,
                              vector<signal_set> *signalSets,
                              vector<transition_set> *transitionSets,
															configInfo config_info)
{
  // Containers for SERE specifications in config_info
  vector_string rules = config_info.rules;

  // Hold the filenames produced for easy access
  vector_string hoaRuleOutputFiles;
  vector_string dotRuleOutputFiles;

  // Process the retreived rules - output automatas for each
  processSERE(rules, &hoaRuleOutputFiles, &dotRuleOutputFiles);

  // For each rule output produced, set it as automata object
  //  so that we may have a unified function for composition of all
  //  rule automatas, IAs, and the final SERE, final IA compostion
  for(int ruleIter = 0; ruleIter < rules.size(); ruleIter++)
  {
    vector_string initState;
    vector_string states;
    vector_string signals;
    vector_string transitions;

    state_set stateSet;
    signal_set signalSet;
    transition_set transitionSet;

    // Parse the HOA files to pull information
    parseHOA(hoaRuleOutputFiles[ruleIter], &initState, &states, &signals, &transitions);

    // Populate stateSet
    processStates(initState, states, &stateSet);

    // Populate signalSet
    processSignals(signals, &signalSet);

    // Populate transitionSet
    processTransitions(transitions, &stateSet, &signalSet, &transitionSet, AutomataType::SERE);

    // // Create an automaton
    // automaton ruleAutomaton(stateSet, signalSet, transitionSet);

    // Add to containers
    stateSets->push_back(stateSet);
    signalSets->push_back(signalSet);
    transitionSets->push_back(transitionSet);
  }
}


// Processes the rules via SPOT and outputs an automata
//  in .dot and .hoa formats
void processSERE(vector_string rules,
                 vector_string *hoaRuleOutputFiles,
                 vector_string *dotRuleOutputFiles)
{
  // NOTE:
  //  SPOT may not be the optimal tool for taking SERE to LTL
  //  in order to represent the rules as automata
  //  If a replacement is needed, this would be the place it to happen

  // Loop through each rule and create outputs for each
  for(int ruleIter = 0; ruleIter < rules.size(); ruleIter++)
  {
    // Set output file for the current rule's automata representations
    char hoaOutputName[MAX_FILE_NAME];
    char hoaOutputIndex[MAX_SERE_RULES];

    strcpy(hoaOutputName, "outputs/sere/rule");
    sprintf(hoaOutputIndex, "%d.hoa", ruleIter);
    strcat(hoaOutputName, hoaOutputIndex);

    // Add to set of output files
    hoaRuleOutputFiles->push_back(hoaOutputName);

    char dotOutputName[MAX_FILE_NAME];
    char dotOutputIndex[MAX_SERE_RULES];

    strcpy(dotOutputName, "outputs/sere/rule");
    sprintf(dotOutputIndex, "%d.dot", ruleIter);
    strcat(dotOutputName, dotOutputIndex);

    // Add to set of output files
    dotRuleOutputFiles->push_back(dotOutputName);

    ofstream hoa_out(hoaOutputName);
    if (!hoa_out)
    {
      cerr << "\n Could not create the hoa output file for Rule" << ruleIter << endl;
      return;
    }

    ofstream dot_out(dotOutputName);
    if (!dot_out)
    {
      cerr << "\n Could not create the dot output file for Rule" << ruleIter << endl;
      return;
    }

    // Parse with SPOT
    spot::parsed_formula pf = spot::parse_infix_psl(rules[ruleIter]);
    if (pf.format_errors(std::cerr))
    {
      cerr << "\nRule" << ruleIter << " has formatting errors\n";
      exit(0);
    }

    // Translate
    spot::translator trans;
    spot::twa_graph_ptr aut = trans.run(pf.f);

    // Post process the automata
    spot::postprocessor post;
    post.set_type(spot::postprocessor::Monitor);
    post.set_pref(spot::postprocessor::Deterministic);
    post.set_pref(spot::postprocessor::Unambiguous);
    post.set_level(spot::postprocessor::High);

    auto newaut = post.run(aut);
    spot::print_hoa(hoa_out, newaut) << '\n';
    spot::print_dot(dot_out, newaut) << '\n';

    // Close the outputs files
    hoa_out.close();
    dot_out.close();
  }
}


// Parse HOA file
void parseHOA(string filename,
              vector_string *initialState,
              vector_string *states,
              vector_string *signals,
              vector_string *transitions)
{
  // Tokens
  string initalStateToken = "Start:";
  string APToken = "AP:";
  string bodyToken = "--BODY--";
  string endBodyToken = "--END--";
  string stateToken = "State:";

  // Open the HOA file
  ifstream hoaFile(filename);
  if(!hoaFile)
  {
    cerr << "\nUnable to open hoa file" << endl;
    exit(0);
  }

  // Used for associating states (as currentStates) with transtions
  bool readingTransitions = false; // Am I currently reading transitions or states?
  string currentState;

  // Read in the SERE expressions
  string line;
  while(getline(hoaFile, line))
  {
    // NOTE:
    //  string.find(token) == std::string::npos - TOKEN NOT FOUND
    //  string.find(token) != std::string::npos - TOKEN FOUND

    // Get all non empty lines
    if(!line.empty())
    {
      // First check for the end token
      if(line.find(endBodyToken) != std::string::npos)
      {
        break;
      }

      // Extract the initial state
      if(line.find(initalStateToken) != std::string::npos)
      {
        string initState = "s" + line.substr(initalStateToken.length()+1, line.find(" "));
        // cout << "Initial state: " << initState << endl;

        initialState->push_back(initState);
      }

      // Extract the atomic propositions (signal names)
      if(line.find(APToken) != std::string::npos)
      {
        // Grab the number of signals
        int numSignals = stoi(line.substr(APToken.length(), line.find(" ")));
        // cout << "Num signals: " << numSignals << endl;

        if(numSignals > 0)
        {
          // Remove the preceding information
          string names = line.substr(line.find_first_of(' "'), line.length());
          int i = 0;
          int found = names.find('"');
          while (found!=std::string::npos)
          {
            string name = names.substr(i, found-i);
            if(name != " " && !name.empty())
            {
              // cout << "Name: " << name << endl;
              signals->push_back(name);
            }

            // Iterate
            i = ++found;
            found = names.find('"', found);
          }
        }

        // If there are no signals in this automata, then we need to parse the actual rule
        //  to get the signal names
        else
        {
          // signals->push_back("rx");
          // signals->push_back("tx");
        }
      }

      // Extract transitions
      if(readingTransitions && line.find(stateToken) == std::string::npos)
      {
        // cout << "TRANS: " << line << endl;

        string formattedTransition;

        string transitionLogic = line.substr(line.find("[")+1, line.find("]")-1);
        string nextState = "s" + line.substr(line.find("]")+2, line.length());

        // cout << "  TransLogic: " << transitionLogic << endl;
        // cout << "  NextState: " << nextState << endl;

        // This simply formats the transition to the way the IA specifies them
        //  Makes it easier for reuse and overload of the processTransitions function
        formattedTransition = currentState + ":" + transitionLogic + ">" + nextState;
        // cout << "  Formatted transition: " << formattedTransition << endl;

        transitions->push_back(formattedTransition);
      }

      // Extract the states
      if(line.find(stateToken) != std::string::npos)
      {
        string stateName = "s" + line.substr(line.find(":")+2, line.length());
        // cout << "STATE: " << stateName << endl;
        states->push_back(stateName);

        currentState = stateName;

        // Set the readingTransitions flag
        readingTransitions = true;
      }
    }
  }
}


// Process the states from SERE configuration into state stucts
void processStates(vector_string initState,
                   vector_string states,
                   state_set *stateSet)
{
  // cout << "In processStates..." << "\n";

  // Loop through all states
  for(int stateIter = 0; stateIter < states.size(); stateIter++)
  {
    state newState;
    newState.index = stateIter; // Set index upon adding to container
    newState.ID = states[stateIter];
    newState.initial = 0;
    newState.accepting = 0;
    newState.illegal = 0; // Upon building state set, all will be deemed legal at this point

    // Is this the initial state?
    if(newState.ID == initState[0])
      newState.initial = 1;

    // NOTE:
    //  The commented out code below is how I grabbed accepting states
    //  with the IA configuration. Buchi automata, from SPOT, are
    //  funny with their accepting condition. I don't believe we require
    //  handling of accepting states now, thus I'm not specifying them for
    //  the SERE rules

    // Is this an accepting state?
    // for(int acceptingStateIter = 0; acceptingStateIter < acceptingStates.size(); acceptingStateIter++)
    // {
    //   if(newState.ID == acceptingStates[acceptingStateIter])
    //   {
    //     newState.accepting = 1;
    //   }
    // }

    // If state wasn't set as accepting, set the parameter
    if(newState.accepting != 1)
      newState.accepting = 0;

    // printStateInfo(newState);

    // Add to the set of states
    stateSet->push_back(newState);
  }

}


// Process the signals from SERE configuration into componentSignal structs
void processSignals(vector_string signals,
                    signal_set *signalSet)
{
  // cout << "In processSignals..." << "\n";

  // Loop through all input signals
  for(int signalIter = 0; signalIter < signals.size(); signalIter++)
  {
    componentSignal newSignal;
    newSignal.index = signalIter; // Set index upon adding to container
    newSignal.ID = signals[signalIter];

    // TODO: These being unset needs to be addressed
    newSignal.type = signalType::unset;

    // printSignalInfo(newSignal);

    // Add to the set of signals
    signalSet->push_back(newSignal);
  }
}



//*****************
//      MISC
//*****************

// Removes the optional comments from the configuration elements
string removeComment(string element)
{
  string baseElement = element;

  string commentToken = "//";
  if(baseElement.find(commentToken) != std::string::npos)
  {
    baseElement = element.substr(0, baseElement.find(commentToken));
  }

  // Removes extra whitespace from the remaining element
  baseElement.erase(std::remove(baseElement.begin(),baseElement.end(),' '),baseElement.end());

  return baseElement;
}

// Extracts an element from a line, stopping at any of the delimiters, removing whitespace
string extractUntil(string element, string delimiters)
{
	size_t start = element.find_first_not_of("\t");		// skip unwanted tabs
	size_t end = element.find_first_of(delimiters);		// stop at the first delimiter found

	string extract = element.substr(start, end - start);

	// removes extra whitespace from extracted element
	extract.erase(remove(extract.begin(), extract.end(), ' '), extract.end());

	return extract;
}


// Print function for state objects
void printStateInfo(state state)
{
  cout << "State: " << state.ID << "\n";
  // cout << "  numID: " << state.numID << "\n";
  cout << "  index in stateSet: " << state.index << "\n";

  if(state.initial == 0) cout << "  initial: no\n";
  else cout << "  initial: yes\n";

  if(state.accepting == 0) cout << "  accepting: no\n";
  else cout << "  accepting: yes\n";

  if(state.illegal == 0) cout << "  illegal: no\n";
  else cout << "  illegal: yes\n";

  cout << "  Enabled signals:\n    ";
  for(int i = 0; i < state.enabledSignals.size(); i++)
  {
    cout << state.enabledSignals[i].ID << ", ";
  }
  cout << endl;

  cout << endl;
}


// Print function for componentSignal objects
void printSignalInfo(componentSignal signal)
{
  cout << "Signal: " << signal.ID << endl;
  // cout << "  numID: " << signal.numID << endl;
  cout << "  index in signalSet: " << signal.index << endl;

  if(signal.type == signalType::input) cout << "  type: input\n";
  else if(signal.type == signalType::output) cout << "  type: output\n";
  else if(signal.type == signalType::internal) cout << "  type: internal\n";
  else if(signal.type == signalType::composite) cout << "  type: composite\n";
  else if(signal.type == signalType::unset) cout << "  type: unset\n";
  else if(signal.type == signalType::alwaysTrue) cout << " type: always true\n";

  if(signal.type == composite)
  {
    cout << "  composite signals:\n   ";
    for(int signalIter = 0; signalIter < signal.compositeSignalSet.size(); signalIter++)
    {
      cout << signal.compositeSignalSet[signalIter].ID << ", " << endl;
    }
  }
  cout << endl;
}


// Print function for transition objects
void printTransitionInfo(transition transition)
{
  cout << "Trans: " << transition.index << endl;
  cout << "  currentState: " << transition.currentState.ID << endl;
  cout << "  currentState index: " << transition.currentState.index << endl;
  cout << "  nextState: " << transition.nextState.ID << endl;
  cout << "  nextState index: " << transition.nextState.index << endl;
  cout << "  componentSignal: " << endl;
  printSignalInfo(transition.transitionSignal);

  if(transition.onSignalEvent == 0) cout << "  onSignalEvent: LOW\n";
  else cout << "  onSignalEvent: HIGH\n";

  cout << "  row: " << transition.row << endl;
  cout << "  col: " << transition.col << endl;
  cout << endl;
}
