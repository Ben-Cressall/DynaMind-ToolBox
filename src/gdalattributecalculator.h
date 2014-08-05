#ifndef GDALATTRIBUTECALCULATOR_H
#define GDALATTRIBUTECALCULATOR_H

#include <dmmodule.h>
#include <dm.h>

class DM_HELPER_DLL_EXPORT GDALAttributeCalculator: public DM::Module
{
	DM_DECLARE_NODE(GDALAttributeCalculator)
private:
	std::string attribute;

	std::map<std::string, std::string> variables;

	std::string equation;

	DM::ViewContainer * leading_view;

	std::map<std::string, DM::ViewContainer *> helper_views_name;

	std::string leading_attribute;

	void initViews();

	double solve_variable(OGRFeature *feat, QStringList link_chain);

public:
	GDALAttributeCalculator();
	void init();
	void run();
};

#endif // GDALATTRIBUTECALCULATOR_H
