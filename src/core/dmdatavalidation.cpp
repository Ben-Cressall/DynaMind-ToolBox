#include "dmdatavalidation.h"
#include "dmcomponent.h"
#include "dmview.h"
#include <QtGlobal>
namespace DM {
bool DataValidation::isVectorOfViewRead(std::vector<DM::View> views) {
    foreach (DM::View v, views) {
        if (v.reads()) {
            return true;
        }
    }
    return false;
}
bool DataValidation::isVectorOfViewWrite(std::vector<DM::View> views) {
    foreach (DM::View v, views) {
        if (v.writes()) {
            return true;
        }
    }
    return false;
}

/*bool DataValidation::isAvalibleInComponent(const DM::View & v,  const DM::Component  *  c) {

    foreach (std::string attr, v.getReadAttributes()) 
	{
        const std::map<std::string, Attribute*> c_attr = c->getAllAttributes();
        if (c_attr.find(attr) == c_attr.end())
            return false;
    }

    return true;

}*/
}
