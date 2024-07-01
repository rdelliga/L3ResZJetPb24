// Some general parameters

#define REDOJES 0

int MAXJETS = 50;

float akradius = 0.4;

bool doTPdijet = true;


float hibins[] = {-1, 0.}; //, 0., 5000.};
const int nhibins = sizeof(hibins)/sizeof(hibins[0])-1;

//float ptedges[] = {80., 100., 120., 140., 80., 140., 100., 5000., 0., 5000};
float ptedges[] = {0., 5000};
const int nptbins = sizeof(ptedges)/sizeof(ptedges[0])-1;

float etaedges[] = {-5.2, -3.9, -2.6, -1.3, 0, 1.3, 2.6, 3.9, 5.2, -5.2, 5.2};
const int netabins = sizeof(etaedges)/sizeof(etaedges[0])-1;


// JER
//string resolutionFile = "JRDatabase/textFiles/Autumn18_RunD_V7b_MC_PtResolution_AK4PF.txt";
//string scaleFactorFile = "JRDatabase/textFiles/Autumn18_RunD_V7b_MC_SF_AK4PF.txt";
