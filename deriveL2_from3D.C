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

void deriveL2_from3D(string inFileName = "HIJEC_results/rebin/pbpbreco_MC_lxplus.root",string inFileNameDT = "HIJEC_results/rebin/pbpbreco_DATA_lxplus.root", string outfilename = "L2residuals_pbpbreco_from3Dlxplus_alpha03_rebin.root", bool dodt = true,   int alphabin = 3) {

//void deriveL2_from3D(string inFileName = "HIJEC_results/ppreco_MC_lxplus.root",string inFileNameDT = "HIJEC_results/ppreco_DATA_lxplus.root", string outfilename = "test.root", bool dodt = true,   int alphabin = 2) {

  // Open file
  TFile *inFile = new TFile(inFileName.c_str(), "READ"); // TODO: safety checks about opening file successfully
  TFile *inFileDT = new TFile(inFileNameDT.c_str(), "READ"); // TODO: safety checks about opening file successfully

  //// These are bins to be processed
  vector<string> etabins = {"eta_-5.2_5.2"};
  int i = 0;

  map<string, TProfile*> asymm3d, data3d, mc3d;
  map<string, TH1D*> nom, denom, responses;
  map<int, TH1D*> respETA;
  // TODO: should one save responses is like 3d thing? means: how to save responses from the different alphas, actually.

  
  TFile *outfile = new TFile(outfilename.c_str(),"RECREATE");
  
  TH1D* vsalpha = new TH1D("testvsalpha","  ; ;",  histograms::nalphavalues, &histograms::alphavalues[0]);

  // Make these as histogram template, then a map, fill histograms in a map?
  
  TH1D* vsalpha_nom = new TH1D("vsalpha_nom","  ; ;",  histograms::nalphavalues, &histograms::alphavalues[0]);
  TH1D* vsalpha_denom = new TH1D("vsalpha_denom","  ; ;",  histograms::nalphavalues, &histograms::alphavalues[0]);

  TH1D* vsalpha_nom_data = new TH1D("vsalpha_nom_data","  ; ;",  histograms::nalphavalues, &histograms::alphavalues[0]);
  TH1D* vsalpha_denom_data = new TH1D("vsalpha_denom_data","  ; ;",  histograms::nalphavalues, &histograms::alphavalues[0]);

  TH1D* vseta_nom = new TH1D("vseta_nom","  ; ;",  histograms::nwetas, &histograms::wetarange[0]);
  TH1D* vseta_denom = new TH1D("vseta_denom","  ; ;",  histograms::nwetas, &histograms::wetarange[0]);


  TH1D* vseta_nom_data = new TH1D("vseta_nom_data","  ; ;",  histograms::nwetas, &histograms::wetarange[0]);
  TH1D* vseta_denom_data = new TH1D("vseta_denom_data","  ; ;",  histograms::nwetas, &histograms::wetarange[0]);
  
  
  asymm3d[etabins[i].c_str()] = (TProfile*)inFile->Get("hibin_-1.0_0.0/eta_-5.2_5.2/dijetasymmetry3D"); // Bins in order: pT, eta, alpha
  data3d[etabins[i].c_str()] = (TProfile*)inFileDT->Get("hibin_-1.0_0.0/eta_-5.2_5.2/dijetasymmetry3D"); // Bins in order: pT, eta, alpha


  cout << etabins[i] <<  " nptbins  " << asymm3d[etabins[i].c_str()]->GetXaxis()->GetNbins() << endl;
  //data3d[etabins[i].c_str()]->Draw();

///////////////// Responses against eta in bin of alpha cut, pt

  for (int ptbin = 1; ptbin <= asymm3d[etabins[i].c_str()]->GetXaxis()->GetNbins(); ++ptbin) {
    //    for (int ptbin = 1; ptbin < 5; ++ptbin) {

    cout << "Getting corrections as function of eta" << endl;
    cout << "pT bin edges: " << asymm3d[etabins[i].c_str()]->GetXaxis()->GetBinLowEdge(ptbin) << " " << asymm3d[etabins[i].c_str()]->GetXaxis()->GetBinLowEdge(ptbin+1) << endl;
    string ptstr = Form("%.0fto%.0f",asymm3d[etabins[i].c_str()]->GetXaxis()->GetBinLowEdge(ptbin),asymm3d[etabins[i].c_str()]->GetXaxis()->GetBinLowEdge(ptbin+1));
    cout << ptstr.c_str() << endl;
  //  data3d[etabins[i].c_str()]->Draw("same");

  cout << "alpha bin: " << alphabin << endl;
  cout << " alpha bin edges: " << asymm3d[etabins[i].c_str()]->GetZaxis()->GetBinLowEdge(alphabin) << " " << asymm3d[etabins[i].c_str()]->GetZaxis()->GetBinLowEdge(alphabin+1) << endl;
  string alphastr = Form("alpha%.1f",asymm3d[etabins[i].c_str()]->GetZaxis()->GetBinLowEdge(alphabin+1));

  // This is for MC
  for (int etabin = 1; etabin <= asymm3d[etabins[i].c_str()]->GetYaxis()->GetNbins(); ++etabin) {
      
    // cout <<  asymm3d[etabins[i].c_str()]->GetBinContent(ptbin,etabin,alphabin) << endl;
      
      double val = asymm3d[etabins[i].c_str()]->GetBinContent(ptbin,etabin,alphabin);
      double err = asymm3d[etabins[i].c_str()]->GetBinError(ptbin,etabin,alphabin);

      //      cout << "Check bins " << vseta_nom->GetBinLowEdge(etabin-1) << endl;
      //  cout << "Check bins " << asymm3d[etabins[i].c_str()]->GetYaxis()->GetBinLowEdge(etabin-1) << endl;
      
      vseta_nom->SetBinContent(etabin, 1+val);
      vseta_nom->SetBinError(etabin, err);

      vseta_denom->SetBinContent(etabin, 1-val);
      vseta_denom->SetBinError(etabin, err);

    }

  TH1D* resp_eta = (TH1D*)vseta_nom->Clone("resp_eta");
  resp_eta->Divide(vseta_denom);
  resp_eta->Draw();
   
  // This is for Data
    for (int etabin = 1; etabin <= data3d[etabins[i].c_str()]->GetYaxis()->GetNbins(); ++etabin) {
      
      //  cout << data3d[etabins[i].c_str()]->GetBinContent(ptbin,etabin,alphabin) << endl;
    
      double val = data3d[etabins[i].c_str()]->GetBinContent(ptbin,etabin,alphabin);
      double err = data3d[etabins[i].c_str()]->GetBinError(ptbin,etabin,alphabin);
     
      vseta_nom_data->SetBinContent(etabin, 1+val);
      vseta_nom_data->SetBinError(etabin, err);

      vseta_denom_data->SetBinContent(etabin, 1-val);
      vseta_denom_data->SetBinError(etabin, err);

    }

  // First part of the correction is the ratio of these
  TH1D* resp_eta_data = (TH1D*)vseta_nom_data->Clone("resp_eta_data");
    resp_eta_data->Divide(vseta_denom_data);
    resp_eta_data->SetLineColor(kRed);
    resp_eta_data->Draw("same");
  
    responses[Form("mc_pt%s_%s",ptstr.c_str(),alphastr.c_str())] = resp_eta; 

    responses[Form("dt_pt%s_%s",ptstr.c_str(),alphastr.c_str())] = resp_eta_data; 

    // responses[Form("ratio_pt%s_%s",ptstr.c_str(),alphastr.c_str())] = (TH1D*)resp_eta->Clone("ratio"); 
    //  responses[Form("ratio_pt%s_%s",ptstr.c_str(),alphastr.c_str())]->Divide(resp_eta_data);
    respETA[ptbin] = (TH1D*)resp_eta->Clone("ratio"); 
    respETA[ptbin]->Divide(resp_eta_data);

  // TODO: do this in a smart way
    responses[Form("mc_pt%s_%s",ptstr.c_str(),alphastr.c_str())]->Write(Form("mc_pt%s_%s",ptstr.c_str(),alphastr.c_str()));
    responses[Form("dt_pt%s_%s",ptstr.c_str(),alphastr.c_str())]->Write(Form("dt_pt%s_%s",ptstr.c_str(),alphastr.c_str()));
    //   responses[Form("ratio_pt%s_%s",ptstr.c_str(),alphastr.c_str())]->Write(Form("ratio_pt%s_%s",ptstr.c_str(),alphastr.c_str()));   // This is the reference for the fit
    respETA[ptbin]->Write(Form("ratio_pt%s_%s",ptstr.c_str(),alphastr.c_str()));   // This is the reference for the fit

  //  Form("mc_%s_%s",ptstr.c_str(),alphastr.c_str())

  }

  /// HERE THE ISR/FSR CORRECTIONS; need to get the responses in bins of pt, eta
  // TODO: check alpha logic
   cout << "number of alpha bins: " << asymm3d[etabins[i].c_str()]->GetZaxis()->GetNbins() << endl;

   
  // loop over bin in pt
  for (int ptbin = 1; ptbin <= asymm3d[etabins[i].c_str()]->GetXaxis()->GetNbins(); ++ptbin) {  // TODO: LIMIT
  // loop over bin in eta
     cout << "NEW PT BIN " << ptbin << endl;
     for (int etabin = 1; etabin <= data3d[etabins[i].c_str()]->GetYaxis()->GetNbins(); ++etabin) {
       cout << "NEW ETA BIN " << data3d[etabins[i].c_str()]->GetYaxis()->GetBinLowEdge(etabin) << " " << data3d[etabins[i].c_str()]->GetYaxis()->GetBinLowEdge(etabin+1) <<  endl;
  // loop over alphas and save in histogram            -            get from ratio or get from data and mc separately and get ratio?
  //     TH1D* respvsalpha = new TH1D("vsa", nalphavalues, &alphavalues[0]);
       
       for (int alphabin = 1; alphabin <= asymm3d[etabins[i].c_str()]->GetZaxis()->GetNbins(); ++alphabin) { // TODO: check bins
	 double val = asymm3d[etabins[i].c_str()]->GetBinContent(ptbin,etabin,alphabin);
	 double err = asymm3d[etabins[i].c_str()]->GetBinError(ptbin,etabin,alphabin);
	 
	       vsalpha_nom->SetBinContent(alphabin, 1+val); 
	       vsalpha_nom->SetBinError(alphabin, err);

	       vsalpha_denom->SetBinContent(alphabin, 1-val);
	       vsalpha_denom->SetBinError(alphabin, err); 

	 double valdt = data3d[etabins[i].c_str()]->GetBinContent(ptbin,etabin,alphabin);
	 double errdt = data3d[etabins[i].c_str()]->GetBinError(ptbin,etabin,alphabin);
	 
	       vsalpha_nom_data->SetBinContent(alphabin, 1+valdt); 
	       vsalpha_nom_data->SetBinError(alphabin, errdt);

	       vsalpha_denom_data->SetBinContent(alphabin, 1-valdt);
	       vsalpha_denom_data->SetBinError(alphabin, errdt); 

	       //cout <<  alphabin << " " << val << endl;

      }
     vsalpha_nom->Divide(vsalpha_denom);
     vsalpha_nom_data->Divide(vsalpha_denom_data);

     vsalpha_nom->Divide(vsalpha_nom_data); // This is the MC/Data responses in bin of alpha
     vsalpha_nom->Write(Form("Respvsa_%d_%d",ptbin,etabin));

     TH1D* vsalpha_norm = (TH1D*)vsalpha_nom->Clone(Form("vsalpha_norm_%d_%d",ptbin,etabin));
     for (int bin = 1; bin <= vsalpha_nom->GetXaxis()->GetNbins(); ++bin) {
         double val =  vsalpha_nom->GetBinContent(bin);
      	 double err = vsalpha_nom->GetBinError(bin);

	 double norm =  respETA[ptbin]->GetBinContent(etabin);
	 double normerr =  respETA[ptbin]->GetBinError(etabin);

	 vsalpha_norm->SetBinContent(bin,val/norm);
	 vsalpha_norm->SetBinError(bin,err/norm); // TODO: check this is correct

	 cout << "in alphabin " << alphabin << " norm " << norm << " lowedge " <<   vsalpha_norm->GetBinLowEdge(bin) << endl;

       
     }
     vsalpha_norm->Write(Form("Respvsa_norm_%d_%d",ptbin,etabin)); // This is a histogram that will be eventually fitted
     //    vsalpha_norm->Fit("pol1","","",0.2,0.5);  // Fit all pt bin at once?
  // fit histogram - do we fit all bins of pt at once actually?
  // save fits?
  // Save value at alpha = 0 in a histogram?
  // Should one actually put in tgraph of smth? for the fit
       
   }

   }


    // Linear fit -> But this is just A, need to calculate response and Data/MC ratio first, and determine value at alpha = 0.2 / 0.3 (Which one is proper working point?)
    // vsalpha->Fit("pol1","","",0.2,1);      // f, "", "", lower range, upper range
    // TODO: save extrapolation to alpha -> 0 in own histograms, with values of eta
 

   
}
