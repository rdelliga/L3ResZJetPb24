// Some general parameters

#define REDOJES 1


// JEC FILES
// 2023ppwithpbpb_MC_L2Relative_AK4PF.txt  2023ppwithpbpb_old_MC_L2Relative_AK4PF.txt  2023ppwithpp_MC_L2Relative_AK4PF.txt  2023ppwithpp_old_MC_L2Relative_AK4PF.txt

//string jecfile = "jecfiles/2023ppwithpbpb_old_MC_L2Relative_AK4PF.txt";
string jecfile = "jecfiles/2023ReReco_L2Relative_AK4PF.txt";
string MCjecfile = "jecfiles/2023ppwithpbpb_old_MC_L2Relative_AK4PF.txt";
string l2file = "jecfiles/L2residual_corrections.txt";


// JER
//string resolutionFile = "JRDatabase/textFiles/Autumn18_RunD_V7b_MC_PtResolution_AK4PF.txt";
//string scaleFactorFile = "JRDatabase/textFiles/Autumn18_RunD_V7b_MC_SF_AK4PF.txt";

int MAXJETS = 50;

float akradius = 0.4;
Float_t jtptmin = 15.;


float hibins[] = {-1, 0.}; //, 0., 5000.};
const int nhibins = sizeof(hibins)/sizeof(hibins[0])-1;

//float ptedges[] = {80., 100., 120., 140., 80., 140., 100., 5000., 0., 5000};
float ptedges[] = {0., 5000};
const int nptbins = sizeof(ptedges)/sizeof(ptedges[0])-1;

//float etaedges[] = {-5.2, -3.9, -2.6, -1.3, 0, 1.3, 2.6, 3.9, 5.2, -5.2, 5.2};
float etaedges[] = {5.2, -5.2, 5.2};
const int netabins = sizeof(etaedges)/sizeof(etaedges[0])-1;

int hltedges[] = {0, 40, 60, 80, 100, 120};
const int nhltbins = sizeof(hltedges)/sizeof(hltedges[0])-1;

