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
#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"

// Need to apply MCTruth correction on the fly

map<string, vector<histograms*> > _histos;

// For JER
/* double _rho = 20; // TODO: from ntuples
JME::JetResolution *_jer(0);
JME::JetResolutionScaleFactor *_jer_sf(0); */


 //void analyse(string inFileName = "testdata/run3_ppref_data_04062024.root", string outputfilename = "testoutput.root", bool isMC = false) {
void analyse(string inFileName = "/cmshome/martikai/data/run3_ppref_data_04062024.root", string outputfilename = "testoutput.root", bool isMC = false) {

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
  
  // Int_t pclusterCompatibilityFilter = 1;
  //  Int_t pphfCoincFilter2Th4 = 1;
  Int_t pprimaryVertexFilter = 1;

  // skimTree->SetBranchAddress("pclusterCompatibilityFilter", &pclusterCompatibilityFilter);
  skimTree->SetBranchAddress("pprimaryVertexFilter", &pprimaryVertexFilter);

  // if (!isMC)  skimTree->SetBranchAddress("pphfCoincFilter2Th4", &pphfCoincFilter2Th4); // MC test tuple does not have this */ 

  //// TRIGGERS: CHECK, add rest of jet triggers
  // 60, 80, 100, 120 are unprescaled
  //40 is prescaled but quite few events, need to be added later
  
  Int_t trigger = 0;
  bool usecalotrig = true;


  Int_t HLT_AK4PFJet60_v1, HLT_AK4PFJet80_v1, HLT_AK4PFJet100_v1, HLT_AK4PFJet120_v1;
  Int_t HLT_AK4CaloJet60_v1, HLT_AK4CaloJet80_v1, HLT_AK4CaloJet100_v1, HLT_AK4CaloJet120_v1;

  auto triggerTree = (TTree*)inFile->Get(triggerPath.c_str());

  triggerTree->SetBranchAddress("HLT_AK4PFJet60_v1",&HLT_AK4PFJet60_v1); 
  triggerTree->SetBranchAddress("HLT_AK4PFJet80_v1",&HLT_AK4PFJet80_v1); 
  triggerTree->SetBranchAddress("HLT_AK4PFJet100_v1",&HLT_AK4PFJet100_v1);
  triggerTree->SetBranchAddress("HLT_AK4PFJet120_v1",&HLT_AK4PFJet120_v1);
  
  triggerTree->SetBranchAddress("HLT_AK4CaloJet60_v1",&HLT_AK4CaloJet60_v1); 
  triggerTree->SetBranchAddress("HLT_AK4CaloJet80_v1",&HLT_AK4CaloJet80_v1); 
  triggerTree->SetBranchAddress("HLT_AK4CaloJet100_v1",&HLT_AK4CaloJet100_v1);
  triggerTree->SetBranchAddress("HLT_AK4CaloJet120_v1",&HLT_AK4CaloJet120_v1);
  
 
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

  Float_t   jtdyn_kt[MAXJETS];
  Float_t   jtdyn_deltaR[MAXJETS];
  Float_t   jtdyn_z[MAXJETS];
 
  jetTree->SetBranchAddress("evt", &evt);
  jetTree->SetBranchAddress("nref", &nref);
  jetTree->SetBranchAddress("jtpt", &jtpt);
  jetTree->SetBranchAddress("jteta", &jteta);
  jetTree->SetBranchAddress("jtphi", &jtphi);
  

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

  // TODO: rho, control this


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
FactorizedJetCorrector* corr;
vector<JetCorrectorParameters> vpar; 
 vpar.push_back(JetCorrectorParameters("jecfiles/2023ppwithpp_old_MC_L2Relative_AK4PF.txt"));			 

 corr = new FactorizedJetCorrector(vpar);

 // jetTree->Print();
// Start event loop to fill histograms:
   cout << "Number of entries :" <<  jetTree->GetEntries()  << endl; 
   for (int i = 0; i < jetTree->GetEntries(); ++i) {
     //for (int i = 0; i < 1000; ++i) {
     evtTree->GetEntry(i);
     skimTree->GetEntry(i);
     triggerTree->GetEntry(i);

     if (usecalotrig) trigger = (HLT_AK4CaloJet60_v1 or HLT_AK4CaloJet80_v1 or HLT_AK4CaloJet100_v1 or HLT_AK4CaloJet120_v1);
     else trigger = (HLT_AK4PFJet60_v1 or HLT_AK4PFJet80_v1 or HLT_AK4PFJet100_v1 or HLT_AK4PFJet120_v1);
   
     if (!trigger) continue;

     evtwt = 1;
     if (isMC) {
       evtwt *=  weight;
       //       evtwt *=  findNcoll(hiBin); // Centrality
     }
     //   cout << weight << " " << evtwt << endl;
          
     // BASIC EVENT FILTERS
     //  if (pprimaryVertexFilter != 1) continue; // TODO: test, for some reason gets always 0 but should be 1
     //     if (hiBin > CENTRALITYHIGH or hiBin < CENTRALITYLOW) continue;
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
	 // REDO JEC
	 corr->setJetPt(jtpt[j]);
	 // corr->setJetE(jteu[jetidx]);
	 corr->setJetEta(jteta[j]);

	 vector<float> v = corr->getSubCorrections();
	 float jes = v.back();

	 //	 cout << "New jes correction: " << jtpt[j] << " " << j << " "  << jes << endl;
	 jtpt_uncorr[j] = jtpt[j];
	 jtpt[j] *= jes;

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
       
       for (int j = 0; j < 2; ++j) {   // Use both jets as t/b in turn, maybe change later
	 
	 tagpt = jtpt[j];
	 probept = jtpt[(j == 0 ? 1 : 0)];
	 tageta = jteta[j];
	 
	 if (abs(tageta) > 1.3) continue;
	 
	 probeeta = jteta[(j == 0 ? 1 : 0)];
	 
	 ptavgtp = 0.5*(tagpt  + probept);
	 asymmtp = probept - tagpt;
	 
	 if (nref > 2) alpha = jtpt[2]/ptavgtp; // Problem if only two jets! -> does it make sense to look at the effect? -> m
	 else alpha = 1;
	 
	 // DPhi requirement?
	 // Fill in average pT
	 // eta bin from probeeta
	 
	 for (auto &histrange : _histos) { 
	   for (auto &h : histrange.second) {
	     
	     if (probeeta >= h->etamin and probeeta < h->etamax and hiBin >= h->hibinmin and hiBin < h->hibinmax) {
	       
	       if (alpha < 0.3)  {
		 h->dijetbalance_a03->Fill(asymmtp/2./ptavgtp);
		 h->dijetasymmetry_a03->Fill(ptavgtp, asymmtp/2./ptavgtp);
	       }
	       
	       // Second: alpha < 1
	       h->dijetbalance_a1->Fill(asymmtp/2./ptavgtp);
	       h->dijetasymmetry_a1->Fill(ptavgtp, asymmtp/2./ptavgtp);
	     }
	   }
	 } 
	 
       }    
     }

	 for (int j = 0; j < nref; ++j ) {


	 for (auto &histrange : _histos) { ///// etabins instead of pts?
	   for (auto &h : histrange.second) {
	     if (jteta[j] >= h->etamin and jteta[j] < h->etamax and hiBin >= h->hibinmin and hiBin < h->hibinmax) {
	       h->jetetaphi->Fill(jteta[j],jtphi[j],weight);
	   
	     //      if (isMC and weight > 0.001) continue; // TODO: study this more?

	       if (j == 0 and nref > 1 and dphi > 2.7) { // Fill dijet system based on leading jet pT
	       // TOOD: impose further criteria for proper tag and probe

		 h->dijetasymmetry->Fill(djetasymm,evtwt);
		 h->dijetdeltaphi->Fill(dphi,evtwt);
		 h->dijetdeltaeta->Fill(ddeta,evtwt);

	       }

	     
	       h->jet_pt->Fill(jtpt[j],evtwt);
	       h->jet_uncorr_pt->Fill(jtpt_uncorr[j],evtwt);
	       h->jet_pt_genweight->Fill(jtpt[j],weight);
	       h->jet_eta->Fill(jteta[j],evtwt);
	       h->jet_phi->Fill(jtphi[j],evtwt);
  
      
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
