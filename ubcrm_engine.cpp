#include "ubcrm_engine.h"

using namespace std;

#define PI 3.14159265
#define G 9.80665
#define RHO 1000  // water density
#define Gs 1.65   // submerged specific gravity

UBCRM_Engine::UBCRM_Engine()
{
    Q = 200;
    S = 0.001;
    D50 = 0.045;
}

void UBCRM_Engine::regimeModel(NodeCHObject *ch)
{
    double Tol = 0.00001;
    double test_plus, test_minus = 0;
    double p, p1, p2, p_upper, p_lower = 0;
    double converg, gradient = 0;
    double gradient_1 = 0;
    double gradient_2 = 0;

    p = 4 * pow( Q, 0.5 );
    ch->width = p * 1.001;
    findStable();
    test_plus = ch->Qb_cap;               // Compute bedload transport, for given width

    ch->width = p * 0.999;
    findStable();
    test_minus = ch->Qb_cap;
    gradient_1 = test_plus - test_minus;
    p1 = p;

    // Now move in the direction of the gradient
    if (gradient_1 > 0)
        p = p + 0.25 * p;
    else
        p = p - 0.25 * p;
    ch->width = p * 1.001;
    findStable();
    test_plus = ch->Qb_cap;

    ch->width = p * 0.999;
    findStable();
    test_minus = ch->Qb_cap;

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
        ch->width = p * 1.001;
        findStable();
        test_plus = ch->Qb_cap;

        ch->width = p * 0.999;
        findStable();
        test_minus = ch->Qb_cap;

        gradient_2 = test_plus - test_minus;
        p2 = p;
    }

    p_upper = max( p1, p2 );
    p_lower = min( p1, p2 );
    p = 0.5 * ( p_upper + p_lower );
    converg = ( p_upper - p_lower ) / p;

    while(converg > Tol)
    {
        ch->width = p * 1.001;
        findStable();
        test_plus = ch->Qb_cap;

        ch->width = p * 0.999;
        findStable();
        test_minus = ch->Qb_cap;

        gradient = test_plus - test_minus;
        if ( gradient > 0 )
            p_lower = p;
        else
            p_upper = p;
        p = 0.5 * ( p_upper + p_lower );
        converg = ( p_upper - p_lower ) / p;
    }

    ch->width = p;
    ch->bankHeight = ch->Hmax + sin( ch->theta * PI / 180 ) * ( (ch->b2b - ch->width) / 2 );
    findStable();
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
    double deltaX = 0.01 * ch.theta;
    double tau_star = 0.02;   // = 0.035;
    int iter = 0;
    int itmax = 250;

    // set the upper and lower theta limits
    double T_upper = ch->theta - deltaX;
    double T_lower = deltaX;
    ch->theta = 0.25 * phi;                     // UBCRM_H uses 1/4; later versions use 2/3

    ch->chFindDepth( ch->QProp * QwCumul[n], D84, bedSlope[n] );
    ch->chComputeStress(f, bedSlope[n]);

    // calculate the bank stability index (Bank SI)
    bank_crit = G * RHO * Gs * D90 * tau_star *
              pow( 1 - ( pow( sin ( ch->theta * PI / 180 ), 2) /
              pow( sin( phi * PI / 180 ), 2) ), 0.5 );
    converg = ( ch->Tbank - bank_crit ) / bank_crit;   // Btest

    if ( ch->bankHeight > ch->Hmax )              // perform a stress partitioning only if Y > H
    {
        while(abs(converg) > Tol)
        {
            if(converg > 0){T_upper = ch->theta;} else {T_lower = ch->theta;}   // new candidate theta
            ch->theta = 0.5 * ( T_upper + T_lower );

            ch->chFindDepth( ch->QProp * QwCumul[n], D84, bedSlope[n] );
            ch->chComputeStress(f, bedSlope[n]);   // Compute Tbed, Tbank
            bank_crit = G * RHO * Gs * D90 * tau_star *
                      pow( 1 - ( pow( sin ( ch->theta * PI / 180 ), 2) /
                      pow( sin( phi * PI / 180 ), 2) ), 0.5 );
            converg = ( ch->Tbank - bank_crit ) / bank_crit;
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
        ch->theta = 89;
        ch->chComputeStress(f, bedSlope[n]);  // Need to know Qb_Cap, regardless
    }
}
