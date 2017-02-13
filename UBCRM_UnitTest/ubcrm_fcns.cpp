#include "ubcrm_fcns.h"

using namespace std;

#define PI 3.14159265
#define G 9.80665
#define RHO 1000  // water density
#define Gs 1.65   // submerged specific gravity

double gammln2(double xx)
{
  double x,y,tmp,ser;
  static double cof[6]={76.18009172947146,    -86.50532032941677,
            24.01409824083091,    -1.231739572450155,
            0.1208650973866179e-2,-0.5395239384953e-5};
  int j;

  y = x = xx;
  tmp = x+5.5;
  tmp -= (x+0.5)*log(tmp);
  ser = 1.000000000190015;
  for (j = 0; j <= 5; j++) ser += cof[j] / ++y;
  return -tmp + log( 2.5066282746310005 * ser / x);
}

NodeGSDObject::NodeGSDObject()
    {
         vector <double> tmp;                  // dummy array for grain sizes
         abrasion.push_back(0.0000060000);
         abrasion.push_back(0.0000060000);
         abrasion.push_back(0.0000060000);

         for (int j = 0; j < 15; j++)
         {
             tmp.push_back(0);
             psi.push_back(-3 + j );           // psi -3 to 11 .. should be 9, but ngsz+1,2 is required throughout
         }

         for (int k = 0; k < 3; k++)
             pct.push_back(tmp);

         dsg = 0.;
         stdv = 0.;
         sand_pct = 0.;
    }

void NodeGSDObject::norm_frac()
{
    float ngsz, nlith, cumtot;
    vector<float> ktot;
    ktot.resize(psi.size());

    ngsz = psi.size() - 2;
    nlith = abrasion.size();

    sand_pct = 0;

    // Normalize
    cumtot = 0.0;
    for ( int j = 0; j < ngsz; j++ )           // Sum mass fractions
    {
        ktot[j] = 0.0;
        for ( int k = 0; k < nlith; k++ )
            if (pct[k][j] > 0)                 // Solves problems with rounding
                ktot[j] += pct[k][j];
            else
                pct[k][j] = 0;
        cumtot += ktot[j];
    }

    for ( int j = 0; j < ngsz; j++ )
        for ( int k = 0; k < nlith; k++ )
        {
            if (pct[k][j] > 0)
                pct[k][j] /= cumtot;
            if (psi[j] <= 0) sand_pct += pct[k][j];   // sum sand fraction
        }
}

void NodeGSDObject::dg_and_std()
{
    float tdev;
    float ngsz, nlith;
    vector<float> ktot;
    ktot.resize(psi.size());

    ngsz = psi.size() - 2;
    nlith = abrasion.size();

    dsg = 0.0;
    d84 = 0.0;
    d90 = 0.0;

    for ( int j = 0; j < ngsz; j++ )
    {
        //if (psi[j] >= -1)   // ***!!!*** Dg50 is based on gsizes >= 2 mm (psi = -1 or less)
        //{
        ktot[j] = 0;
        for ( int k = 0; k < nlith; k++ )
            ktot[j] += pct[k][j];               // lithology values for each size fraction are summed.
        dsg += 0.50 * (psi[j] + psi[j+1]) * ktot[j];
        d84 += 0.84 * (psi[j] + psi[j+1]) * ktot[j];
        d90 += 0.90 * (psi[j] + psi[j+1]) * ktot[j];
        //}
    }

    stdv = 0.0;
    for ( int j = 0; j < ngsz; j++ )
    {
        ktot[j] = 0;
        for ( int k = 0; k < nlith; k++ )
            ktot[j] = ktot[j] + pct[k][j];

        tdev = 0.5 * (psi[j] + psi[j+1]) - dsg;
        stdv += 0.5 * tdev * tdev * ktot[j];
    }

    if (stdv > 0)
        stdv = sqrt(stdv);
}

NodeCHObject::NodeCHObject()
{
    QProp = 0.;                             // Proportion of total flow directed to this channel
    depth = 1.;                                // Given the proportion of flow in the channel, this is the computed depth - modified later in xsGeom()
    width = 0.;
    bankHeight = 3.;                           // Measured relative to channel bottom
    b2b = 0.0;
    flowArea = 0.0;
    flowPerim = 0.0;
    hydRadius = 0.0;
    ovBank = 0;
    Tbed = 20.;
    Tbank = 20.;
    Qb_cap = 0.2;
    comp_D = 0.005;
    K = 0;

    Hmax = 0.5;
    mu = 1.5;                                 // Not used - perhaps in future versions
    theta = 30.;
}

void NodeCHObject::chGeom(double relDepth)
{   /* Update channel cross-section area and perimeter
     'eta' in the long profile is tied to bank top of the cross section.
     Thus, depth is assessed relative to the bank top. relDepth is zero at bankfull
     This is only for the channel itself. Overbank flows are handled in the NodeXSObject */

    float theta_rad = theta * PI / 180;        // theta is always in degrees

    depth = bankHeight + relDepth;             /* bankHeight is different for each channel, and flow depth
                                                      within NodeCH objects is always set relative to this. */

    if ( depth < 0 )
    {
        cout << "Cocksucker went negative!!! " << "\n";
        exit(1);
    }


    b2b = width + 2 * ( bankHeight - Hmax) / tan( theta_rad );

    if ( depth <= ( bankHeight - Hmax ) )      // w.s.l. is below sloping bottom edges near bottom of channel
    {
        flowArea = width * depth + pow ( depth, 2 ) / tan( theta_rad );
        flowPerim = width + 2 * depth / sin ( theta_rad );
    }
    else
    {
        flowArea = b2b * depth - pow ( ( bankHeight - Hmax ), 2 ) / tan( theta_rad );
        flowPerim = width + 2 * ( bankHeight - Hmax ) / sin ( theta_rad ) + 2 * ( depth - (bankHeight - Hmax) );
    }

    if (depth > bankHeight)
        ovBank = 1;
    else
        ovBank = 0;

    hydRadius = flowArea / flowPerim;
    aspect = width / depth;
}

void NodeCHObject::chFindDepth(double Q, double D84, double Slope)
    // Work out bankfull depth (bankHeight) for a given discharge.
    // Based on 'FindQ' routine in UBCRM
{

    double Res, converg, testQ1, testQ2, B, M;
    double deltaX = 0.001 * pow( Q, 0.3 );
    double tol = 0.00001;
    int iter = 0;
    int itmax = 250;

    bankHeight = Hmax + 0.3 * pow( Q, 0.3 );
    chGeom( 0 );
    Res = ( 1 / 0.4 ) * log( 12.2 * hydRadius / ( D84 ) );               // Keulegan equation
    chVelocity = Res * pow( ( G * hydRadius * Slope ), 0.5 );
    testQ1 = flowArea * chVelocity;
    converg = ( ( testQ1 ) - Q ) / Q;
    itmax = 250;

    while ( abs ( converg ) > tol )
    {
        bankHeight += deltaX;
        chGeom( 0 );
        Res = ( 1 / 0.4 ) * log( 12.2 * hydRadius / ( D84 ) );
        chVelocity = Res * pow( ( G * hydRadius * Slope ), 0.5 );
        testQ2 = flowArea * chVelocity;
        M = ( testQ2 - testQ1 ) / deltaX;
        B = ( testQ1 - Q ) - M * ( bankHeight - deltaX );

        bankHeight = - B / M;
        if ( bankHeight < Hmax ) { bankHeight = Hmax + 0.01; }
        chGeom( 0 );
        Res = ( 1 / 0.4 ) * log( 12.2 * hydRadius / ( D84 ) );               // Keulegan equation
        chVelocity = Res * pow( ( G * hydRadius * Slope ), 0.5 );
        testQ1 = flowArea * chVelocity;
        converg = ( ( testQ1 ) - Q ) / Q;
        iter++;
        if ( iter > itmax )
        {
            cout << "Iteration exceed in NodeCHObject::chFindDepth" << endl;
            break;
        }
    }
}

void NodeCHObject::chComputeStress(NodeGSDObject f, double Slope)       // Compute stress on bed and banks of channel.
{
    double X, arg;
    double SFbank = 0;
    double tau_star_ref, tau_ref, totstress, W_star;
    double D50 = pow( 2, f.dsg ) / 1000;
    float theta_rad = theta * PI / 180;

    arg =  -1.4026 * log10( width / ( flowPerim - width ) + 1.5 ) + 0.247;
    SFbank = pow ( 10.0 , arg );                   // Partioning equation, M&Q93 Eqn.8, E&M04 Eqn.2
    totstress = G * RHO * bankHeight * Slope;      // We are assuming bankfull flow, thus bankHeight
    Tbed =  totstress * (1 - SFbank) *
            ( b2b / (2 * width) + 0.5 );           // bed_str = stress acting on the bed, M&Q93 Eqn.10, E&M04 Eqn.4
    Tbank =  totstress * SFbank *
            ( b2b + width ) * sin( theta_rad ) / (4 * bankHeight );

    // estimate the largest grain that the flow can move
    comp_D = Tbed / (0.02 * G * RHO * Gs );

    // estimate the division between key stones and bed material load
    //   (this corresponds to the approximate limit of full mobility)
    K = Tbed / (0.04 * G * RHO * Gs);

    // use Wilcock and Crowe to estimate the sediment transport rate
    tau_star_ref = 0.021 + 0.015 * exp (-20 * f.sand_pct);
    tau_ref = tau_star_ref * G * RHO * Gs * D50;
    X = Tbed / tau_ref;

    if (X < 1.35)
        W_star = 0.002 * pow( X, 7.5 );
    else
        W_star = 14 * pow( ( 1 - ( 0.894 / pow( X, 0.5 ) ) ), ( 4.5 ) );

    Qb_cap = width * ( W_star / ( 1.65 * G ) ) * pow( ( Tbed / RHO ), ( 3 / 2 ) );

}

UBCRM_Fcns::UBCRM_Fcns()
{

}
