// Derive responses from dijet asymmetries using the 3D profile

//#include settings.h
#include "histograms.h"

// How stat errors actually work in this case?
/*
  string intputMC = "results/pbpbreco_witholdmctruth_abseta.root";
  string inputData = "results/ppreco_witholdmctruth_abseta.root";
*/

// Want to
// - fill 1-A/1+A histograms
// - calculate responses
// - these are done in bins of pt and eta
// - do alpha-extrapolation
// - need both data and MC


//void deriveL2(string inFileName = "results/pbpbreco_witholdmctruth_abseta.root", string outfilename = "L2residuals_pbpbreco.root") {
//void deriveL2_from3D(string inFileName = "ppreco_MC_test3D.root", string outfilename = "L2residuals_ppreco_from3D.root", bool dodt = true) {

//void deriveL2_from3D(string inFileName = "HIJEC_results/pbpbreco_MC_lxplus.root",string inFileNameDT = "HIJEC_results/pbpbreco_DATA_lxplus.root", string outfilename = "L2residuals_pbpbreco_from3Dlxplus_alpha03.root", bool dodt = true,   int alphabin = 3) {

void deriveL2_from3D(string inFileName = "HIJEC_results/ppreco_MC_lxplus.root",string inFileNameDT = "HIJEC_results/ppreco_DATA_lxplus.root", string outfilename = "L2residuals_ppreco_from3Dlxplus_alpha02.root", bool dodt = true,   int alphabin = 2) {

  // Open file
  TFile *inFile = new TFile(inFileName.c_str(), "READ"); // TODO: safety checks about opening file successfully
  TFile *inFileDT = new TFile(inFileNameDT.c_str(), "READ"); // TODO: safety checks about opening file successfully

  //// These are bins to be processed
  vector<string> etabins = {"eta_-5.2_5.2"};
  int i = 0;
  //vector<string> ptbins = {"eta_-5.2_5.2"}; // TODO: is this needed?

  int ptbin = 2; // Maybe loop over bins?
  // check how these correspond!!!!!!!!!!

  map<string, TProfile*> asymm3d, data3d, mc3d;
  map<string, TH1D*> nom, denom, responses, respETA;
  // TODO: should one save responses is like 3d thing? means: how to save responses from the different alphas, actually.

  
  TFile *outfile = new TFile(outfilename.c_str(),"RECREATE");
  
  TH1D* vsalpha = new TH1D("testvsalpha","  ; ;",  histograms::nalphavalues, &histograms::alphavalues[0]);

  // Make these as histogram template, then a map, fill histograms in a map?
  
  TH1D* vsalpha_nom = new TH1D("vsalpha_nom","  ; ;",  histograms::nalphavalues, &histograms::alphavalues[0]);
  TH1D* vsalpha_denom = new TH1D("vsalpha_denom","  ; ;",  histograms::nalphavalues, &histograms::alphavalues[0]);

  TH1D* vseta_nom = new TH1D("vseta_nom","  ; ;",  histograms::nwetas, &histograms::wetarange[0]);
  TH1D* vseta_denom = new TH1D("vseta_denom","  ; ;",  histograms::nwetas, &histograms::wetarange[0]);


  TH1D* vseta_nom_data = new TH1D("vseta_nom_data","  ; ;",  histograms::nwetas, &histograms::wetarange[0]);
  TH1D* vseta_denom_data = new TH1D("vseta_denom_data","  ; ;",  histograms::nwetas, &histograms::wetarange[0]);
  
     
  asymm3d[etabins[i].c_str()] = (TProfile*)inFile->Get("hibin_-1.0_0.0/eta_-5.2_5.2/dijetasymmetry3D"); // Bins in order: pT, eta, alpha
  data3d[etabins[i].c_str()] = (TProfile*)inFileDT->Get("hibin_-1.0_0.0/eta_-5.2_5.2/dijetasymmetry3D"); // Bins in order: pT, eta, alpha

  cout << etabins[i] << endl;
  //data3d[etabins[i].c_str()]->Draw();

///////////////// Start development from here

//TODO: make this a pt bin loop

  for (int ptbin = 1; ptbin < 7; ++ptbin) {

    cout << "Getting corrections as function of eta" << endl;
    cout << "pT bin edges: " << asymm3d[etabins[i].c_str()]->GetXaxis()->GetBinLowEdge(ptbin) << " " << asymm3d[etabins[i].c_str()]->GetXaxis()->GetBinLowEdge(ptbin+1) << endl;
    string ptstr = Form("%.0fto%.0f",asymm3d[etabins[i].c_str()]->GetXaxis()->GetBinLowEdge(ptbin),asymm3d[etabins[i].c_str()]->GetXaxis()->GetBinLowEdge(ptbin+1));
    cout << ptstr.c_str() << endl;
  //  data3d[etabins[i].c_str()]->Draw("same");

  cout << "check alpha bin: " << asymm3d[etabins[i].c_str()]->GetZaxis()->GetNbins() << endl;
  cout << " bin edges: " << asymm3d[etabins[i].c_str()]->GetZaxis()->GetBinLowEdge(alphabin) << " " << asymm3d[etabins[i].c_str()]->GetZaxis()->GetBinLowEdge(alphabin+1) << endl;
  string alphastr = Form("alpha%.1f",asymm3d[etabins[i].c_str()]->GetZaxis()->GetBinLowEdge(alphabin+1));

  // This is for MC
  for (int etabin = 1; etabin <= asymm3d[etabins[i].c_str()]->GetYaxis()->GetNbins(); ++etabin) {
      
    // cout <<  asymm3d[etabins[i].c_str()]->GetBinContent(ptbin,etabin,alphabin) << endl;
      
      double val = asymm3d[etabins[i].c_str()]->GetBinContent(ptbin,etabin,alphabin);
      double err = asymm3d[etabins[i].c_str()]->GetBinError(ptbin,etabin,alphabin);

      vseta_nom->SetBinContent(etabin-1, 1+val);
      vseta_nom->SetBinError(etabin-1, err);

      vseta_denom->SetBinContent(etabin-1, 1-val);
      vseta_denom->SetBinError(etabin-1, err);

    }

  TH1D* resp_eta = (TH1D*)vseta_nom->Clone("resp_eta");
  respETA[Form("resp_eta_%d",ptbin)] = (TH1D*)vseta_nom->Clone("resp_eta");
  resp_eta->Divide(vseta_denom);
  resp_eta->Draw();
    // Response is the ratio of these

  
  // This is for Data
    for (int etabin = 1; etabin <= data3d[etabins[i].c_str()]->GetYaxis()->GetNbins(); ++etabin) {
      
      //  cout << data3d[etabins[i].c_str()]->GetBinContent(ptbin,etabin,alphabin) << endl;
    
      double val = data3d[etabins[i].c_str()]->GetBinContent(ptbin,etabin,alphabin);
      double err = data3d[etabins[i].c_str()]->GetBinError(ptbin,etabin,alphabin);
     
      vseta_nom_data->SetBinContent(etabin-1, 1+val);
      vseta_nom_data->SetBinError(etabin-1, err);

      vseta_denom_data->SetBinContent(etabin-1, 1-val);
      vseta_denom_data->SetBinError(etabin-1, err);

    }

  // Response is the ratio of these
  TH1D* resp_eta_data = (TH1D*)vseta_nom_data->Clone("resp_eta_data");
  resp_eta_data->Divide(vseta_denom_data);
  resp_eta_data->SetLineColor(kRed);
  resp_eta_data->Draw("same");

   responses[Form("mc_pt%s_%s",ptstr.c_str(),alphastr.c_str())] = resp_eta; // Loop somehow to save these

  // TODO: data handling; this is a placeholder
    responses[Form("dt_pt%s_%s",ptstr.c_str(),alphastr.c_str())] = resp_eta_data; // Loop somehow to save these

    responses[Form("ratio_pt%s_%s",ptstr.c_str(),alphastr.c_str())] = (TH1D*)resp_eta->Clone("ratio"); // Loop somehow to save these
    responses[Form("ratio_pt%s_%s",ptstr.c_str(),alphastr.c_str())]->Divide(resp_eta_data);

  // TODO: do this in a smart way
    responses[Form("mc_pt%s_%s",ptstr.c_str(),alphastr.c_str())]->Write(Form("mc_pt%s_%s",ptstr.c_str(),alphastr.c_str()));
    responses[Form("dt_pt%s_%s",ptstr.c_str(),alphastr.c_str())]->Write(Form("dt_pt%s_%s",ptstr.c_str(),alphastr.c_str()));
    responses[Form("ratio_pt%s_%s",ptstr.c_str(),alphastr.c_str())]->Write(Form("ratio_pt%s_%s",ptstr.c_str(),alphastr.c_str()));
  //  Form("mc_%s_%s",ptstr.c_str(),alphastr.c_str())

  }
  
  // Need to save the ratios too
  
    // TODO: in different bins of pt, save, also data?
    // Make map(s)? -> then save later


    /// HERE THE ISR/FSR CORRECTIONS; need to get the responses in bins of pt, eta
    // so maybe above actually do 3D map thing?
    // Could do just 2D/3D histograms? maybe easier to deal with 1Ds?

    
    
    /* cout << "LOOP OVER BINS OF ALPHA - these are asymmetries in a selected bin of pt, eta" << endl;
    int    etabin = 2; 

    // Do in bins of pT, eta: as function of alpha
    for (int alpha = 1; alpha <= asymm3d[etabins[i].c_str()]->GetZaxis()->GetNbins(); ++alpha) {
      cout <<  asymm3d[etabins[i].c_str()]->GetBinContent(ptbin,etabin,alpha) << endl;
      double val =   asymm3d[etabins[i].c_str()]->GetBinContent(ptbin,etabin,alpha);
      double err =   asymm3d[etabins[i].c_str()]->GetBinError(ptbin,etabin,alpha);

      
      vsalpha_nom->SetBinContent(alpha-1, 1+val); // histogram bins start from 0
      vsalpha_nom->SetBinError(alpha-1, err); // histogram bins start from 0

      vsalpha_denom->SetBinContent(alpha-1, 1-val); // histogram bins start from 0
      vsalpha_denom->SetBinError(alpha-1, err); // histogram bins start from 0
  
    }
    vsalpha_denom->Draw();
    vsalpha_nom->Draw("same"); */

    // Linear fit -> But this is just A, need to calculate response and Data/MC ratio first, and determine value at alpha = 0.2 / 0.3 (Which one is proper working point?)
    // vsalpha->Fit("pol1","","",0.2,1);      // f, "", "", lower range, upper range
    // TODO: save extrapolation to alpha -> 0 in own histograms, with values of eta
    
    // Do in bins of pT, alpha: as function of eta

    
   
    /*     cout << asymm3d[etabins[i].c_str()]->GetBinContent(ptbin,1,1) << endl;
    cout << asymm3d[etabins[i].c_str()]->GetYaxis()->GetBinLowEdge(1) << endl;
    cout << asymm3d[etabins[i].c_str()]->GetYaxis()->GetNbins() << endl;
    cout << asymm3d[etabins[i].c_str()]->GetYaxis()->GetBinLowEdge(asymm3d[etabins[i].c_str()]->GetYaxis()->GetNbins() +1 ) << endl; //? */
    

  

    /* nom[etabins[i].c_str()]->Write();
    denom[etabins[i].c_str()]->Write();
    response[etabins[i].c_str()]->Write(); */

   
}
