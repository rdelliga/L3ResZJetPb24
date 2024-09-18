

void dofits(float fitmin = 0.1, float fitmax = 0.4, string outfilename = "testfitresults.root") {
  // input file is from 3D derivation

  TFile *inFile = new TFile("L2residuals_pbpbreco_from3Dlxplus_alpha03_rebin.root", "READ");
  //TFile *inFile = new TFile("test.root", "READ");
  //   TFile *outfile = new TFile(outfilename.c_str(),"RECREATE");

  // ROOT::Fit::DataRange range(0.3,0.4);
  ROOT::Fit::DataOptions opt; 
  ROOT::Fit::DataRange range; 
  range.SetRange(fitmin,fitmax);

 

  // TODO: change this so that histogram contents are copied into TGraphs and the reference alpha is excluded
  // TODO: remove reference alpha from the fit
  
  for (int etabin = 1; etabin < 37; ++etabin) { // TODO bins

     ROOT::Fit::BinData data(opt,range); 
     map<int, TH1D*> histos;

    //for (int etabin = 18; etabin < 19; ++etabin) { // TODO bins
     TCanvas *c1 = new TCanvas("c1","c1",800,600);
    //int etabin = 18;
    // pick histos for different pT:s
  for (int ptbin = 1; ptbin <= 5; ++ptbin) {
      histos[ptbin] = (TH1D*)inFile->Get(Form("Respvsa_norm_%d_%d",ptbin,etabin)); // TODO: histo name logic will change

      ROOT::Fit::FillData(data, histos[ptbin]);   // TODO: can one fill tgraphs?

      histos[ptbin]->SetMaximum(1.15);
      histos[ptbin]->SetMinimum(0.85);
      histos[ptbin]->Draw("same");
      // TODO: colours
    }

  

   
  TF1 * f1 = new TF1("f1","pol1",fitmin,fitmax);
  f1->SetParameters(1,0);

  ROOT::Math::WrappedTF1 wf(*f1);

  ROOT::Fit::Fitter fitter;
  fitter.SetFunction(wf);

  fitter.Fit(data);
  ROOT::Fit::FitResult result = fitter.Result();
  result.Print(std::cout);

  cout << result.Chi2() <<  " " << result.Ndf() << " " << result.Parameter(0) << " " << result.ParError(0) << endl;

  // TODO: Get and save params somehow

  f1->Draw("same");

  c1->Print(Form("fits/fits_eta_%d.pdf",etabin));

  }
}
