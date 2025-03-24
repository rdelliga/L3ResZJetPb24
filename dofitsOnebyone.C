#include "histograms.h"

// Do not fit the radiation correction factors but C directly to MC/Data ratio

int pts[] = {15, 25, 55, 80, 120, 170, 1000}; // Temporary
//int pts[] = {40, 55, 80, 120, 170, 1000}; // Temporary

int startbin = 4;
int endbin = 6;

//void dofitsOnebyone(float fitmin = 0.15, float fitmax = 0.45, string outfilename = "testfitresults_onebyone_wideeta.root", bool doabseta = false, bool dowideabseta = true, bool usenorm = false) {
void dofitsOnebyone(float fitmin = 0.15, float fitmax = 0.35, string outfilename = "RERUNALL/fits_HP_jetid_PFTRIG_a3.root", bool doabseta = true, bool dowideabseta = false, bool usenorm = false) {
  //void dofitsOnebyone(float fitmin = 0.15, float fitmax = 0.35, string outfilename = "RERUNALL/fits_ZB_jetid_a3.root", bool doabseta = true, bool dowideabseta = false, bool usenorm = false) {

// input file is from 3D derivation
  gStyle->SetOptStat(0);
  string outfolder = "RERUNALL/HP_PFTRIG_jetid_a3_fits_onebyone";
  //string outfolder = "RERUNALL/ZP_jetid_a3_fits_onebyone";

  //  TFile *inFile = new TFile("RERUNALL/L2residuals_pbpbreco_rerunall_zerobias_jetid.root", "READ");
  TFile *inFile = new TFile("RERUNALL/L2residuals_pbpbreco_rerunall_HP_PFTRIG_jetid_a3.root", "READ");  
  TFile *outfile = new TFile(outfilename.c_str(),"RECREATE");

   // Initialising histograms for correction factors  
  map<int, TH1D*> factors;
  for (int ptbin = startbin; ptbin <= endbin; ++ptbin) {
    //    factors[ptbin] = (TH1D*)inFile->Get("ratio_pt40to55_alpha0.2"); // There needs to be a switch...
    factors[ptbin] = (TH1D*)inFile->Get("ratio_pt25to55_alpha0.3"); // There needs to be a switch...
    factors[ptbin]->Reset();
  }
 
  int colours[] = {209, 226, 213, 51, 206};

  TLatex* txt = new TLatex();
  txt->SetTextSize(0.03);
  txt->SetNDC();
       
  for (int etabin = 1; etabin < 15; ++etabin) { // Too little stats elsewhere // For all etas
    //for (int etabin = 1; etabin < 8; ++etabin) { // Too little stats elsewhere // For wide eta
    // for (int etabin = 1; etabin < 2; ++etabin) { // TODO bins
 
     map<int, TH1D*> histos;
     map<int, TGraphErrors*> graphs;
     auto multifit = new TMultiGraph();

    //for (int etabin = 18; etabin < 19; ++etabin) { // TODO bins

     
    //int etabin = 18;
    // pick histos for different pT:s
     for (int ptbin = startbin; ptbin <= endbin; ++ptbin) {
      if (dowideabseta) {
	 if (etabin > 4 and ptbin == 6) break;   // UPDATE: 170-1000
	 if (etabin > 6 and ptbin == 5) break;   // UPDATE: 120-170
       }

       else {
	 if (etabin > 8 and ptbin == 6) break;
	 if (etabin > 11 and ptbin == 5) break;
	 }
  

    
      TCanvas *c1 = new TCanvas("c1","c1",800,600);

      auto leg = new TLegend(0.57,0.7,0.85,0.85); //  x, y, x, y
      leg->SetTextSize(0.03);
      leg->SetBorderSize(0);
      leg->SetFillStyle(0);

      if (usenorm) histos[ptbin] = (TH1D*)inFile->Get(Form("Respvsa_norm_%d_%d",ptbin,etabin)); // TODO: histo name logic will change
      else histos[ptbin] = (TH1D*)inFile->Get(Form("Respvsa_%d_%d",ptbin,etabin)); // TODO: histo name logic will change

      // These are for TGraphs : TODO alpha binning slightly off
      Double_t results[histograms::nalphavaluesgraph];
      Double_t errors[histograms::nalphavaluesgraph];
      Double_t errorsx[histograms::nalphavaluesgraph];
      
      for (int abin = 0; abin < histograms::nalphavaluesgraph; ++abin) {
	
        results[abin] =  histos[ptbin]->GetBinContent(abin+1);
	errors[abin] =  histos[ptbin]->GetBinError(abin+1);
	//		errors[abin] = 0;
	cout << "TEST: " << results[abin] << " " << errors[abin] << endl;
      }
      
      graphs[ptbin] = new TGraphErrors(histograms::nalphavaluesgraph, histograms::alphavaluesgraph, results, errorsx, errors); // TODO: do not include reference?
      //      cout << graphs[ptbin] << endl

      // histograms::alphavaluesgraph[]
      //ROOT::Fit::FillData(data, histos[ptbin]); 
      // ROOT::Fit::FillData(data, graphs[ptbin]); 
      // multifit->Add(graphs[ptbin],"P");
      
      histos[ptbin]->SetMaximum(1.15);
      histos[ptbin]->SetMinimum(0.85);

      histos[ptbin]->GetXaxis()->SetTitle("#alpha");
      
      if (usenorm) histos[ptbin]->GetYaxis()->SetTitle(" < MC / Data > / < MC / Data >_{#alpha < 0.2}");    // TODO: correc reference label
      else histos[ptbin]->GetYaxis()->SetTitle(" < MC / Data > ");    // TODO: correc reference label
      
      //     histos[ptbin]->SetLineColor(colours[ptbin-1]);
        //histos[ptbin]->Draw("same E1");
         histos[ptbin]->Draw("AXIS");
	 //graphs[ptbin]->Draw("same");
	 graphs[ptbin]->SetMarkerStyle(kFullCircle);
	 graphs[ptbin]->Draw("P");

      TF1 * f1 = new TF1("f1","pol1",fitmin,fitmax); // TODO: check that this range selection works
       f1->SetParameters(1,0);
       //histos[ptbin]->Fit("f1","R");
       graphs[ptbin]->Fit("f1","R");
       f1->Draw("same");
       
       // leg->AddEntry(histos[ptbin], Form("%d < p_{T} < %d",pts[ptbin-1],pts[ptbin])); // TODO: correct bin edges
      // TODO: colours

       if (doabseta) txt->DrawLatex( 0.2, 0.8, Form("%.2f < |#eta| < %.2f",histograms::wabsetarange[etabin-1], histograms::wabsetarange[etabin])); // TODO: correct bin
       else if (dowideabseta) txt->DrawLatex( 0.2, 0.8, Form("%.2f < |#eta| < %.2f",histograms::dwabsetarange[etabin-1], histograms::dwabsetarange[etabin])); // TODO: correct bin
       else txt->DrawLatex( 0.2, 0.8, Form("%.2f < #eta < %.2f",histograms::wetarange[etabin-1], histograms::wetarange[etabin])); // TODO: correct bin
       
       txt->DrawLatex( 0.2, 0.85, Form("p0 = %.5f #pm %.5f",f1->GetParameter(0), f1->GetParError(0)));
       txt->DrawLatex( 0.2, 0.75, Form("%d < p_{T} < %d",pts[ptbin-1],pts[ptbin]));
  
       if (dowideabseta) c1->Print(Form("%s/fits_eta_%d_pt_%d.png",outfolder.c_str(),etabin,ptbin));
	else c1->Print(Form("%s/fits_eta_%d_pt_%d.png",outfolder.c_str(),etabin,ptbin));

       factors[ptbin]->SetBinContent(etabin, f1->GetParameter(0));
       factors[ptbin]->SetBinError(etabin, f1->GetParError(0));
    } 
  
   }


  // TODO: axis labels, other fancies, ptbins
  for (int ptbin = startbin; ptbin <= endbin; ++ptbin) {
    TCanvas *c2 = new TCanvas("c2","c2",800,600);
 
    if (doabseta or dowideabseta)  factors[ptbin]->GetXaxis()->SetTitle(" |#eta| ");
    else  factors[ptbin]->GetXaxis()->SetTitle(" #eta ");

    if (!usenorm) factors[ptbin]->GetYaxis()->SetTitle(" < MC / Data >_{#alpha #rightarrow 0} ");    // TODO: correc reference label

    factors[ptbin]->SetMaximum(1.15);
    factors[ptbin]->SetMinimum(0.85); 
 
    factors[ptbin]->Draw("same");

    txt->DrawLatex( 0.2, 0.85, Form("%d < p_{T} < %d",pts[ptbin-1],pts[ptbin]));
    
    factors[ptbin]->Write(Form("corrs_%dto%d",pts[ptbin-1],pts[ptbin]));

    if (dowideabseta) c2->Print(Form("%s/corrections_%d.png",outfolder.c_str(),ptbin));
    else c2->Print(Form("%s/corrections_%d.png",outfolder.c_str(),ptbin));
  }
  
}
