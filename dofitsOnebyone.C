#include "histograms.h"

// Do not fit the radiation correctoin factors but C directly to MC/Data ratio

 int pts[] = {40, 55, 80, 120, 170, 1000}; // Temporary


// usenorm: do correction factors as pp style radiation corrections
void dofitsOnebyone(float fitmin = 0.1, float fitmax = 0.45, string outfilename = "testfitresults_onebyone.root", bool doabseta = true, bool usenorm = false) {
  // input file is from 3D derivation
  gStyle->SetOptStat(0);

  TFile *inFile = new TFile("L2residuals_pbpbreco_from3Dlxplus_alpha03_rebin_abs.root", "READ");
  //TFile *inFile = new TFile("test.root", "READ");
  TFile *outfile = new TFile(outfilename.c_str(),"RECREATE");

  // ROOT::Fit::DataRange range(0.3,0.4);
  ROOT::Fit::DataOptions opt; 
  ROOT::Fit::DataRange range; 
  range.SetRange(fitmin,fitmax);
  
   //  mg->Add(g2,"L");
 
  // Initialising histograms for correction factors  
  map<int, TH1D*> factors;
  for (int ptbin = 2; ptbin <= 5; ++ptbin) {
    factors[ptbin] = (TH1D*)inFile->Get("ratio_pt40to55_alpha0.2"); // There needs to be a switch...
    factors[ptbin]->Reset();
  }
  // TODO: change this so that histogram contents are copied into TGraphs and the reference alpha is excluded
  // TODO: remove reference alpha from the fit
  int colours[] = {209, 226, 213, 51, 206};
  //  for (int etabin = 1; etabin < 37; ++etabin) { // TODO bins
  for (int etabin = 1; etabin < 19; ++etabin) { // TODO bins
    // for (int etabin = 1; etabin < 2; ++etabin) { // TODO bins
 
     ROOT::Fit::BinData data(opt,range); 
     map<int, TH1D*> histos;
     map<int, TGraphErrors*> graphs;
     auto multifit = new TMultiGraph();

    //for (int etabin = 18; etabin < 19; ++etabin) { // TODO bins

     
    //int etabin = 18;
    // pick histos for different pT:s
  for (int ptbin = 2; ptbin <= 5; ++ptbin) {
      if (etabin >= 14 and ptbin == 5) break;
      if (etabin >= 17 and ptbin == 4) break;
      
      TCanvas *c1 = new TCanvas("c1","c1",800,600);

      auto leg = new TLegend(0.57,0.7,0.85,0.85); //  x, y, x, y
      leg->SetTextSize(0.03);
      leg->SetBorderSize(0);
      leg->SetFillStyle(0);

      if (usenorm) histos[ptbin] = (TH1D*)inFile->Get(Form("Respvsa_norm_%d_%d",ptbin,etabin)); // TODO: histo name logic will change
      else histos[ptbin] = (TH1D*)inFile->Get(Form("Respvsa_%d_%d",ptbin,etabin)); // TODO: histo name logic will change

      // These are for TGraphs
      Double_t results[histograms::nalphavalues];
      Double_t errors[histograms::nalphavalues];
      Double_t errorsx[histograms::nalphavalues];
      
      for (int abin = 0; abin < histograms::nalphavalues; ++abin) {
	
        results[abin] =  histos[ptbin]->GetBinContent(abin+1);
	errors[abin] =  histos[ptbin]->GetBinContent(abin+1);
	errors[abin] = 0; 
      }
      
      graphs[ptbin] = new TGraphErrors(histograms::nalphavalues, histograms::alphavalues, results, errorsx, errors); // TODO: do not include reference?
      //      cout << graphs[ptbin] << endl

      // histograms::alphavalues[]
      // ROOT::Fit::FillData(data, histos[ptbin]);   // TODO: can one fill tgraphs?
      // ROOT::Fit::FillData(data, graphs[ptbin]);   // TODO: Replace with TMultiGraph fitter
      // multifit->Add(graphs[ptbin],"P");
      
      histos[ptbin]->SetMaximum(1.15);
      histos[ptbin]->SetMinimum(0.85);

      histos[ptbin]->GetXaxis()->SetTitle("#alpha");
      
      if (usenorm) histos[ptbin]->GetYaxis()->SetTitle(" < MC / Data > / < MC / Data >_{#alpha < 0.2}");    // TODO: correc reference label
      else histos[ptbin]->GetYaxis()->SetTitle(" < MC / Data > ");    // TODO: correc reference label
      
      //     histos[ptbin]->SetLineColor(colours[ptbin-1]);
      histos[ptbin]->Draw("same E1");

      TF1 * f1 = new TF1("f1","pol1",fitmin,fitmax); // TODO: check that this range selection works
       f1->SetParameters(1,0);
       histos[ptbin]->Fit("f1","R");
       f1->Draw("same");
       
       // leg->AddEntry(histos[ptbin], Form("%d < p_{T} < %d",pts[ptbin-1],pts[ptbin])); // TODO: correct bin edges
      // TODO: colours


       TLatex* txt = new TLatex();
       txt->SetTextSize(0.03);
       txt->SetNDC();

       if (!doabseta) txt->DrawLatex( 0.2, 0.8, Form("%.2f < #eta < %.2f",histograms::wetarange[etabin-1], histograms::wetarange[etabin])); // TODO: correct bin
       else txt->DrawLatex( 0.2, 0.8, Form("%.2f < |#eta| < %.2f",histograms::wabsetarange[etabin-1], histograms::wabsetarange[etabin])); // TODO: correct bin

       txt->DrawLatex( 0.2, 0.85, Form("p0 = %.5f #pm %.5f",f1->GetParameter(0), f1->GetParError(0)));
       txt->DrawLatex( 0.2, 0.75, Form("%d < p_{T} < %d",pts[ptbin-1],pts[ptbin]));
  
       c1->Print(Form("fitsrmptbins_onebyone/fits_eta_%d_pt_%d.pdf",etabin,ptbin));


       // TODO: factors TH1D per pt bin
       
       factors[ptbin]->SetBinContent(etabin, f1->GetParameter(0));
       factors[ptbin]->SetBinError(etabin, f1->GetParError(0));

       //    TCanvas *c3 = new TCanvas("c3","c3",800,600);
       //  graphs[3]->Draw("");
 

    } 
  
  // leg->Draw("same");
   
 

  // cout << result.Chi2() <<  " " << result.Ndf() << " " << result.Parameter(0) << " " << result.ParError(0) << endl;



  }


  // TODO: axis labels, other fancies, ptbins
 for (int ptbin = 2; ptbin <= 5; ++ptbin) {
    TCanvas *c2 = new TCanvas("c2","c2",800,600);
 
    if (doabseta)  factors[ptbin]->GetXaxis()->SetTitle(" |#eta| ");
    else  factors[ptbin]->GetXaxis()->SetTitle(" #eta ");

    factors[ptbin]->SetMaximum(1.15);
    factors[ptbin]->SetMinimum(0.85); 
 
    factors[ptbin]->Draw("same");
    factors[ptbin]->Write(Form("corrs_%dto%d",pts[ptbin-1],pts[ptbin]));

    c2->Print(Form("fitsrmptbins_onebyone/corrections_%d.pdf",ptbin));
 }

  
  // TODO: save extrapolations = par0 somehow. Those are in bins of eta. So TH1D created using same eta binning as in input, then do fits etc, calculate extrapolation
  // will need tgraph for doing properly?
  // then put value in the histogram, plot. how to handle errors? from fit?

  
}
