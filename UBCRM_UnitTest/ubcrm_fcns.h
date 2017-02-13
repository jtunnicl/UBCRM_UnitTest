#ifndef UBCRM_FCNS_H
#define UBCRM_FCNS_H

#include "ubcrmwindow.h"
#include <vector>
using namespace std;

double gammln2(double xx);

class NodeGSDObject
{

    // Object intended to hold grain size info at each node

public:

    NodeGSDObject();

    vector < double > abrasion;                // abrasion value for each lithology type (3)
    vector < double > psi;                     // psi (base 2) grain size categories
    vector < vector < double > > pct;          // Grain-size  (ngsz x nlith)
    float dsg;                                 // Geometric mean grain size
    float d84;
    float d90;
    float stdv;                                // Standard deviation in GSD
    float sand_pct;                            // Percentage of sand (< 2 mm) in GSD

    void norm_frac();

    void dg_and_std();                         // Calculate D50, sand%, geometric (log2)
};

class NodeCHObject
{

    // Object intended to hold info on channel configuration - multiple channels may exist within a reach

public:

    NodeCHObject();

    double QProp;                              // Proportion of total flow going into this channel
    double depth;                              // Flow depth (m from channel bottom)
    double width;                              // Channel width (m) at bottom of trapezoid for each node
    double b2b;                                // Bank-to-bank width (top of in-channel flow section)
    double flowArea;                           // Flow area within the channel
    double flowPerim;                          // Perimeter, within the channel
    double hydRadius;                          // Hydraulic radius for channel (note duplicate variable for XS)
    double chVelocity;                         // Flow velocity (m/s) within the channel
    double aspect;                             // width/depth ratio of the channel

    int ovBank;                                // Flag; flow has gone overbank, for this channel
    double Tbed;                               // Shear stress acting on the channel bed (Pa)
    double Tbank;                              // Shear stress acting on the channel banks (Pa)
    double Qb_cap;                             // Transport capacity (m3/s)
    double comp_D;                             // The largest grain that the flow can move
    double K;                                  // Estimated division between key stones and bed material load
    double bankHeight;                         // Height above channel bottom (m)
    double Hmax;                               // Bank strength as a vertical upper bank section (m)
    double mu;                                 // Bank strength, relative to bed (afer Millar, 2005)
    double theta;                              // Bank sideslope angle (degrees)

    void chGeom(double relDepth);              // Calculate x-sec area for a given depth

    void chFindDepth(double Q, double D84, double Slope);
                                               // Work out bankfull depth (bankHeight) for a given discharge.
    void chComputeStress(NodeGSDObject f, double Slope);
                                               // Compute stress on banks
};


class UBCRM_Fcns
{

public:

    double Q;           // Discharge
    double S;           // Slope
    double D50;         // D50

    UBCRM_Fcns();                            // Constructor

    void initData();
}

#endif // UBCRM_H_H
