#ifndef DM_CREATECOMPONENT_H
#define DM_CREATECOMPONENT_H

#include <dmmodule.h>
#include <dm.h>

class DM_HELPER_DLL_EXPORT DM_CreateComponent: public DM::Module
{
	DM_DECLARE_NODE(DM_CreateComponent)
public:
	DM_CreateComponent();
	void run();
	void init();

	string component_type;
	bool create_empty;

	std::map<string, int> comp_as_int;

	std::string getHelpUrl();

	std::string lead_view_name;
	DM::ViewContainer lead_view;

};

#endif // DM_CREATECOMPONENT_H
