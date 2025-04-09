// THIS IS AN OLD VERSION USING TH1?


// Takes input from JERSF_fits -> TODO: fix these names?
// TODO: fit using TGraphs

// Do the linear fits as function of alpha. put in histograms per eta, as function of pT,ave.


// TODO: do we want to fit DT and MC separately, or the ratio?

float halfeta[] = {0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0}; // Could just create folders doing these + a wide one

//Float_t pts[] = {40, 55, 80, 120, 170, 1000}; // Temporary
Float_t pts[] = {15, 25, 80, 120, 1000}; // Temporary
//int pts[] = {80, 120, 170, 1000}; // Temporary
int npts = 4;

//void JERSF_polfits(string infile = "jersfsigmas.root") {
//void JERSF_polfits(string infile = "JERSF_sigmas_RMS.root") {
void JERSF_polfits(string infile = "JERSF_sigmas_fits.root") {
  gStyle->SetOptStat(0);
  TFile *f = new TFile(infile.c_str(),"READ");

  // These are per eta?
  TH1D* p0DT = new TH1D("p0DT","",npts,&pts[0]);
  TH1D* p0MC = new TH1D("p0MC","",npts,&pts[0]);
  
  TCanvas *c1 = new TCanvas("c1","c1",800,600);
  TCanvas *c2 = new TCanvas("c2","c2",800,600);
  //per bin of eta
  for (int ieta = 1; ieta <= 6; ++ieta) {
    p0MC->Reset();
    p0DT->Reset();
    for (int ipt = 2; ipt <= npts; ++ipt) {
      c1->cd();
  // DATA hist
      TH1D* DT = (TH1D*)f->Get(Form("sigmasDTpt%deta%d",ipt,ieta));
      DT->SetTitle("");
      DT->Draw();

      DT->Fit("pol1");
      TF1 *fit1 = DT->GetFunction("pol1");
      
      p0DT->SetBinContent(ipt, fit1->GetParameter(0));
      p0DT->SetBinError(ipt, fit1->GetParError(0));

  // MC hist
      TH1D* MC = (TH1D*)f->Get(Form("sigmasMCpt%deta%d",ipt,ieta));
      MC->SetTitle("");
      MC->Draw("same");

      MC->Fit("pol1");
      TF1 *fit2 = MC->GetFunction("pol1");
      p0MC->SetBinContent(ipt, fit2->GetParameter(0));
      p0MC->SetBinError(ipt, fit2->GetParError(0));

      auto leg = new TLegend(0.57,0.8,0.85,0.9); //  x, y, x, y
      leg->SetTextSize(0.03);
      leg->SetBorderSize(0);
      leg->SetFillStyle(0);
      
      leg->AddEntry(DT,"Data");
      leg->AddEntry(MC,"MC");
      
      leg->Draw("same");

      c1->Print(Form("jersfpolfits_update/fits_pt%d_eta%d.png",ipt,ieta));
    }
    c2->cd();

   
    p0MC->SetLineColor(kRed);
    auto rp = new TRatioPlot(p0DT, p0MC);
    rp->SetH1DrawOpt("E");
    rp->SetGridlines({},0);
    
    c2->SetLogx();

    rp->Draw();
    rp->GetLowerRefYaxis()->SetTitle("Data/MC");
    rp->GetUpperRefXaxis()->SetTitle("p_{T,avg}");
    rp->GetUpperRefYaxis()->SetTitle("#sigma");
   
    rp->GetUpperPad()->cd();
    auto leg = new TLegend(0.7,0.1,0.9,0.25); //  x, y, x, y

    leg->SetTextSize(0.05);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    
    leg->AddEntry(p0DT,"Data");
    leg->AddEntry(p0MC,"MC");
      
    leg->Draw("same");

    //if (i == 0) rp->GetLowerRefGraph()->SetMaximum(5);
    
    auto *th3 = new TLatex(0.15,0.2,Form("%.1f < |#eta| < %.1f",halfeta[ieta-1],halfeta[ieta]));// TODO: This doesnt show up opn top pad
    th3->SetNDC();
    th3->SetTextSize(0.04);
    th3->Draw();
    c2->Update();
   //    c2->cd(0); c2->Modified(); c2->Update();

    c2->Print(Form("jersfpolfits_update/p0s_eta%d.png",ieta));

  }

  // TODO: take the ratios; fit one by-one to get SF per eta (pt dependency?)
  


}
