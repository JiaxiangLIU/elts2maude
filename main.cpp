#include <iostream>
#include <fstream>
#include "GrammarManager.h"

using namespace std;

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

    int i = 0;
    Parse_Module *module;
    for(i = 0; i < moduleNum; i++) {
        module = (model -> m_modules)[i];

        /*
        ostringstream oss;
        module->Print(oss, 0);
        cout << oss.str() << endl; */

        // declare ops for module names
        string moduleName = module -> m_moduleName -> m_node;
        moduleDeclarations.push_back("op " + moduleName + " : -> Oid .");

        // check variables definitions
        int j;
        for(j = 0; j < (module -> m_varNames).size(); j++) {
            Parse_Var *var = (module->m_varNames)[j];
            variableDeclarations.push_back("op " + var -> m_varName -> m_node + " : -> Variable .");

            if(var -> m_value != NULL) {
                memoryInitialization[var -> m_varName -> m_node] = var -> m_value -> to_string();
            }
        }

        // check labels definitions
        // omitted for the moment

        // check locations definitions
        for(j = 0; j < module->m_locationNames.size(); j++) {
            Parse_StringNode *loc = (module->m_locationNames)[j];
            locationDeclarations.push_back("op " + loc->m_node + " : -> Location .");
        }

        // check intial location
        initialLocation = module->m_initLocationId->m_node;
    }

    int j = 0;
    for(j = 0; j < moduleDeclarations.size(); j++)
        cout << moduleDeclarations[j] << endl;
    for(j = 0; j < variableDeclarations.size(); j++)
        cout << variableDeclarations[j] << endl;
    for(j = 0; j < locationDeclarations.size(); j++)
        cout << locationDeclarations[j] << endl;
    cout << "Init Location: " << initialLocation << endl;

    map<string, string>::iterator iter;
    cout << "Memory Begin: " << endl;
    for(iter = memoryInitialization.begin(); iter != memoryInitialization.end(); iter++) {
        cout << "( " << iter->first << " -> " << iter->second << " )" << endl;
    }
    cout << "Memory End" << endl;

    cout << result << endl;
    cout << "done 2nd" << endl;


    return 0;
}
