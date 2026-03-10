// Print uncertainty component loglin vs. const fit for L2 residual
// Takes as input a file with fit functions, and also the file containing k factors (eta bins read from the histogram used to store the factors)

int nparams = 5;

float pts[] = {5, 10, 15};
int npts = 3;
  
void L2res_uncertainty(TString inFileName = "ptfits/testing_pt_dep.root", TString kfactFile = "L2fits_AK4CHSjetveto_all/correctionfile.root", TString outFileName = "uncertainty-param.txt") {

   TFile *inFile = new TFile(inFileName, "READ");
   TFile *inKfacts = new TFile(kfactFile, "READ");

   TFile *inFilesys = new TFile(inFileName, "READ");
   TFile *inKfactssys = new TFile(kfactFile, "READ");

   auto kfacts = (TH1D*)inKfacts->Get("ratio");
   auto kfactssys = (TH1D*)inKfacts->Get("ratio");

   // ([0]+[1]*log(x))*[2]
   
   string header = "{1 JetEta 1 JetPt "" Correction Uncertainty}";
   //    string header = "{1 JetEta 1 JetPt "" Correction JECSource}";
   
   ofstream txtfile;
   txtfile.open(outFileName);
   
   std::cout << "Producing txt file " << outFileName << " with header " << header.c_str() << std::endl;
   txtfile << header.c_str() << std::endl;

   map<int, TH1D*> facts;
 
   float ptmin = 25; float ptmax = 1000;
   for (int etabin = kfacts->GetXaxis()->GetNbins(); etabin >= 1; --etabin) {

     float loweta = kfacts->GetXaxis()->GetBinLowEdge(etabin+1);
     float higheta = kfacts->GetXaxis()->GetBinLowEdge(etabin);

     auto params_loglin =  (TF1*)inFile->Get(Form("loglin_eta%d",  etabin));
     auto params_const =  (TF1*)inFile->Get(Form("const_eta%d",  etabin));
     
     if (kfacts->GetXaxis()->GetBinLowEdge(etabin) < 2.9) {

       //  cout << loweta << " " << higheta << " " << npts*3;
       if (higheta!=0) txtfile << -loweta << " " << -higheta << " " << npts*3;
       else txtfile << -loweta << " " << higheta << " " << npts*3;
	 
       for (int i = 0; i < npts; i++) {

	 float unc = (kfacts->GetBinContent(etabin)*(params_loglin->GetParameter(0)+params_loglin->GetParameter(1)*log(pts[i])-params_const->GetParameter(0)))*0.5;

	 // cout << " " << pts[i] << " " << unc << " " << unc;
	 txtfile << " " << pts[i] << " " << unc << " " << unc;
	    
       }
       txtfile << endl;
     }     
   }
  
  for (int etabin = 1; etabin <= kfacts->GetXaxis()->GetNbins(); ++etabin) {
    float loweta = kfacts->GetXaxis()->GetBinLowEdge(etabin);
    float higheta = kfacts->GetXaxis()->GetBinLowEdge(etabin+1);


    auto params_loglin =  (TF1*)inFile->Get(Form("loglin_eta%d",  etabin));
    auto params_const =  (TF1*)inFile->Get(Form("const_eta%d",  etabin));
     
     if (kfacts->GetXaxis()->GetBinLowEdge(etabin) < 2.9) {

       //  cout << loweta << " " << higheta << " " << npts*3;
       txtfile << loweta << " " << higheta << " " << npts*3;
	 
       for (int i = 0; i < npts; i++) {

	 float unc = (kfacts->GetBinContent(etabin)*(params_loglin->GetParameter(0)+params_loglin->GetParameter(1)*log(pts[i])-params_const->GetParameter(0)))*0.5;

	 // cout << " " << pts[i] << " " << unc << " " << unc;
	 txtfile << " " << pts[i] << " " << unc << " " << unc;
	    
       }
       txtfile << endl;
     }     
  
  }

  txtfile.close();

}
