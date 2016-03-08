#include <iostream>
#include <fstream>
#include "GrammarManager.h"

using namespace std;

string expToMaudeExp (Parse_Expression *exp) {
    switch (exp->m_op) {
    case E_ADD :
        return "(" + expToMaudeExp(exp->m_operand1) + " + " + expToMaudeExp(exp->m_operand2) + ")";
    case E_MINUS :
        return "(" + expToMaudeExp(exp->m_operand1) + " - " + expToMaudeExp(exp->m_operand2) + ")";
    case E_SLT :
    case E_ULT :
        return "(" + expToMaudeExp(exp->m_operand1) + " < " + expToMaudeExp(exp->m_operand2) + ")";
    case E_SLE :
    case E_ULE :
        return "(" + expToMaudeExp(exp->m_operand1) + " <= " + expToMaudeExp(exp->m_operand2) + ")";
    case E_SGT :
    case E_UGT :
        return "(" + expToMaudeExp(exp->m_operand1) + " > " + expToMaudeExp(exp->m_operand2) + ")";
    case E_SGE :
    case E_UGE :
        return "(" + expToMaudeExp(exp->m_operand1) + " >= " + expToMaudeExp(exp->m_operand2) + ")";
    case E_CONSTANT :
        return exp->m_constant->m_node;
    case E_VAR :
        return "(M[" + exp->m_varId->m_node + "])";
    case E_LOR :
        return "(" + expToMaudeExp(exp->m_operand1) + " | " + expToMaudeExp(exp->m_operand2) + ")";
    default :
        return "";
    }
    return NULL;
}


int main() {
    GrammarManager *manager = new GrammarManager();
    manager -> ProcessFile("example.elts");
    string result = manager -> GetGrammarModel() -> getString();

    Grammar_Model *model = manager -> GetGrammarModel();
    int moduleNum = (model -> m_modules).size();
    cout << "Module Number: " << moduleNum << endl;

    vector<string> moduleDeclarations;
    vector<string> variableDeclarations;
    map<string, string> memoryInitialization;
    vector<string> locationDeclarations;
    string initialLocation;
    vector<string> initialStateDeclaration;
    vector<string> ruleDeclarations;


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

        // check variables definitions
        int j;
        for (j = 0; j < (module->m_varNames).size(); j++) {
            Parse_Var *var = (module->m_varNames)[j];
            variableDeclarations.push_back("op " + var->m_varName->m_node + " : -> Variable .");

            if(var->m_value != NULL) {
                memoryInitialization[var->m_varName->m_node] = var->m_value->to_string();
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
            memoryInitialization[action->m_varId->m_node] = action->m_value->to_string();
        }

        // now we can define the initial state for this module in Maude
        initialStateDeclaration.push_back("op init-state : -> Object .");
        map<string, string>::iterator iter;
        string initialMemory = "empty";
        for (iter = memoryInitialization.begin(); iter != memoryInitialization.end(); iter++) {
            initialMemory += ", ( " + iter->first + " -> " + iter->second + " )";
        }
        initialStateDeclaration.push_back("eq init-state = < " + moduleName
                + " : Module | loc : " + initialLocation + ", mem : ( " + initialMemory + " )> .");

        // check all transitions
        Parse_Transition *transition;
        for (j = 0; j < module->m_transitions.size(); j++) {
            transition = module->m_transitions[j];

            // if (transition->m_guard == NULL)
            //    ruleDeclarations.push_back("rl :");
            //else
            ruleDeclarations.push_back("crl ");

            ruleDeclarations.push_back("  < " + moduleName + " : Module | loc : " + transition->m_location1Id->m_node
                    + ", mem : M >");
            ruleDeclarations.push_back("=> < " + moduleName + " : Module | loc : " + transition->m_location2Id->m_node
                    + ", mem : M' >");

            string modifiedMem = "M";
            for (int k = 0; k < transition->m_actionBlock.size(); k++) {
                action = transition->m_actionBlock[k];
                modifiedMem += "[" + action->m_varId->m_node + " := " + expToMaudeExp(action->m_value) + "]";
            }

            if (transition->m_guard != NULL) {
                ruleDeclarations.push_back("  if " + expToMaudeExp(transition->m_guard));
                ruleDeclarations.push_back("     /\\ M' := " + modifiedMem + " .");
            } else {
                ruleDeclarations.push_back("  if M' := " + modifiedMem + " .");
            }

        }
    }

    int j = 0;
    for (j = 0; j < moduleDeclarations.size(); j++)
        cout << moduleDeclarations[j] << endl;
    for (j = 0; j < variableDeclarations.size(); j++)
        cout << variableDeclarations[j] << endl;
    for (j = 0; j < locationDeclarations.size(); j++)
            cout << locationDeclarations[j] << endl;
    for (j = 0; j < initialStateDeclaration.size(); j++)
            cout << initialStateDeclaration[j] << endl;
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
    cout << "done 2nd" << endl;


    return 0;

}


