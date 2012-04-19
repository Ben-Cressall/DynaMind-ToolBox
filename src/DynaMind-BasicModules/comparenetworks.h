#ifndef COMPARENETWORKSMODULE_H
#define COMPARENETWORKSMODULE_H

#include "dmcompilersettings.h"
#include "dmmodule.h"
using namespace DM;
class DM_HELPER_DLL_EXPORT CompareNetworks : public  Module {

DM_DECLARE_NODE( CompareNetworks )

    public:
        DM::System *n1, *n2;
        std::string n1i, n2i;

        CompareNetworks();
        void run();
        virtual ~CompareNetworks();
};


#endif // CheckShape_H
