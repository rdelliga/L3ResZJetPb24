void MCJER(string inFileName = "HIJEC_results/debugged_plus_JER/pbpbreco_MC.root") {
 
  int pts[] = {40, 55, 80, 120, 170, 1000}; // Temporary
  float etabins[] = {0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0}; // Temporary

  float ptsforwidths[] = {80, 120, 170, 1000}; // Temporary
  TH1D* widths = new TH1D("widths","widths",3,&ptsforwidths[0]); // TODO: for different bins of eta

  map<int, TH1D*> ws;
  int cols[]  = {1, 2, 3, 4, 6, 7};

  for (int ebin = 1; ebin <= 6; ++ebin) {
    ws[ebin] =  new TH1D(Form("widths_%d",ebin),"",3,&ptsforwidths[0]); // TODO: for different bins of eta
    }

  TFile *inFile = new TFile(inFileName.c_str(), "READ"); 

  // Get 3D monster
  TH3D* responseprofile = (TH3D*)inFile->Get("hibin_-1.0_0.0/eta_-5.2_5.2/responses3D");

  //responseprofile->Draw();
  //cout << responseprofile->GetXaxis()->GetNbins() << endl;
  //cout << responseprofile->GetYaxis()->GetNbins() << endl;
  TCanvas *c1 = new TCanvas("c1","c1",800,600);

  for (int ptbin = 3; ptbin <= responseprofile->GetXaxis()->GetNbins(); ++ptbin) {
      for (int etabin = 1; etabin <= responseprofile->GetYaxis()->GetNbins(); ++etabin) {
  
	responseprofile->GetXaxis()->SetRange(ptbin,ptbin);    // pt axis - 2to5
	responseprofile->GetYaxis()->SetRange(etabin,etabin);    // eta axis

	TH1D* resp = (TH1D*)responseprofile->Project3D("z");

	resp->Scale(1./resp->Integral(),"width");
	resp->SetTitle("");

	resp->GetXaxis()->SetTitle("p_{T,reco}/p_{T,gen}");

	// JER should come from a gaussian fit here
	resp->Draw();

	// Iterate?
	resp->Fit("gaus");
	
	c1->SetLogy();

	// Put the labels in

	TF1 *fit = resp->GetFunction("gaus");

	auto txt = new TLatex();
	txt->SetNDC();
	txt->SetTextSize(0.03);
	txt->DrawLatex( 0.2, 0.35, Form("%.1f < |#eta| < %.1f",etabins[etabin-1],etabins[etabin]));
	txt->DrawLatex( 0.2, 0.4, Form("%d < p_{T,gen} < %d",pts[ptbin-1],pts[ptbin]));
	txt->DrawLatex( 0.2, 0.45, Form("#sigma = %.5f",fit->GetParameter(2)));

	ws[etabin]->SetBinContent(ptbin-2,fit->GetParameter(2));
	ws[etabin]->SetBinError(ptbin-2,fit->GetParError(2));
	ws[etabin]->SetMaximum(0.2);
       

	c1->Print(Form("mcjerhists/resp_ptbin_%d_etabin_%d.pdf",ptbin,etabin));
      }
  }

  //  

  auto leg = new TLegend(0.57,0.6,0.85,0.9);
  TCanvas *c2 = new TCanvas("c2","c2",800,600);
  c2->SetLogx();
  gStyle->SetOptStat(0);
  for (int ebin = 1; ebin <= 6; ++ebin) {
    ws[ebin]->SetLineColor(cols[ebin-1]);
    ws[ebin]->GetXaxis()->SetTitle("p_{T,ptcl}");
    ws[ebin]->GetYaxis()->SetTitle("#sigma");
    ws[ebin]->Draw("same");
    leg->AddEntry(ws[ebin],Form("%d",ebin));
  }
  leg->Draw();

  c2->Print("mcjerhists/allmcjers.pdf");
}
