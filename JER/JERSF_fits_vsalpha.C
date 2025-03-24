// 20.2.2025

void JERSF_fits_vsalpha(string filein = "../JERSF_sigmas_fits_forjer_wideeta.root", string outfilename = "JERSFs_fromfits.root") {
  gStyle->SetOptStat(0);
  
  float fitmin = 0.2, fitmax = 0.4;
  double ptbins[] = {15, 25, 80, 120, 1000};
  int nptbins = 4;

  double etas[] = {0.0, 1.3, 2.5, 3.0};
  int netas = 2;
  
  // inputs are in same file for data and MC
  auto file = new TFile(filein.c_str(),"READ");

  auto outfile = new TFile(outfilename.c_str(),"RECREATE");

  auto txt = new TLatex();
  txt->SetNDC();
  txt->SetTextSize(0.035);

  auto SFs = new TH1D("SF","JER scale factor",netas,&etas[0]);

  map<int, TGraph*> DTs, MCs;

   
  for (int etabin = 1; etabin < 3; ++etabin) {
    auto hdata = new TH1D(Form("hdata_%d",etabin),Form("hdata_%d",etabin),nptbins,&ptbins[0] );
    auto hmc = new TH1D(Form("hmc_%d",etabin),Form("hmc_%d",etabin),nptbins,&ptbins[0] );

    for (int ptbin = 2; ptbin <= 4; ++ptbin) {
  
      auto DT = (TGraphErrors*)file->Get(Form("gsigmasDTpt%deta%d",ptbin,etabin));  // pt 2-4 eta 1-3
      DTs[ptbin] = (TGraph*)file->Get(Form("gsigmasDTpt%deta%d",ptbin,etabin));  // pt 2-4 eta 1-3
      auto MC = (TGraphErrors*)file->Get(Form("gsigmasMCpt%deta%d",ptbin,etabin));  // pt 2-4 eta 1-3

      DTs[ptbin]->SetTitle("");      MC->SetTitle("");
      DTs[ptbin]->GetXaxis()->SetTitle("#alpha");

      DT->SetTitle("");      MC->SetTitle("");
      DT->GetXaxis()->SetTitle("#alpha");

      // Remove points where alpha < 0.2
      DT->RemovePoint(1); MC->RemovePoint(1);
      DT->RemovePoint(0); MC->RemovePoint(0);

   
      auto c = new TCanvas(Form("c_%d_%d",ptbin,etabin),Form("c_%d_%d",ptbin,etabin),600,600);
      DT->SetMarkerColor(kBlue+1); DT->SetMarkerStyle(kFullCircle);

      MC->SetMarkerColor(kRed+1); MC->SetMarkerStyle(kFullCircle);

      DT->Draw("ap");
      MC->Draw("p");
  
      TF1 * f1 = new TF1("f1","pol1",fitmin,fitmax);
      f1->SetParameters(1,0);
      DT->Fit("f1","R");
      f1->Draw("same");
      float ydt = f1->Eval(0.5);
      double sigmadt = f1->GetParameter(0);
      double errdt = f1->GetParError(0);
    
      MC->Fit("f1","R");
      double sigmamc = f1->GetParameter(0);
      double errmc = f1->GetParError(0);
      float ymc = f1->Eval(0.5);

      double ratio = sigmadt/sigmamc;

      DT->SetMaximum(max(ymc,ydt));
      DT->SetMinimum(min(sigmamc,sigmadt));

      gPad->Update();
	
      //       txt->DrawLatex(0.15,0.2,Form("DT: %.3f#pm%.3f, MC: %.3f#pm%.3f, ratio: %.3f",sigmadt,errdt,sigmamc,errmc,ratio));
      txt->DrawLatex(0.15,0.2,Form("DT: %.3f#pm%.3f, MC: %.3f#pm%.3f",sigmadt,errdt,sigmamc,errmc));
      txt->DrawLatex(0.15,0.8, Form("%.1f <|#eta| < %.1f, %.0f < p_{T,avg} < %.0f",etas[etabin-1],etas[etabin],ptbins[ptbin-1],ptbins[ptbin]));

      hdata->SetBinContent(ptbin,sigmadt);
      hdata->SetBinError(ptbin,errdt);
    
      hmc->SetBinContent(ptbin,sigmamc);
      hmc->SetBinError(ptbin,errmc);

      //  save SF vs. abs(eta)
      auto leg = new TLegend(0.8,0.1,0.9,0.3);
      leg->AddEntry(DT, "Data");
      leg->AddEntry(MC, "MC");
      leg->Draw();

      c->Print(Form("JERSF_alphafit_ptbin%d_etabin%d.png",ptbin,etabin));

    }

    auto hratio = (TH1D*)hdata->Clone(Form("ratio_%d",etabin));
    hratio->Divide(hmc);

    auto cr = new TCanvas(Form("ratio_%d",etabin),Form("ratio_%d",etabin),600,600);
    cr->SetLogx();
    hratio->SetTitle("");
    hratio->GetXaxis()->SetTitle("p_{T}^{avg}");
    hratio->SetStats(0);
    hratio->SetMarkerStyle(kFullCircle);
    hratio->Draw();
    txt->DrawLatex(0.3,0.3, Form("%.1f <|#eta| < %.1f",etas[etabin-1],etas[etabin]));

    // fit vs. pT in separate bins of eta? = calculate average? pol fit?
    TF1 * f2 = new TF1("f2","pol0",25,1000);
    //      f2->SetParameters(0,1);
    hratio->Fit("f2","R"); // TODO: might want to fit a pol1?
    f2->Draw("same");

    SFs->SetBinContent(etabin,f2->GetParameter(0));
    SFs->SetBinError(etabin,f2->GetParError(0));

    hdata->Write();
    hmc->Write();
    hratio->Write();

    cr->Print(Form("JERSF_SFperpT_etabin%d.png",etabin));
    
  }

  auto csf = new TCanvas("sf","sf",600,600);
  SFs->GetXaxis()->SetRangeUser(0.,2.6);
  SFs->SetMarkerStyle(kFullCircle);
  SFs->GetXaxis()->SetTitle("|#eta|");
  SFs->Draw();
  SFs->Write();

  csf->Print("JER_SFs.png");
  

}
