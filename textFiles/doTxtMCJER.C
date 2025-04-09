// Create .txt file with the L2res corrections

void doTxtMCJER(int nparams = 6) { // fit params plus pt range
  static constexpr double etabins[] = {0.0, 0.522, 0.783, 1.044, 1.305, 1.566, 2.043, 2.322, 2.65, 2.853, 3.139, 3.485, 5.191};
  static constexpr unsigned int netabins = sizeof(etabins)/sizeof(etabins[0])-1;
 
  int pts[] = {15, 3000}; // pT just fullest possible range
 
  TFile *inFile = new TFile("../nscfits.root", "READ");
  // TFile *inFile2 = new TFile("", "READ");
  
  string header = "{1 JetEta 1 JetPt (sqrt([0]*abs([0])/(x*x)+[1]*[1]*pow(x,[3])+[2]*[2])) Resolution}";
  string filename = "jtptres.txt";
  
  ofstream txtfile;
  txtfile.open(filename.c_str());

  std::cout << "Producing txt file " << filename.c_str() << " with header " << header.c_str() << std::endl;
  txtfile << header.c_str() << std::endl;

  for (int etabin = netabins; etabin >= 1; --etabin) {   // These are the narrow eta bins
	  auto func = (TF1*)inFile->Get(Form("fit_eta_%.3fto%.3f",etabins[etabin-1],etabins[etabin])); // could do scanf
				
	  float p0 = func->GetParameter(0);
	  float p1 = func->GetParameter(1);
	  float p2 = func->GetParameter(2);
	  float p3 = func->GetParameter(3);

	  if (etabin == 1) txtfile << -etabins[etabin] << " " << etabins[etabin-1] << " " << nparams << " " << pts[0] << " " << pts[1] << " " << p0 << " "  << p1 << " "  << p2 << " "  << p3 << " " << std::endl;
	  else txtfile << -etabins[etabin] << " " << -etabins[etabin-1] << " " << nparams << " " << pts[0] << " " << pts[1] << " " << p0 << " "  << p1 << " "  << p2 << " "  << p3 << " " << std::endl; 
  }


  for (int etabin = 1; etabin <= netabins; ++etabin) {   // These are the narrow eta bins
	  auto func = (TF1*)inFile->Get(Form("fit_eta_%.3fto%.3f",etabins[etabin-1],etabins[etabin])); // could do scanf
				
	  float p0 = func->GetParameter(0);
	  float p1 = func->GetParameter(1);
	  float p2 = func->GetParameter(2);
	  float p3 = func->GetParameter(3);

	  txtfile << etabins[etabin-1] << " " << etabins[etabin] << " " << nparams << " " << pts[0] << " " << pts[1] << " " << p0 << " "  << p1 << " "  << p2 << " "  << p3 << " " << std::endl; 
  }

  txtfile.close();

}
