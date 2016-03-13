#include <iostream>
#include <fstream>
#include <string>
#include "GrammarManager.h"

using namespace std;

class Rule {
public:
    vector<string> redexes;
    vector<string> reducts;
    vector<string> conditions;
    vector<string> assignments;

public:
    Rule() {}
};

/*
class MyTransition {
public:
    Parse_Transition* transition;
    string moduleName;

public:
    MyTransition() {
        transition = NULL;
        moduleName = "";
    }

    MyTransition(Parse_Transition* trans, string module) {
        transition = trans;
        moduleName = module;
    }
};
*/

string strToLower (string str) {
    string result = "";
    for (int i = 0; i < str.size(); i++)
        result.push_back(tolower(str[i]));
    return result;
}

string expToMaudeExp (string mem, Parse_Expression *exp) {
    switch (exp->m_op) {
    case E_NOT :
        return "(not " + expToMaudeExp(mem, exp->m_operand1) + ")";
    case E_MULTIPLE :
        return "(" + expToMaudeExp(mem, exp->m_operand1) + " * " + expToMaudeExp(mem, exp->m_operand2) + ")";
    case E_DIVIDE :
    case E_UDIVIDE :
        return "(" + expToMaudeExp(mem, exp->m_operand1) + " quo " + expToMaudeExp(mem, exp->m_operand2) + ")";
    case E_ADD :
        return "(" + expToMaudeExp(mem, exp->m_operand1) + " + " + expToMaudeExp(mem, exp->m_operand2) + ")";
    case E_MINUS :
        return "(" + expToMaudeExp(mem, exp->m_operand1) + " - " + expToMaudeExp(mem, exp->m_operand2) + ")";
    case E_SLT :
    case E_ULT :
        return "(" + expToMaudeExp(mem, exp->m_operand1) + " < " + expToMaudeExp(mem, exp->m_operand2) + ")";
    case E_SLE :
    case E_ULE :
        return "(" + expToMaudeExp(mem, exp->m_operand1) + " <= " + expToMaudeExp(mem, exp->m_operand2) + ")";
    case E_SGT :
    case E_UGT :
        return "(" + expToMaudeExp(mem, exp->m_operand1) + " > " + expToMaudeExp(mem, exp->m_operand2) + ")";
    case E_SGE :
    case E_UGE :
        return "(" + expToMaudeExp(mem, exp->m_operand1) + " >= " + expToMaudeExp(mem, exp->m_operand2) + ")";
    case E_CE :
        return "(" + expToMaudeExp(mem, exp->m_operand1) + " == " + expToMaudeExp(mem, exp->m_operand2) + ")";
    case E_NE :
        return "(" + expToMaudeExp(mem, exp->m_operand1) + " =/= " + expToMaudeExp(mem, exp->m_operand2) + ")";
    case E_LAND :
        return "(" + expToMaudeExp(mem, exp->m_operand1) + " & " + expToMaudeExp(mem, exp->m_operand2) + ")";
    case E_LXOR :
        return "(" + expToMaudeExp(mem, exp->m_operand1) + " xor " + expToMaudeExp(mem, exp->m_operand2) + ")";
    case E_LOR :
        return "(" + expToMaudeExp(mem, exp->m_operand1) + " | " + expToMaudeExp(mem, exp->m_operand2) + ")";
    case E_AND :
        return "(" + expToMaudeExp(mem, exp->m_operand1) + " and " + expToMaudeExp(mem, exp->m_operand2) + ")";
    case E_OR :
        return "(" + expToMaudeExp(mem, exp->m_operand1) + " or " + expToMaudeExp(mem, exp->m_operand2) + ")";
    case E_CONSTANT :
        return strToLower(exp->m_constant->m_node);
    case E_VAR :
        return "(" + mem + "[" + exp->m_varId->m_node + "])";
    case E_VARREF :
        return "(" + exp->m_varRef->m_moduleName->m_node + "-M[" + exp->m_varRef->m_varName->m_node + "])";
    default :
        return "";
    }
    return NULL;
}

// extract rules of type _Rule_, recursively, from all the transitions with a same label
//   transitions: maps module names to the transitions in this module
//   start: the current point in the _transitions_, indicating the ones before _start_ are not cared
vector<Rule> extractRules(map<string, vector<Parse_Transition*>>* transitions,
        map<string, vector<Parse_Transition*>>::iterator start) {
    vector<Rule> rules;
    if (start == transitions->end()) {
        // added an empty rule as a fixed point
        Rule rule;
        rules.push_back(rule);
    } else { // recursive step
        string moduleName = start->first;
        int i, j, k;
        Parse_Transition* transition;
        Parse_Action* action;
        map<string, vector<Parse_Transition*>>::iterator current = start;

        // recursion
        start++;
        vector<Rule> recResult = extractRules(transitions, start);
	
        for (i = 0; i < current->second.size(); i++) {
            transition = current->second[i];
            for (j = 0; j < recResult.size(); j++) {
                Rule rule = recResult[j];

                rule.redexes.insert(rule.redexes.begin(),
                        "< " + moduleName + " : Module | loc : " + transition->m_location1Id->m_node
                        + ", mem : " + moduleName + "-M >");

                rule.reducts.insert(rule.reducts.begin(),
                        "< " + moduleName + " : Module | loc : " + transition->m_location2Id->m_node
                        + ", mem : " + moduleName + "-M' >");

                if (transition->m_guard != NULL)
                    rule.conditions.insert(rule.conditions.begin(),
                            expToMaudeExp(moduleName + "-M", transition->m_guard));
                else
                    rule.conditions.insert(rule.conditions.begin(), "true");

                string modifiedMem = moduleName + "-M";
                for (k = 0; k < transition->m_actionBlock.size(); k++) {
                    action = transition->m_actionBlock[k];
                    modifiedMem += "[" + action->m_varId->m_node + " := "
                            + expToMaudeExp(moduleName + "-M", action->m_value) + "]";
                }
                rule.assignments.insert(rule.assignments.begin(), moduleName + "-M' := " + modifiedMem);

                // add to the returned vector of rules
                rules.push_back(rule);
            }
        }
    }

    return rules;
}

int main() {
    GrammarManager *manager = new GrammarManager();
    manager -> ProcessFile("example.elts");
    string result = manager -> GetGrammarModel() -> getString();

    Grammar_Model *model = manager -> GetGrammarModel();
    int moduleNum = (model -> m_modules).size();
    cout << "Module Number: " << moduleNum << endl;

    vector<string> moduleDeclarations;
    vector<string> moduleIDs;
    vector<string> maudeVarDeclarations;
    vector<string> variableDeclarations;
    map<string, string> memoryInitialization;
    vector<string> locationDeclarations;
    string initialLocation;
    vector<string> initialStateDeclaration;
    vector<string> ruleDeclarations;

    map<string, map<string, vector<Parse_Transition*>>> allTransitions;


    int i = 0;
    Parse_Module *module;
    for (i = 0; i < moduleNum; i++) {
        module = (model->m_modules)[i];

        /*
        ostringstream oss;
        module->Print(oss, 0);
        cout << oss.str() << endl; */

        // declare ops for module names
        string moduleName = module->m_moduleName->m_node;
        moduleDeclarations.push_back("op " + moduleName + " : -> Oid .");
        moduleIDs.push_back(moduleName);
        maudeVarDeclarations.push_back("vars " + moduleName + "-M " + moduleName + "-M' : Memory .");

        // check variables definitions
        int j;
        for (j = 0; j < (module->m_varNames).size(); j++) {
            Parse_Var *var = (module->m_varNames)[j];
            variableDeclarations.push_back("op " + var->m_varName->m_node + " : -> Variable .");

            if(var->m_value != NULL) {
                memoryInitialization[var->m_varName->m_node] = strToLower(var->m_value->to_string());
            }
        }

        // check labels definitions
        // omitted for the moment

        // check locations definitions
        for (j = 0; j < module->m_locationNames.size(); j++) {
            Parse_StringNode *loc = (module->m_locationNames)[j];
            locationDeclarations.push_back("op " + loc->m_node + " : -> Location .");
        }

        // check initial location
        initialLocation = module->m_initLocationId->m_node;

        // check initial actions
        Parse_Action *action;
        for (j = 0; j < module->m_initialActionBlock.size(); j++) {
            action = module->m_initialActionBlock[j];
            memoryInitialization[action->m_varId->m_node] = strToLower(action->m_value->to_string());
        }

        // now we can define the initial state for this module in Maude
        initialStateDeclaration.push_back("op init-" + moduleName + " : -> Object .");
        map<string, string>::iterator iter;
        string initialMemory = "empty";
        for (iter = memoryInitialization.begin(); iter != memoryInitialization.end(); iter++) {
            initialMemory += ", ( " + iter->first + " -> " + iter->second + " )";
        }
        initialStateDeclaration.push_back("eq init-" + moduleName + " = < " + moduleName
                + " : Module | loc : " + initialLocation + ", mem : ( " + initialMemory + " )> .");

        // check all transitions
        Parse_Transition *transition;
        for (j = 0; j < module->m_transitions.size(); j++) {
            transition = module->m_transitions[j];

            // if (transition->m_guard == NULL)
            //    ruleDeclarations.push_back("rl :");
            //else

            /*
            ruleDeclarations.push_back("crl ");

            ruleDeclarations.push_back("  < " + moduleName + " : Module | loc : " + transition->m_location1Id->m_node
                    + ", mem : " + moduleName + "-M >");
            ruleDeclarations.push_back("=> < " + moduleName + " : Module | loc : " + transition->m_location2Id->m_node
                    + ", mem : " + moduleName + "-M' >");

            string modifiedMem = moduleName + "-M";
            for (int k = 0; k < transition->m_actionBlock.size(); k++) {
                action = transition->m_actionBlock[k];
                modifiedMem += "[" + action->m_varId->m_node + " := " + expToMaudeExp(moduleName + "-M", action->m_value) + "]";
            }

            if (transition->m_guard != NULL) {
                ruleDeclarations.push_back("  if " + expToMaudeExp(moduleName + "-M", transition->m_guard));
                ruleDeclarations.push_back("     /\\ " + moduleName + "-M' := " + modifiedMem + " .");
            } else {
                ruleDeclarations.push_back("  if " + moduleName + "-M' := " + modifiedMem + " .");
            }

            ruleDeclarations.push_back("");
            */

            // new things
            for (int k = 0; k < transition->m_labelIds.size(); k++) {
                allTransitions[transition->m_labelIds[k]->m_node][moduleName].push_back(transition);
            }
        }
    }

    int j = 0, k;

    string initStateVal = "none";
    for (j = 0; j < moduleIDs.size(); j++)
        initStateVal += " init-" + moduleIDs[j];
    initialStateDeclaration.push_back("op init-state : -> Object .");
    initialStateDeclaration.push_back("eq init-state = " + initStateVal + " .");

    // computing all rules
    map<string, map<string, vector<Parse_Transition*>>>::iterator iteri;
    map<string, vector<Parse_Transition*>>::iterator iterj;
    int iterk;
    for (iteri = allTransitions.begin(); iteri != allTransitions.end(); iteri++) {
        vector<Rule> rules = extractRules(&(iteri->second), iteri->second.begin());
        for (j = 0; j < rules.size(); j++) {
            ruleDeclarations.push_back("crl [" + iteri->first + "-" + to_string(j) + "] :");
            for (k = 0; k < rules[j].redexes.size(); k++) {
                ruleDeclarations.push_back("  " + rules[j].redexes[k]);
            }
            for (k = 0; k < rules[j].reducts.size(); k++) {
                if (k == 0)
                    ruleDeclarations.push_back("=> " + rules[j].reducts[k]);
                else
                    ruleDeclarations.push_back("   " + rules[j].reducts[k]);
            }
            for (k = 0; k < rules[j].conditions.size(); k++) {
                if (k == 0)
                    ruleDeclarations.push_back("if " + rules[j].conditions[k]);
                else
                    ruleDeclarations.push_back("   /\\ " + rules[j].conditions[k]);
            }
            for (k = 0; k < rules[j].assignments.size(); k++) {
                if (k != rules[j].assignments.size()-1)
                    ruleDeclarations.push_back("   /\\ " + rules[j].assignments[k]);
                else
                    ruleDeclarations.push_back("   /\\ " + rules[j].assignments[k] + " .");
            }
            ruleDeclarations.push_back("");
        }


        /*
        for (iterj = iteri->second.begin(); iterj != iteri->second.end(); iterj++) {
            for (iterk = 0; iterk < iterj->second.size(); iterk++) {
                cout << iteri->first << " : " << iterj->first << " : " << iterj->second[iterk]->m_location1Id->m_node
                        << " to " << iterj->second[iterk]->m_location2Id->m_node << endl;
            }
        }*/
    }


    for (j = 0; j < moduleDeclarations.size(); j++)
        cout << moduleDeclarations[j] << endl;
    cout << endl;
    for (j = 0; j < variableDeclarations.size(); j++)
        cout << variableDeclarations[j] << endl;
    cout << endl;
    for (j = 0; j < locationDeclarations.size(); j++)
            cout << locationDeclarations[j] << endl;
    cout << endl;
    for (j = 0; j < initialStateDeclaration.size(); j++)
            cout << initialStateDeclaration[j] << endl;
    cout << endl;
    for (j = 0; j < maudeVarDeclarations.size(); j++)
            cout << maudeVarDeclarations[j] << endl;
    cout << endl;
    for (j = 0; j < ruleDeclarations.size(); j++)
            cout << ruleDeclarations[j] << endl;
    cout << "Init Location: " << initialLocation << endl;

    map<string, string>::iterator iter;
    cout << "Memory Begin: " << endl;
    for (iter = memoryInitialization.begin(); iter != memoryInitialization.end(); iter++) {
        cout << "( " << iter->first << " -> " << iter->second << " )" << endl;
    }
    cout << "Memory End" << endl;

    cout << result << endl;
    cout << "done" << endl;


    /*
    map<string, map<string, vector<Parse_Transition*>>>::iterator iteri;
    map<string, vector<Parse_Transition*>>::iterator iterj;
    int iterk;
    for (iteri = allTransitions.begin(); iteri != allTransitions.end(); iteri++) {
        for (iterj = iteri->second.begin(); iterj != iteri->second.end(); iterj++) {
            for (iterk = 0; iterk < iterj->second.size(); iterk++) {
                cout << iteri->first << " : " << iterj->first << " : " << iterj->second[iterk]->m_location1Id->m_node
                        << " to " << iterj->second[iterk]->m_location2Id->m_node << endl;
            }
        }
    }*/


    return 0;

}


