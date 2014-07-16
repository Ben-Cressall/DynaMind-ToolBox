#ifndef urbandevelWaterAttributes_H
#define urbandevelWaterAttributes_H

#include <dm.h>
#include <dmcompilersettings.h>

using namespace DM;
class DM_HELPER_DLL_EXPORT urbandevelWaterAttributes : public Module
{
    DM_DECLARE_NODE(urbandevelWaterAttributes);

private:
    DM::View city;
    DM::View houses;
    DM::View parcels;
    DM::View building_model;
    DM::View footprint;


    int buildingyear;
    int stories;
    double width;
    double ratio;
    bool onSignal;
    bool rotate90;
    bool paramfromCity;
    bool createPopulation;
    bool create3DGeometry;
    double dwf_per_person;
    double wsd_per_person;
    double space_required;

    void createWindows(DM::Face * f, double distance, double width, double height);
public:
    urbandevelWaterAttributes();
    void run();
    void init();
    std::string getHelpUrl();

};

#endif // CREATESINGLEFAMILYHOUSES_H
