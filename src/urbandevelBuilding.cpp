#include "urbandevelBuilding.h"
#include "dmgeometry.h"
#include "cgalgeometry.h"
#include "tbvectordata.h"
#include <QPolygonF>
#include <QTransform>
#include <dmhelper.h>
#include <tbvectordata.h>
#include <littlegeometryhelpers.h>
#ifdef _OPENMP
#include <omp.h>
#endif

DM_DECLARE_NODE_NAME(urbandevelBuilding, DynAlp)

urbandevelBuilding::urbandevelBuilding()
{
    buildingyear = 2010;
    stories = 3;
    length = 20;
    width = 15;
    onSignal = TRUE;
    l_on_parcel_b = FALSE;
    yearFromCity = TRUE;
    create3DGeometry = TRUE;

    this->addParameter("l", DM::DOUBLE, &length);
    this->addParameter("b", DM::DOUBLE, &width);
    this->addParameter("stories", DM::INT, &stories);
    this->addParameter("year", DM::INT, &buildingyear);
    this->addParameter("year_from_city", DM::BOOL, &yearFromCity);

    this->addParameter("onSignal", DM::BOOL, &onSignal);

    this->addParameter("l_on_parcel_b", DM::BOOL, &l_on_parcel_b);
    this->addParameter("create3DGeometry", DM::BOOL, &create3DGeometry);

}

void urbandevelBuilding::init()
{
    cityView = DM::View("CITY", DM::COMPONENT, DM::READ);
    cityView.addAttribute("year", DM::Attribute::DOUBLE, DM::READ);

    parcels = DM::View("PARCEL", DM::FACE, DM::READ);
    parcels.addAttribute("status", DM::Attribute::DOUBLE, DM::WRITE);

    houses = DM::View("BUILDING", DM::COMPONENT, DM::WRITE);

    houses.addAttribute("centroid_x", DM::Attribute::DOUBLE, DM::WRITE);
    houses.addAttribute("centroid_y", DM::Attribute::DOUBLE, DM::WRITE);

    houses.addAttribute("built_year", DM::Attribute::DOUBLE, DM::WRITE);
    houses.addAttribute("stories", DM::Attribute::DOUBLE, DM::WRITE);
    houses.addAttribute("stories_below", DM::Attribute::DOUBLE, DM::WRITE);
    houses.addAttribute("stories_height", DM::Attribute::DOUBLE, DM::WRITE);

    houses.addAttribute("floor_area", DM::Attribute::DOUBLE, DM::WRITE);
    houses.addAttribute("roof_area", DM::Attribute::DOUBLE, DM::WRITE);
    houses.addAttribute("gross_floor_area", DM::Attribute::DOUBLE, DM::WRITE);

    houses.addAttribute("l_bounding", DM::Attribute::DOUBLE, DM::WRITE);
    houses.addAttribute("b_bounding", DM::Attribute::DOUBLE, DM::WRITE);
    houses.addAttribute("h_bounding", DM::Attribute::DOUBLE, DM::WRITE);
//    houses.addAttribute("alhpa_bounding", DM::Attribute::DOUBLE, DM::WRITE);

//    houses.addAttribute("alpha_roof", DM::Attribute::DOUBLE, DM::WRITE);

    houses.addAttribute("cellar_used", DM::Attribute::DOUBLE, DM::WRITE);
    houses.addAttribute("roof_used", DM::Attribute::DOUBLE, DM::WRITE);

//    houses.addAttribute("T_heating", DM::Attribute::DOUBLE, DM::WRITE);
//   houses.addAttribute("T_cooling", DM::Attribute::DOUBLE, DM::WRITE);

    houses.addAttribute("Geometry", "Geometry", DM::WRITE);
    houses.addAttribute("V_living", DM::Attribute::DOUBLE, DM::WRITE);

    building_model = DM::View("Geometry", DM::FACE, DM::WRITE);
    building_model.addAttribute("type", DM::Attribute::DOUBLE, DM::WRITE);
    building_model.addAttribute("color", DM::Attribute::DOUBLEVECTOR, DM::WRITE);
    building_model.addAttribute("parent", DM::Attribute::DOUBLE, DM::WRITE);

    parcels.addAttribute("BUILDING", houses.getName(), DM::WRITE);
    houses.addAttribute("PARCEL", parcels.getName(), DM::WRITE);
    houses.addAttribute("Geometry", building_model.getName(), DM::WRITE);

    std::vector<DM::View> data;
    data.push_back(houses);
    data.push_back(parcels);
    data.push_back(building_model);
    if (this->yearFromCity)
        data.push_back(cityView);

    this->addData("City", data);

}

void urbandevelBuilding::run()
{
    DM::System * city = this->getData("City");
    DM::SpatialNodeHashMap spatialNodeMap(city, 100,false);

    std::vector<Component*> city_comps = city->getAllComponentsInView(cityView);
    if (city_comps.size() != 0)
        buildingyear = city_comps[0]->getAttribute("year")->getDouble();

    int numberOfHouseBuild = 0;

    foreach(DM::Component* c, city->getAllComponentsInView(parcels))
    {
        DM::Face* parcel = (DM::Face*)c;

        if (parcel->getAttribute("status")->getString() != "develop" && onSignal == true)
            continue;

        std::vector<DM::Node * > nodes  = TBVectorData::getNodeListFromFace(city, parcel);

        std::vector<DM::Node> bB;
        //Calcualte bounding minial bounding box
        std::vector<double> size;
        double angle = CGALGeometry::CalculateMinBoundingBox(nodes, bB,size);
        if (l_on_parcel_b)
            angle+=90;

        Node centroid = DM::CGALGeometry::CaclulateCentroid2D(parcel);

        QPointF f1 (- length/2,  - width/2);
        QPointF f2 (+ length/2,- width/2);
        QPointF f3 ( + length/2,  + width/2);
        QPointF f4 (- length/2,  + width/2);

        QPolygonF original = QPolygonF() << f1 << f2 << f3 << f4;
        QTransform transform = QTransform().rotate(angle);
        QPolygonF rotated = transform.map(original);

        std::vector<DM::Node * > houseNodes;

        foreach (QPointF p, rotated) {
            houseNodes.push_back(spatialNodeMap.addNode(p.x()+centroid.getX(), p.y()+centroid.getY(), 0, 0.01, DM::View()));

        }
        if (houseNodes.size() < 2) {
            Logger(Error) << "Can't create House";
            continue;
        }

        DM::Component * building = city->addComponent(new Component(), houses);

        //Create Building and Footprints
        DM::Face * foot_print = city->addFace(houseNodes, footprint);
        foot_print->addAttribute("area", DM::CGALGeometry::CalculateArea2D(foot_print));
        foot_print->addAttribute("year", buildingyear);
        foot_print->addAttribute("height", stories*3);
        Node  n = DM::CGALGeometry::CaclulateCentroid2D(foot_print);
        building->addAttribute("type", "single_family_house");
        building->addAttribute("year", buildingyear);
        building->addAttribute("stories", stories);
        building->addAttribute("stories_below", 0); //cellar counts as story
        building->addAttribute("stories_height",3 );

        building->addAttribute("building_area", length*width);
        building->addAttribute("park_area", length*width);
        building->addAttribute("roof_area", length*width);
        building->addAttribute("gross_floor_area", length * width * stories * 3);

        building->addAttribute("centroid_x", n.getX());
        building->addAttribute("centroid_y", n.getY());

        building->addAttribute("l_bounding", length);
        building->addAttribute("b_bounding", width);
        building->addAttribute("h_bounding", stories * 3);

        building->addAttribute("alpha_bounding", angle);

        building->addAttribute("cellar_used", 1);
        building->addAttribute("roof_used", 0);

        building->addAttribute("V_living", length*width*stories * 3);

        //Create Links
        building->getAttribute("Footprint")->addLink(foot_print, footprint.getName());
        foot_print->getAttribute("BUILDING")->addLink(building, houses.getName());
        building->getAttribute("PARCEL")->addLink(parcel, parcels.getName());
        parcel->getAttribute("BUILDING")->addLink(building, houses.getName());
        parcel->addAttribute("is_built",1);
        numberOfHouseBuild++;

        if (!create3DGeometry) {
            city->addComponentToView(foot_print, building_model);
            building->getAttribute("Geometry")->addLink(foot_print, building_model.getName());
            continue;
        }
        LittleGeometryHelpers::CreateStandardBuilding(city, houses, building_model, building, houseNodes, stories);
/*        if (alpha > 10) {
            LittleGeometryHelpers::CreateRoofRectangle(city, houses, building_model, building, houseNodes, stories*3, alpha);
        }
*/


    }
    Logger(Standard) << "Created Houses " << numberOfHouseBuild;
}

string urbandevelBuilding::getHelpUrl()
{
    return "https://github.com/iut-ibk/DynaMind-BasicModules/blob/master/doc/urbandevelBuilding.md";
}
