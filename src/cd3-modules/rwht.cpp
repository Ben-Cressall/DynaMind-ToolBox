#include "rwht.h"


CD3_DECLARE_NODE_NAME(RWHT)

RWHT::RWHT() {
    addParameter(ADD_PARAMETERS(storage_volume))
                .setUnit("m^3");

    addInPort(ADD_PARAMETERS(in_sw));
    addInPort(ADD_PARAMETERS(in_np));

    addOutPort(ADD_PARAMETERS(out_sw));
    addOutPort(ADD_PARAMETERS(out_np));

    addState(ADD_PARAMETERS(spills));
    addState(ADD_PARAMETERS(dry));

}

RWHT::~RWHT() {
}

int RWHT::f(ptime time, int dt) {
    (void) time;

    //default returns
    out_np[0] = 0;
    out_sw[0] = 0;

    //to avoid over use while raining the demand is removed at the end of this timestep and the added volume can max be the storage volume
    double vol_increase = in_sw[0];
    if (vol_increase > storage_volume) {//solves problem is storage is really small
        vol_increase = storage_volume;
    }

    double vol_straight_to_overflow = in_sw[0] - vol_increase;

    //Current Volume availible
    current_volume = current_volume + vol_increase + in_np[0]; //demand is negative (in_np)

    //tank is full
    if (current_volume > storage_volume) {
        out_np[0] = 0;
        out_sw[0] = current_volume - storage_volume + vol_straight_to_overflow;
        current_volume = storage_volume;
        spills++;
        return dt;
    }

    //demand is not fullfiled (tank is empty)
    if (current_volume <= 0.) {
        out_np[0] = current_volume;
        current_volume = 0;
         out_sw[0] = 0;
        dry++;
    }


    if (vol_straight_to_overflow > 0) {//solves problem is storage is really small
        spills++;
        out_sw[0] = vol_straight_to_overflow;
    }

    return dt;
}


bool RWHT::init(ptime start, ptime end, int dt) {
    (void) start;
    (void) end;

    total_in = 0;
    total_out = 0;
    current_volume = 0;
    spills = 0;
    dry = 0;
    return true;
}

