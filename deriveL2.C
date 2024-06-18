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

  vector<string> etabins = {"eta_-5.2_-3.9", "eta_-3.9_-2.6", "eta_-2.6_-1.3", "eta_-1.3_0.0", "eta_0.0_1.3", "eta_1.3_2.6", "eta_2.6_3.9", "eta_3.9_5.2"};

  map<string, TProfile*> asymm;
  map<string, TH1D*> nom, denom, response;

  TFile *outfile = new TFile("L2residuals.root","RECREATE");

  for (int i = 0; i < etabins.size(); i++) {
    
    asymm[etabins[i].c_str()] = (TProfile*)inFile->Get(Form("hibin_-1.0_0.0/%s/dijetasymmetry_a1",etabins[i].c_str())); // dijetasymmetry_a1 -> get this

    cout << etabins[i] << endl;


    nom[etabins[i].c_str()] = asymm[etabins[i].c_str()]->ProjectionX(Form("nom_%s",etabins[i].c_str()));
    denom[etabins[i].c_str()] = asymm[etabins[i].c_str()]->ProjectionX(Form("denom_%s",etabins[i].c_str()));

    nom[etabins[i].c_str()]->Reset();
    denom[etabins[i].c_str()]->Reset();

    
    for (int j = 1; j <= nom[etabins[i].c_str()]->GetNbinsX(); ++j) {
      //cout << nom->GetBinCenter(j) << endl;
      double as = asymm[etabins[i].c_str()]->GetBinContent(j);
      double err = asymm[etabins[i].c_str()]->GetBinError(j);
      
      nom[etabins[i].c_str()]->SetBinContent(j,1+as); 
      nom[etabins[i].c_str()]->SetBinError(j,err); 
      denom[etabins[i].c_str()]->SetBinContent(j,1-as);
      denom[etabins[i].c_str()]->SetBinError(j,err); 

    }

    response[etabins[i].c_str()] =  (TH1D*)nom[etabins[i].c_str()]->Clone(Form("response_%s",etabins[i].c_str()));
    response[etabins[i].c_str()]->Divide(denom[etabins[i].c_str()]);

    nom[etabins[i].c_str()]->Write();
    denom[etabins[i].c_str()]->Write();
    response[etabins[i].c_str()]->Write();

  }
   
}
