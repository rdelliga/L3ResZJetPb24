#include "TMath.h"

inline double DPhi(double phi1, double phi2) { // Return value between 0 and phi.
    double dphi = fabs(phi1 - phi2);
    return (dphi <= TMath::Pi())? dphi : TMath::TwoPi() - dphi;
 }
