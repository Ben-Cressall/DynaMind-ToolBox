#ifndef INOUT2MODULE_H
#define INOUT2MODULE_H

#include "compilersettings.h"
#include "module.h"
using namespace DM;
class DM_HELPER_DLL_EXPORT InOut2 : public  Module {

DM_DECLARE_NODE( InOut2 )

    public:
        DM::System * sys_in;
        double a;


        InOut2();
        void run();
        virtual ~InOut2();
};


#endif // INOUT2MODULE_H
