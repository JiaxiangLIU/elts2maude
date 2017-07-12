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
    case E_LAND :   // bit operation, "&" in C-like language
        return "(" + expToMaudeExp(mem, exp->m_operand1) + " & " + expToMaudeExp(mem, exp->m_operand2) + ")";
    case E_LXOR :   // bit operation, "^" in C-like language
        return "(" + expToMaudeExp(mem, exp->m_operand1) + " xor " + expToMaudeExp(mem, exp->m_operand2) + ")";
    case E_LOR :    // bit operation, "|" in C-like language
        return "(" + expToMaudeExp(mem, exp->m_operand1) + " | " + expToMaudeExp(mem, exp->m_operand2) + ")";
    case E_AND :    // "&&" in C-like language
        return "(" + expToMaudeExp(mem, exp->m_operand1) + " and " + expToMaudeExp(mem, exp->m_operand2) + ")";
    case E_OR :     // "||" in C-like language
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
//   start: the current point in the _transitions_, indicating the ones after _start_ are not cared
// NOTE that we use reverse_iterator to traverse the _transitions_ from the back end.
vector<Rule> extractRules(map<string, vector<Parse_Transition*>>* transitions,
        map<string, vector<Parse_Transition*>>::reverse_iterator start) {
    vector<Rule> rules;
    if (start == transitions->rend()) {
        // added an empty rule as a fixed point
        Rule rule;
        rules.push_back(rule);
    } else { // recursive step
        string moduleName = start->first;
        int i, j, k;
        Parse_Transition* transition;
        Parse_Action* action;
        map<string, vector<Parse_Transition*>>::reverse_iterator current = start;

        // recursion
        start++;
        vector<Rule> recResult = extractRules(transitions, start);
	
        for (i = 0; i < current->second.size(); i++) {
            transition = current->second[i];
            for (j = 0; j < recResult.size(); j++) {
                Rule rule = recResult[j];

                rule.redexes.push_back("< " + moduleName + " : Module | loc : "
                        + transition->m_location1Id->m_node + ", mem : " + moduleName + "-M >");

                rule.reducts.push_back("< " + moduleName + " : Module | loc : "
                        + transition->m_location2Id->m_node + ", mem : " + moduleName + "-M' >");

                if (transition->m_guard != NULL)
                    rule.conditions.push_back(expToMaudeExp(moduleName + "-M", transition->m_guard));
                else
                    rule.conditions.push_back("true");

                string modifiedMem = moduleName + "-M";
                for (k = 0; k < transition->m_actionBlock.size(); k++) {
                    action = transition->m_actionBlock[k];
                    // the use of modifiedMem instead of moduleName-M in _expToMaudeExp_
                    // indicates that the actions in an action block are executed sequentially.
                    modifiedMem += "[" + action->m_varId->m_node + " := "
                            + expToMaudeExp(modifiedMem, action->m_value) + "]";
                }
                rule.assignments.push_back(moduleName + "-M' := " + modifiedMem);

                // add to the returned vector of rules
                rules.push_back(rule);
            }
        }
    }

    return rules;
}

int main() {
    GrammarManager* manager = new GrammarManager();
    manager->ProcessFile("example.elts");
    string result = manager->GetGrammarModel()->getString();

    Grammar_Model* model = manager->GetGrammarModel();

    vector<string> moduleNames;
    vector<string> moduleDeclarations;
    vector<string> maudeVarDeclarations;
    vector<string> variableDeclarations;
    map<string, string> memoryInitialization;
    vector<string> locationDeclarations;
    vector<string> initialStateDeclaration;
    vector<string> ruleDeclarations;
    map<string, map<string, vector<Parse_Transition*>>> allTransitions;

    int i, j, k;
    Parse_Module* module;
    string moduleName;
    Parse_Var* var;
    Parse_StringNode* loc;
    string initialLocation;
    Parse_Action* action;
    map<string, string>::iterator iteri;
    Parse_Transition* transition;

    for (i = 0; i < model->m_modules.size(); i++) {
        module = (model->m_modules)[i];

        // declare ops for module names
        moduleName = module->m_moduleName->m_node;
        moduleNames.push_back(moduleName);
        moduleDeclarations.push_back("op " + moduleName + " : -> Oid .");

        // declare maude-variables for each module-memory
        maudeVarDeclarations.push_back("vars " + moduleName + "-M " + moduleName + "-M' : Memory .");

        // check variables definitions
        for (j = 0; j < module->m_varNames.size(); j++) {
            var = module->m_varNames[j];
            variableDeclarations.push_back("op " + var->m_varName->m_node + " : -> Variable .");

            if(var->m_value != NULL) {
                memoryInitialization[var->m_varName->m_node] = strToLower(var->m_value->to_string());
            }
        }

        // check locations definitions
        for (j = 0; j < module->m_locationNames.size(); j++) {
            loc = module->m_locationNames[j];
            locationDeclarations.push_back("op " + loc->m_node + " : -> Location .");
        }

        // check initial location
        initialLocation = module->m_initLocationId->m_node;

        // check initial actions
        for (j = 0; j < module->m_initialActionBlock.size(); j++) {
            action = module->m_initialActionBlock[j];
            memoryInitialization[action->m_varId->m_node] = strToLower(action->m_value->to_string());
        }

        // now we can define the initial state for this module in Maude
        initialStateDeclaration.push_back("op init-" + moduleName + " : -> Object .");
        string initialMemory = "empty";
        for (iteri = memoryInitialization.begin(); iteri != memoryInitialization.end(); iteri++) {
            initialMemory += ", ( " + iteri->first + " -> " + iteri->second + " )";
        }
        memoryInitialization.clear();
        initialStateDeclaration.push_back("eq init-" + moduleName + " = < " + moduleName
                + " : Module | loc : " + initialLocation + ", mem : ( " + initialMemory + " )> .");

        // check all transitions
        for (j = 0; j < module->m_transitions.size(); j++) {
            transition = module->m_transitions[j];
            for (k = 0; k < transition->m_labelIds.size(); k++) {
                allTransitions[transition->m_labelIds[k]->m_node][moduleName].push_back(transition);
            }
        }
    }

    // compute the initial state of the whole system
    string initStateVal = "none";
    for (j = 0; j < moduleNames.size(); j++)
        initStateVal += " init-" + moduleNames[j];
    initialStateDeclaration.push_back("op init-state : -> Object .");
    initialStateDeclaration.push_back("eq init-state = " + initStateVal + " .");

    // computing all rules
    map<string, map<string, vector<Parse_Transition*>>>::iterator iterj;
    for (iterj = allTransitions.begin(); iterj != allTransitions.end(); iterj++) {
        vector<Rule> rules = extractRules(&(iterj->second), iterj->second.rbegin());
        for (j = 0; j < rules.size(); j++) {
            ruleDeclarations.push_back("crl [" + iterj->first + "-" + to_string(j) + "] :");
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
                if (k != rules[j].assignments.size() - 1)
                    ruleDeclarations.push_back("   /\\ " + rules[j].assignments[k]);
                else
                    ruleDeclarations.push_back("   /\\ " + rules[j].assignments[k] + " .");
            }
            ruleDeclarations.push_back("");
        }
    }


    // Generate the maude model to a maude file
    ofstream out("result.maude");
    if (out.is_open()) {
        out << "(omod SYSTEM is protecting MODULE . " << endl << endl;

        for (j = 0; j < moduleDeclarations.size(); j++)
            out << "  " << moduleDeclarations[j] << endl;
        out << endl;

        for (j = 0; j < variableDeclarations.size(); j++)
            out << "  " << variableDeclarations[j] << endl;
        out << endl;

        for (j = 0; j < locationDeclarations.size(); j++)
            out << "  " << locationDeclarations[j] << endl;
        out << endl;

        for (j = 0; j < initialStateDeclaration.size(); j++)
            out << "  " << initialStateDeclaration[j] << endl;
        out << endl;

        for (j = 0; j < maudeVarDeclarations.size(); j++)
            out << "  " << maudeVarDeclarations[j] << endl;
        out << endl;

        for (j = 0; j < ruleDeclarations.size(); j++)
            out << "  " << ruleDeclarations[j] << endl;

        out << "endom)" << endl;

        out.close();
    }

    cout << endl << result << endl;
    cout << "Done" << endl;

    return 0;
}


