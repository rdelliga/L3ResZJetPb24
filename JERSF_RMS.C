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

  TCanvas *c1 = new TCanvas("c1","c1",600,600);
  
  int nalphas= 7;
  for (int i = 0; i < nalphas; ++i) {
    cout << "alpha: " << alphabins[i] << " " << Form("hibin_-1.0_0.0/eta_-5.2_5.2/absasymmdist3D_a%d",alphabins[i]) <<  " file " << inFileNameDT.c_str() << endl;
    asymmMCs[i] = (TH3D*)inFile->Get(Form("hibin_-1.0_0.0/eta_-5.2_5.2/absasymmdist3D_a%d",alphabins[i]));
    asymmDTs[i] = (TH3D*)inFileDT->Get(Form("hibin_-1.0_0.0/eta_-5.2_5.2/absasymmdist3D_a%d",alphabins[i]));

    // if (i != 0) break; // debug

  // Need to fit these histograms and get the width; is it iterative?
  // trunct. RMS: 98.5% of events in the core

    for (int ptbin = 2; ptbin <= asymmDTs[i]->GetXaxis()->GetNbins(); ++ptbin) {
      for (int etabin = 1; etabin <= asymmDTs[i]->GetYaxis()->GetNbins(); ++etabin) {
	//	cout << asymmDT->GetBinContent(ptbin,etabin,20) << " " <<  asymmDT->GetBinError(ptbin,etabin,20) << endl;
  
	// 	TH1D* asMC = (TH1D*)asymmMC->ProjectionZ("MC",ptbin,ptbin,etabin,etabin);
	//TH1D* asDT = (TH1D*)asymmDT->ProjectionZ("DT",ptbin,ptbin,etabin,etabin);
	TH1D* asDT = (TH1D*)asymmDTs[i]->ProjectionZ(Form("DT_%d%d%d",ptbin,etabin,i),ptbin,ptbin,etabin,etabin);
	TH1D* asMC = (TH1D*)asymmMCs[i]->ProjectionZ(Form("MC_%d%d%d",ptbin,etabin,i),ptbin,ptbin,etabin,etabin);
	asDT->GetXaxis()->SetRangeUser(0,1.4);
	asMC->GetXaxis()->SetRangeUser(0,1.4);

	
        asDT->Scale(1./asDT->Integral(),"");
	asMC->Scale(1./asMC->Integral(),"");
	
	asDT->SetTitle("");
	asDT->GetXaxis()->SetTitle("A");
	cout << "Integrate: " <<  asDT->Integral() << " get entries: " << asDT->GetEntries() << endl;
	cout << "Scan histrogram" << endl;


	//// DATA
	float einbins = 0, fracdt = 0, fracmc = 0, linepointdt = 0, linepointmc = 0;
	float all =  asDT->Integral();
	TH1D* trunctAsDT = (TH1D*)asDT->Clone(Form("trunct%d",etabin));
	trunctAsDT->Reset();
	//	for (int i = 1; i < asDT->GetXaxis()->GetNbins(); ++i) {

	for (int j = 1;  einbins/all < cut ; ++j) {
	  float einbinsold = einbins;
	  einbins += asDT->GetBinContent(j);
	  // Debug: 
	  cout << " in bins " << einbins << " binc " << asDT->GetBinContent(j) << " ratio " <<   einbins/all << endl;

	  if (einbins/all < cut) {
	    trunctAsDT->SetBinContent(j, asDT->GetBinContent(j));
	  }
	  else {
	    fracdt = (cut*all-einbinsold)/ asDT->GetBinContent(j);
	    //    cout << "FRAC is: " << fracdt << " in bin then " << fracdt*asDT->GetBinContent(j) << " " << asDT->GetBinContent(j) << endl; // << " " << cut << " " << all << " " << einbins << endl;
	    trunctAsDT->SetBinContent(j, fracdt*asDT->GetBinContent(j));
	    linepointdt = fracdt*asDT->GetBinWidth(j) + asDT->GetBinLowEdge(j);
	  }
	}
	double RMSDT =  GetRootMeanSquare(trunctAsDT);
	cout << "RMS in trunct data: " << RMSDT << endl;

	//// MC
	einbins = 0;
        all =  asMC->Integral();
	TH1D* trunctAsMC = (TH1D*)asMC->Clone(Form("trunct%d",etabin));
	trunctAsMC->Reset();

	for (int j = 1;  einbins/all < cut ; ++j) { // TODO: while loop
	  float einbinsold = einbins;
	  einbins += asMC->GetBinContent(j);
	  // Debug: 
	   cout << " in bins " << einbins << " binc " << asMC->GetBinContent(j) << " ratio " <<   einbins/all << endl;

	  if (einbins/all < cut) trunctAsMC->SetBinContent(j, asMC->GetBinContent(j));
	  else {
	    fracmc = (cut*all-einbinsold)/ asMC->GetBinContent(j);
	    cout << "FRAC is: " << fracmc << " in bin then " << fracmc*asMC->GetBinContent(j) << " " << asMC->GetBinContent(j) << endl; // << " " << cut << " " << all << " " << einbins << endl;
	    trunctAsMC->SetBinContent(j, fracmc*asMC->GetBinContent(j));
	    linepointmc = fracmc*asMC->GetBinWidth(j) + asMC->GetBinLowEdge(j);
	  }
	}
	double RMSMC = GetRootMeanSquare(trunctAsMC);
	cout << "RMS in trunct MC: " << RMSMC << endl;

	/// TODO: change tline to 
	////////// Draw data
	auto linedt  = new TLine(linepointdt, 0, linepointdt, 0.3); // TODO: max
	asDT->SetStats(0);
	asDT->Draw(""); 
	linedt->Draw();
	//	trunctAsDT->Draw("same");

	///////// Draw MC
	auto linemc  = new TLine(linepointmc, 0, linepointmc, 0.3);
	linemc->SetLineStyle(kDashed);
	asMC->SetLineColor(kRed);
	asMC->Draw("same");
	linemc->Draw();
	trunctAsMC->SetLineColor(kRed);
	//	trunctAsMC->Draw("same");
	
	//	asDT->Fit("gaus");
	auto leg = new TLegend(0.6, 0.43, 0.85, 0.53);
	leg->SetTextSize(0.032);
	leg->SetBorderSize(0);
	leg->AddEntry(asDT,Form("Data"),"l");
	leg->AddEntry(asMC,Form("MC"),"l");
	leg->Draw();
	
	auto leg2 = new TLegend(0.6, 0.12, 0.85, 0.22);
	leg2->SetTextSize(0.032);
	leg2->SetBorderSize(0);
	leg2->AddEntry(linedt,Form("%.1f%% data",cut*100),"l");
	leg2->AddEntry(linemc,Form("%.1f%% mc",cut*100),"l");
	leg2->Draw();

	//	TF1 *fit = asDT->GetFunction("gaus");
	auto txt = new TLatex();
	txt->SetNDC();
	txt->SetTextSize(0.03);
	txt->DrawLatex( 0.62, 0.4, Form("%d < p_{T,avg} < %d",pts[ptbin-1],pts[ptbin]));
	txt->DrawLatex( 0.62, 0.35, Form("%.1f < |#eta| < %.1f, #alpha < %.2f",etabins[etabin-1],etabins[etabin],0.01*alphabins[i]));
	txt->DrawLatex( 0.62, 0.3, Form("RMS DT: %g",RMSDT));
	txt->DrawLatex( 0.62, 0.25, Form("RMS MC: %g",RMSMC));
	//	txt->DrawLatex( 0.2, 0.45, Form("#sigma = %.5f",fit->GetParameter(2)));
       
	c1->SetLogy();
        c1->Print(Form("jersfhists_L2_rerunall/absasymm_RMS_ptbin_%d_etabin_%d_a%d.png",ptbin,etabin,i));
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

