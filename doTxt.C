void doTxt( string file = "testfitresults_onebyone.root", int nparams = 5) {

  int pts[] = {40, 55, 80, 120, 170, 1000}; // Temporary

  TFile *inFile = new TFile(file.c_str(), "READ");
  
  ofstream myfile;
  myfile.open ("example.txt");

  std::cout << "{ 1 JetEta 1 JetPt ([0]+[1]*log(x))*[2] Correction L2Relative }" << std::endl;
  myfile << "{ 1 JetEta 1 JetPt ([0]+[1]*log(x))*[2] Correction L2Relative }" << std::endl;
  

  map<int, TH1D*> facts;
  for (int ptbin = 2; ptbin <= 5; ++ptbin) {
    facts[ptbin] = (TH1D*)inFile->Get(Form("corrs_%dto%d",pts[ptbin-1],pts[ptbin])); // There needs to be a switch...
  }

  
  for (int etabin = facts[2]->GetXaxis()->GetNbins(); etabin >= 1; --etabin) {
        for (int ptbin = 2; ptbin < 5; ++ptbin) {
	  auto factors = facts[ptbin];
	  
       if (factors->GetXaxis()->GetBinLowEdge(etabin) < 3.0) {    myfile << -factors->GetXaxis()->GetBinLowEdge(etabin+1) << " " << -factors->GetXaxis()->GetBinLowEdge(etabin) << " " << nparams << " " << pts[ptbin] << " " << pts[ptbin+1] << " " << factors->GetBinContent(etabin) << "0" << " " << "1"
									    << std::endl;        
	}
     }
}
  
     for (int etabin = 1; etabin <= facts[2]->GetXaxis()->GetNbins(); ++etabin) {
         for (int ptbin = 2; ptbin < 5; ++ptbin) {
	  auto factors = facts[ptbin];
	  
       if (factors->GetXaxis()->GetBinLowEdge(etabin) < 3.0) {

	 myfile << factors->GetXaxis()->GetBinLowEdge(etabin) << " " << factors->GetXaxis()->GetBinLowEdge(etabin+1) << " " << nparams << " " << pts[ptbin] << " " << pts[ptbin+1] << " " << factors->GetBinContent(etabin) << " 0 1"  << std::endl; 
       
	}
     }
}


  myfile.close();

}
