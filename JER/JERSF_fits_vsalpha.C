void JERSF_fits_vsalpha(string filein = "../JERSF_sigmas_fits_forjer_wideeta.root") {
  float fitmin = 0.2, fitmax = 0.4;
  double ptbins[] = {15, 25, 80, 120, 1000};
  int nptbins = 4;

  double etas[] = {0.0, 1.3, 2.5, 3.0};
  
  // inputs are in same file for data and MC
  auto file = new TFile(filein.c_str(),"READ");

  auto txt = new TLatex();
  txt->SetNDC();
  
  for (int etabin = 1; etabin < 3; ++etabin) {
    auto hdata = new TH1D(Form("hdata_%d",etabin),Form("hdata_%d",etabin),nptbins,&ptbins[0] );
    auto hmc = new TH1D(Form("hmc_%d",etabin),Form("hmc_%d",etabin),nptbins,&ptbins[0] );

    for (int ptbin = 2; ptbin <= 4; ++ptbin) {
  
      auto DT = (TGraph*)file->Get(Form("gsigmasDTpt%deta%d",ptbin,etabin));  // pt 2-4 eta 1-3
      auto MC = (TGraph*)file->Get(Form("gsigmasMCpt%deta%d",ptbin,etabin));  // pt 2-4 eta 1-3
      DT->SetTitle("");      MC->SetTitle("");

      // Remove points where alpha < 0.2
      DT->RemovePoint(1); MC->RemovePoint(1);
      DT->RemovePoint(0); MC->RemovePoint(0);

      DT->SetMaximum(0.245);
      DT->SetMinimum(0.09);

      auto c = new TCanvas(Form("c_%d_%d",ptbin,etabin),Form("c_%d_%d",ptbin,etabin),600,600);

      DT->SetMarkerColor(kBlue+1); DT->SetMarkerStyle(kFullCircle);
      MC->SetMarkerColor(kRed+1); MC->SetMarkerStyle(kFullCircle);
      DT->Draw("");
      MC->Draw("P same");

  
      TF1 * f1 = new TF1("f1","pol1",fitmin,fitmax); // TODO: check that this range selection works
      f1->SetParameters(1,0);
      DT->Fit("f1","R");
      f1->Draw("same");
      double sigmadt = f1->GetParameter(0);
      double errdt = f1->GetParError(0);
    
      MC->Fit("f1","R");
      double sigmamc = f1->GetParameter(0);
      double errmc = f1->GetParError(0);

      double ratio = sigmadt/sigmamc;
    
      txt->DrawLatex(0.2,0.2,Form("DT: %.2f MC: %.2f ratio: %.2f",sigmadt,sigmamc,ratio));

      hdata->SetBinContent(ptbin,sigmadt);
      hdata->SetBinError(ptbin,errdt);
    
      hmc->SetBinContent(ptbin,sigmamc);
      hmc->SetBinError(ptbin,errmc);

  //  save SF vs. abs(eta)

    }

    auto hratio = (TH1D*)hdata->Clone(Form("ratio_%d",etabin));
    hratio->Divide(hmc);

    auto cr = new TCanvas(Form("ratio_%d",etabin),Form("ratio_%d",etabin),600,600);
    cr->SetLogx();
    hratio->SetTitle("");
    hratio->GetXaxis()->SetTitle("p_{T}^{avg}");
    hratio->SetStats(0);
    hratio->Draw();
    txt->DrawLatex(0.3,0.3, Form("%.1f <|#eta| < %.1f",etas[etabin-1],etas[etabin]));

    // fit vs. pT in separate bins of eta? = calculate average? pol fit?
    TF1 * f2 = new TF1("f2","pol0",25,1000); // TODO: check that this range selection works
    //      f2->SetParameters(0,1);
      hratio->Fit("f2","R");
      f2->Draw("same");
    
  }
  

}
