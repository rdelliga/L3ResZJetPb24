#include "histograms.h"
// Should somehow control which binning is picked, flag?

 int pts[] = {40, 55, 80, 120, 170, 1000}; // Temporary

void dofits(float fitmin = 0.1, float fitmax = 0.4, string outfilename = "testfitresults.root", bool doabseta = true) {
  // input file is from 3D derivation
  gStyle->SetOptStat(0);

  TFile *inFile = new TFile("L2residuals_pbpbreco_from3Dlxplus_alpha03_rebin_abs.root", "READ");
  //TFile *inFile = new TFile("test.root", "READ");
  //   TFile *outfile = new TFile(outfilename.c_str(),"RECREATE");

  // ROOT::Fit::DataRange range(0.3,0.4);
  ROOT::Fit::DataOptions opt; 
  ROOT::Fit::DataRange range; 
  range.SetRange(fitmin,fitmax);

 

   //  mg->Add(g2,"L");
 

  // This is for correcton factors - hiso against eta
  auto factors = (TH1D*)inFile->Get("ratio_pt40to55_alpha0.2"); // There needs to be a switch...
  factors->Reset();

  // TODO: change this so that histogram contents are copied into TGraphs and the reference alpha is excluded
  // TODO: remove reference alpha from the fit
  int colours[] = {209, 226, 213, 51, 206};
  //  for (int etabin = 1; etabin < 37; ++etabin) { // TODO bins
  for (int etabin = 1; etabin < 19; ++etabin) { // TODO bins

     ROOT::Fit::BinData data(opt,range); 
     map<int, TH1D*> histos;
     map<int, TGraphErrors*> graphs;
     auto multifit = new TMultiGraph();

    //for (int etabin = 18; etabin < 19; ++etabin) { // TODO bins
     TCanvas *c1 = new TCanvas("c1","c1",800,600);

     auto leg = new TLegend(0.57,0.7,0.85,0.85); //  x, y, x, y
     leg->SetTextSize(0.03);
     leg->SetBorderSize(0);
     leg->SetFillStyle(0);


     
    //int etabin = 18;
    // pick histos for different pT:s
  for (int ptbin = 2; ptbin <= 5; ++ptbin) {
      if (etabin >= 14 and ptbin == 5) break;
      if (etabin >= 17 and ptbin == 4) break;
      histos[ptbin] = (TH1D*)inFile->Get(Form("Respvsa_norm_%d_%d",ptbin,etabin)); // TODO: histo name logic will change

    // TODO: Handle Graphs here?   

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
      ROOT::Fit::FillData(data, histos[ptbin]);   // TODO: can one fill tgraphs?
      // ROOT::Fit::FillData(data, graphs[ptbin]);   // TODO: Replace with TMultiGraph fitter
      multifit->Add(graphs[ptbin],"P");
      
      histos[ptbin]->SetMaximum(1.15);
      histos[ptbin]->SetMinimum(0.85);

      histos[ptbin]->GetXaxis()->SetTitle("#alpha");
      histos[ptbin]->GetYaxis()->SetTitle(" < MC / Data > / < MC / Data >_{#alpha < 0.2}");    // TODO: correc reference label
      
      histos[ptbin]->SetLineColor(colours[ptbin-1]);
      histos[ptbin]->Draw("same E1");

      leg->AddEntry(histos[ptbin], Form("%d < p_{T} < %d",pts[ptbin-1],pts[ptbin])); // TODO: correct bin edges
      // TODO: colours
    } 
  
  leg->Draw("same");
   
  TF1 * f1 = new TF1("f1","pol1",fitmin,fitmax);
  f1->SetParameters(1,0);

   ROOT::Math::WrappedTF1 wf(*f1);

  ROOT::Fit::Fitter fitter;
  fitter.SetFunction(wf);

  fitter.Fit(data);
  ROOT::Fit::FitResult result = fitter.Result();
  result.Print(std::cout);

  cout << result.Chi2() <<  " " << result.Ndf() << " " << result.Parameter(0) << " " << result.ParError(0) << endl;

  f1->Draw("same");

  // multifit->Fit("pol1");

  factors->SetBinContent(etabin, result.Parameter(0));
 factors->SetBinError(etabin, result.ParError(0));

  // TODO: debug this
  TLatex* txt = new TLatex();
  txt->SetTextSize(0.03);
  txt->SetNDC();

  if (!doabseta) txt->DrawLatex( 0.2, 0.8, Form("%.2f < #eta < %.2f",histograms::wetarange[etabin-1], histograms::wetarange[etabin])); // TODO: correct bin
  else txt->DrawLatex( 0.2, 0.8, Form("%.2f < |#eta| < %.2f",histograms::wabsetarange[etabin-1], histograms::wabsetarange[etabin])); // TODO: correct bin

  txt->DrawLatex( 0.2, 0.85, Form("p0 = %.5f #pm %.5f",result.Parameter(0), result.ParError(0)));

  
  c1->Print(Form("fitsrmptbins/fits_eta_%d.pdf",etabin));

  //    TCanvas *c3 = new TCanvas("c3","c3",800,600);
  //  graphs[3]->Draw("");
 

  }

  TCanvas *c2 = new TCanvas("c2","c2",800,600);
 
  if (doabseta)  factors->GetXaxis()->SetTitle(" |#eta| ");
  else  factors->GetXaxis()->SetTitle(" #eta ");
  // TODO: save this, save histogram
  factors->Draw();
  c2->Print("fitsrmptbins/corrections.pdf");
  
  // TODO: save extrapolations = par0 somehow. Those are in bins of eta. So TH1D created using same eta binning as in input, then do fits etc, calculate extrapolation
  // will need tgraph for doing properly?
  // then put value in the histogram, plot. how to handle errors? from fit?

  
}
