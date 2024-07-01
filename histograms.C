#include "histograms.h"


// Could to something similar as in jetphys what comes to directories, just replace etas with pTs
// Need to have the rho stuff?
// Versions with Btagging will be needed too


histograms::histograms(TDirectory *dir, float etamin, float etamax, float hibinmin, float hibinmax, bool ismc) {
  //histograms::histograms(float ptmin, float ptmax, bool ismc) {
  // Do we need a directory too?

  TDirectory *curdir = gDirectory;
  bool enter = dir->cd();
  assert(enter);
  this->dir = dir;
  
  this->etamin = etamin;
  this->etamax = etamax;
  this->isMC = ismc;

  this->hibinmin = hibinmin;
  this->hibinmax = hibinmax;

// Weight vs reco pT profile? scatter plot? reco pt vs weight and gen pt vs. weight?


// Jet histograms
// TODO: UPDATE TO JERC BINS
  
  jet_pt = new TH1D("reco jet pT", "reco jet p_{T}; reco jet p_{T};", 100, 15, 1000);
  jet_uncorr_pt = new TH1D("reco jet pT uncorr", "reco jet p_{T}; reco jet p_{T};", 100, 15, 1000);
  jet_pt_genweight = new TH1D("reco jet pT, gen w", "reco jet p_{T}, gen weight only; reco jet p_{T};", 100, 100, 1000);
  jet_eta = new TH1D("reco jet eta"," reco jet #eta; reco jet #eta;", 40, -5.2, 5.2);
  jet_phi = new TH1D("reco jet phi"," reco jet #phi; reco jet #phi;", 25, -3.1415926535, 3.1415926535);



  // PF composition
  jet_nhf = new TProfile("reco jet nhf", "reco jet nhf; reco jet p_{T};", 100, 15, 1000);
  jet_chf = new TProfile("reco jet chf", "reco jet chf; reco jet p_{T};", 100, 15, 1000);
  jet_nef = new TProfile("reco jet nef", "reco jet nef; reco jet p_{T};", 100, 15, 1000); 
  jet_cef = new TProfile("reco jet cef", "reco jet cef; reco jet p_{T};", 100, 15, 1000);
  jet_muf = new TProfile("reco jet muf", "reco jet muf; reco jet p_{T};", 100, 15, 1000);


  jetetaphi = new TH2D("eta-phi distribution",";#eta; #phi",netas,etarange,nphis,phirange);
  
  if (ismc) {

    genjet_pt = new TH1D("gen jet pT", "gen jet p_{T}; gen jet p_{T};", 100, 100, 1000);
    genjet_eta = new TH1D("gen jet eta"," gen jet #eta; gen jet #eta;", 20, -2.5, 2.5);
    genjet_phi = new TH1D("gen jet phi"," gen jet #phi; gen jet #phi;", 20, -2.5, 2.5);


// Residuals
    jetresponse = new TProfile("response","",100,100,1000);

// Definition: (reco-true)/true
    ptres = new TH1D("pT res"," pT ; (pT_reco-pT_gen)/pT_gen;", 40, -2, 2);
 
  }

  // Dijets
  dijetasymmetry = new TH1D("dijetasymmetry","  ; asymmetry;", 40, 0, 1);
  dijetdeltaphi  = new TH1D("dijetdeltaphi"," ; delta phi;", 40, 0, 3.1415926535);
  dijetdeltaeta  = new TH1D("dijetdeltaeta"," ; delta eta;", 40, 0, 5.2);

  dijetbalance_a03 = new TH1D("dijetbalance_a03","  ; ;", 20, -2, 2);
  dijetasymmetry_a03 = new TProfile("dijetasymmetry_a03","  ; ;",  nptforjec, &ptforjec[0]);
  dijetbalance_a1= new TH1D("dijetbalance_a1","  ; ;", 20, -2, 2);
  dijetasymmetry_a1 = new TProfile("dijetasymmetry_a1","  ; ;",  nptforjec, &ptforjec[0]);




  // Add: dijet respone in different bins?

  ptgenvsptreco = new TH2D("ptgenvsptreco","",200,0,1500,200,0,1500);
  ptrecovsweight = new TH2D("ptrecovsweight","",200,0,1500,200,0,0.1);
  ptgenvsweight = new TH2D("ptgenvsweight","",200,0,1500,200,0,0.1);


// JES related controls etc


}


void histograms::Write() {
  dir->cd();
  dir->Write();
  }
