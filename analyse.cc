#include <iostream>
using std::cout;
using std::endl;

#include "TRandom.h"
#include "TH1D.h"
#include "TCanvas.h"
#include <iterator>
#include "TMath.h"
#include <cmath>
#include <cstdio>
#include <ctime>
#include "THStack.h"
#include "TFile.h"
#include "TLegend.h"
#include <typeinfo>
#include "TAxis.h"
#include "TTree.h"


#include "histograms.h"
// #include "tables.h"
#include "settings.h"
#include "eventhistograms.h"
#include "helpers.h"

// TODO: if local etc
// #include "JetMETCorrections/Modules/interface/JetResolution.h"

#if REDOJES == 1
#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"
#endif

// Need to apply MCTruth correction on the fly

map<string, vector<histograms*> > _histos;

// For JER
/* double _rho = 20; // TODO: from ntuples
JME::JetResolution *_jer(0);
JME::JetResolutionScaleFactor *_jer_sf(0); */

// 2023ppwithpbpb_MC_L2Relative_AK4PF.txt  2023ppwithpbpb_old_MC_L2Relative_AK4PF.txt  2023ppwithpp_MC_L2Relative_AK4PF.txt  2023ppwithpp_old_MC_L2Relative_AK4PF.txt
//void analyse(string inFileName = "/cmshome/martikai/data/run3_pprefpbpbreco_data_05072024.root", string outputfilename = "pbpbreco_witholdmctruth.root", string jecfile = "jecfiles/2023ppwithpbpb_old_MC_L2Relative_AK4PF.txt", bool isMC = false) {

  //void analyse(string inFileName = "testdata/run3_ppref_data_04062024.root", string outputfilename = "testoutput.root", string jecfile = "jecfiles/2023ppwithpp_old_MC_L2Relative_AK4PF.txt", bool isMC = false) {

//void analyse(string inFileName = "testdata/run3_pprefpbpbreco_data_05072024.root", string outputfilename = "testsmallbins.root", string jecfile = "jecfiles/2023ppwithpp_old_MC_L2Relative_AK4PF.txt", bool isMC = false) {

// void analyse(string inFileName = "/cmshome/martikai/data/run3_ppref_data_04062024.root", string outputfilename = "testoutput.root", string jecfile = "jecfiles/2023ppwithpp_old_MC_L2Relative_AK4PF.txt", bool isMC = false) {


//void analyse(string inFileName = "/home/laura/Data/jecmc/MC_pprefwpbpbreco_privateforjec.root", string outputfilename = "pbpbreco_MC.root", string jecfile = "jecfiles/2023ppwithpbpb_old_MC_L2Relative_AK4PF.txt", bool isMC = true) {

// void analyse(string inFileName = "/home/laura/Data/jecmc/MC_ppref_privateforjec.root", string outputfilename = "ppreco_MC_fixabsetas.root", string jecfile = "jecfiles/2023ppwithpp_old_MC_L2Relative_AK4PF.txt", bool isMC = true) {
// void analyse(string inFileName = "/home/laura/Data/jecmc/MC_ppref_privateforjec.root", string outputfilename = "ppreco_MC_test3D.root", string jecfile = "jecfiles/2023ppwithpp_old_MC_L2Relative_AK4PF.txt", bool isMC = true) {


// LXPLUS
// DATA 
//void analyse(string inFileName = "/eos/user/l/lamartik/run3_pprefpbpbreco_data_05072024.root", string outputfilename = "/eos/user/l/lamartik/HIJEC_results/rebin/pbpbreco_DATA_wideeta_lxplus.root", string jecfile = "jecfiles/2023ppwithpbpb_old_MC_L2Relative_AK4PF.txt", bool isMC = false) {
//void analyse(string inFileName = "/eos/user/l/lamartik/run3_ppref_data_04062024.root", string outputfilename = "/eos/user/l/lamartik/HIJEC_results/rebin/ppreco_DATA_lxplus.root", string jecfile = "jecfiles/2023ppwithpp_old_MC_L2Relative_AK4PF.txt", bool isMC = false) {

  // MC
void analyse(string inFileName = "/eos/cms/store/group/phys_heavyions/lamartik/tuples/MC_pprefwpbpbreco_privateforjec.root", string outputfilename = "/eos/user/l/lamartik/HIJEC_results/rebin/pbpbreco_MC_wideeta_lxplus.root", string jecfile = "jecfiles/2023ppwithpbpb_old_MC_L2Relative_AK4PF.txt", bool isMC = true) {
//void analyse(string inFileName = "/eos/cms/store/group/phys_heavyions/lamartik/tuples/MC_ppref_privateforjec.root", string outputfilename = "/eos/user/l/lamartik/HIJEC_results/rebin/ppreco_MC_lxplus.root", string jecfile = "jecfiles/2023ppwithpp_old_MC_L2Relative_AK4PF.txt", bool isMC = true) {


//void analyse(string inFileName = "/home/laura/Data/jecmc/MC_ppref_privateforjec.root", string outputfilename = "ppreco_MC_test3D.root", string jecfile = "jecfiles/2023ppwithpp_old_MC_L2Relative_AK4PF.txt", bool isMC = true) {
  
  TRandom3 r;
  
  // Define and activate branches
  std::string evtPath = "hiEvtAnalyzer/HiTree";
  std::string triggerPath = "hltanalysis/HltTree";
  std::string skimPath = "skimanalysis/HltTree";
  std::string jetPath = "ak4PFJetAnalyzer/t";

  cout << "Opening input file" << endl;
  TFile *inFile = new TFile(inFileName.c_str(), "READ"); // TODO: safety checks about opening file successfully

  auto evtTree = (TTree*)inFile->Get(evtPath.c_str());
  
  // Cuts and weights from event tree
  Int_t       hiBin = 0;
  Float_t     weight = 1;
  Float_t     vz = 0;
  Float_t     pthat = 0;
  Float_t     evtwt = 1;
  
  evtTree->SetBranchAddress("hiBin", &hiBin);
  evtTree->SetBranchAddress("vz", &vz);
  if (isMC) evtTree->SetBranchAddress("weight", &weight);
  if (isMC) evtTree->SetBranchAddress("pthat", &pthat);

  evtTree->SetBranchStatus("*",0);
  evtTree->SetBranchStatus("hiBin",1);
  evtTree->SetBranchStatus("vz",1);
  if (isMC) evtTree->SetBranchStatus("weight",1);
  if (isMC) evtTree->SetBranchStatus("pthat",1);

  //// EVENT FILTERS - CHECK
  auto skimTree = (TTree*)inFile->Get(skimPath.c_str());
  if (!isMC) skimTree->SetBranchStatus("*",1);   // Temporary fix because f'd up tuples

  // Int_t pclusterCompatibilityFilter = 1;
  // Int_t pphfCoincFilter2Th4 = 1;

  Int_t pprimaryVertexFilter = 1;

  // skimTree->SetBranchAddress("pclusterCompatibilityFilter", &pclusterCompatibilityFilter);
  if (!isMC) skimTree->SetBranchAddress("pprimaryVertexFilter", &pprimaryVertexFilter);

  // if (!isMC)  skimTree->SetBranchAddress("pphfCoincFilter2Th4", &pphfCoincFilter2Th4); // MC test tuple does not have this */ 

  //// TRIGGERS: CHECK, add rest of jet triggers
  // 60, 80, 100, 120 are unprescaled - also 70?
  
  Int_t trigger = 0;
  bool usecalotrig = true;


  Int_t HLT_AK4PFJet60_v1, HLT_AK4PFJet80_v1, HLT_AK4PFJet100_v1, HLT_AK4PFJet120_v1;
  Int_t HLT_AK4CaloJet60_v1, HLT_AK4CaloJet80_v1, HLT_AK4CaloJet100_v1, HLT_AK4CaloJet120_v1;
  // In MC with PbPb different trigger setting
  Int_t HLT_HIPuAK4CaloJet60_Eta5p1_MinBiasHF1AND_v2;
  
  auto triggerTree = (TTree*)inFile->Get(triggerPath.c_str());

  if (!ismc) {
    triggerTree->SetBranchAddress("HLT_AK4PFJet60_v1",&HLT_AK4PFJet60_v1); 
    triggerTree->SetBranchAddress("HLT_AK4PFJet80_v1",&HLT_AK4PFJet80_v1); 
    triggerTree->SetBranchAddress("HLT_AK4PFJet100_v1",&HLT_AK4PFJet100_v1);
    triggerTree->SetBranchAddress("HLT_AK4PFJet120_v1",&HLT_AK4PFJet120_v1);
  
    triggerTree->SetBranchAddress("HLT_AK4CaloJet60_v1",&HLT_AK4CaloJet60_v1); 
    triggerTree->SetBranchAddress("HLT_AK4CaloJet80_v1",&HLT_AK4CaloJet80_v1); 
    triggerTree->SetBranchAddress("HLT_AK4CaloJet100_v1",&HLT_AK4CaloJet100_v1);
    triggerTree->SetBranchAddress("HLT_AK4CaloJet120_v1",&HLT_AK4CaloJet120_v1);
  }
  else {
    triggerTree->SetBranchAddress("HLT_AK4PFJet60_v1",&HLT_HIPuAK4CaloJet60_Eta5p1_MinBiasHF1AND_v2); 
  }
      


  triggerTree->SetBranchStatus("*",0);
  if (!usecalotrig) {  cout << "Use PF triggers" << endl;

    triggerTree->SetBranchStatus("HLT_AK4PFJet60_v1",1);
    triggerTree->SetBranchStatus("HLT_AK4PFJet80_v1",1);
    triggerTree->SetBranchStatus("HLT_AK4PFJet100_v1",1);
    triggerTree->SetBranchStatus("HLT_AK4PFJet120_v1",1);
  
   }
   else {
    cout << "Use Calo triggers" << endl;

    triggerTree->SetBranchStatus("HLT_AK4CaloJet60_v1",1);
    triggerTree->SetBranchStatus("HLT_AK4CaloJet80_v1",1);
    triggerTree->SetBranchStatus("HLT_AK4CaloJet100_v1",1);
    triggerTree->SetBranchStatus("HLT_AK4CaloJet120_v1",1);
   }

  // Get to JETS 
  auto jetTree = (TTree*)inFile->Get(jetPath.c_str());
  jetTree->SetBranchStatus("*",1);    

  Int_t     evt;
  
  // Reconstruted jet information
  Int_t     nref;
  Float_t   jtpt[MAXJETS];
  Float_t   jtpt_uncorr[MAXJETS];
  Float_t   jteta[MAXJETS];
  Float_t   jtphi[MAXJETS];

  Float_t   jtnhf[MAXJETS];
  Float_t   jtchf[MAXJETS];
  Float_t   jtnef[MAXJETS];
  Float_t   jtcef[MAXJETS];
  Float_t   jtmuf[MAXJETS];

  Float_t   jtdyn_kt[MAXJETS];
  Float_t   jtdyn_deltaR[MAXJETS];
  Float_t   jtdyn_z[MAXJETS];
 
  jetTree->SetBranchAddress("evt", &evt);
  jetTree->SetBranchAddress("nref", &nref);
  jetTree->SetBranchAddress("jtpt", &jtpt);
  jetTree->SetBranchAddress("jteta", &jteta);
  jetTree->SetBranchAddress("jtphi", &jtphi);

  jetTree->SetBranchAddress("jtPfNHF", &jtnhf);
  jetTree->SetBranchAddress("jtPfCHF", &jtchf);
  jetTree->SetBranchAddress("jtPfNEF", &jtnef);
  jetTree->SetBranchAddress("jtPfCEF", &jtcef);
  jetTree->SetBranchAddress("jtPfMUF", &jtmuf);
  

  Float_t leadpt = 0;
  Float_t subleadpt = 0;
  Float_t leadeta = 0;
  Float_t subleadeta = 0;
  Float_t dphi = 0;
  Float_t ddeta = 0;
  Float_t djetasymm = 0;
  Float_t avgpt = 0;

  // Gen level jet information
  Float_t   jtpt_gen[MAXJETS];
  Float_t   jteta_gen[MAXJETS];
  Float_t   jtphi_gen[MAXJETS];


  if (isMC) {
    jetTree->SetBranchAddress("refpt", &jtpt_gen);
    jetTree->SetBranchAddress("refeta", &jteta_gen);
    jetTree->SetBranchAddress("refphi", &jtphi_gen);
  
  }

  // TODO: rho?

  TFile *outfile = new TFile(outputfilename.c_str(),"RECREATE");

  for (int j = 0; j < nhibins; ++j) { 
  
      if (hibins[j] < hibins[j+1]) {
	string name = Form("hibin_%.1f_%.1f",hibins[j],hibins[j+1]);
	outfile->mkdir(name.c_str());
	
	TDirectory *dir = outfile->GetDirectory(name.c_str()); assert(dir);
	dir->cd();
    
	for (int i = 0; i < netabins ; ++i) { 
	  if (etaedges[i] < etaedges[i+1]) {
	    string name2 = Form("eta_%.1f_%.1f",etaedges[i],etaedges[i+1]);
	    dir->mkdir(name2.c_str());
	
	    TDirectory *dir2 = dir->GetDirectory(name2.c_str()); assert(dir2);
	    dir2->cd();
	    
	    histograms *h = new histograms(dir2, etaedges[i], etaedges[i+1], hibins[j],hibins[j+1], isMC);
	    _histos[name2.c_str()].push_back(h);
	  }
	}

      }
}

  outfile->mkdir("event");
  TDirectory *dir = outfile->GetDirectory("event"); assert(dir);
  dir->cd();

  eventhistograms *eh = new eventhistograms(dir, isMC);
   
  // JEC stuff
#if REDOJES == 1  
FactorizedJetCorrector* corr;
vector<JetCorrectorParameters> vpar; 
 vpar.push_back(JetCorrectorParameters(jecfile.c_str()));			 

 corr = new FactorizedJetCorrector(vpar);
#endif

 // jetTree->Print();
// Start event loop to fill histograms:
   cout << "Number of entries :" <<  jetTree->GetEntries()  << endl; 

for (int i = 0; i < jetTree->GetEntries(); ++i) {
  //for (int i = 0; i < 100000; ++i) {
     evtTree->GetEntry(i);
  
     triggerTree->GetEntry(i);

     if (usecalotrig) trigger = (HLT_AK4CaloJet60_v1 or HLT_AK4CaloJet80_v1 or HLT_AK4CaloJet100_v1 or HLT_AK4CaloJet120_v1);
     else trigger = (HLT_AK4PFJet60_v1 or HLT_AK4PFJet80_v1 or HLT_AK4PFJet100_v1 or HLT_AK4PFJet120_v1);
     if (isMC) trigger = true; // TEMPORARY FIX
     
     if (!trigger) continue;

     evtwt = 1;
     if (isMC) {
       evtwt *=  weight;
       //       evtwt *=  findNcoll(hiBin); // Centrality
     }
     //   cout << weight << " " << evtwt << endl;
          
     // BASIC EVENT FILTERS
     if (!isMC) {
       skimTree->GetEntry(i);
       if (pprimaryVertexFilter != 1) continue; // TODO: test, for some reason gets always 0 but should be 1
     //     if (hiBin > CENTRALITYHIGH or hiBin < CENTRALITYLOW) continue;
     }
     jetTree->GetEntry(i);

     if (nref < 1) continue;
     
     //MC: JER resmear? -> needs rho

     // _jer = new JME::JetResolution(resolutionFile);
     // _jer_sf = new JME::JetResolutionScaleFactor(scaleFactorFile);

     // Event histograms
     // TODO: Do we need event info e.g. after and before reweights? Likely?
     eh->event_vz->Fill(vz, evtwt);
     //     if (weight > 0.1) cout << weight << " " << evtwt << endl;
     if (isMC)  eh->event_pthatwsgenweight->Fill(pthat,weight);

     // This is dijet with tag and probe

     double tagpt, probept, tageta, probeeta, pt3, ptavgtp, alpha;
     double asymmtp;
     double djrespasymm;

     // Apply MCtruth JEC

     for (int j = 0; j < nref; ++j ) {
      	 jtpt_uncorr[j] = jtpt[j];
         #if REDOJES == 1
	 // REDO JEC
	 // cout << "Applying JES" << endl;
	 corr->setJetPt(jtpt[j]);
	 // corr->setJetE(jteu[jetidx]);
	 corr->setJetEta(jteta[j]);

	 vector<float> v = corr->getSubCorrections();
	 float jes = v.back();

	 //	 cout << "New jes correction: " << jtpt[j] << " " << j << " "  << jes << endl;
	 jtpt[j] *= jes;
#endif
     }

    // Get dijet system (do not impose any cuts here)
     if (nref > 1) {
       dphi = DPhi(jtphi[0],jtphi[1]);
       leadpt = jtpt[0];
       subleadpt = jtpt[1];
       leadeta = jteta[0];
       subleadeta = jteta[1];
       ddeta = abs(jteta[0]-jteta[1]);
       avgpt = 0.5*(leadpt+subleadpt);
       djetasymm = (leadpt-subleadpt)/(leadpt+subleadpt);
     }     

     //     if (nref > 1 and doTPdijet) {
     if (nref > 1) {
    
       int tagind = -1;
       // Here was a bug; absolute values were missing; check effect
       if (abs(jteta[0]) > 1.3 and abs(jteta[1]) <= 1.3)  tagind = 1;
       else if (abs(jteta[1]) > 1.3 and abs(jteta[0]) <= 1.3)  tagind = 0;
       else if (abs(jteta[0]) <= 1.3 and abs(jteta[1]) <= 1.3)  {
	 const auto rand = r.Rndm();
	 if (rand < 0.5) tagind = 1;
	 else tagind = 0;
       }

       int probeind = 1-tagind;
       tagpt = jtpt[tagind];
       probept = jtpt[probeind];

       tageta = jteta[tagind];
       probeeta = jteta[probeind];

       float dphitp = DPhi(jtphi[tagind],jtphi[probeind]);


       //    for (int j = 0; j < 2; ++j) {   // Use both jets as t/b in turn, change
	 // Change to: if on of jets is in the barrel, use
	 // if both jets in barrel, do randomly
	 
	 //tagpt = jtpt[j];
	 //probept = jtpt[(j == 0 ? 1 : 0)];
	 //tageta = jteta[j];
	 
	 //if (abs(tageta) > 1.3) continue;
	 
       // probeeta = jteta[(j == 0 ? 1 : 0)];
	 
	 ptavgtp = 0.5*(tagpt  + probept);
	 asymmtp = probept - tagpt;
	 
	 if (nref > 2) alpha = jtpt[2]/ptavgtp; 
	 else alpha = 0; // In case only two jets
	 
	 // Fill in average pT, eta bin from probeeta
	 
	 for (auto &histrange : _histos) {    // DPhi?
	   for (auto &h : histrange.second) {
	     
	     if (probeeta >= h->etamin and probeeta < h->etamax and hiBin >= h->hibinmin and hiBin < h->hibinmax and dphitp > 2.7) {

	       // This is the full eta range, actual derivation
	       if ((h->etamin - h->etamax) < -10) {

		 if (alpha < 0.1)   {
		   h->dijetasymmetry3D->Fill(ptavgtp, probeeta, 0.1-0.0001, asymmtp/2./ptavgtp);
		   h->dijetasymmetry3Dabseta->Fill(ptavgtp, abs(probeeta), 0.1-0.0001, asymmtp/2./ptavgtp);

		   h->dijetasymmetry3Dnarrow->Fill(ptavgtp, probeeta, 0.1-0.0001, asymmtp/2./ptavgtp);
		   h->dijetasymmetry3Dabsetanarrow->Fill(ptavgtp, abs(probeeta), 0.1-0.0001, asymmtp/2./ptavgtp);
		   h->dijetasymmetry3Dabsetawide->Fill(ptavgtp, abs(probeeta), 0.1-0.0001, asymmtp/2./ptavgtp);

		   h->dijetasymmetry2D_a01->Fill(ptavgtp, probeeta, asymmtp/2./ptavgtp);
		 }

		 if (alpha < 0.15) {
		   h->dijetasymmetry3D->Fill(ptavgtp, probeeta, 0.15-0.0001, asymmtp/2./ptavgtp);
		   h->dijetasymmetry3Dabseta->Fill(ptavgtp, abs(probeeta), 0.15-0.0001, asymmtp/2./ptavgtp);

		   h->dijetasymmetry3Dnarrow->Fill(ptavgtp, probeeta, 0.15-0.0001, asymmtp/2./ptavgtp);
		   h->dijetasymmetry3Dabsetanarrow->Fill(ptavgtp, abs(probeeta), 0.15-0.0001, asymmtp/2./ptavgtp);
		   h->dijetasymmetry3Dabsetawide->Fill(ptavgtp, abs(probeeta), 0.15-0.0001, asymmtp/2./ptavgtp);
}
		 if (alpha < 0.2)  {
		   h->dijetasymmetry3D->Fill(ptavgtp, probeeta, 0.2-0.0001, asymmtp/2./ptavgtp);
		   h->dijetasymmetry3Dabseta->Fill(ptavgtp, abs(probeeta), 0.2-0.0001, asymmtp/2./ptavgtp);

		   h->dijetasymmetry3Dnarrow->Fill(ptavgtp, probeeta, 0.2-0.0001, asymmtp/2./ptavgtp);
		   h->dijetasymmetry3Dabsetanarrow->Fill(ptavgtp, abs(probeeta), 0.2-0.0001, asymmtp/2./ptavgtp);
		   h->dijetasymmetry3Dabsetawide->Fill(ptavgtp, abs(probeeta), 0.2-0.0001, asymmtp/2./ptavgtp);

		   h->dijetasymmetry2D_a02->Fill(ptavgtp, probeeta, asymmtp/2./ptavgtp);
		 }

		 if (alpha < 0.25) {
		   h->dijetasymmetry3D->Fill(ptavgtp, probeeta, 0.25-0.0001, asymmtp/2./ptavgtp);
		   h->dijetasymmetry3Dabseta->Fill(ptavgtp, abs(probeeta), 0.25-0.0001, asymmtp/2./ptavgtp);

		   h->dijetasymmetry3Dnarrow->Fill(ptavgtp, probeeta, 0.25-0.0001, asymmtp/2./ptavgtp);
		   h->dijetasymmetry3Dabsetanarrow->Fill(ptavgtp, abs(probeeta), 0.25-0.0001, asymmtp/2./ptavgtp);
		   h->dijetasymmetry3Dabsetawide->Fill(ptavgtp, abs(probeeta), 0.25-0.0001, asymmtp/2./ptavgtp);

		 }
		 if (alpha < 0.3)  {
		   h->dijetbalance_a03->Fill(asymmtp/2./ptavgtp);
		   h->dijetasymmetry2D_a03->Fill(ptavgtp, probeeta, asymmtp/2./ptavgtp);
		   h->dijetasymmetry_a03->Fill(ptavgtp, asymmtp/2./ptavgtp);

		   h->dijetasymmetry3D->Fill(ptavgtp, probeeta, 0.3-0.0001, asymmtp/2./ptavgtp);
		   h->dijetasymmetry3Dabseta->Fill(ptavgtp, abs(probeeta), 0.3-0.0001, asymmtp/2./ptavgtp);

		   h->dijetasymmetry3Dnarrow->Fill(ptavgtp, probeeta, 0.3-0.0001, asymmtp/2./ptavgtp);
		   h->dijetasymmetry3Dabsetanarrow->Fill(ptavgtp, abs(probeeta), 0.3-0.0001, asymmtp/2./ptavgtp);
		   h->dijetasymmetry3Dabsetawide->Fill(ptavgtp, abs(probeeta), 0.3-0.0001, asymmtp/2./ptavgtp);
}
		 //else if (alpha < 0.35)   h->dijetasymmetry2D_a035->Fill(ptavgtp, probeeta, asymmtp/2./ptavgtp);
		 if (alpha < 0.35) {
		   h->dijetasymmetry3D->Fill(ptavgtp, probeeta, 0.35-0.0001, asymmtp/2./ptavgtp);
		   h->dijetasymmetry3Dabseta->Fill(ptavgtp, abs(probeeta), 0.35-0.0001, asymmtp/2./ptavgtp);

		   h->dijetasymmetry3Dnarrow->Fill(ptavgtp, probeeta, 0.35-0.0001, asymmtp/2./ptavgtp);
		   h->dijetasymmetry3Dabsetanarrow->Fill(ptavgtp, abs(probeeta), 0.35-0.0001, asymmtp/2./ptavgtp);
		   h->dijetasymmetry3Dabsetawide->Fill(ptavgtp, abs(probeeta), 0.35-0.0001, asymmtp/2./ptavgtp);

		 }
		 if (alpha < 0.4)   {
		   h->dijetasymmetry2D_a04->Fill(ptavgtp, probeeta, asymmtp/2./ptavgtp);
		   h->dijetasymmetry3D->Fill(ptavgtp, probeeta, 0.4-0.0001, asymmtp/2./ptavgtp); // These should match with bins?
		   h->dijetasymmetry3Dabseta->Fill(ptavgtp, abs(probeeta), 0.4-0.0001, asymmtp/2./ptavgtp);

		   h->dijetasymmetry3Dnarrow->Fill(ptavgtp, probeeta, 0.4-0.0001, asymmtp/2./ptavgtp); // These should match with bins?
		   h->dijetasymmetry3Dabsetanarrow->Fill(ptavgtp, abs(probeeta), 0.4-0.0001, asymmtp/2./ptavgtp);
		   h->dijetasymmetry3Dabsetawide->Fill(ptavgtp, abs(probeeta), 0.4-0.0001, asymmtp/2./ptavgtp);
		 }

		 if (alpha < 0.45) {
		   h->dijetasymmetry3D->Fill(ptavgtp, probeeta, 0.45-0.0001, asymmtp/2./ptavgtp);
		   h->dijetasymmetry3Dabseta->Fill(ptavgtp, abs(probeeta), 0.45-0.0001, asymmtp/2./ptavgtp);

		   h->dijetasymmetry3Dnarrow->Fill(ptavgtp, probeeta, 0.45-0.0001, asymmtp/2./ptavgtp);
		   h->dijetasymmetry3Dabsetanarrow->Fill(ptavgtp, abs(probeeta), 0.45-0.0001, asymmtp/2./ptavgtp);
		   h->dijetasymmetry3Dabsetawide->Fill(ptavgtp, abs(probeeta), 0.45-0.0001, asymmtp/2./ptavgtp);
		 }
		 
		 if (alpha < 0.5) {
		   h->dijetasymmetry2D_a05->Fill(ptavgtp, probeeta, asymmtp/2./ptavgtp);
		   h->dijetasymmetry3D->Fill(ptavgtp, probeeta, 0.5-0.0001, asymmtp/2./ptavgtp); // These should match with bins?
		   h->dijetasymmetry3Dabseta->Fill(ptavgtp, abs(probeeta), 0.5-0.0001, asymmtp/2./ptavgtp);

		   h->dijetasymmetry3Dnarrow->Fill(ptavgtp, probeeta, 0.5-0.0001, asymmtp/2./ptavgtp); // These should match with bins?
		   h->dijetasymmetry3Dabsetanarrow->Fill(ptavgtp, abs(probeeta), 0.5-0.0001, asymmtp/2./ptavgtp);
		   h->dijetasymmetry3Dabsetawide->Fill(ptavgtp, abs(probeeta), 0.5-0.0001, asymmtp/2./ptavgtp);

		 }
		 if (alpha < 0.6) {
		   h->dijetasymmetry2D_a06->Fill(ptavgtp, probeeta, asymmtp/2./ptavgtp);
		   h->dijetasymmetry3D->Fill(ptavgtp, probeeta, 0.6-0.0001, asymmtp/2./ptavgtp); // These should match with bins?
		   h->dijetasymmetry3Dabseta->Fill(ptavgtp, abs(probeeta), 0.6-0.0001, asymmtp/2./ptavgtp);
		   h->dijetasymmetry3Dabsetawide->Fill(ptavgtp, abs(probeeta), 0.6-0.0001, asymmtp/2./ptavgtp);
		 }
	       }
		 // Second: alpha < 1
		 h->dijetbalance_a1->Fill(asymmtp/2./ptavgtp);
		 h->dijetasymmetry_a1->Fill(ptavgtp, asymmtp/2./ptavgtp);
	     }
	   }
	 } 
	 
	 //       }    
     }

	 for (int j = 0; j < nref; ++j ) {


	 for (auto &histrange : _histos) { ///// etabins instead of pts?
	   for (auto &h : histrange.second) {
	     if (jteta[j] >= h->etamin and jteta[j] < h->etamax and hiBin >= h->hibinmin and hiBin < h->hibinmax) {
	       h->jetetaphi->Fill(jteta[j],jtphi[j],weight);


	       if (j == 0) {
		 // Trigger checks; TODO: what all to  fill?
		 if (HLT_AK4CaloJet60_v1) h->HLT60->Fill(jtpt[0],evtwt);
		 if (HLT_AK4CaloJet80_v1) h->HLT80->Fill(jtpt[0],evtwt);
		 if (HLT_AK4CaloJet100_v1) h->HLT100->Fill(jtpt[0],evtwt);
		 if (HLT_AK4CaloJet120_v1) h->HLT120->Fill(jtpt[0],evtwt);
		 if (!HLT_AK4CaloJet60_v1 and !HLT_AK4CaloJet80_v1 and !HLT_AK4CaloJet100_v1 and HLT_AK4CaloJet120_v1) h->HLT60vs120->Fill(jtpt[0],evtwt);
		 if (!HLT_AK4CaloJet60_v1 and !HLT_AK4CaloJet80_v1 and HLT_AK4CaloJet100_v1 and !HLT_AK4CaloJet120_v1) h->HLT60vs100->Fill(jtpt[0],evtwt);
		 if (!HLT_AK4CaloJet60_v1 and HLT_AK4CaloJet80_v1 and !HLT_AK4CaloJet100_v1 and !HLT_AK4CaloJet120_v1) h->HLT60vs80->Fill(jtpt[0],evtwt);
		   
	       }
	   
	     //      if (isMC and weight > 0.001) continue; // TODO: study this more?

	       if (j == 0 and nref > 1 and dphi > 2.7) { // Fill dijet system based on leading jet pT
		 h->dijetasymmetry->Fill(djetasymm,evtwt);
		 h->dijetdeltaphi->Fill(dphi,evtwt);
		 h->dijetdeltaeta->Fill(ddeta,evtwt);

	       }
	     
	       h->jet_pt->Fill(jtpt[j],evtwt);
	       h->jet_uncorr_pt->Fill(jtpt_uncorr[j],evtwt);
	       h->jet_pt_genweight->Fill(jtpt[j],weight);
	       h->jet_eta->Fill(jteta[j],evtwt);
	       h->jet_phi->Fill(jtphi[j],evtwt);

	       // Fill without t&p, TODO: add t&p versions
	       h->jet_nef->Fill(jtpt[j],jtnef[j],evtwt);
	       h->jet_cef->Fill(jtpt[j],jtcef[j],evtwt);
	       h->jet_nhf->Fill(jtpt[j],jtnhf[j],evtwt);
	       h->jet_chf->Fill(jtpt[j],jtchf[j],evtwt);
	       h->jet_muf->Fill(jtpt[j],jtmuf[j],evtwt);       

	       //cout << jtnef[j] << endl;


	       if (isMC) {
	     
		 h->genjet_pt->Fill(jtpt_gen[j],evtwt);
		 h->genjet_eta->Fill(jteta_gen[j],evtwt);
		 h->genjet_phi->Fill(jtphi_gen[j],evtwt);
		 
		 h->jetresponse->Fill(jtpt_gen[j],jtpt[j]/jtpt_gen[j],evtwt);
	     
		 h->ptres->Fill((jtpt[j]-jtpt_gen[j])/jtpt_gen[j],evtwt);

		 h->ptgenvsptreco->Fill(jtpt_gen[j],jtpt[j],evtwt);
		 h->ptrecovsweight->Fill(jtpt[j],weight);
		 h->ptgenvsweight->Fill(jtpt_gen[j],weight);
		 
	       }
	     }
	   }
	 }
     }    
  }

  // Write output histograms
  
  for (auto &histrange : _histos) {
    for (auto &h : histrange.second) {
      h->Write();
    }  
  }
  eh->Write();
 
  cout << "Wrote " << outputfilename.c_str() << endl;

 }
