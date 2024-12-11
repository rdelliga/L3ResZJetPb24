// Do the fits as function of alpha
// TODO: version with trunct. RMS?
// TODO: save sigmas as function of alpha for DT and MC
// TODO: zero bias for low pT:s


double GetRootMeanSquare( const TH1* h1) { 
   double stats[TH1::kNstat]; 
   h1->GetStats(stats); 
   return (stats[0] > 0) ? std::sqrt(stats[3]/stats[0]) : 0;
}


//void deriveL2_from3D(string inFileName = "HIJEC_results/rerunall_combinedbins/MC_AK4_jetid.root",string inFileNameDT = "HIJEC_results/rerunall_combinedbins/HP_AK4_PFTRIG_jetid_l2corr.root", string outfilename = "RERUNALL_combinedbins/L2residuals_pbpbreco_rerunall_HP_jetid_l2closure.root", bool dodt = true,   int alphabin = 5, bool useabs = true, bool usewideabs = false) {
// zerobiasall_jetid_l2corr.root


//void JERSF_fits(string inFileName = "HIJEC_results/rerunall_combinedbins/MC_AK4_jetid.root", string inFileNameDT = "HIJEC_results/rerunall_combinedbins/HP_AK4_PFTRIG_jetid_l2corr.root") {
void JERSF_RMS(string inFileName = "HIJEC_results/rerunall_combinedbins/MC_AK4_jetid.root", string inFileNameDT = "HIJEC_results/rerunall_combinedbins/zerobiasall_jetid_l2corr.root") {
  float cut = 0.985; // cut for trunctuating the |A| histograms
  
  //  int pts[] = {40, 55, 80, 120, 170, 1000}; // Temporary
  int pts[] = {15, 25, 80, 120, 1000}; // Temporary

  float etabins[] = {0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0}; // Temporary
  
  TFile *inFile = new TFile(inFileName.c_str(), "READ");
  TFile *inFileDT = new TFile(inFileNameDT.c_str(), "READ"); 

  // Get 3D monsters: TODO: EDIT: maybe map to different alphas
  TH3D* asymmMC = (TH3D*)inFile->Get("hibin_-1.0_0.0/eta_-5.2_5.2/asymmdist3D");
  TH3D* asymmDT = (TH3D*)inFileDT->Get("hibin_-1.0_0.0/eta_-5.2_5.2/asymmdist3D");

  map<int, TH3D*> asymmMCs;
  map<int, TH3D*> asymmDTs; // _a10 to a55?
  int alphabins[] = {10, 15, 20, 25, 30, 35, 40, 45};
  // Loop over alphas?

  TCanvas *c1 = new TCanvas("c1","c1",800,600);
  
  int nalphas= 7;
  for (int i = 0; i < nalphas; ++i) {
    cout << "alpha: " << alphabins[i] << " " << Form("hibin_-1.0_0.0/eta_-5.2_5.2/absasymmdist3D_a%d",alphabins[i]) <<  " file " << inFileNameDT.c_str() << endl;
    asymmMCs[i] = (TH3D*)inFile->Get(Form("hibin_-1.0_0.0/eta_-5.2_5.2/absasymmdist3D_a%d",alphabins[i]));
    asymmDTs[i] = (TH3D*)inFile->Get(Form("hibin_-1.0_0.0/eta_-5.2_5.2/absasymmdist3D_a%d",alphabins[i]));

  // Need to fit these histograms and get the width; is it iterative?
  // trunct. RMS: 98.5% of events in the core

    for (int ptbin = 2; ptbin <= asymmDTs[i]->GetXaxis()->GetNbins(); ++ptbin) {
      for (int etabin = 1; etabin <= asymmDTs[i]->GetYaxis()->GetNbins(); ++etabin) {
	//	cout << asymmDT->GetBinContent(ptbin,etabin,20) << " " <<  asymmDT->GetBinError(ptbin,etabin,20) << endl;
  
	// 	TH1D* asMC = (TH1D*)asymmMC->ProjectionZ("MC",ptbin,ptbin,etabin,etabin);
	//TH1D* asDT = (TH1D*)asymmDT->ProjectionZ("DT",ptbin,ptbin,etabin,etabin);
	TH1D* asDT = (TH1D*)asymmDTs[i]->ProjectionZ("DT",ptbin,ptbin,etabin,etabin);

	
	//	asDT->Scale(1./asDT->Integral(),"width"); // For some reason using "scale" leads to histograms not plotted with error bars after the first iteration
	asDT->SetTitle("");
	asDT->GetXaxis()->SetTitle("A");
	cout << "Integrate: " <<  asDT->Integral() << " get entries: " << asDT->GetEntries() << endl;
	cout << "Scan histrogrm" << endl;
	float einbins = 0;
	float all =  asDT->Integral();
	TH1D* trunctAsDT = (TH1D*)asDT->Clone("trunct");
	trunctAsDT->Reset();
	//	for (int i = 1; i < asDT->GetXaxis()->GetNbins(); ++i) {
	for (int i = 1;  einbins/all < cut ; ++i) {
	  einbins += asDT->GetBinContent(i);
	  cout << " in bins " << einbins << " binc " << asDT->GetBinContent(i) << " ratio " <<   einbins/all <<  endl;

	  if (einbins/all < cut) trunctAsDT->SetBinContent(i, asDT->GetBinContent(i));
	}
	cout << "RMS: " << GetRootMeanSquare(trunctAsDT) << endl;
	// ota binin indeksi missä 95.5% ylittyy, aseta binit nollaan?
	// Tosin halunnee plotata alkuperäsen
	// Kopioi TH1D, resetoi, aseta bin contentit while-loopissa, laske RMS
	// tarvii binin ylärajan missä täyttyy
	// mitä jos 98.5 keskellä biniä?

	//	cout << asymmDT->GetBinContent(ptbin,etabin,20) << " " <<  asymmDT->GetBinError(ptbin,etabin,20) << endl;
	//      cout << asDT->GetBinContent(20) << " " <<  asDT->GetBinError(20) << endl;
	asDT->Draw("E0");
	trunctAsDT->Draw("same");
	
	//	asDT->Fit("gaus");

	//	TF1 *fit = asDT->GetFunction("gaus");
	auto txt = new TLatex();
	txt->SetNDC();
	txt->SetTextSize(0.03);
	txt->DrawLatex( 0.2, 0.35, Form("%.1f < |#eta| < %.1f",etabins[etabin-1],etabins[etabin]));
	txt->DrawLatex( 0.2, 0.4, Form("%d < p_{T,avg} < %d",pts[ptbin-1],pts[ptbin]));
	//	txt->DrawLatex( 0.2, 0.45, Form("#sigma = %.5f",fit->GetParameter(2)));
       
	c1->SetLogy();
        c1->Print(Form("jersfhists_L2_rerunall/absasymm_DT_ptbin_%d_etabin_%d_a%d.png",ptbin,etabin,i));
	//c1->Print(Form("jersfhists_L2_rerunall/asymm_DT_ptbin_%d_etabin_%d_a.pdf",ptbin,etabin));
      }
      }
 


    /*  for (int ptbin = 2; ptbin <= asymmDT->GetXaxis()->GetNbins(); ++ptbin) {
      for (int etabin = 1; etabin <= asymmDT->GetYaxis()->GetNbins(); ++etabin) {
	//	cout << asymmDT->GetBinContent(ptbin,etabin,20) << " " <<  asymmDT->GetBinError(ptbin,etabin,20) << endl;
  	TH1D* asMC = (TH1D*)asymmMCs[i]->ProjectionZ("MC",ptbin,ptbin,etabin,etabin);
	
	// asMC->Scale(1./asMC->Integral(),"width");
	// Technically we'd need the lumi normalization?
	
	asMC->SetTitle("");

	asMC->GetXaxis()->SetTitle("A");

	//	cout << asymmDT->GetBinContent(ptbin,etabin,20) << " " <<  asymmDT->GetBinError(ptbin,etabin,20) << endl;
	//      cout << asDT->GetBinContent(20) << " " <<  asDT->GetBinError(20) << endl;
	asMC->Draw("");
	//	asMC->Fit("gaus");
	
	c1->SetLogy();

	// Put the labels in

	//TF1 *fit = asMC->GetFunction("gaus");
     	//  cout << fit->GetParameter(1) << endl;    //Mean, sigma is 2

	auto txt = new TLatex();
	txt->SetNDC();
	txt->SetTextSize(0.03);
	txt->DrawLatex( 0.2, 0.35, Form("%.1f < |#eta| < %.1f",etabins[etabin-1],etabins[etabin]));
	txt->DrawLatex( 0.2, 0.4, Form("%d < p_{T,avg} < %d",pts[ptbin-1],pts[ptbin]));
	//txt->DrawLatex( 0.2, 0.45, Form("#sigma = %.5f",fit->GetParameter(2)));
       
	c1->Print(Form("jersfhists_L2_rerunall/absasymm_MC_ptbin_%d_etabin_%d_a%d.png",ptbin,etabin,i));
      }
  } */
  }
  
  // SAVE: old: jersfsigmas.root"
  // We want to save the sigmas as function of alphe for both MC and DT
  
}

