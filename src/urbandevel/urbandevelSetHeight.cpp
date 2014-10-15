#include<urbandevelSetHeight.h>
#include<dm.h>
#include<tbvectordata.h>
#include<dmgeometry.h>
#include<dahelper.h>
#include<algorithm>
#include<iterator>

DM_DECLARE_NODE_NAME(urbandevelSetHeight, DynAlp)

urbandevelSetHeight::urbandevelSetHeight()
{
//    std::vector<DM::View> data;
//    data.push_back(  DM::View ("dummy", DM::SUBSYSTEM, DM::MODIFY) );

    numbernearest = 9;
    this->heightView = "";

    this->addParameter("Height from: ", DM::STRING, & this->heightView);
    this->addParameter("number of objects", DM::INT, &this->numbernearest);

//    this->addData("Data", data);
}

urbandevelSetHeight::~urbandevelSetHeight()
{
}

void urbandevelSetHeight::init()
{
    city = DM::View("CITY", DM::NODE, DM::READ);
    sb = DM::View("SUPERBLOCK", DM::FACE, DM::MODIFY);
    hview = DM::View(heightView, DM::FACE, DM::READ);

    sb_cent = DM::View("SUPERBLOCK_CENTROIDS", DM::NODE, DM::READ);
    hview_cent = DM::View(heightView + "_CENTROIDS", DM::NODE, DM::READ);

    sb.addAttribute("height_max", DM::Attribute::DOUBLE, DM::WRITE);
    sb.addAttribute("height_min", DM::Attribute::DOUBLE, DM::WRITE);
    sb.addAttribute("height_avg", DM::Attribute::DOUBLE, DM::WRITE);
    hview.addAttribute("height", DM::Attribute::DOUBLE, DM::READ);

    // push the view-access settings into the module via 'addData'
    std::vector<DM::View> views;
    views.push_back(city);
    views.push_back(sb);
    views.push_back(hview);
    views.push_back(sb_cent);
    views.push_back(hview_cent);
    this->addData("data", views);
}

void urbandevelSetHeight::run()
{
    // get data from stream/port
    DM::System * sys = this->getData("data");

    std::vector<DM::Component *> superblocks = sys->getAllComponentsInView(sb);
    std::vector<DM::Component *> superblocks_centroids = sys->getAllComponentsInView(sb_cent);
    std::vector<DM::Component *> buildings = sys->getAllComponentsInView(hview);
    std::vector<DM::Component *> buildings_centroids = sys->getAllComponentsInView(hview);

    int max = static_cast<int>(numbernearest);

    if ( buildings.size() < numbernearest )
        max = buildings.size();

    std::vector<int> distance;
    std::vector<int> height;
    std::map<int, int> distheight;

    for (int i = 0; i < superblocks.size(); i++)
    {
        std::vector<DM::Component*> sblink = superblocks[i]->getAttribute("SUPERBLOCK_CENTROIDS")->getLinkedComponents();

        if( sblink.size() < 1 )
        {
            DM::Logger(DM::Error) << "no area - centroid link";
            return;
        }

        DM::Node * sbcentroid = dynamic_cast<DM::Node*>(sblink[0]);

        for (int j = 0; j < buildings.size(); j++)
        {
            std::vector<DM::Component*> bdlink = buildings[j]->getAttribute(heightView+"_CENTROIDS")->getLinkedComponents();

            if( bdlink.size() < 1 )
            {
                DM::Logger(DM::Error) << "no area - centroid link";
                return;
            }

            DM::Node * bdcentroid = dynamic_cast<DM::Node*>(bdlink[0]);

            distance.push_back(static_cast<int>( TBVectorData::calculateDistance((DM::Node*)sbcentroid,(DM::Node*)bdcentroid) ));
            height.push_back(static_cast<int>( buildings[j]->getAttribute("height")->getDouble())) ;

        }

        if (distance.size() != height.size() )
        {
            DM::Logger(DM::Error) << "distance and height vector lengths differ";
            return;
        }
        for (int k = 0; k < distance.size(); k++)
        {
            distheight[distance[k]] = height[k];

        }
        int avgheight = 0;
        int maxheight = 0;
        int minheight = 999;

        std::map<int,int>::iterator element = distheight.begin();


        for (int k = 0; k < max; k++)
        {
            std::advance(element,k);

            int actualheight = element->second;
            DM::Logger(DM::Debug) << "actual height " << actualheight;

            if (actualheight > maxheight)
            {
                maxheight = actualheight;
                DM::Logger(DM::Debug) << "maxheight " << maxheight;
            }
            else if (actualheight > 0 && actualheight < minheight)
            {
                minheight = actualheight;
            }
            avgheight = avgheight + actualheight;
            DM::Logger(DM::Debug) << "heightinc " << avgheight;
        }

        avgheight = avgheight/max;
        DM::Logger(DM::Debug) << "avgheight " << avgheight;
        DM::Logger(DM::Debug) << "minheight " << minheight;
        DM::Logger(DM::Debug) << "maxheight " << maxheight;
        superblocks[i]->changeAttribute("height_avg", avgheight);
        superblocks[i]->changeAttribute("height_min", minheight);
        superblocks[i]->changeAttribute("height_max", maxheight);
    }
}
