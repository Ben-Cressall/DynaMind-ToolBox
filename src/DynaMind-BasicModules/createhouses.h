#ifndef CREATEHOUSES_H
#define CREATEHOUSES_H

#include <dm.h>

using namespace DM;
class CreateHouses : public Module
{
    DM_DECLARE_NODE(CreateHouses);

private:
    DM::View houses;
    DM::View parcels;
    DM::View building_model;
    DM::View footprint;
    DM::View cityView;

    double heatingT;
    double coolingT;
    int buildyear;
    int stories;
    double alpha;
    double l;
    double b;
    bool onSingal;
    bool l_on_parcel_b;

    void createWindows(DM::Face * f, double distance, double width, double height);
public:
    CreateHouses();
    void run();

};

#endif // CREATESINGLEFAMILYHOUSES_H
