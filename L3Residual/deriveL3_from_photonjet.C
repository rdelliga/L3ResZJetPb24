// Derive L3 residual corrections from photon+jet balancing using 3D profiles
// Balance = jet_pT / photon_pT
// L3 Residual = balance_MC / balance_data (corrects data to match MC response)

#include <iostream>
#include <fstream>
#include "../fillhistograms/histograms.h"

void deriveL3_from_photonjet(
    TString mcFile = "/eos/cms/store/group/phys_heavyions/bharikri/JetMinPOG/L3ResPhotonJet/PHOTONMC_AK4_photonjet.root",
    TString dataFile = "/eos/cms/store/group/phys_heavyions/bharikri/JetMinPOG/L3ResPhotonJet/PHOTONHP_AK4_photonjet.root",
    TString outfilename = "L3Residual_PhotonJet.root",
    bool dodt = true,
    int alphabin = 5,
    bool useabs = true,
    bool usewideabs = false) {

  // Open MC file
  TFile *inFileMC = TFile::Open(mcFile);
  if (!inFileMC || inFileMC->IsZombie()) {
    cout << "ERROR: Cannot open MC file: " << mcFile << endl;
    return;
  }

  // Open Data file
  TFile *inFileDT = TFile::Open(dataFile);
  if (!inFileDT || inFileDT->IsZombie()) {
    cout << "ERROR: Cannot open Data file: " << dataFile << endl;
    return;
  }

  // These are bins to be processed
  vector<string> etabins = {"eta_-5.2_5.2"};
  int i = 0;

  map<string, TProfile3D*> mc3d, data3d;
  map<string, TH1D*> responses;
  map<int, TH1D*> respETA;

  TFile *outfile = new TFile(outfilename, "RECREATE");

  // Histograms for alpha dependence study
  TH1D* vsalpha_mc = new TH1D("vsalpha_mc", "MC balance vs alpha; alpha; balance", histograms::nalphavalues, &histograms::alphavalues[0]);
  TH1D* vsalpha_data = new TH1D("vsalpha_data", "Data balance vs alpha; alpha; balance", histograms::nalphavalues, &histograms::alphavalues[0]);

  // Histograms for eta dependence - declare pointers
  TH1D* vseta_mc(0);
  TH1D* vseta_data(0);
  TH1D* aerrormc(0);
  TH1D* aerrordt(0);

  // Get the appropriate 3D balance profiles based on eta binning choice
  if (useabs) {
    vseta_mc = new TH1D("vseta_mc", "MC balance; |#eta_{jet}|; balance", histograms::nwabsetas, &histograms::wabsetarange[0]);
    vseta_data = new TH1D("vseta_data", "Data balance; |#eta_{jet}|; balance", histograms::nwabsetas, &histograms::wabsetarange[0]);
    aerrormc = new TH1D("aerrormc", "MC error; |#eta_{jet}|; error", histograms::nwabsetas, &histograms::wabsetarange[0]);
    aerrordt = new TH1D("aerrordt", "Data error; |#eta_{jet}|; error", histograms::nwabsetas, &histograms::wabsetarange[0]);

    mc3d[etabins[i].c_str()] = (TProfile3D*)inFileMC->Get("hibin_-1.0_0.0/eta_-5.2_5.2/photonjet_balance3Dabseta");
    data3d[etabins[i].c_str()] = (TProfile3D*)inFileDT->Get("hibin_-1.0_0.0/eta_-5.2_5.2/photonjet_balance3Dabseta");
  }
  else if (usewideabs) {
    vseta_mc = new TH1D("vseta_mc", "MC balance; |#eta_{jet}|; balance", histograms::ndwabsetas, &histograms::dwabsetarange[0]);
    vseta_data = new TH1D("vseta_data", "Data balance; |#eta_{jet}|; balance", histograms::ndwabsetas, &histograms::dwabsetarange[0]);
    aerrormc = new TH1D("aerrormc", "MC error; |#eta_{jet}|; error", histograms::ndwabsetas, &histograms::dwabsetarange[0]);
    aerrordt = new TH1D("aerrordt", "Data error; |#eta_{jet}|; error", histograms::ndwabsetas, &histograms::dwabsetarange[0]);

    mc3d[etabins[i].c_str()] = (TProfile3D*)inFileMC->Get("hibin_-1.0_0.0/eta_-5.2_5.2/photonjet_balance3Dabsetawide");
    data3d[etabins[i].c_str()] = (TProfile3D*)inFileDT->Get("hibin_-1.0_0.0/eta_-5.2_5.2/photonjet_balance3Dabsetawide");
  }
  else {
    vseta_mc = new TH1D("vseta_mc", "MC balance; #eta_{jet}; balance", histograms::nwetas, &histograms::wetarange[0]);
    vseta_data = new TH1D("vseta_data", "Data balance; #eta_{jet}; balance", histograms::nwetas, &histograms::wetarange[0]);
    aerrormc = new TH1D("aerrormc", "MC error; #eta_{jet}; error", histograms::nwetas, &histograms::wetarange[0]);
    aerrordt = new TH1D("aerrordt", "Data error; #eta_{jet}; error", histograms::nwetas, &histograms::wetarange[0]);

    mc3d[etabins[i].c_str()] = (TProfile3D*)inFileMC->Get("hibin_-1.0_0.0/eta_-5.2_5.2/photonjet_balance3D");
    data3d[etabins[i].c_str()] = (TProfile3D*)inFileDT->Get("hibin_-1.0_0.0/eta_-5.2_5.2/photonjet_balance3D");
  }

  // Check that histograms were found
  if (!mc3d[etabins[i].c_str()]) {
    cout << "ERROR: Cannot find MC photon+jet balance histogram" << endl;
    return;
  }
  if (!data3d[etabins[i].c_str()]) {
    cout << "ERROR: Cannot find Data photon+jet balance histogram" << endl;
    return;
  }

  cout << etabins[i] << " MC nptbins: " << mc3d[etabins[i].c_str()]->GetXaxis()->GetNbins() << endl;
  cout << etabins[i] << " Data nptbins: " << data3d[etabins[i].c_str()]->GetXaxis()->GetNbins() << endl;
  cout << "Alpha bin: " << alphabin << endl;
  cout << "Alpha bin edges: " << mc3d[etabins[i].c_str()]->GetZaxis()->GetBinLowEdge(alphabin)
       << " to " << mc3d[etabins[i].c_str()]->GetZaxis()->GetBinLowEdge(alphabin+1) << endl;

  string alphastr = Form("alpha%.2f", mc3d[etabins[i].c_str()]->GetZaxis()->GetBinLowEdge(alphabin+1));

  ///////////////// Balance vs eta in bins of pT (for a given alpha cut)

  for (int ptbin = 1; ptbin <= mc3d[etabins[i].c_str()]->GetXaxis()->GetNbins(); ++ptbin) {

    cout << "\nGetting L3 corrections as function of eta" << endl;
    cout << "pT bin edges: " << mc3d[etabins[i].c_str()]->GetXaxis()->GetBinLowEdge(ptbin)
         << " to " << mc3d[etabins[i].c_str()]->GetXaxis()->GetBinLowEdge(ptbin+1) << endl;

    string ptstr = Form("%.0fto%.0f",
                        mc3d[etabins[i].c_str()]->GetXaxis()->GetBinLowEdge(ptbin),
                        mc3d[etabins[i].c_str()]->GetXaxis()->GetBinLowEdge(ptbin+1));

    // Process MC: get balance vs eta for this pT bin and alpha cut
    for (int etabin = 1; etabin <= mc3d[etabins[i].c_str()]->GetYaxis()->GetNbins(); ++etabin) {
      double val = mc3d[etabins[i].c_str()]->GetBinContent(ptbin, etabin, alphabin);
      double err = mc3d[etabins[i].c_str()]->GetBinError(ptbin, etabin, alphabin);

      vseta_mc->SetBinContent(etabin, val);
      vseta_mc->SetBinError(etabin, (TMath::IsNaN(err) ? 0. : err));
      aerrormc->SetBinContent(etabin, (TMath::IsNaN(err) ? 0. : err));
    }

    // Process Data: get balance vs eta for this pT bin and alpha cut
    for (int etabin = 1; etabin <= data3d[etabins[i].c_str()]->GetYaxis()->GetNbins(); ++etabin) {
      double val = data3d[etabins[i].c_str()]->GetBinContent(ptbin, etabin, alphabin);
      double err = data3d[etabins[i].c_str()]->GetBinError(ptbin, etabin, alphabin);

      vseta_data->SetBinContent(etabin, val);
      vseta_data->SetBinError(etabin, (TMath::IsNaN(err) ? 0. : err));
      aerrordt->SetBinContent(etabin, (TMath::IsNaN(err) ? 0. : err));
    }

    // Clone and save MC balance
    TH1D* balance_eta_mc = (TH1D*)vseta_mc->Clone(Form("mc_balance_pt%s_%s", ptstr.c_str(), alphastr.c_str()));
    balance_eta_mc->SetLineColor(kBlue);

    // Clone and save Data balance
    TH1D* balance_eta_data = (TH1D*)vseta_data->Clone(Form("dt_balance_pt%s_%s", ptstr.c_str(), alphastr.c_str()));
    balance_eta_data->SetLineColor(kRed);

    // L3 Residual correction = balance_MC / balance_data
    // This corrects data jets to match MC response
    // If data jets are lower (balance_data < balance_mc), correction > 1
    TH1D* l3res_eta = (TH1D*)balance_eta_mc->Clone(Form("l3res_pt%s_%s", ptstr.c_str(), alphastr.c_str()));
    l3res_eta->Divide(balance_eta_data);

    // Propagate errors properly
    for (int bin = 1; bin <= l3res_eta->GetXaxis()->GetNbins(); ++bin) {
      double mc_val = balance_eta_mc->GetBinContent(bin);
      double mc_err = balance_eta_mc->GetBinError(bin);
      double dt_val = balance_eta_data->GetBinContent(bin);
      double dt_err = balance_eta_data->GetBinError(bin);

      if (dt_val > 0 && mc_val > 0) {
        double ratio = mc_val / dt_val;
        double rel_err = sqrt(pow(mc_err/mc_val, 2) + pow(dt_err/dt_val, 2));
        l3res_eta->SetBinError(bin, ratio * rel_err);
      } else {
        l3res_eta->SetBinContent(bin, 1.0);
        l3res_eta->SetBinError(bin, 0.0);
      }
    }

    // Store for later use
    responses[Form("mc_pt%s_%s", ptstr.c_str(), alphastr.c_str())] = balance_eta_mc;
    responses[Form("dt_pt%s_%s", ptstr.c_str(), alphastr.c_str())] = balance_eta_data;
    respETA[ptbin] = (TH1D*)l3res_eta->Clone(Form("l3res_ptbin%d", ptbin));

    // Write histograms
    balance_eta_mc->Write();
    balance_eta_data->Write();
    l3res_eta->Write();
    respETA[ptbin]->Write(Form("ratio_pt%s_%s", ptstr.c_str(), alphastr.c_str()));
  }

  ///////////////// Balance vs alpha in bins of pT and eta (for ISR/FSR studies)

  cout << "\n\nNumber of alpha bins: " << mc3d[etabins[i].c_str()]->GetZaxis()->GetNbins() << endl;

  for (int ptbin = 1; ptbin <= mc3d[etabins[i].c_str()]->GetXaxis()->GetNbins(); ++ptbin) {
    cout << "\nNEW PT BIN " << ptbin << endl;

    for (int etabin = 1; etabin <= data3d[etabins[i].c_str()]->GetYaxis()->GetNbins(); ++etabin) {
      cout << "NEW ETA BIN " << data3d[etabins[i].c_str()]->GetYaxis()->GetBinLowEdge(etabin)
           << " to " << data3d[etabins[i].c_str()]->GetYaxis()->GetBinLowEdge(etabin+1) << endl;

      // Loop over alpha bins
      for (int abin = 1; abin <= mc3d[etabins[i].c_str()]->GetZaxis()->GetNbins(); ++abin) {
        // MC
        double val_mc = mc3d[etabins[i].c_str()]->GetBinContent(ptbin, etabin, abin);
        double err_mc = mc3d[etabins[i].c_str()]->GetBinError(ptbin, etabin, abin);
        vsalpha_mc->SetBinContent(abin, val_mc);
        vsalpha_mc->SetBinError(abin, (TMath::IsNaN(err_mc) ? 0. : err_mc));

        // Data
        double val_dt = data3d[etabins[i].c_str()]->GetBinContent(ptbin, etabin, abin);
        double err_dt = data3d[etabins[i].c_str()]->GetBinError(ptbin, etabin, abin);
        vsalpha_data->SetBinContent(abin, val_dt);
        vsalpha_data->SetBinError(abin, (TMath::IsNaN(err_dt) ? 0. : err_dt));
      }

      // Write balance vs alpha for MC and Data
      vsalpha_mc->Write(Form("Balance_vsa_mc_%d_%d", ptbin, etabin));
      vsalpha_data->Write(Form("Balance_vsa_data_%d_%d", ptbin, etabin));

      // Compute ratio (L3Res vs alpha)
      TH1D* vsalpha_ratio = (TH1D*)vsalpha_mc->Clone(Form("L3Res_vsa_%d_%d", ptbin, etabin));
      vsalpha_ratio->Divide(vsalpha_data);
      vsalpha_ratio->Write();

      // Normalize to the chosen alpha bin value
      TH1D* vsalpha_norm = (TH1D*)vsalpha_ratio->Clone(Form("L3Res_vsa_norm_%d_%d", ptbin, etabin));
      double norm = respETA[ptbin]->GetBinContent(etabin);
      if (norm > 0) {
        for (int bin = 1; bin <= vsalpha_norm->GetXaxis()->GetNbins(); ++bin) {
          double val = vsalpha_norm->GetBinContent(bin);
          double err = vsalpha_norm->GetBinError(bin);
          vsalpha_norm->SetBinContent(bin, val / norm);
          vsalpha_norm->SetBinError(bin, err / norm);
        }
      }
      vsalpha_norm->Write();
    }
  }

  ///////////////// Write L3 residual corrections in JEC text format

  TString txtfilename = outfilename;
  txtfilename.ReplaceAll(".root", ".txt");

  ofstream outtext(txtfilename.Data());
  outtext << "# L3 Residual Corrections from Photon+Jet Balancing" << endl;
  outtext << "# Format: {eta_min eta_max} N pt_min pt_max correction ..." << endl;
  outtext << "# Alpha cut bin: " << alphabin << " (alpha < "
          << mc3d[etabins[i].c_str()]->GetZaxis()->GetBinLowEdge(alphabin+1) << ")" << endl;
  outtext << "# MC file: " << mcFile << endl;
  outtext << "# Data file: " << dataFile << endl;

  int nEtaBins = mc3d[etabins[i].c_str()]->GetYaxis()->GetNbins();
  int nPtBins = mc3d[etabins[i].c_str()]->GetXaxis()->GetNbins();

  for (int etabin = 1; etabin <= nEtaBins; ++etabin) {
    float eta_min = mc3d[etabins[i].c_str()]->GetYaxis()->GetBinLowEdge(etabin);
    float eta_max = mc3d[etabins[i].c_str()]->GetYaxis()->GetBinLowEdge(etabin+1);

    outtext << "{" << eta_min << " " << eta_max << "} " << (2 + nPtBins * 3) << " ";

    // Get pT range for this eta bin
    float pt_min_overall = mc3d[etabins[i].c_str()]->GetXaxis()->GetBinLowEdge(1);
    float pt_max_overall = mc3d[etabins[i].c_str()]->GetXaxis()->GetBinLowEdge(nPtBins+1);
    outtext << pt_min_overall << " " << pt_max_overall << " ";

    for (int ptbin = 1; ptbin <= nPtBins; ++ptbin) {
      float pt_min = mc3d[etabins[i].c_str()]->GetXaxis()->GetBinLowEdge(ptbin);
      float pt_max = mc3d[etabins[i].c_str()]->GetXaxis()->GetBinLowEdge(ptbin+1);

      // Get L3 residual correction from stored histogram
      float correction = respETA[ptbin]->GetBinContent(etabin);

      // Sanity check
      if (correction < 0.5 || correction > 2.0 || TMath::IsNaN(correction)) {
        cout << "WARNING: eta [" << eta_min << ", " << eta_max
             << "], pT [" << pt_min << ", " << pt_max
             << "], L3Res = " << correction << endl;
        correction = 1.0;
      }

      outtext << pt_min << " " << pt_max << " " << correction << " ";
    }
    outtext << endl;
  }

  outtext.close();
  cout << "\nL3 residual corrections written to: " << txtfilename << endl;

  ///////////////// Create summary 2D maps

  TH2D* balanceMap_mc = new TH2D("balanceMap_mc", "MC Balance;p_{T,avg} (GeV);|#eta_{jet}|;Balance",
                                  nPtBins, mc3d[etabins[i].c_str()]->GetXaxis()->GetXmin(),
                                  mc3d[etabins[i].c_str()]->GetXaxis()->GetXmax(),
                                  nEtaBins, mc3d[etabins[i].c_str()]->GetYaxis()->GetXmin(),
                                  mc3d[etabins[i].c_str()]->GetYaxis()->GetXmax());

  TH2D* balanceMap_data = new TH2D("balanceMap_data", "Data Balance;p_{T,avg} (GeV);|#eta_{jet}|;Balance",
                                    nPtBins, mc3d[etabins[i].c_str()]->GetXaxis()->GetXmin(),
                                    mc3d[etabins[i].c_str()]->GetXaxis()->GetXmax(),
                                    nEtaBins, mc3d[etabins[i].c_str()]->GetYaxis()->GetXmin(),
                                    mc3d[etabins[i].c_str()]->GetYaxis()->GetXmax());

  TH2D* l3resMap = new TH2D("l3resMap", "L3 Residual (MC/Data);p_{T,avg} (GeV);|#eta_{jet}|;L3Res",
                            nPtBins, mc3d[etabins[i].c_str()]->GetXaxis()->GetXmin(),
                            mc3d[etabins[i].c_str()]->GetXaxis()->GetXmax(),
                            nEtaBins, mc3d[etabins[i].c_str()]->GetYaxis()->GetXmin(),
                            mc3d[etabins[i].c_str()]->GetYaxis()->GetXmax());

  for (int etabin = 1; etabin <= nEtaBins; ++etabin) {
    for (int ptbin = 1; ptbin <= nPtBins; ++ptbin) {
      double mc_val = mc3d[etabins[i].c_str()]->GetBinContent(ptbin, etabin, alphabin);
      double dt_val = data3d[etabins[i].c_str()]->GetBinContent(ptbin, etabin, alphabin);

      balanceMap_mc->SetBinContent(ptbin, etabin, mc_val);
      balanceMap_data->SetBinContent(ptbin, etabin, dt_val);

      if (dt_val > 0) {
        l3resMap->SetBinContent(ptbin, etabin, mc_val / dt_val);
      } else {
        l3resMap->SetBinContent(ptbin, etabin, 1.0);
      }
    }
  }

  balanceMap_mc->Write();
  balanceMap_data->Write();
  l3resMap->Write();

  // Save original 3D histograms for reference
  mc3d[etabins[i].c_str()]->Write("balance3D_mc");
  data3d[etabins[i].c_str()]->Write("balance3D_data");

  outfile->Close();
  cout << "Output ROOT file: " << outfilename << endl;

  inFileMC->Close();
  inFileDT->Close();
}
