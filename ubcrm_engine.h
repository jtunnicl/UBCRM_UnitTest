#ifndef UBCRM_ENGINE_H
#define UBCRM_ENGINE_H

#include "ubcrmwindow.h"
#include "ubcrm_fcns.h"
#include <vector>

class UBCRM_Engine
{

public:

    double Q;           // Discharge
    double S;           // Slope
    double D50;         // D50 in m
    NodeCHObject ch;

    UBCRM_Engine();                            // Constructor

    //void initEngine();

    void regimeModel(NodeCHObject *ch);               // Compute Millar-Eaton equilibrium channel width

    void findStable();

};



#endif // UBCRM_ENGINE_H
