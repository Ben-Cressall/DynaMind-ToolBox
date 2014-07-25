#include<urbandevelSetType.h>
#include<dm.h>
#include<tbvectordata.h>
#include<dmgeometry.h>
#include<dahelper.h>
#include<algorithm>
#include<iterator>

DM_DECLARE_NODE_NAME(urbandevelSetType, DynAlp)

urbandevelSetType::urbandevelSetType()
{
    numbernearest = 9;
    from_SB = 0;
    from_nearest = 1;
    this->addParameter("no superblocks considered", DM::INT, &this->numbernearest);
    this->addParameter("type from SB", DM::BOOL, &this->from_SB);
    this->addParameter("type from neighbors", DM::BOOL, &this->from_nearest);
    typevec.push_back("res");
    typevec.push_back("com");
    typevec.push_back("ind");
}

urbandevelSetType::~urbandevelSetType()
{
}

void urbandevelSetType::init()
{
    city = DM::View("CITY", DM::NODE, DM::READ);
    sb = DM::View("SUPERBLOCK", DM::FACE, DM::MODIFY);
    sb_cent = DM::View("SUPERBLOCK_CENTROIDS", DM::NODE, DM::READ);

    sb.addAttribute("type", DM::Attribute::DOUBLE, DM::WRITE);

    // push the view-access settings into the module via 'addData'
    std::vector<DM::View> views;
    views.push_back(city);
    views.push_back(sb);
    views.push_back(sb_cent);
    this->addData("data", views);
}

void urbandevelSetType::run()
{
    // get data from stream/port
    DM::System * sys = this->getData("data");

    std::vector<DM::Component *> superblocks = sys->getAllComponentsInView(sb);
    std::vector<DM::Component *> superblocks_centroids = sys->getAllComponentsInView(sb_cent);

    std::map<std::string,int> typebool;

    for (int active = 0; active < superblocks.size(); active++)
    {
        std::map<double,std::string> disttype;

        std::string actualtype = superblocks[active]->getAttribute("type")->getString();

        if ( actualtype == "" ) continue;
        if ( find(typevec.begin(), typevec.end(), actualtype) == typevec.end() )
        {
            DM::Logger(DM::Warning) << "type '" << actualtype << " is not defined ... skipping";
            continue;
        }

        std::vector<DM::Component*> sblinkactive = superblocks[active]->getAttribute("SUPERBLOCK_CENTROIDS")->getLinkedComponents();

        if( sblinkactive.size() < 1 )
        {
            DM::Logger(DM::Error) << "no area - centroid link";
            return;
        }

        DM::Node * sbcentroidactive = dynamic_cast<DM::Node*>(sblinkactive[0]);

        for (int compare = 0; compare < superblocks.size(); compare++)
        {
            std::string comparetype = superblocks[compare]->getAttribute("type")->getString();

            if ( active == compare || comparetype.empty() )
                continue;

            std::vector<DM::Component*> sblinkcompare = superblocks[compare]->getAttribute("SUPERBLOCK_CENTROIDS")->getLinkedComponents();

            if( sblinkcompare.size() < 1 )
            {
                DM::Logger(DM::Error) << "no area - centroid link";
                return;
            }

            DM::Node * sbcentroidcompare = dynamic_cast<DM::Node*>(sblinkcompare[0]);

            double distance = TBVectorData::calculateDistance((DM::Node*)sbcentroidactive,(DM::Node*)sbcentroidcompare)/1000;
            std::string type =static_cast<string>( superblocks[compare]->getAttribute("type")->getString() );

            disttype[distance] = type;
        }

        int max = numbernearest;
        if (disttype.size() < numbernearest)
            max = disttype.size();

        map<std::string, pair<double,int> > rnktype;

        std::string settype = "";
        double setdist = 0;

        for (map<double,std::string>::iterator it = disttype.begin(); it != disttype.end(); it++)
        {
            double distance = it->first;
            std::string type = it->second;

            map<std::string, pair<double,int> >::iterator rnkit = rnktype.find(type);

            if ( rnkit == rnktype.end() )   // initialize if first element with type x
            {
                rnktype[type].first = 1/distance;
                rnktype[type].second = 1;
            }
            else                            // if type already exists increase invert distance and count
            {
                rnktype[type].first += 1/distance;
                rnktype[type].second++;
            }

            // set largest accumulated invert distance as typedistance

            if ( rnktype[type].first > setdist) {
                setdist = rnktype[type].first;
                settype = type;
            }

            if ( rnktype[type].second > max )
            {
                break;
            }
            DM::Logger(DM::Debug) << "type = " << type << " num = " << rnktype[type].second << " dist = " << rnktype[type].first;
        }
        superblocks[active]->changeAttribute("type", settype);
        typebool[settype] = 1;
    }

    // correcting for missing types below

    for (int i = 0; i < typevec.size(); ++i)
    {
        map<std::string,int>::iterator rnkit = typebool.find(typevec[i]);

        if (rnkit == typebool.end())
        {
            DM::Logger(DM::Warning) << "never used: " << typevec[i];
            //
        }
    }

}
