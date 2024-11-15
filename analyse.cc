
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
#include "settings.h"
#include "eventhistograms.h"
#include "helpers.h"

// TODO: if local etc
// #include "JetMETCorrections/Modules/interface/JetResolution.h"

#if REDOJES == 1
#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"
#endif

map<string, vector<histograms*> > _histos;

// For JER
/* JME::JetResolution *_jer(0);
JME::JetResolutionScaleFactor *_jer_sf(0); */

// 2023ppwithpbpb_MC_L2Relative_AK4PF.txt  2023ppwithpbpb_old_MC_L2Relative_AK4PF.txt  2023ppwithpp_MC_L2Relative_AK4PF.txt  2023ppwithpp_old_MC_L2Relative_AK4PF.txt

//void analyse(string inFileName = "testdata/run3_pprefpbpbreco_data_05072024.root", string outputfilename = "testsmallbins.root", string jecfile = "jecfiles/2023ppwithpp_old_MC_L2Relative_AK4PF.txt", bool isMC = false) {

//void analyse(string inFileName = "/home/laura/Data/jecmc/MC_pprefwpbpbreco_privateforjec.root", string outputfilename = "pbpbreco_MC.root", string jecfile = "jecfiles/2023ppwithpbpb_old_MC_L2Relative_AK4PF.txt", bool isMC = true) {

// LXPLUS
// DATA - HARD PROBES
void analyse(string inFileName = "/eos/user/l/lamartik/run3_pprefpbpbreco_data_05072024.root", string outputfilename = "/eos/user/l/lamartik/HIJEC_results/debugged_plus_JER/trigstudy_hardprobes_pf.root", string jecfile = "jecfiles/2023ppwithpbpb_old_MC_L2Relative_AK4PF.txt", string l2file = "jecfiles/L2residual_2023PbPb.txt", bool isMC = false, bool iszb = false) {

  // DATA - ZB 
//void analyse(string inFileName = "/eos/user/l/lamartik/HIJEC_ZEROBIAS_TUPLES/zerobias0pbpb.root", string outputfilename = "/eos/user/l/lamartik/HIJEC_results/debugged_plus_JER/trigstudy_zerobias0.root", string jecfile = "jecfiles/2023ppwithpbpb_old_MC_L2Relative_AK4PF.txt", string l2file = "jecfiles/L2residual_2023PbPb.txt", bool isMC = false, bool iszb = true) {

  
  //void analyse(string inFileName = "/eos/user/l/lamartik/run3_ppref_data_04062024.root", string outputfilename = "/eos/user/l/lamartik/HIJEC_results/rebin/ppreco_DATA_lxplus.root", string jecfile = "jecfiles/2023ppwithpp_old_MC_L2Relative_AK4PF.txt", bool isMC = false) {

  // MC
//void analyse(string inFileName = "/eos/cms/store/group/phys_heavyions/lamartik/tuples/MC_pprefwpbpbreco_privateforjec.root", string outputfilename = "/eos/user/l/lamartik/HIJEC_results/debugged_plus_JER/pbpbreco_MC_JERHCALbinningHF.root", string jecfile = "jecfiles/2023ppwithpbpb_old_MC_L2Relative_AK4PF.txt", bool isMC = true) {

  bool usecalotrig = false;
  bool checkvalidjet = false; // this is for checking valid jet range. now for tightly limited range.TODO: do something smarter  

  TRandom3 r;
  // Define and activate branches
  std::string evtPath = "hiEvtAnalyzer/HiTree";
  std::string triggerPath = "hltanalysis/HltTree";
  std::string skimPath = "skimanalysis/HltTree";
  std::string jetPath = "ak4PFJetAnalyzer/t";

  cout << "Opening input file" << endl;
  TFile *inFile = new TFile(inFileName.c_str(), "READ"); // TODO: safety checks about opening file successfully
  cout << "Opened" << endl;
  auto evtTree = (TTree*)inFile->Get(evtPath.c_str());
  
  // Cuts and weights from event tree
  Int_t       hiBin = -1;
  Float_t     weight = 1;
  Float_t     vz = 0;
  Float_t     pthat = 0;
  Float_t     evtwt = 1;

  Float_t jtptmin = 10.;
  
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
  if (!isMC) skimTree->SetBranchStatus("*",1); 

  Int_t pprimaryVertexFilter = 1;
  if (!isMC) skimTree->SetBranchAddress("pprimaryVertexFilter", &pprimaryVertexFilter);
  
  Int_t trigger = 0;

  // Triggger paths in the files

  Int_t HLT_ZB, HLT_40, HLT_60, HLT_80, HLT_100, HLT_120;
  // The prescale stuff is at the moment kinda fixed to 2023 data assumption
  Int_t ZBpsnum = 0, ZBpsdenom = 0, ZBL1ps = 0;
  Int_t J40psnum = 0, J40psdenom = 0, J40L1ps = 0;
  
  auto triggerTree = (TTree*)inFile->Get(triggerPath.c_str());
 
    triggerTree->SetBranchAddress("HLT_PPRefZeroBias_v1",&HLT_ZB);

    triggerTree->SetBranchAddress("HLT_PPRefZeroBias_v1_PrescaleNumerator",&ZBpsnum);
    triggerTree->SetBranchAddress("HLT_PPRefZeroBias_v1_PrescaleDenominator",&ZBpsdenom);

    triggerTree->SetBranchAddress("L1_ZeroBias_Prescl",&ZBL1ps);
  
  if (!usecalotrig) {  cout << "Use PF triggers" << endl;
  
    triggerTree->SetBranchAddress("HLT_AK4PFJet40_v1",&HLT_40);

    triggerTree->SetBranchAddress("HLT_AK4PFJet40_v1_PrescaleNumerator",&J40psnum);
    triggerTree->SetBranchAddress("HLT_AK4PFJet40_v1_PrescaleDenominator",&J40psdenom);
    
    triggerTree->SetBranchAddress("HLT_AK4PFJet60_v1",&HLT_60); 
    //    triggerTree->SetBranchAddress("HLT_AK4PFJet80_v1",&HLT_80); 
    triggerTree->SetBranchAddress("HLT_AK4PFJet100_v1",&HLT_100);
    triggerTree->SetBranchAddress("HLT_AK4PFJet120_v1",&HLT_120);
 
    triggerTree->SetBranchStatus("*",0);

    triggerTree->SetBranchStatus("HLT_PPRefZeroBias_v1",1);
    triggerTree->SetBranchStatus("HLT_AK4PFJet40_v1",1);
    triggerTree->SetBranchStatus("HLT_AK4PFJet40_v1_PrescaleNumerator",1);
    triggerTree->SetBranchStatus("HLT_AK4PFJet40_v1_PrescaleDenominator",1);



    triggerTree->SetBranchStatus("HLT_AK4PFJet60_v1",1);
    // triggerTree->SetBranchStatus("HLT_AK4PFJet80_v1",1);
    triggerTree->SetBranchStatus("HLT_AK4PFJet100_v1",1);
    triggerTree->SetBranchStatus("HLT_AK4PFJet120_v1",1);

    triggerTree->SetBranchStatus("HLT_PPRefZeroBias_v1_PrescaleNumerator",1);
    triggerTree->SetBranchStatus("HLT_PPRefZeroBias_v1_PrescaleDenominator",1);

    triggerTree->SetBranchStatus("L1_ZeroBias_Prescl",1);

  }
   else {    cout << "Use Calo triggers" << endl;
     
    triggerTree->SetBranchAddress("HLT_AK4CaloJet40_v1",&HLT_40);
    triggerTree->SetBranchAddress("HLT_AK4CaloJet60_v1",&HLT_60); 
    //triggerTree->SetBranchAddress("HLT_AK4CaloJet80_v1",&HLT_80); 
    triggerTree->SetBranchAddress("HLT_AK4CaloJet100_v1",&HLT_100);
    triggerTree->SetBranchAddress("HLT_AK4CaloJet120_v1",&HLT_120);
 
    triggerTree->SetBranchStatus("*",0);

    triggerTree->SetBranchStatus("HLT_PPRefZeroBias_v1",1);
    triggerTree->SetBranchStatus("HLT_AK4CaloJet40_v1",1);
    triggerTree->SetBranchStatus("HLT_AK4CaloJet60_v1",1);
    //triggerTree->SetBranchStatus("HLT_AK4CaloJet80_v1",1);
    triggerTree->SetBranchStatus("HLT_AK4CaloJet100_v1",1);
    triggerTree->SetBranchStatus("HLT_AK4CaloJet120_v1",1);

    triggerTree->SetBranchStatus("HLT_PPRefZeroBias_v1_PrescaleNumerator",1);
    triggerTree->SetBranchStatus("HLT_PPRefZeroBias_v1_PrescaleDenominator",1);

    triggerTree->SetBranchStatus("L1_ZeroBias_Prescl",1);
    
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
// This is MCTruth
 vpar.push_back(JetCorrectorParameters(jecfile.c_str()));
 // L2 residual
 //if (!isMC) vpar.push_back(JetCorrectorParameters(l2file.c_str()));

 
 corr = new FactorizedJetCorrector(vpar);
#endif

 // jetTree->Print();
 // Start event loop to fill histograms:
   cout << "Number of entries :" <<  jetTree->GetEntries()  << endl; 

 for (int i = 0; i < jetTree->GetEntries(); ++i) {
   // for (int i = 0; i < 10000; ++i) {
     evtTree->GetEntry(i);
     triggerTree->GetEntry(i);

     // TODO: something smarter
     //     if (usecalotrig) trigger = (HLT_AK4CaloJet60_v1 or HLT_AK4CaloJet80_v1 or HLT_AK4CaloJet100_v1 or HLT_AK4CaloJet120_v1);
     // else trigger = (HLT_AK4PFJet60_v1 or HLT_AK4PFJet80_v1 or HLT_AK4PFJet100_v1 or HLT_AK4PFJet120_v1);
     trigger = HLT_ZB or HLT_40 or HLT_60;
     if (isMC) trigger = true; // TEMPORARY FIX

     if (!trigger) continue;
     
     evtwt = 1;
     if (isMC) {
       evtwt *=  weight;
     }
     if (iszb) {
       evtwt *= ZBpsnum;
       evtwt /= ZBpsdenom;
       evtwt *= ZBL1ps;
     }
     // cout << weight << " " << evtwt << endl;
          
     // BASIC EVENT FILTERS
     if (!isMC) {
       skimTree->GetEntry(i);
       if (pprimaryVertexFilter != 1) continue;
     }
     jetTree->GetEntry(i);

     if (nref < 1) continue;
     if (jtpt[0] < jtptmin) continue;
     
     //     cout << "TEST " << nref << " " << jtpt[0] << " " << jtpt[1] << " " << jtpt[2] << endl;
	  
     //MC: JER resmear
     // _jer = new JME::JetResolution(resolutionFile);
     // _jer_sf = new JME::JetResolutionScaleFactor(scaleFactorFile);

     eh->event_vz->Fill(vz, evtwt);
     //     if (weight > 0.1) cout << weight << " " << evtwt << endl;
     if (isMC)  eh->event_pthatwsgenweight->Fill(pthat,weight);

     // This is dijet with tag and probe
     double tagpt, probept, tageta, probeeta, pt3, ptavgtp, alpha;
     double asymmtp;
     double djrespasymm; 

     if (checkvalidjet) {
       for (int j = 0; j < nref; ++j ) {
	 if (abs(jteta[j]) > 2.964) jtpt[j] = 0;   // Always invalid jets
	 
	 else if (abs(jteta[j]) > 2.5 and jtpt[j] > 120) jtpt[j] = 0;  // ?
	 else if (abs(jteta[j]) > 1.93 and jtpt[j] > 170) jtpt[j] = 0;  // ? 
	 
	 if (jtpt[j] < 80) jtpt[j] = 0;
	 // if 80-120 abseta < 2.964
	 // 120-170 < 2.5
	 // 170-1000 < 1.93
       }
     }
 
     // Apply JEC
     for (int j = 0; j < nref; ++j ) {
 	 jtpt_uncorr[j] = jtpt[j];

#if REDOJES == 1	 // REDO JEC
	 // cout << "Applying JES" << endl;
	 corr->setJetPt(jtpt[j]);
	 // corr->setJetE(jteu[jetidx]);
	 corr->setJetEta(jteta[j]);

	 vector<float> v = corr->getSubCorrections();
	 float jes = v.back();

	 //	 cout << "New jes correction jet pt: " << jtpt[j] << " " << jteta[j] << " "  << jes << endl;
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
      
       if (abs(jteta[0]) > 1.3 and abs(jteta[1]) <= 1.3)  tagind = 1;
       else if (abs(jteta[1]) > 1.3 and abs(jteta[0]) <= 1.3)  tagind = 0;
       else if (abs(jteta[0]) <= 1.3 and abs(jteta[1]) <= 1.3)  {
	 const auto rand = r.Rndm();
	 if (rand < 0.5) tagind = 1;
	 else tagind = 0;
       }
       if (jtpt[0] < 1 or jtpt[1] < 1) tagind = -1;

       int probeind = 1-tagind;
       tagpt = jtpt[tagind];
       probept = jtpt[probeind];

       tageta = jteta[tagind];
       probeeta = jteta[probeind];
       //       if (tagpt > 80)       cout << tageta << " " << probeeta << endl;
       float dphitp = DPhi(jtphi[tagind],jtphi[probeind]);

	 ptavgtp = 0.5*(tagpt  + probept);
	 asymmtp = probept - tagpt;
	 
	 if (nref > 2) alpha = jtpt[2]/ptavgtp;
	 else alpha = 0; // In case only two jets
	 
	 // cout << jtpt[0] << " " << jtpt[1] << " " << jtpt[2] << " " << tagpt << endl;
	 //	 if (asymmtp/.2/ptavgtp > 0.9) cout << "probeeta " << probeeta << " tagpt " << tagpt << " ptavg " << ptavgtp << " nref " << nref << endl;
	 // Fill in average pT, eta bin from probeeta
	 
	 for (auto &histrange : _histos) {    // DPhi?
	   for (auto &h : histrange.second) {
	     
	     if (probeeta >= h->etamin and probeeta < h->etamax and hiBin >= h->hibinmin and hiBin < h->hibinmax and dphitp > 2.7  and nref >= 2 and tagind > -1) {

	       // This is the full eta range, actual derivation
	       if ((h->etamin - h->etamax) < -10) {

		 if (HLT_ZB) h->HLTZB_ptav->Fill(ptavgtp, evtwt);
		 if (HLT_40) h->HLT40_ptav->Fill(ptavgtp, evtwt*J40psnum/J40psdenom);
		 if (HLT_60) h->HLT60_ptav->Fill(ptavgtp, evtwt);
		 if (HLT_100) h->HLT100_ptav->Fill(ptavgtp, evtwt);
		 if (HLT_120) h->HLT120_ptav->Fill(ptavgtp, evtwt);
		   
		 h->asymmdist3D->Fill(ptavgtp, abs(probeeta), asymmtp/2./ptavgtp, evtwt);
		 h->absasymmdist3D->Fill(ptavgtp, abs(probeeta), abs(asymmtp/2./ptavgtp), evtwt);

		 if (alpha < 0.1)   {
		   h->asymmdist3D_a10->Fill(ptavgtp, abs(probeeta), asymmtp/2./ptavgtp, evtwt);
		   h->absasymmdist3D_a10->Fill(ptavgtp, abs(probeeta), abs(asymmtp/2./ptavgtp), evtwt);
		   
		   h->dijetasymmetry3D->Fill(ptavgtp, probeeta, 0.1-0.0001, asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry3Dabseta->Fill(ptavgtp, abs(probeeta), 0.1-0.0001, asymmtp/2./ptavgtp, evtwt);

		   h->dijetasymmetry3Dnarrow->Fill(ptavgtp, probeeta, 0.1-0.0001, asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry3Dabsetanarrow->Fill(ptavgtp, abs(probeeta), 0.1-0.0001, asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry3Dabsetawide->Fill(ptavgtp, abs(probeeta), 0.1-0.0001, asymmtp/2./ptavgtp, evtwt);
 
		   h->dijetasymmetry2D_a01->Fill(ptavgtp, probeeta, asymmtp/2./ptavgtp, evtwt);
		 }

		 if (alpha < 0.15) {
		   h->asymmdist3D_a15->Fill(ptavgtp, abs(probeeta), asymmtp/2./ptavgtp, evtwt);
		   h->absasymmdist3D_a15->Fill(ptavgtp, abs(probeeta), abs(asymmtp/2./ptavgtp), evtwt);

		   h->dijetasymmetry3D->Fill(ptavgtp, probeeta, 0.15-0.0001, asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry3Dabseta->Fill(ptavgtp, abs(probeeta), 0.15-0.0001, asymmtp/2./ptavgtp, evtwt);

		   h->dijetasymmetry3Dnarrow->Fill(ptavgtp, probeeta, 0.15-0.0001, asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry3Dabsetanarrow->Fill(ptavgtp, abs(probeeta), 0.15-0.0001, asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry3Dabsetawide->Fill(ptavgtp, abs(probeeta), 0.15-0.0001, asymmtp/2./ptavgtp, evtwt);
}
		 if (alpha < 0.2)  {
		   h->asymmdist3D_a20->Fill(ptavgtp, abs(probeeta), asymmtp/2./ptavgtp, evtwt);
		   h->absasymmdist3D_a20->Fill(ptavgtp, abs(probeeta), abs(asymmtp/2./ptavgtp), evtwt);

		   h->dijetasymmetry3D->Fill(ptavgtp, probeeta, 0.2-0.0001, asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry3Dabseta->Fill(ptavgtp, abs(probeeta), 0.2-0.0001, asymmtp/2./ptavgtp, evtwt);

		   h->dijetasymmetry3Dnarrow->Fill(ptavgtp, probeeta, 0.2-0.0001, asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry3Dabsetanarrow->Fill(ptavgtp, abs(probeeta), 0.2-0.0001, asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry3Dabsetawide->Fill(ptavgtp, abs(probeeta), 0.2-0.0001, asymmtp/2./ptavgtp, evtwt);

		   h->dijetasymmetry2D_a02->Fill(ptavgtp, probeeta, asymmtp/2./ptavgtp, evtwt);
		 }

		 if (alpha < 0.25) {
		   h->asymmdist3D_a25->Fill(ptavgtp, abs(probeeta), asymmtp/2./ptavgtp, evtwt);
		   h->absasymmdist3D_a25->Fill(ptavgtp, abs(probeeta), abs(asymmtp/2./ptavgtp), evtwt);

		   h->dijetasymmetry3D->Fill(ptavgtp, probeeta, 0.25-0.0001, asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry3Dabseta->Fill(ptavgtp, abs(probeeta), 0.25-0.0001, asymmtp/2./ptavgtp, evtwt);

		   h->dijetasymmetry3Dnarrow->Fill(ptavgtp, probeeta, 0.25-0.0001, asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry3Dabsetanarrow->Fill(ptavgtp, abs(probeeta), 0.25-0.0001, asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry3Dabsetawide->Fill(ptavgtp, abs(probeeta), 0.25-0.0001, asymmtp/2./ptavgtp, evtwt);

		 }
		 if (alpha < 0.3)  {
		   h->asymmdist3D_a30->Fill(ptavgtp, abs(probeeta), asymmtp/2./ptavgtp, evtwt);
		   h->absasymmdist3D_a30->Fill(ptavgtp, abs(probeeta), abs(asymmtp/2./ptavgtp), evtwt);

		   h->dijetbalance_a03->Fill(asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry2D_a03->Fill(ptavgtp, probeeta, asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry_a03->Fill(ptavgtp, asymmtp/2./ptavgtp, evtwt);

		   h->dijetasymmetry3D->Fill(ptavgtp, probeeta, 0.3-0.0001, asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry3Dabseta->Fill(ptavgtp, abs(probeeta), 0.3-0.0001, asymmtp/2./ptavgtp, evtwt);

		   h->dijetasymmetry3Dnarrow->Fill(ptavgtp, probeeta, 0.3-0.0001, asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry3Dabsetanarrow->Fill(ptavgtp, abs(probeeta), 0.3-0.0001, asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry3Dabsetawide->Fill(ptavgtp, abs(probeeta), 0.3-0.0001, asymmtp/2./ptavgtp, evtwt);
}
		 //else if (alpha < 0.35)   h->dijetasymmetry2D_a035->Fill(ptavgtp, probeeta, asymmtp/2./ptavgtp, evtwt);
		 if (alpha < 0.35) {
		   h->asymmdist3D_a35->Fill(ptavgtp, abs(probeeta), asymmtp/2./ptavgtp, evtwt);
		   h->absasymmdist3D_a35->Fill(ptavgtp, abs(probeeta), abs(asymmtp/2./ptavgtp), evtwt);

		   h->dijetasymmetry3D->Fill(ptavgtp, probeeta, 0.35-0.0001, asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry3Dabseta->Fill(ptavgtp, abs(probeeta), 0.35-0.0001, asymmtp/2./ptavgtp, evtwt);

		   h->dijetasymmetry3Dnarrow->Fill(ptavgtp, probeeta, 0.35-0.0001, asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry3Dabsetanarrow->Fill(ptavgtp, abs(probeeta), 0.35-0.0001, asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry3Dabsetawide->Fill(ptavgtp, abs(probeeta), 0.35-0.0001, asymmtp/2./ptavgtp, evtwt);

		 }
		 if (alpha < 0.4)   {
		   h->asymmdist3D_a40->Fill(ptavgtp, abs(probeeta), asymmtp/2./ptavgtp, evtwt);
		   h->absasymmdist3D_a40->Fill(ptavgtp, abs(probeeta), abs(asymmtp/2./ptavgtp), evtwt);

		   h->dijetasymmetry2D_a04->Fill(ptavgtp, probeeta, asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry3D->Fill(ptavgtp, probeeta, 0.4-0.0001, asymmtp/2./ptavgtp, evtwt); // These should match with bins?
		   h->dijetasymmetry3Dabseta->Fill(ptavgtp, abs(probeeta), 0.4-0.0001, asymmtp/2./ptavgtp, evtwt);

		   h->dijetasymmetry3Dnarrow->Fill(ptavgtp, probeeta, 0.4-0.0001, asymmtp/2./ptavgtp, evtwt); // These should match with bins?
		   h->dijetasymmetry3Dabsetanarrow->Fill(ptavgtp, abs(probeeta), 0.4-0.0001, asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry3Dabsetawide->Fill(ptavgtp, abs(probeeta), 0.4-0.0001, asymmtp/2./ptavgtp, evtwt);
		 }

		 if (alpha < 0.45) {
		   h->asymmdist3D_a45->Fill(ptavgtp, abs(probeeta), asymmtp/2./ptavgtp, evtwt);
		   h->absasymmdist3D_a45->Fill(ptavgtp, abs(probeeta), abs(asymmtp/2./ptavgtp), evtwt);

		   h->dijetasymmetry3D->Fill(ptavgtp, probeeta, 0.45-0.0001, asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry3Dabseta->Fill(ptavgtp, abs(probeeta), 0.45-0.0001, asymmtp/2./ptavgtp, evtwt);

		   h->dijetasymmetry3Dnarrow->Fill(ptavgtp, probeeta, 0.45-0.0001, asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry3Dabsetanarrow->Fill(ptavgtp, abs(probeeta), 0.45-0.0001, asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry3Dabsetawide->Fill(ptavgtp, abs(probeeta), 0.45-0.0001, asymmtp/2./ptavgtp, evtwt);
		 }
		 
		 if (alpha < 0.5) {
		   h->dijetasymmetry2D_a05->Fill(ptavgtp, probeeta, asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry3D->Fill(ptavgtp, probeeta, 0.5-0.0001, asymmtp/2./ptavgtp, evtwt); // These should match with bins?
		   h->dijetasymmetry3Dabseta->Fill(ptavgtp, abs(probeeta), 0.5-0.0001, asymmtp/2./ptavgtp, evtwt);

		   h->dijetasymmetry3Dnarrow->Fill(ptavgtp, probeeta, 0.5-0.0001, asymmtp/2./ptavgtp, evtwt); // These should match with bins?
		   h->dijetasymmetry3Dabsetanarrow->Fill(ptavgtp, abs(probeeta), 0.5-0.0001, asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry3Dabsetawide->Fill(ptavgtp, abs(probeeta), 0.5-0.0001, asymmtp/2./ptavgtp, evtwt);

		 }
		 if (alpha < 0.6) {
		   //		   h->dijetasymmetry2D_a06->Fill(ptavgtp, probeeta, asymmtp/2./ptavgtp, evtwt);
		   //		   h->dijetasymmetry3D->Fill(ptavgtp, probeeta, 0.6-0.0001, asymmtp/2./ptavgtp, evtwt); // These should match with bins?
		   h->dijetasymmetry3Dabseta->Fill(ptavgtp, abs(probeeta), 0.6-0.0001, asymmtp/2./ptavgtp, evtwt);
		   h->dijetasymmetry3Dabsetawide->Fill(ptavgtp, abs(probeeta), 0.6-0.0001, asymmtp/2./ptavgtp, evtwt);
		 }
	       }
		 // Second: alpha < 1
		 h->dijetbalance_a1->Fill(asymmtp/2./ptavgtp, evtwt);
		 h->dijetasymmetry_a1->Fill(ptavgtp, asymmtp/2./ptavgtp, evtwt);
	     }
	   }
	 } 
     }

	 for (int j = 0; j < nref; ++j ) {

	 for (auto &histrange : _histos) {
	   for (auto &h : histrange.second) {
	     //	     if (jteta[j] >= h->etamin and jteta[j] < h->etamax and hiBin >= h->hibinmin and hiBin < h->hibinmax) {
	     if (jteta[j] >= h->etamin and jteta[j] < h->etamax and hiBin >= h->hibinmin and hiBin < h->hibinmax) {
	       h->jetetaphi->Fill(jteta[j],jtphi[j],weight);


	       if (j == 0) {
		 // Trigger checks; leading jet pt
		 if (HLT_ZB) h->HLTZB->Fill(jtpt[0],evtwt);
		 if (HLT_40) h->HLT40->Fill(jtpt[0],evtwt*J40psnum/J40psdenom);
		 if (HLT_60) h->HLT60->Fill(jtpt[0],evtwt);
		 if (HLT_80) h->HLT80->Fill(jtpt[0],evtwt);
		 if (HLT_100) h->HLT100->Fill(jtpt[0],evtwt);
		 if (HLT_120) h->HLT120->Fill(jtpt[0],evtwt);
		 //if (!HLT_AK4CaloJet60_v1 and !HLT_AK4CaloJet80_v1 and !HLT_AK4CaloJet100_v1 and HLT_AK4CaloJet120_v1) h->HLT60vs120->Fill(jtpt[0],evtwt);
		 // if (!HLT_AK4CaloJet60_v1 and !HLT_AK4CaloJet80_v1 and HLT_AK4CaloJet100_v1 and !HLT_AK4CaloJet120_v1) h->HLT60vs100->Fill(jtpt[0],evtwt);
		 //if (!HLT_AK4CaloJet60_v1 and HLT_AK4CaloJet80_v1 and !HLT_AK4CaloJet100_v1 and !HLT_AK4CaloJet120_v1) h->HLT60vs80->Fill(jtpt[0],evtwt);
	       }
	   
	       if (j == 0 and nref > 1 and dphi > 2.7) { // Fill dijet system based on leading jet pT
		 h->dijetasymmetry->Fill(abs(djetasymm),evtwt);
		 h->dijetasymmetry_now->Fill(abs(djetasymm));
		 h->dijetdeltaphi->Fill(dphi,evtwt);
		 h->dijetdeltaeta->Fill(ddeta,evtwt);

	       }
	     
	       h->jet_pt->Fill(jtpt[j],evtwt);
	       h->jet_pt_now->Fill(jtpt[j],1);
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

	       if (isMC) {
	     
		 h->genjet_pt->Fill(jtpt_gen[j],evtwt);
		 h->genjet_eta->Fill(jteta_gen[j],evtwt);
		 h->genjet_phi->Fill(jtphi_gen[j],evtwt);
		 
		 h->jetresponse->Fill(jtpt_gen[j],jtpt[j]/jtpt_gen[j],evtwt);
	     
		 h->ptres->Fill((jtpt[j]-jtpt_gen[j])/jtpt_gen[j],evtwt);

		 h->ptgenvsptreco->Fill(jtpt_gen[j],jtpt[j],evtwt);
		 h->ptrecovsweight->Fill(jtpt[j],weight);
		 h->ptgenvsweight->Fill(jtpt_gen[j],weight);

		 h->responses3D->Fill(jtpt_gen[j], jteta_gen[j], jtpt[j]/jtpt_gen[j], evtwt);
		 
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
