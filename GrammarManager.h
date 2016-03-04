//
// Created by Dexi Wang
// sv-team, Tsinghua Unviersity
// 20150619
//

#include "GrammarModel.h"

class GrammarManager {
private:
	Grammar_Model *m_system;
public:
	// DW: constructor
	GrammarManager() {}

	// DW: destructor
	virtual ~GrammarManager() {
		//delete m_system;
	}

	// DW: read model from ELTS file
	void ProcessFile(const char *fileName);

	// DW: get model
	// DW: the first const means the pointed model returned should not be modified
	// DW: the second const means this member function will not (cause) modify of member vars
	// DW: ref http://stackoverflow.com/questions/8788863/can-const-member-function-return-a-non-const-pointer-to-a-data-member
	Grammar_Model *GetGrammarModel();
};
