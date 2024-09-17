

void dofits(float fitmin = 0.1, float fitmax = 0.4) {
  // input file is from 3D derivation

  TFile *inFile = new TFile("test.root", "READ"); 

  // ROOT::Fit::DataRange range(0.3,0.4);
  ROOT::Fit::DataOptions opt; 
  ROOT::Fit::DataRange range; 
  range.SetRange(fitmin,fitmax);

  ROOT::Fit::BinData data(opt,range); 
  map<int, TH1D*> histos;

  
  // We want to fit one eta bin

  // TODO: change this so that histogram contents are copied into TGraphs and the reference alpha is excluded 
  //  for (int etabin = 19; etabin < 20; ++etabin) { // TODO bins
  int etabin = 21;
    // pick histos for different pT:s
    for (int ptbin = 3; ptbin <= 5; ++ptbin) {
      histos[ptbin] = (TH1D*)inFile->Get(Form("Respvsa_norm_%d_%d",ptbin,etabin)); // TODO: histo name logic will change

      ROOT::Fit::FillData(data, histos[ptbin]);

      histos[ptbin]->SetMaximum(1.15);
      histos[ptbin]->SetMinimum(0.85);
      histos[ptbin]->Draw("same");
    }

    // }

   
  TF1 * f1 = new TF1("f1","pol1",fitmin,fitmax);
  f1->SetParameters(1,0);

  ROOT::Math::WrappedTF1 wf(*f1);

  ROOT::Fit::Fitter fitter;
  fitter.SetFunction(wf);

  fitter.Fit(data);
  ROOT::Fit::FitResult result = fitter.Result();
  result.Print(std::cout);

  cout << result.Chi2() <<  " " << result.Ndf() << " " << result.Parameter(0) << " " << result.ParError(0) << endl;

  // Get and save params somehow

  f1->Draw("same");

}
