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

void deriveL2_from3D(string inFileName = "HIJEC_results/pbpbreco_MC_lxplus.root",string inFileNameDT = "HIJEC_results/pbpbreco_DATA_lxplus.root", string outfilename = "L2residuals_ppreco_from3Dlpxlus.root", bool dodt = true) {

  // Open file
  TFile *inFile = new TFile(inFileName.c_str(), "READ"); // TODO: safety checks about opening file successfully
  TFile *inFileDT = new TFile(inFileNameDT.c_str(), "READ"); // TODO: safety checks about opening file successfully

  //// These are bins to be processed
  vector<string> etabins = {"eta_-5.2_5.2"};
  int i = 0;
  //vector<string> ptbins = {"eta_-5.2_5.2"}; // TODO: is this needed?

  int ptbin = 2; // Maybe loop over bins?

  map<string, TProfile*> asymm3d, data3d, mc3d;
  map<string, TH1D*> nom, denom, responses;
  // TODO: should one save responses is like 3d thing? means: how to save responses from the different alphas, actually.

  
  TFile *outfile = new TFile(outfilename.c_str(),"RECREATE");
  
  TH1D* vsalpha = new TH1D("testvsalpha","  ; ;",  histograms::nalphavalues, &histograms::alphavalues[0]);
  
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

  cout << "Getting corrections as function of eta" << endl;
  cout << "pT bin edges: " << asymm3d[etabins[i].c_str()]->GetXaxis()->GetBinLowEdge(ptbin) << " " << asymm3d[etabins[i].c_str()]->GetXaxis()->GetBinLowEdge(ptbin+1) << endl;

  //  data3d[etabins[i].c_str()]->Draw("same");

  // This is for MC
  for (int etabin = 1; etabin <= asymm3d[etabins[i].c_str()]->GetYaxis()->GetNbins(); ++etabin) {
      
      cout <<  asymm3d[etabins[i].c_str()]->GetBinContent(ptbin,etabin,1) << endl;
      
      double val = asymm3d[etabins[i].c_str()]->GetBinContent(ptbin,etabin,1);
      double err = asymm3d[etabins[i].c_str()]->GetBinError(ptbin,etabin,1);

      //double val = data3d[etabins[i].c_str()]->GetBinContent(ptbin,etabin,1);
      //double err = data3d[etabins[i].c_str()]->GetBinError(ptbin,etabin,1);

      
      vseta_nom->SetBinContent(etabin-1, 1+val);
      vseta_nom->SetBinError(etabin-1, err);

      vseta_denom->SetBinContent(etabin-1, 1-val);
      vseta_denom->SetBinError(etabin-1, err);

    }

  TH1D* resp_eta = (TH1D*)vseta_nom->Clone("resp_eta");
  resp_eta->Divide(vseta_denom);
  resp_eta->Draw();
    // Response is the ratio of these

  
  // This is for Data
    for (int etabin = 1; etabin <= data3d[etabins[i].c_str()]->GetYaxis()->GetNbins(); ++etabin) {
      
      cout << data3d[etabins[i].c_str()]->GetBinContent(ptbin,etabin,1) << endl;
    
      double val = data3d[etabins[i].c_str()]->GetBinContent(ptbin,etabin,1);
      double err = data3d[etabins[i].c_str()]->GetBinError(ptbin,etabin,1);
     
      vseta_nom_data->SetBinContent(etabin-1, 1+val);
      vseta_nom_data->SetBinError(etabin-1, err);

      vseta_denom_data->SetBinContent(etabin-1, 1-val);
      vseta_denom_data->SetBinError(etabin-1, err);

    }

  TH1D* resp_eta_data = (TH1D*)vseta_nom_data->Clone("resp_eta_data");
  resp_eta_data->Divide(vseta_denom_data);
  resp_eta_data->SetLineColor(kRed);
  resp_eta_data->Draw("same");


   // Response is the ratio of these

  

  responses["mc_pt40to60_alpha02"] = resp_eta; // Loop somehow to save these



  // TODO: data handling; this is a placeholder
  responses["dt_pt40to60_alpha02"] = resp_eta; // Loop somehow to save these


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
    

  
   
    

    // Need to do nominator and denominator with bin content


    // TODO: can't use projection like this?
    // R in bins of pt, eta
    // nom[etabins[i].c_str()] = asymm[etabins[i].c_str()]->ProjectionX(Form("nom_%s",etabins[i].c_str()));
    //  denom[etabins[i].c_str()] = asymm[etabins[i].c_str()]->ProjectionX(Form("denom_%s",etabins[i].c_str()));

    // nom[etabins[i].c_str()]->Reset();
    // denom[etabins[i].c_str()]->Reset();

    
    /* for (int j = 1; j <= nom[etabins[i].c_str()]->GetNbinsX(); ++j) {
      //cout << nom->GetBinCenter(j) << endl;
        double as = asymm[etabins[i].c_str()]->GetBinContent(j);
       double err = asymm[etabins[i].c_str()]->GetBinError(j);
      
      nom[etabins[i].c_str()]->SetBinContent(j,1+as); 
      nom[etabins[i].c_str()]->SetBinError(j,err); 
      denom[etabins[i].c_str()]->SetBinContent(j,1-as);
      denom[etabins[i].c_str()]->SetBinError(j,err); 

      } */ 

    // response[etabins[i].c_str()] =  (TH1D*)nom[etabins[i].c_str()]->Clone(Form("response_%s",etabins[i].c_str()));
    // response[etabins[i].c_str()]->Divide(denom[etabins[i].c_str()]);

    /* nom[etabins[i].c_str()]->Write();
    denom[etabins[i].c_str()]->Write();
    response[etabins[i].c_str()]->Write(); */

   
}
