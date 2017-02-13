#include "ubcrm_engine.h"

using namespace std;

#define PI 3.14159265
#define G 9.80665
#define RHO 1000  // water density
#define Gs 1.65   // submerged specific gravity

UBCRM_Engine::UBCRM_Engine()
{
    initEngine();
    Q =
}



void UBCRM_Engine::regimeModel(NodeCHObject *ch)
{
    double Tol = 0.00001;
    double Q = 200;
    double test_plus, test_minus = 0;
    double p, p1, p2, p_upper, p_lower = 0;
    double converg, gradient = 0;
    double gradient_1 = 0;
    double gradient_2 = 0;

    p = 4 * pow( Q, 0.5 );
    ch.width = p * 1.001;
    findStable( n, m, r );
    test_plus = ch.Qb_cap;               // Compute bedload transport, for given width

    CH.width = p * 0.999;
    findStable( n, m, r );
    test_minus = CH.Qb_cap;
    gradient_1 = test_plus - test_minus;
    p1 = p;

    // Now move in the direction of the gradient
    if (gradient_1 > 0)
        p = p + 0.25 * p;
    else
        p = p - 0.25 * p;
    CH.width = p * 1.001;
    findStable( n, m, r );
    test_plus = CH.Qb_cap;

    CH.width = p * 0.999;
    findStable( n, m, r );
    test_minus = CH.Qb_cap;

    gradient_1 = test_plus - test_minus;
    p2 = p;


    while( gradient_1 / gradient_2 > 0 )
    {
        gradient_1 = gradient_2;
        p1 = p;
        if (gradient_1 > 0)
            p = p + 0.25 * p;
        else
            p = p - 0.25 * p;
        CH.width = p * 1.001;
        findStable( n, m, r );
        test_plus = CH.Qb_cap;

        CH.width = p * 0.999;
        findStable( n, m, r );
        test_minus = CH.Qb_cap;

        gradient_2 = test_plus - test_minus;
        p2 = p;
    }

    p_upper = max( p1, p2 );
    p_lower = min( p1, p2 );
    p = 0.5 * ( p_upper + p_lower );
    converg = ( p_upper - p_lower ) / p;

    while(converg > Tol)
    {
        CH.width = p * 1.001;
        findStable( n, m, r );
        test_plus = CH.Qb_cap;

        CH.width = p * 0.999;
        findStable( n, m, r );
        test_minus = CH.Qb_cap;

        gradient = test_plus - test_minus;
        if ( gradient > 0 )
            p_lower = p;
        else
            p_upper = p;
        p = 0.5 * ( p_upper + p_lower );
        converg = ( p_upper - p_lower ) / p;
    }

    CH.width = p;
    CH.bankHeight = CH.Hmax + sin( CH.theta * PI / 180 ) * ( (CH.b2b - CH.width) / 2 );
    findStable( n, m, r );
}

void UBCRM_Engine::findStable()
{
    // find optimal theta for the specified Q and Pbed
    NodeCHObject& CH = r->RiverXS[n].CHList[ch_idx];
    NodeGSDObject& f = r->F[n];

    // specify constants
    double converg, bank_crit;
    double D84 = pow( 2, f.d84 ) / 1000;       // grain size in m
    double D90 = pow( 2, f.d90 ) / 1000;
    double phi = 40.;                          // friction angle for bank sediment
    double Tol = 0.001;
    double deltaX = 0.01 * CH.theta;
    double tau_star = 0.02;   // = 0.035;
    int iter = 0;
    int itmax = 250;

    // set the upper and lower theta limits
    double T_upper = CH.theta - deltaX;
    double T_lower = deltaX;
    CH.theta = 0.25 * phi;                     // UBCRM_H uses 1/4; later versions use 2/3

    CH.chFindDepth( CH.QProp * QwCumul[n], D84, bedSlope[n] );
    CH.chComputeStress(f, bedSlope[n]);

    // calculate the bank stability index (Bank SI)
    bank_crit = G * RHO * Gs * D90 * tau_star *
              pow( 1 - ( pow( sin ( CH.theta * PI / 180 ), 2) /
              pow( sin( phi * PI / 180 ), 2) ), 0.5 );
    converg = ( CH.Tbank - bank_crit ) / bank_crit;   // Btest

    if ( CH.bankHeight > CH.Hmax )              // perform a stress partitioning only if Y > H
    {
        while(abs(converg) > Tol)
        {
            if(converg > 0){T_upper = CH.theta;} else {T_lower = CH.theta;}   // new candidate theta
            CH.theta = 0.5 * ( T_upper + T_lower );

            CH.chFindDepth( CH.QProp * QwCumul[n], D84, bedSlope[n] );
            CH.chComputeStress(f, bedSlope[n]);   // Compute Tbed, Tbank
            bank_crit = G * RHO * Gs * D90 * tau_star *
                      pow( 1 - ( pow( sin ( CH.theta * PI / 180 ), 2) /
                      pow( sin( phi * PI / 180 ), 2) ), 0.5 );
            converg = ( CH.Tbank - bank_crit ) / bank_crit;
            iter++;
            if ( iter > itmax )
            {
                cout << "Iteration exceed in UBCRM_Engine::findStable" << endl;
                break;
            }
        }
    }
    else  // Flow is lower than Hmax, so assume rectangular channel
    {
        CH.theta = 89;
        CH.chComputeStress(f, bedSlope[n]);  // Need to know Qb_Cap, regardless
    }
}

void UBCRM_Engine::setRegimeWidth()
{

    // Adjust channel regime one cross-section at a time, marching upstream

    NodeXSObject& XS = r->RiverXS[regimeCounter];
    double splitRatio;                         // Random (0-1) variable for splitting channel
    float deltaArea, oldArea = 0;              // Change in reach cross-section area
    float deltaEta = 0;
    float reachDrop = 0;                       // Drop in elevation over reach river length
    float oldBankHeight = 0;
    double Tol = 50.;                          // Maximum allowed channel aspect (w/d)
    int m, n = 0;                              // Counters

    oldBankHeight = XS.maxBankHt;
    oldArea = XS.xsFlowArea[2];

    XS.RegimeReset();                          // Clear XS info; Start on assumption of just one bankfull channel
    regimeModel( regimeCounter, 0, r );        // Regime assessment of 1st channel
    XS.xsGeom();
    XS.numChannels = 1;

    for (m = 0; m < 5; m++)                    // Number of sweeps to check for any w/d aspects >Tol
    {
        for (n = 0; n < 10; n++)               // Iterate through CH Objects
        {
            splitRatio = ((double)rand() / RAND_MAX);
            //if ( n > 0 ) { XS.CHList[n].QProp = 0; }
            if (( XS.CHList[n].aspect > Tol ) && ( XS.numChannels < 10 ))   // Any channels exceeding tolerance are split, at random proportions
            {
                XS.numChannels++;
                XS.CHList[XS.numChannels-1].QProp = splitRatio * XS.CHList[n].QProp;
                regimeModel( regimeCounter, XS.numChannels-1, r );   // Assess regime proportions, for given flow
                XS.CHList[n].QProp *= ( 1 - splitRatio );
                regimeModel( regimeCounter, n, r );
            }
            XS.xsGeom();
        }
     }

    if ( (r->counter > 260 ) )                 // Update floodplain volume
    {
        deltaArea = oldArea - r->RiverXS[regimeCounter].xsFlowArea[2];       // Change induced by floodplain erosion
        deltaEta = r->RiverXS[regimeCounter].maxBankHt - oldBankHeight;      // Change induced by channel aggr/degr
        deltaEta += deltaArea / r->RiverXS[regimeCounter+1].fpWidth;         // Total lateral change is a product of the two

        reachDrop = bedSlope[regimeCounter] * r->dx * r->RiverXS[regimeCounter].chSinu;
        // new sinuosity
        r->RiverXS[regimeCounter].chSinu *= ( (reachDrop + deltaEta ) / reachDrop );
        if ( r->RiverXS[regimeCounter].chSinu < 1 )
            r->RiverXS[regimeCounter].chSinu = 1;
        if ( r->RiverXS[regimeCounter].chSinu > 2.6 )
            r->RiverXS[regimeCounter].chSinu = 2;
        // readjust downstream elevation to account for gains or losses during width adjustment
        // [ old area - new area ] - positive value if material removed, channel widening.

    }
    regimeCounter --;
    if (regimeCounter < 2)
        regimeCounter = (r->nnodes-2);
}
