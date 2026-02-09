// Do fits for pt-parametization of L2residuals
// This is written for 2023PbPb studies, a lot of things have been unfortunately hardcoded
// Nick's parametrization: 1./([p0]+[p1]*log10(0.01*x)+[p2]/(x/10.0))


#include "../fillhistograms/histograms.h"


//int pts[] = {15, 25, 55, 80, 120, 170, 1000}; 
const float pts[] = {25, 80, 120, 1000};
int nptbins = 3;
//string ptbins[] = {"25to55", "55to80", "80to120", "120to170", "170to1000"};
string ptbins[] = {"25to80", "80to120", "120to1000"};

void fit_pt_param(TString inzb = "L2fits_AK4CHSjetveto_all/L2residuals_pbpbreco_rereco_zb_jetid.root", TString inHP =  "L2fits_AK4CHSjetveto_all/L2residuals_pbpbreco_rereco_hp_jetid.root", float fitmin = 25., float fitmax = 1000, string outfilename = "testing_pt_dep", bool doabseta = true) {

  // input file is from 3D derivation
  gStyle->SetOptStat(0);

  TFile *inFilezb = new TFile(inzb, "READ");
  TFile *inFile = new TFile(inHP, "READ");

  string outfolder = "ptfits";
  TFile *outfile = new TFile(Form("%s/%s.root",outfolder.c_str(),outfilename.c_str()),"RECREATE");
  
 
  // This is for correcton factors - hiso against eta
  auto factors = (TH1D*)inFile->Get("ratio_pt25to80_alpha0.3");
  factors->Reset();

  int colours[] = {209, 226, 213, 51, 206, 209};

  TCanvas *c3 = new TCanvas("c3","c3",600,600);
  c3->SetLogx();

  // Fro mresponses
  map<string, TH1D*> histos;
  map<int, TH1D*> histosvspt;

  // Hardcoded for 2023
  // histos["15to25"] = (TH1D*)inFilezb->Get("ratio_pt25to55_alpha0.3"); 
  histos["25to80"] = (TH1D*)inFilezb->Get("ratio_pt25to80_alpha0.3");
  
  // histos["55to80"] = (TH1D*)inFilezb->Get("ratio_pt55to80_alpha0.3");
  histos["80to120"] = (TH1D*)inFile->Get("ratio_pt80to120_alpha0.3"); 
  histos["120to1000"] = (TH1D*)inFile->Get("ratio_pt120to1000_alpha0.3");
  //  histos["170to1000"] = (TH1D*)inFile->Get("ratio_pt170to1000_alpha0.3");

  auto txt = new TLatex();
  txt->SetTextSize(0.03);
  txt->SetNDC();

  TF1 * f1 = new TF1("f1","[0] + [1]*log(x)",fitmin,fitmax);
  TF1 * f2 = new TF1("f2","pol0",fitmin,fitmax);
  TF1 * f3 = new TF1("f3","1./([0]+[1]*log10(0.01*x)+[2]/(x/10.))",fitmin,fitmax); // Run3 parametrization
  
  // Histograms vs. pT
  for (int ebin = 1; ebin < 15; ++ebin) {
    auto leg2 = new TLegend(0.15,0.17,0.35,0.32); //  x, y, x, y
    leg2->SetTextSize(0.03);
    leg2->SetBorderSize(0);
    leg2->SetFillStyle(0);
    
    histosvspt[ebin] = new TH1D(Form("ebin_%d",ebin),"",nptbins,&pts[0]); 

    // etabins for
    for (int ptbin = 1; ptbin <= nptbins; ++ptbin) {
      if (ebin > 12 and ptbin > 2) continue; // 2023 adaptation, not enough stats in EC

      histosvspt[ebin]->SetBinContent(ptbin,histos[ptbins[ptbin-1].c_str()]->GetBinContent(ebin));
      histosvspt[ebin]->SetBinError(ptbin,histos[ptbins[ptbin-1].c_str()]->GetBinError(ebin));

    }

 
    histosvspt[ebin]->SetMaximum(1.1);
    histosvspt[ebin]->SetMinimum(0.9);
    histosvspt[ebin]->GetXaxis()->SetTitle("p_{T,avg} (GeV)");
    histosvspt[ebin]->Draw();
 
    f1->SetParameters(1,0);
    histosvspt[ebin]->Fit("f1","R");
  
    f1->Draw("same");

    f2->SetParameters(1);
    f2->SetLineStyle(kDashed);
    histosvspt[ebin]->Fit("f2","R");
    f2->Draw("same");

    f3->SetLineStyle(kDashed);
    f3->SetLineColor(kBlue+1);
    histosvspt[ebin]->Fit("f3","R");
    f3->Draw("same");

    leg2->AddEntry(f1,Form("a + b*log(p_{T}), a = %.3f #pm %.3f, b = %.3f #pm %.3f",f1->GetParameter(0),f1->GetParError(0),f1->GetParameter(1),f1->GetParError(1)));
    leg2->AddEntry(f2, Form("p0 = %.3f #pm %.3f",f2->GetParameter(0), f2->GetParError(0)));
    leg2->Draw();

    txt->DrawLatex(0.5,0.8,Form("%.3f < |#eta| < %.3f ",histos["25to80"]->GetBinLowEdge(ebin),histos["25to80"]->GetBinLowEdge(ebin+1)));

    f1->Write(Form("loglin_eta%d",ebin));
    f2->Write(Form("const_eta%d",ebin));
    c3->Print(Form("ptfits/ebin_%d.png",ebin));
  }
  
  outfile->Close();
  
}

