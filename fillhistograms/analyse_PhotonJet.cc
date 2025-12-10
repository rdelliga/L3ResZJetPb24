#include <iostream>
using std::cout;
using std::endl;

#include "TFile.h"
#include "TH1D.h"
#include "TMath.h"
#include "TRandom.h"
#include "TTree.h"
#include <cmath>
#include <cstdio>
#include <ctime>
#include <iterator>
#include <typeinfo>

#include "configurations.h"
#include "histograms.h"
#include "settings.h"

#include "eventhistograms.h"
#include "helpers.h"

#include "JetMETCorrections/Modules/interface/JetResolution.h"
JME::JetResolution *_jer(0);
JME::JetResolutionScaleFactor *_jer_sf(0);
float rho = 0.;

std::mt19937 _mersennetwister;
std::uint32_t _seed = 4;
//_seed = 4;

#if REDOJES == 1
#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"
#endif

bool debug = false;
bool applyjetvetomap = false;

// Photon+Jet analysis for L3 residual corrections
void analyse_PhotonJet(string era = "PHOTONHP",
                       string outputfiletag = "AK4_photonjet",
                       bool isMC = false, bool checkjetid = false) {

  bool usecalotrig = false;
  bool checkvalidjet =
      false; // this is for checking valid jet range after applying l2. now for
             // tightly limited range. TODO: do something smarter

  //  string outputfilename =
  //  Form("/eos/user/l/lamartik/HIJEC_rereco_results_HI2023MCTruth/%s_%s.root",era.c_str(),outputfiletag.c_str());
  string outputfilename = Form("/eos/cms/store/group/phys_heavyions/bharikri/"
                               "JetMinPOG/L3ResPhotonJet/%s_%s.root",
                               era.c_str(), outputfiletag.c_str());
  // string outputfilename =
  // Form("/eos/user/l/lamartik/HIJEC_rereco_results_HI2023MCTruth_chs/%s_%s.root",era.c_str(),outputfiletag.c_str());
  if (debug)
    outputfilename = "test.root";

  TRandom3 r;
  // Define and activate branches
  std::string evtPath = "hiEvtAnalyzer/HiTree";
  std::string triggerPath = "hltanalysis/HltTree";
  std::string skimPath = "skimanalysis/HltTree";
  std::string photonPath = "ggHiNtuplizer/EventTree";

  // TODO: jet type ; rereco has PF and PFCHS jets
  // std::string jetPath = "ak4PFCHSJetAnalyzer/t";
  std::string jetPath = "ak4PFJetAnalyzer/t";
  // if (!isMC) jetPath = "ak0PFJetAnalyzer/t";

  cout << "Opening input file" << endl;
  //  TFile *inFile = new TFile(inFileName.c_str(), "READ"); // TODO: safety
  //  checks about opening file successfully
  TFile *inFile =
      new TFile(filenames[era.c_str()].c_str(),
                "READ"); // TODO: safety checks about opening file successfully

  auto evtTree = (TTree *)inFile->Get(evtPath.c_str());

  // Get photon tree
  auto photonTree = (TTree *)inFile->Get(photonPath.c_str());
  if (!photonTree) {
    cout << "ERROR: Cannot find photon tree: " << photonPath << endl;
    return;
  }

  // Cuts and weights from event tree
  Int_t hiBin = -1;
  Float_t weight = 1, vz = 0, pthat = 0, evtwt = 1;

  // Photon variables (vectors)
  Int_t nPho;
  std::vector<float> *phoEt = 0;
  std::vector<float> *phoEta = 0;
  std::vector<float> *phoPhi = 0;
  std::vector<float> *phoE = 0;
  std::vector<float> *phoSCEta = 0;
  std::vector<float> *phoSCPhi = 0;
  std::vector<float> *phoHoverE = 0;
  std::vector<float> *phoSigmaIEtaIEta = 0;
  std::vector<float> *phoR9 = 0;
  std::vector<float> *pfcIso3subUEec = 0;  // PF charged hadron isolation
  std::vector<float> *pfnIso3subUEec = 0;  // PF neutral hadron isolation
  std::vector<float> *pfpIso3subUEec = 0;  // PF photon isolation

  evtTree->SetBranchStatus("*", 0);
  evtTree->SetBranchStatus("hiBin", 1);
  evtTree->SetBranchStatus("vz", 1);
  if (isMC)
    evtTree->SetBranchStatus("weight", 1);
  if (isMC)
    evtTree->SetBranchStatus("pthat", 1);

  //// EVENT FILTERS
  // auto skimTree = (TTree*)inFile->Get(skimPath.c_str());
  // if (!isMC) skimTree->SetBranchStatus("*",1);

  // Int_t pprimaryVertexFilter = 1;
  // if (!isMC) skimTree->SetBranchAddress("pprimaryVertexFilter",
  // &pprimaryVertexFilter);

  Int_t trigger = 0;

  // Photon trigger
  Int_t HLT_Photon30 = 0;

  // auto triggerTree = (TTree*)inFile->Get(triggerPath.c_str());

  // if (!isMC) {
  //   cout << "Use Photon trigger: HLT_PPRefGEDPhoton30_v1" << endl;
  //   triggerTree->SetBranchStatus("*", 0);
  //   triggerTree->SetBranchStatus("HLT_PPRefGEDPhoton30_v1", 1);
  //   triggerTree->SetBranchAddress("HLT_PPRefGEDPhoton30_v1", &HLT_Photon30);
  // }

  // JETS
  auto jetTree = (TTree *)inFile->Get(jetPath.c_str());
  jetTree->SetBranchStatus("*", 1);

  Int_t evt;

  // Reconstruted jet information
  Int_t nref;
  Float_t jtpt[MAXJETS];
  Float_t jtpt_uncorr[MAXJETS];
  Float_t jteta[MAXJETS];
  Float_t jtphi[MAXJETS];

  Float_t jtnhf[MAXJETS];
  Float_t jtchf[MAXJETS];
  Float_t jtnef[MAXJETS];
  Float_t jtcef[MAXJETS];
  Float_t jtmuf[MAXJETS];

  Int_t jtchm[MAXJETS]; // charged multiplicity

  Int_t jtn[MAXJETS];

  jetTree->SetBranchAddress("evt", &evt);
  jetTree->SetBranchAddress("nref", &nref);
  jetTree->SetBranchAddress("jtpt", &jtpt);
  jetTree->SetBranchAddress("jteta", &jteta);
  jetTree->SetBranchAddress("jtphi", &jtphi);

  // jetTree->SetBranchAddress("jtPfNHF", &jtnhf);
  // jetTree->SetBranchAddress("jtPfCHF", &jtchf);
  // jetTree->SetBranchAddress("jtPfNEF", &jtnef);
  // jetTree->SetBranchAddress("jtPfCEF", &jtcef);
  // jetTree->SetBranchAddress("jtPfMUF", &jtmuf);
  // jetTree->SetBranchAddress("jtPfCHM", &jtchm);

  // Gen level jet information
  Float_t jtpt_gen[MAXJETS];
  Float_t jteta_gen[MAXJETS];
  Float_t jtphi_gen[MAXJETS];
  Float_t refdrjt[MAXJETS];

  if (isMC) {
    jetTree->SetBranchAddress("refpt", &jtpt_gen);
    jetTree->SetBranchAddress("refeta", &jteta_gen);
    jetTree->SetBranchAddress("refphi", &jtphi_gen);
    jetTree->SetBranchAddress("refdrjt", &refdrjt);
  }

  // Set photon branch addresses (vectors)
  photonTree->SetBranchAddress("nPho", &nPho);
  photonTree->SetBranchAddress("phoEt", &phoEt);
  photonTree->SetBranchAddress("phoEta", &phoEta);
  photonTree->SetBranchAddress("phoPhi", &phoPhi);
  photonTree->SetBranchAddress("phoE", &phoE);
  photonTree->SetBranchAddress("phoSCEta", &phoSCEta);
  photonTree->SetBranchAddress("phoSCPhi", &phoSCPhi);
  photonTree->SetBranchAddress("phoHoverE", &phoHoverE);
  photonTree->SetBranchAddress("phoSigmaIEtaIEta_2012", &phoSigmaIEtaIEta);
  photonTree->SetBranchAddress("phoR9_2012", &phoR9);
  photonTree->SetBranchAddress("pfcIso3subUEec", &pfcIso3subUEec);
  photonTree->SetBranchAddress("pfnIso3subUEec", &pfnIso3subUEec);
  photonTree->SetBranchAddress("pfpIso3subUEec", &pfpIso3subUEec);

  TFile *outfile = new TFile(outputfilename.c_str(), "RECREATE");

  // Local map for histogram storage (not global to avoid ROOT cleanup issues)
  map<string, vector<histograms *>> _histos;

  // Create folders for centrality bins. Mostly a placeholder in case PbPb
  // MC/data is checked.
  for (int j = 0; j < nhibins; ++j) {

    if (hibins[j] < hibins[j + 1]) {
      string name = Form("hibin_%.1f_%.1f", hibins[j], hibins[j + 1]);
      outfile->mkdir(name.c_str());

      TDirectory *dir = outfile->GetDirectory(name.c_str());
      assert(dir);
      dir->cd();

      for (int i = 0; i < netabins; ++i) {
        if (etaedges[i] < etaedges[i + 1]) {
          string name2 = Form("eta_%.1f_%.1f", etaedges[i], etaedges[i + 1]);
          dir->mkdir(name2.c_str());

          TDirectory *dir2 = dir->GetDirectory(name2.c_str());
          assert(dir2);
          dir2->cd();

          histograms *h = new histograms(dir2, etaedges[i], etaedges[i + 1],
                                         hibins[j], hibins[j + 1], isMC);
          _histos[name2.c_str()].push_back(h);
        }
      }
    }
  }

  outfile->mkdir("event");
  TDirectory *dir = outfile->GetDirectory("event");
  assert(dir);
  dir->cd();

  eventhistograms *eh = new eventhistograms(dir, isMC);

#if REDOJES == 1
  cout << "Applying MC JEC from file " << jecfile.c_str() << endl;
  FactorizedJetCorrector *corr;
  vector<JetCorrectorParameters> vpar;
  // This is MCTruth
  vpar.push_back(JetCorrectorParameters(jecfile.c_str()));
  // No L2 residual for photon+jet analysis (L3 only)
  corr = new FactorizedJetCorrector(vpar);
#endif

  // JER not needed for photon+jet L3 residual analysis

  // Jet veto map
  //   auto mapfile = new
  //   TFile("jecfiles/Summer23BPixPrompt23_RunD_v1.root","READ"); auto vetomap
  //   = (TH2D*)mapfile->Get("jetvetomap_all");

  cout << "Number of entries :" << jetTree->GetEntries() << endl;
  int nentries = jetTree->GetEntries();
  if (debug)
    nentries = 1000;

  cout << "Processing " << nentries << endl;
  for (int i = 0; i < nentries; ++i) {
    evtTree->GetEntry(i);
    //  triggerTree->GetEntry(i);
    photonTree->GetEntry(i);

    // Photon trigger logic
    //  if (!isMC) {
    //    trigger = HLT_Photon30;
    //  }
    //  if (isMC) trigger = true; // MC: no trigger requirement

    //  if (!trigger) continue;

    evtwt = 1;
    if (isMC) {
      evtwt *= weight;
    }

    // cout << weight << " " << evtwt << endl;

    // BASIC EVENT FILTERS
    //  if (!isMC) {
    //    skimTree->GetEntry(i);
    //    if (pprimaryVertexFilter != 1) continue;
    //  }
    jetTree->GetEntry(i);

    // Photon+jet: need at least 1 photon and 1 jet
    if (nPho < 1)
      continue;
    if (nref < 1)
      continue;
    if (jtpt[0] < jtptmin) {
      continue;
    }

    eh->event_vz->Fill(vz, evtwt);
    if (isMC)
      eh->event_pthatwsgenweight->Fill(pthat, weight);

    // This is photon+jet analysis
    double photon_pt, photon_eta, photon_phi, jet_pt, jet_eta, jet_phi, ptavgtp,
        alpha, balance;
    double asymmtp;
    double djrespasymm;

    if (checkvalidjet) { // This is based on validity of JEC. - obsolete?
      for (int j = 0; j < nref; ++j) {
        if (abs(jteta[j]) > 2.964)
          jtpt[j] = 0; // Always invalid jets

        else if (abs(jteta[j]) > 2.5 and jtpt[j] > 120)
          jtpt[j] = 0; // ?
        else if (abs(jteta[j]) > 1.93 and jtpt[j] > 170)
          jtpt[j] = 0; // ?

        if (jtpt[j] < 80)
          jtpt[j] = 0;
        // if 80-120 abseta < 2.964
        // 120-170 < 2.5
        // 170-1000 < 1.93
      }
    }

    // JET ID - this is 2023 AK4CHS jet selection 12/2024
    // fill passjteta for all jets in the event?
    bool passjetid[nref];
    for (int j = 0; j < nref; ++j) {
      passjetid[j] = true;
      // if (checkjetid) {
      //   if (abs(jteta[j]) <= 2.6) {
      //     if (jtnhf[j] >= 0.99)
      //       passjetid[j] = false;
      //     if (jtnef[j] >= 0.9)
      //       passjetid[j] = false;
      //     if (jtchf[j] <= 0.01)
      //       passjetid[j] = false;
      //     if (jtcef[j] >= 0.8)
      //       passjetid[j] = false;
      //     if (jtmuf[j] >= 0.8)
      //       passjetid[j] = false;
      //     if (jtchm[j] <= 0)
      //       passjetid[j] = false;
      //   } else if (abs(jteta[j]) <= 2.7) {
      //     if (jtnhf[j] >= 0.9)
      //       passjetid[j] = false;
      //     if (jtnef[j] >= 0.99)
      //       passjetid[j] = false;
      //     if (jtmuf[j] >= 0.8)
      //       passjetid[j] = false;
      //     if (jtcef[j] >= 0.8)
      //       passjetid[j] = false;

      //   } else if (abs(jteta[j]) <= 3.0) {
      //     if (jtnhf[j] >= 0.99)
      //       passjetid[j] = false;
      //     if (jtnef[j] >= 0.99)
      //       passjetid[j] = false;
      //   } else if (abs(jteta[j]) <= 5.0) {
      //     if (jtnef[j] >= 0.4)
      //       passjetid[j] = false;
      //   }
      //   //	   if (jtpt[j] > jtptmin)	   cout << passjetid[j] << endl;
      //   //  if (passjetid[j] < 2 and nref > 2  and jtpt[j] > 70  and jtpt[1] >
      //   //  40) cout << "Pass jetid: " << passjetid[j] << " pt: " << jtpt[j] <<
      //   //  " " << jteta[j] << " " << j << " " << i <<  endl;
      // }
    }

    // Apply JEC
    for (int j = 0; j < nref; ++j) {
      jtpt_uncorr[j] = jtpt[j];

#if REDOJES == 1
      // cout << "Applying JES" << endl;
      corr->setJetPt(jtpt[j]);
      // corr->setJetE(jteu[jetidx]);
      corr->setJetEta(jteta[j]);
      // 	 corr->setJetPhi(jthpi[j]);

      vector<float> v = corr->getSubCorrections();
      float jes = v.back();

      //	 cout << "New jes correction jet pt: " << jtpt[j] << " " <<
      //jteta[j] << " "  << jes << endl;
      jtpt[j] *= jes;
#endif

      // JER not applied for photon+jet L3 residual analysis
    }

    // ========================================
    // PHOTON+JET SELECTION
    // ========================================

    // 1. Find leading photon passing selection
    int leadPhotonIdx = -1;
    float leadPhotonPt = 0;

    for (int ipho = 0; ipho < nPho; ipho++) {
      // Kinematic cuts
      if ((*phoEt)[ipho] < 30.0)
        continue; // Trigger threshold
      if (abs((*phoEta)[ipho]) > 1.44)
        continue; // Barrel only

      // Photon ID cuts
      if ((*phoHoverE)[ipho] > 0.3)
        continue;
      if ((*phoSigmaIEtaIEta)[ipho] < 0.002)
        continue;
      if ((*phoSigmaIEtaIEta)[ipho] > 0.03)
        continue;

      // Find highest pT photon
      if ((*phoEt)[ipho] > leadPhotonPt) {
        leadPhotonPt = (*phoEt)[ipho];
        leadPhotonIdx = ipho;
      }
    }

    // No good photon found
    if (leadPhotonIdx < 0)
      continue;

    // 2. Find leading and subleading away-side jets
    // First: identify all jets back-to-back with photon (dphi > 2.7)
    int awayJetIndices[MAXJETS];
    int nAwayJets = 0;

    for (int j = 0; j < nref; j++) {
      // Apply jet ID
      if (checkjetid && passjetid[j] == 0)
        continue;

      // Jet kinematic cuts
      if (jtpt[j] < 15.0)
        continue; // Minimum jet pT

      // Calculate delta-phi with photon
      float dphi = abs(jtphi[j] - (*phoPhi)[leadPhotonIdx]);
      if (dphi > TMath::Pi())
        dphi = 2 * TMath::Pi() - dphi;

      // Back-to-back requirement
      if (dphi < 2.0943951)
        continue; // 2*pi/3 = 2.0943951

      // Calculate delta-R (reject jets close to photon)
      float deta = jteta[j] - (*phoEta)[leadPhotonIdx];
      float deltaR = sqrt(deta * deta + dphi * dphi);
      if (deltaR < 0.4)
        continue; // Isolation cone

      // This jet is away-side
      awayJetIndices[nAwayJets] = j;
      nAwayJets++;
    }

    // Need at least one away-side jet
    if (nAwayJets < 1)
      continue;

    // Sort away-side jets by pT (descending)
    for (int i = 0; i < nAwayJets - 1; i++) {
      for (int j = i + 1; j < nAwayJets; j++) {
        if (jtpt[awayJetIndices[j]] > jtpt[awayJetIndices[i]]) {
          int temp = awayJetIndices[i];
          awayJetIndices[i] = awayJetIndices[j];
          awayJetIndices[j] = temp;
        }
      }
    }

    // Leading away-side jet (probe jet)
    int awayJetIdx = awayJetIndices[0];

    // 3. Calculate alpha (using 2nd away-side jet if available)
    alpha = 0;
    if (nAwayJets >= 2) {
      int secondAwayJetIdx = awayJetIndices[1];
      float ptavg_temp = 0.5 * ((*phoEt)[leadPhotonIdx] + jtpt[awayJetIdx]);
      alpha = jtpt[secondAwayJetIdx] / ptavg_temp;
    } else {
      alpha = 0; // Only one away-side jet
    }

    // 4. Calculate photon+jet variables
    photon_pt = (*phoEt)[leadPhotonIdx];
    photon_eta = (*phoEta)[leadPhotonIdx];
    photon_phi = (*phoPhi)[leadPhotonIdx];

    jet_pt = jtpt[awayJetIdx];
    jet_eta = jteta[awayJetIdx];
    jet_phi = jtphi[awayJetIdx];

    float dphi_photonjet = abs(jet_phi - photon_phi);
    if (dphi_photonjet > TMath::Pi())
      dphi_photonjet = 2 * TMath::Pi() - dphi_photonjet;

    ptavgtp = 0.5 * (photon_pt + jet_pt);
    balance = jet_pt / photon_pt; // Response
    asymmtp = balance;            // For compatibility with histogram filling

    // cout << "TP:" << tagpt << " " << probept << " " << alpha << endl;
    // ========================================
    // FILL PHOTON+JET HISTOGRAMS
    // ========================================

    for (auto &histrange : _histos) {
      for (auto &h : histrange.second) {

        // Check if jet eta and centrality are in range for this histogram set
        if (jet_eta >= h->etamin && jet_eta < h->etamax &&
            hiBin >= h->hibinmin && hiBin < h->hibinmax) {

          // Photon properties
          h->photon_pt->Fill(photon_pt, evtwt);
          h->photon_eta->Fill(photon_eta, evtwt);
          h->photon_phi->Fill(photon_phi, evtwt);
          h->photon_HoverE->Fill((*phoHoverE)[leadPhotonIdx], evtwt);
          h->photon_sigmaIetaIeta->Fill((*phoSigmaIEtaIEta)[leadPhotonIdx],
                                        evtwt);

          // Away-side jet properties
          h->awayside_jet_pt->Fill(jet_pt, evtwt);
          h->awayside_jet_eta->Fill(jet_eta, evtwt);
          h->awayside_jet_phi->Fill(jet_phi, evtwt);
          h->awayside_jet_uncorr_pt->Fill(jtpt_uncorr[awayJetIdx], evtwt);

          // Photon+Jet system
          h->photonjet_dphi->Fill(dphi_photonjet, evtwt);
          h->photonjet_balance->Fill(balance, evtwt);
          h->photonjet_ptavg->Fill(ptavgtp, evtwt);
          h->photonjet_alpha->Fill(alpha, evtwt);

          // Trigger histograms
          if (HLT_Photon30) {
            h->HLTPhoton30->Fill(1, evtwt);
            h->HLTPhoton30_ptav->Fill(ptavgtp, evtwt);
          }

          // Alpha-dependent balance profiles (analogous to dijet asymmetry)
          if (alpha < 0.1) {
            h->photonjet_balance_a01->Fill(ptavgtp, balance, evtwt);
            h->photonjet_balance2D_a01->Fill(ptavgtp, jet_eta, balance, evtwt);
          }
          if (alpha < 0.2) {
            h->photonjet_balance_a02->Fill(ptavgtp, balance, evtwt);
            h->photonjet_balance2D_a02->Fill(ptavgtp, jet_eta, balance, evtwt);
          }
          if (alpha < 0.3) {
            h->photonjet_balance_a03->Fill(ptavgtp, balance, evtwt);
            h->photonjet_balance2D_a03->Fill(ptavgtp, jet_eta, balance, evtwt);

            // Jet composition for alpha < 0.3 (like dijets)
            h->jet_nef->Fill(jet_pt, jtnef[awayJetIdx], evtwt);
            h->jet_cef->Fill(jet_pt, jtcef[awayJetIdx], evtwt);
            h->jet_nhf->Fill(jet_pt, jtnhf[awayJetIdx], evtwt);
            h->jet_chf->Fill(jet_pt, jtchf[awayJetIdx], evtwt);
            h->jet_muf->Fill(jet_pt, jtmuf[awayJetIdx], evtwt);
          }
          if (alpha < 0.4) {
            h->photonjet_balance_a04->Fill(ptavgtp, balance, evtwt);
            h->photonjet_balance2D_a04->Fill(ptavgtp, jet_eta, balance, evtwt);
          }
          if (alpha < 0.5) {
            h->photonjet_balance_a05->Fill(ptavgtp, balance, evtwt);
            h->photonjet_balance2D_a05->Fill(ptavgtp, jet_eta, balance, evtwt);
          }
          if (alpha < 0.6) {
            h->photonjet_balance_a06->Fill(ptavgtp, balance, evtwt);
            h->photonjet_balance2D_a06->Fill(ptavgtp, jet_eta, balance, evtwt);
          }

          // 3D balance profiles (KEY HISTOGRAMS for L3 residual derivation)
          // Only fill in the wide eta bin since these have internal eta binning
          if ((h->etamin - h->etamax) < -10) {
            h->photonjet_balance3D->Fill(ptavgtp, jet_eta, alpha, balance,
                                         evtwt);
            h->photonjet_balance3Dwide->Fill(ptavgtp, jet_eta, alpha, balance,
                                             evtwt);
            h->photonjet_balance3Dnarrow->Fill(ptavgtp, jet_eta, alpha, balance,
                                               evtwt);
            h->photonjet_balance3Dabseta->Fill(ptavgtp, abs(jet_eta), alpha,
                                               balance, evtwt);
            h->photonjet_balance3Dabsetawide->Fill(ptavgtp, abs(jet_eta), alpha,
                                                   balance, evtwt);
            h->photonjet_balance3Dabsetanarrow->Fill(ptavgtp, abs(jet_eta),
                                                     alpha, balance, evtwt);
          }
        }
      }
    }

    // Additional jet histograms for all jets in the event
    for (int j = 0; j < nref; ++j) {

      for (auto &histrange : _histos) {
        for (auto &h : histrange.second) {

          if (jteta[j] >= h->etamin and jteta[j] < h->etamax and
              hiBin >= h->hibinmin and hiBin < h->hibinmax) {

            if (checkjetid and passjetid[j] == 0)
              continue;

            h->jetetaphi->Fill(jteta[j], jtphi[j], weight);

            if (j == 0 and passjetid[0]) {
              // Photon trigger check; leading jet pt
              if (HLT_Photon30)
                h->HLTPhoton30->Fill(jtpt[awayJetIndices[0]], evtwt);
            }

            // These are actually obsolete after all the selections
            /*	       if (j == 0 and nref > 1 and dphitp > 2.7) { // Fill dijet
              system based on leading jet pT
              h->dijetasymmetry->Fill(abs(djetasymm),evtwt);
              h->dijetasymmetry_now->Fill(abs(djetasymm));
              h->dijetdeltaphi->Fill(dphi,evtwt);
              h->dijetdeltaeta->Fill(ddeta,evtwt);
              } */

            h->jet_pt->Fill(jtpt[j], evtwt);
            h->jet_pt_now->Fill(jtpt[j], 1);
            h->jet_uncorr_pt->Fill(jtpt_uncorr[j], evtwt);
            h->jet_pt_genweight->Fill(jtpt[j], weight);
            h->jet_eta->Fill(jteta[j], evtwt);
            h->jet_phi->Fill(jtphi[j], evtwt);

            if (isMC) {

              h->genjet_pt->Fill(jtpt_gen[j], evtwt);
              h->genjet_eta->Fill(jteta_gen[j], evtwt);
              h->genjet_phi->Fill(jtphi_gen[j], evtwt);

              h->jetresponse->Fill(jtpt_gen[j], jtpt[j] / jtpt_gen[j], evtwt);

              h->ptres->Fill((jtpt[j] - jtpt_gen[j]) / jtpt_gen[j], evtwt);

              h->ptgenvsptreco->Fill(jtpt_gen[j], jtpt[j], evtwt);
              h->ptrecovsweight->Fill(jtpt[j], weight);
              h->ptgenvsweight->Fill(jtpt_gen[j], weight);

              h->responses3D->Fill(jtpt_gen[j], jteta_gen[j],
                                   jtpt[j] / jtpt_gen[j], evtwt);
              h->phiresponse->Fill(jtpt_gen[j], jteta_gen[j],
                                   jtphi[j] - jtphi_gen[j], evtwt);
              h->etaresponse->Fill(jtpt_gen[j], jteta_gen[j],
                                   jteta[j] - jteta_gen[j], evtwt);
            }
          }
        }
      }
    }
  } // end of event loop

  // Write output histograms

  for (auto &histrange : _histos) {
    for (auto &h : histrange.second) {
      h->Write();
    }
  }
  eh->Write();

  // Close output file - this writes and ROOT takes ownership of histograms
  outfile->Close();

  // Close input file
  inFile->Close();

  cout << "Wrote " << outputfilename.c_str() << endl;
}
