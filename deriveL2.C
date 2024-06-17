// Derive responses from dijet asymmetries

//#include settings.h

// How stat errors actually work in this case?

void deriveL2(string inFileName = "output/dijet.root") {



  // Open file
  TFile *inFile = new TFile(inFileName.c_str(), "READ"); // TODO: safety checks about opening file successfully


  // Loop over etas - get eta range from where?
  
  // Get asymmetry histograms
  //TH1D* asymm = (TH1D*)inFile->Get("hbin_-1.0_0.0/eta_0.0_1.3/dijetasymmetry"); // dijetasymmetry_a1 -> get this
  // Should loop over eta bins
  TProfile* asymm = (TProfile*)inFile->Get("hibin_-1.0_0.0/eta_0.0_1.3/dijetasymmetry_a1"); // dijetasymmetry_a1 -> get this


  TH1D *nom = asymm->ProjectionX("nom");
  TH1D *denom = asymm->ProjectionX("denom");

  nom->Reset();
  denom->Reset();

  for (int i = 1; i <= nom->GetNbinsX(); ++i) {
    //cout << nom->GetBinCenter(i) << endl;
    double as = asymm->GetBinContent(i);
    double err = asymm->GetBinError(i);
    nom->SetBinContent(i,1+as); // ERRORS
    nom->SetBinError(i,err); // ERRORS
    denom->SetBinContent(i,1-as); // ERRORS
    denom->SetBinError(i,err); // ERRORS

  }
   

  //  nom->Add(de,1);
  //  denom->Add(asymm,-1);
  // Need Binning from above
  //TH1D* response = (TH1D*)asymm->Clone("response");

  //nom->Divide(nom,denom,1,1,"B");
  nom->Divide(denom);
  nom->Draw();

  TFile *outfile = new TFile("L2residuals.root","RECREATE");
  nom->Write();
}
