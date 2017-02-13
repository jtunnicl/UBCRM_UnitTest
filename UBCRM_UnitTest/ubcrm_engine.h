#ifndef UBCRM_ENGINE_H
#define UBCRM_ENGINE_H

#include "ubcrmwindow.h"
#include "ubcrm_fcns.h"


class UBCRM_Engine
{

public:

    double Q;           // Discharge
    double S;           // Slope
    double D50;         // D50

    UBCRM_Engine();                            // Constructor

    void initEngine();

    void regimeModel(NodeCHObject *ch);               // Compute Millar-Eaton equilibrium channel width

    void findStable();

    void setRegimeWidth();
}



#endif // UBCRM_ENGINE_H
