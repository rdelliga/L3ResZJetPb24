// Fit L3 Residual corrections from photon+jet balancing
// Production-ready version with TDR style output
// Follows template from L3Res.C
//
// Usage:
//   root -l -b -q 'dofits_L3.C("input.root", 30, 100, "output", false)'
//
// Author: Based on L3Res.C template

#include "TFile.h"
#include "TProfile2D.h"
#include "TProfile3D.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TLine.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TROOT.h"

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <cmath>

#include "tdrStyle.C"
#include "CMS_lumi.C"
#include "../fillhistograms/histograms.h"

using namespace std;

TLegend *_leg = nullptr;
string _run = "2024ppRef";
bool fitG = true;
bool fitD = false;
bool doClosure = false;

TH1D* drawCleaned(TH1D* h, string data, double ptmin, double ptmax,
                  int marker, int color) {
  if (!h) return nullptr;
  TH1D* hc = (TH1D*)h->Clone(Form("hc_%s_%s", h->GetName(), _run.c_str()));
  for (int i = 1; i <= hc->GetNbinsX(); ++i) {
    double ptbinmin = hc->GetBinLowEdge(i);
    double ptbinmax = hc->GetBinLowEdge(i+1);
    bool keep = false;
    if (data == "G" && ptbinmin >= ptmin && ptbinmax <= ptmax) keep = true;
    if (data == "D" && ptbinmin >= ptmin && ptbinmax <= ptmax) keep = true;
    if (h->GetBinContent(i) > 1.5 || h->GetBinContent(i) < 0.5) keep = false;
    if (h->GetBinError(i) <= 0 || h->GetBinContent(i) <= 0) keep = false;
    if (!keep) {
      hc->SetBinContent(i, 0.);
      hc->SetBinError(i, 0.);
    } else {
      double errmin = 0.002;
      hc->SetBinError(i, sqrt(pow(h->GetBinError(i), 2) + pow(errmin, 2)));
    }
  }
  hc->SetMarkerStyle(marker);
  hc->SetMarkerColor(color);
  hc->SetLineColor(color);
  return hc;
}

TGraphErrors* cleanGraph(TGraphErrors* g) {
  if (!g) return nullptr;
  for (int i = g->GetN() - 1; i >= 0; --i) {
    if (g->GetY()[i] == 0 && g->GetEY()[i] == 0) g->RemovePoint(i);
  }
  return g;
}

void dofits_L3(TString inFileL3Derived = "L3_derived.root",
               double ptminG = 30.,
               double ptmaxG = 100.,
               string outfilename = "L3Res_photonjet",
               bool closure = false,
               string runLabel = "2024ppRef",
               string lumiLabel = "pp Reference",
               bool plotRawResponses = false,
               bool saveAlphaExtrap = false) {

  doClosure = closure;
  _run = runLabel;

  setTDRStyle();
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);

  writeExtraText = true;
  extraText = doClosure ? "Closure Test" : "Preliminary";
  lumi_sqrtS = Form("%s, #sqrt{s} = 5.36 TeV", lumiLabel.c_str());

  string outfolder = "L3fits_" + outfilename;
  string pngFolder = outfolder + "/pdf"; // keep folder name for compatibility
  string txtFolder = outfolder + "/textfiles";
  string rawFolder = outfolder + "/raw";
  string alphaFolder = outfolder + "/alpha_extrap";
  gSystem->mkdir(outfolder.c_str(), kTRUE);
  gSystem->mkdir(pngFolder.c_str(), kTRUE);
  gSystem->mkdir(txtFolder.c_str(), kTRUE);
  if (plotRawResponses) gSystem->mkdir(rawFolder.c_str(), kTRUE);
  if (saveAlphaExtrap) gSystem->mkdir(alphaFolder.c_str(), kTRUE);

  TFile* inFile = TFile::Open(inFileL3Derived, "READ");
  if (!inFile || inFile->IsZombie()) {
    cout << "ERROR: Cannot open input file: " << inFileL3Derived << endl;
    return;
  }

  cout << "============================================" << endl;
  cout << "L3 Residual Fitting" << endl;
  cout << "Input file: " << inFileL3Derived << endl;
  cout << "Photon+jet pT range: " << ptminG << " - " << ptmaxG << " GeV" << endl;
  cout << "Closure test: " << (doClosure ? "YES" : "NO") << endl;
  cout << "============================================" << endl;

  TProfile3D* balance3D_mc = (TProfile3D*)inFile->Get("balance3D_mc");
  TProfile3D* balance3D_data = (TProfile3D*)inFile->Get("balance3D_data");

  TH3D* counts3D_mc = (TH3D*)inFile->Get("counts3D_mc");
  TH3D* counts3D_data = (TH3D*)inFile->Get("counts3D_data");
  if (!counts3D_mc) counts3D_mc = (TH3D*)inFile->Get("counts3D_mc_wide");
  if (!counts3D_mc) counts3D_mc = (TH3D*)inFile->Get("counts3D_mc_narrow");
  if (!counts3D_data) counts3D_data = (TH3D*)inFile->Get("counts3D_data_wide");
  if (!counts3D_data) counts3D_data = (TH3D*)inFile->Get("counts3D_data_narrow");

  if (!balance3D_mc || !balance3D_data) {
    cout << "ERROR: Cannot find balance3D profiles in input file" << endl;
    return;
  }

  int nptbins = balance3D_mc->GetXaxis()->GetNbins();
  int netabins = balance3D_mc->GetYaxis()->GetNbins();
  int nalphabins = balance3D_mc->GetZaxis()->GetNbins();

  cout << "Detected binning: " << nptbins << " pT bins, "
       << netabins << " eta bins, " << nalphabins << " alpha bins" << endl;
  cout << "Alpha bin edges: ";
  for (int a = 1; a <= nalphabins + 1; ++a) cout << balance3D_mc->GetZaxis()->GetBinLowEdge(a) << " ";
  cout << endl;

  if (!counts3D_mc) cout << "WARNING: counts3D_mc not found" << endl;
  if (!counts3D_data) cout << "WARNING: counts3D_data not found" << endl;

  TFile* outfile = new TFile(Form("%s/%s.root", outfolder.c_str(), outfilename.c_str()), "RECREATE");

  // Optional: build alpha-extrapolation (per pT, eta) from the 3D profiles
  if (saveAlphaExtrap) {
    cout << "Saving alpha extrapolation histograms to " << alphaFolder << endl;
    TAxis* zaxis = balance3D_mc->GetZaxis();
    int nAlphaBins = zaxis->GetNbins();
    std::vector<double> alphaEdges(nAlphaBins + 1);
    for (int b = 1; b <= nAlphaBins + 1; ++b) alphaEdges[b-1] = zaxis->GetBinLowEdge(b);

    for (int ptbin = 1; ptbin <= nptbins; ++ptbin) {
      for (int etabin = 1; etabin <= netabins; ++etabin) {
        TH1D* hAlphaMc = new TH1D(Form("alpha_mc_pt%d_eta%d", ptbin, etabin), "MC balance vs #alpha;#alpha;Balance", nAlphaBins, alphaEdges.data());
        TH1D* hAlphaDt = new TH1D(Form("alpha_dt_pt%d_eta%d", ptbin, etabin), "Data balance vs #alpha;#alpha;Balance", nAlphaBins, alphaEdges.data());
        TH1D* hAlphaRatio = new TH1D(Form("alpha_ratio_pt%d_eta%d", ptbin, etabin), "MC/Data vs #alpha;#alpha;L3Res", nAlphaBins, alphaEdges.data());

        for (int abin = 1; abin <= nAlphaBins; ++abin) {
          double mc = balance3D_mc->GetBinContent(ptbin, etabin, abin);
          double emc = balance3D_mc->GetBinError(ptbin, etabin, abin);
          double dt = balance3D_data->GetBinContent(ptbin, etabin, abin);
          double edt = balance3D_data->GetBinError(ptbin, etabin, abin);

          hAlphaMc->SetBinContent(abin, mc);
          hAlphaMc->SetBinError(abin, emc);
          hAlphaDt->SetBinContent(abin, dt);
          hAlphaDt->SetBinError(abin, edt);

          if (dt > 0 && mc > 0) {
            double ratio = mc / dt;
            double relErr2 = 0.0;
            if (mc > 0 && emc > 0) relErr2 += pow(emc / mc, 2);
            if (dt > 0 && edt > 0) relErr2 += pow(edt / dt, 2);
            hAlphaRatio->SetBinContent(abin, ratio);
            hAlphaRatio->SetBinError(abin, ratio * sqrt(relErr2));
          }
        }

        // Fit ratio vs alpha with linear function to extrapolate to alpha->0
        TF1* fAlpha = new TF1(Form("fAlpha_pt%d_eta%d", ptbin, etabin), "[0]+[1]*x", alphaEdges.front(), alphaEdges.back());
        fAlpha->SetParameters(1.0, 0.0);
        hAlphaRatio->Fit(fAlpha, "QNR");

        TCanvas* cAlpha = new TCanvas(Form("cAlpha_%d_%d", ptbin, etabin), Form("cAlpha_%d_%d", ptbin, etabin), 800, 600);
        cAlpha->cd();
        hAlphaRatio->SetMinimum(0.5);
        hAlphaRatio->SetMaximum(1.5);
        hAlphaRatio->Draw("PE");
        fAlpha->SetLineColor(kRed);
        fAlpha->SetLineWidth(2);
        fAlpha->Draw("SAME");
        CMS_lumi(cAlpha, 0, 0);
        cAlpha->SaveAs(Form("%s/L3Res_%s_ptbin%d_etabin%d_alpha.png", alphaFolder.c_str(), _run.c_str(), ptbin, etabin));

        outfile->cd();
        hAlphaMc->Write();
        hAlphaDt->Write();
        hAlphaRatio->Write();
        fAlpha->Write();

        delete cAlpha;
        delete hAlphaMc;
        delete hAlphaDt;
        delete hAlphaRatio;
        delete fAlpha;
      }
    }
  }

  cout << "\n=== Loading pT-dependent response ratios ===" << endl;
  map<int, TH1D*> ratioVsPt;
  for (int alpha = 1; alpha <= nalphabins; ++alpha) {
    TH1D* h = (TH1D*)inFile->Get(Form("ratio_vsphotonpt_alpha%d", alpha));
    if (h) {
      ratioVsPt[alpha] = h;
      cout << "  Loaded ratio_vsphotonpt_alpha" << alpha << endl;
    }
  }
  if (ratioVsPt.empty()) {
    cout << "ERROR: No ratio histograms available" << endl;
    return;
  }

  // Prepare text output (inclusive eta, last alpha bin as reference)
  ofstream ftxt(Form("%s/%s.txt", txtFolder.c_str(), outfilename.c_str()));
  ftxt << "{ 1 JetEta 1 JetPt [0]+[1]*log10(0.01*x)+[2]/(x/10.) Correction L3Residual}\n";

  cout << "\n=== Creating plots for all alpha bins ===" << endl;

  for (auto& alphaPair : ratioVsPt) {
    int alphaBin = alphaPair.first;
    TH1D* hRatio = alphaPair.second;
    if (!hRatio) continue;

    cout << "\nProcessing alpha bin " << alphaBin << endl;
    double alphaCutVal = balance3D_mc->GetZaxis()->GetBinUpEdge(alphaBin);
    cout << "  Alpha cut: alpha < " << alphaCutVal << endl;

    // Ratio plot
    TH1D* hFrame1 = new TH1D(Form("hFrame1_a%d", alphaBin), ";p_{T}^{#gamma} (GeV);Response (MC/Data)", 100, 20, 600);
    hFrame1->SetMinimum(0.5);
    hFrame1->SetMaximum(1.2);
    TCanvas* c1 = new TCanvas(Form("c1_a%d", alphaBin), Form("c1_a%d", alphaBin), 800, 600);
    c1->SetLogx();
    c1->cd();
    hFrame1->Draw();
    TLine* line = new TLine();
    line->SetLineStyle(kDashed);
    line->SetLineColor(kGray+1);
    line->DrawLine(20, 1, 600, 1);
    hRatio->SetMarkerStyle(kFullCircle);
    hRatio->SetMarkerColor(kBlue);
    hRatio->SetLineColor(kBlue);
    hRatio->Draw("PE SAME");
    TLegend* leg1 = new TLegend(0.55, 0.75, 0.88, 0.88);
    leg1->SetBorderSize(0);
    leg1->SetFillStyle(0);
    leg1->SetTextFont(42);
    leg1->SetTextSize(0.035);
    leg1->AddEntry(hRatio, Form("#gamma+jet (#alpha < %.2f)", alphaCutVal), "PLE");
    leg1->Draw();
    TLatex* tex = new TLatex();
    tex->SetNDC();
    tex->SetTextFont(42);
    tex->SetTextSize(0.035);
    double etaMinAll = balance3D_mc->GetYaxis()->GetBinLowEdge(1);
    double etaMaxAll = balance3D_mc->GetYaxis()->GetBinLowEdge(netabins + 1);
    tex->DrawLatex(0.55, 0.70, Form("|#eta_{jet}| in [%.3f, %.3f] (inclusive)", etaMinAll, etaMaxAll));
    CMS_lumi(c1, 0, 0);
    c1->SaveAs(Form("%s/L3Res_%s_alpha%d_ratio.png", pngFolder.c_str(), _run.c_str(), alphaBin));
    if (saveAlphaExtrap) {
      c1->SaveAs(Form("%s/L3Res_%s_alpha%d_ratio.png", alphaFolder.c_str(), _run.c_str(), alphaBin));
    }

    // Optional raw MC/Data responses (no fits, no ratio)
    if (plotRawResponses) {
      TH1D* hMcRaw = (TH1D*)inFile->Get(Form("balance_vsphotonpt_mc_alpha%d", alphaBin));
      TH1D* hDtRaw = (TH1D*)inFile->Get(Form("balance_vsphotonpt_data_alpha%d", alphaBin));
      if (hMcRaw && hDtRaw) {
        TCanvas* cRaw = new TCanvas(Form("cRaw_a%d", alphaBin), Form("cRaw_a%d", alphaBin), 800, 600);
        cRaw->SetLogx();
        TH1D* frameRaw = new TH1D(Form("hFrameRaw_a%d", alphaBin), ";p_{T}^{#gamma} (GeV);Balance", 100, 20, 600);
        frameRaw->SetMinimum(0.5);
        frameRaw->SetMaximum(1.5);
        frameRaw->Draw();
        hMcRaw->SetMarkerStyle(kFullCircle);
        hMcRaw->SetMarkerColor(kBlue);
        hMcRaw->SetLineColor(kBlue);
        hMcRaw->Draw("PE SAME");
        hDtRaw->SetMarkerStyle(kFullSquare);
        hDtRaw->SetMarkerColor(kRed);
        hDtRaw->SetLineColor(kRed);
        hDtRaw->Draw("PE SAME");
        TLegend* legRaw = new TLegend(0.55, 0.75, 0.88, 0.88);
        legRaw->SetBorderSize(0);
        legRaw->SetFillStyle(0);
        legRaw->SetTextFont(42);
        legRaw->SetTextSize(0.035);
        legRaw->AddEntry(hMcRaw, Form("MC (#alpha < %.2f)", alphaCutVal), "PLE");
        legRaw->AddEntry(hDtRaw, Form("Data (#alpha < %.2f)", alphaCutVal), "PLE");
        legRaw->Draw();
        CMS_lumi(cRaw, 0, 0);
        cRaw->SaveAs(Form("%s/L3Res_%s_alpha%d_raw.png", rawFolder.c_str(), _run.c_str(), alphaBin));
        delete frameRaw;
        delete cRaw;
      }
    }

    // Fits
    TH1D* hFrame2 = new TH1D(Form("hFrame2_a%d", alphaBin), ";p_{T}^{#gamma} (GeV);Response (MC/Data)", 100, 20, 600);
    hFrame2->SetMinimum(0.5);
    hFrame2->SetMaximum(1.2);
    TCanvas* c2 = new TCanvas(Form("c2_a%d", alphaBin), Form("c2_a%d", alphaBin), 800, 600);
    c2->SetLogx();
    c2->cd();
    hFrame2->Draw();
    line->DrawLine(20, 1, 600, 1);
    line->SetLineStyle(kDotted);
    line->SetLineColor(kGray);
    line->DrawLine(ptminG, 0.5, ptminG, 1.2);
    line->DrawLine(ptmaxG, 0.5, ptmaxG, 1.2);
    line->SetLineStyle(kDashed);
    line->SetLineColor(kGray+1);
    TH1D* hRatioFull = (TH1D*)hRatio->Clone(Form("hRatioFull_a%d", alphaBin));
    hRatioFull->SetMarkerStyle(kOpenCircle);
    hRatioFull->SetMarkerColor(kGray);
    hRatioFull->SetLineColor(kGray);
    hRatioFull->Draw("PE SAME");
    TH1D* hRatioClean = drawCleaned(hRatio, "G", ptminG, ptmaxG, kFullCircle, kBlue);
    hRatioClean->Draw("PE SAME");
    TMultiGraph* mg = new TMultiGraph(Form("mg_a%d", alphaBin), "mg");
    if (fitG && hRatioClean) {
      TGraphErrors* gG = cleanGraph(new TGraphErrors(hRatioClean));
      if (gG && gG->GetN() > 0) mg->Add(gG, "P");
    }
    double fitRangeMin = ptminG;
    double fitRangeMax = ptmaxG;
    TF1* f0 = new TF1(Form("f0_a%d", alphaBin), "[0]", fitRangeMin, fitRangeMax);
    TF1* f1 = new TF1(Form("f1_a%d", alphaBin), "[0]+[1]*log10(0.01*x)", fitRangeMin, fitRangeMax);
    TF1* f2 = new TF1(Form("f2_a%d", alphaBin), "[0]+[1]*log10(0.01*x)+[2]/(x/10.)", fitRangeMin, fitRangeMax);
    TF1* f3 = new TF1(Form("f3_a%d", alphaBin), "[0]+[1]*log10(0.01*x)+[2]*pow(log10(0.01*x),2)", fitRangeMin, fitRangeMax);
    TF1* f4 = new TF1(Form("f4_a%d", alphaBin), "[0]+[1]*log10(0.01*x)+[2]*pow(log10(0.01*x),2)+[3]/(x/10.)", fitRangeMin, fitRangeMax);
    TF1* fref = new TF1(Form("fref_a%d", alphaBin), "[0]+[1]*log10(0.01*x)+[2]/(x/10.)", 15., 3500.);
    f0->SetParameter(0, 1.0); mg->Fit(f0, "QRN");
    f1->SetParameters(f0->GetParameter(0), -0.01); mg->Fit(f1, "QRN");
    f2->SetParameters(f1->GetParameter(0), f1->GetParameter(1), 0.02); f2->SetParLimits(2, -0.5, 0.5); mg->Fit(f2, "QRN");
    f3->SetParameters(f1->GetParameter(0), f1->GetParameter(1), 0.005); mg->Fit(f3, "QRN");
    f4->SetParameters(f3->GetParameter(0), f3->GetParameter(1), f3->GetParameter(2), f2->GetParameter(2)); f4->SetParLimits(3, -0.5, 0.5); mg->Fit(f4, "QRN");
    fref->SetParameters(f2->GetParameter(0), f2->GetParameter(1), f2->GetParameter(2)); fref->SetParLimits(2, -0.5, 0.5); mg->Fit(fref, "QRN");
    f0->SetLineColor(kMagenta+2); f0->SetLineStyle(kDotted); f0->Draw("SAME");
    f1->SetLineColor(kCyan+1); f1->SetLineStyle(kDashed); f1->Draw("SAME");
    f2->SetLineColor(kGreen+2); f2->SetLineWidth(2); f2->Draw("SAME");
    f3->SetLineColor(kOrange+2); f3->SetLineStyle(kDashDotted); f3->Draw("SAME");
    fref->SetLineColor(kRed); fref->SetLineWidth(2); fref->Draw("SAME");
    TLegend* leg2 = new TLegend(0.50, 0.65, 0.88, 0.88);
    leg2->SetBorderSize(0);
    leg2->SetFillStyle(0);
    leg2->SetTextFont(42);
    leg2->SetTextSize(0.030);
    leg2->AddEntry(hRatioClean, "#gamma+jet (cleaned)", "PLE");
    leg2->AddEntry(f0, "f_{0}: const", "L");
    leg2->AddEntry(f1, "f_{1}: log(p_{T})", "L");
    leg2->AddEntry(f2, "f_{2}: log(p_{T}) + 1/p_{T}", "L");
    leg2->AddEntry(f3, "f_{3}: log^{2}(p_{T})", "L");
    leg2->AddEntry(fref, Form("Ref: #chi^{2}/ndf = %.1f/%d", fref->GetChisquare(), fref->GetNDF()), "L");
    leg2->Draw();
    tex->DrawLatex(0.20, 0.85, Form("Fit range: %.0f-%.0f GeV", ptminG, ptmaxG));
    tex->DrawLatex(0.20, 0.80, Form("#alpha < %.2f (bin %d)", alphaCutVal, alphaBin));
    tex->DrawLatex(0.20, 0.75, Form("|#eta_{jet}| < %.1f", balance3D_mc->GetYaxis()->GetBinLowEdge(netabins + 1)));
    CMS_lumi(c2, 0, 0);
    c2->SaveAs(Form("%s/L3Res_%s_alpha%d_fits.png", pngFolder.c_str(), _run.c_str(), alphaBin));

    // write JEC text once (highest alpha bin)
    if (alphaBin == nalphabins) {
      double etaMin = 0.0;
      double etaMax = balance3D_mc->GetYaxis()->GetBinLowEdge(netabins + 1);
      ftxt << Form("  %6.3f %6.3f  5  %5.0f %5.0f  %8.5f %8.5f %8.5f\n",
                   etaMin, etaMax, ptminG, 500.,
                   fref->GetParameter(0), fref->GetParameter(1), fref->GetParameter(2));
    }

    // Closure
    if (doClosure) {
      TH1D* hFrame3 = new TH1D(Form("hFrame3_a%d", alphaBin), ";p_{T}^{#gamma} (GeV);Corrected Response", 100, 20, 600);
      hFrame3->SetMinimum(0.7);
      hFrame3->SetMaximum(1.3);
      TCanvas* c3 = new TCanvas(Form("c3_a%d", alphaBin), Form("c3_a%d", alphaBin), 800, 600);
      c3->SetLogx();
      c3->cd();
      hFrame3->Draw();
      line->SetLineStyle(kDashed);
      line->SetLineColor(kGray+1);
      line->DrawLine(20, 1, 600, 1);
      TH1D* hCorrected = (TH1D*)hRatio->Clone(Form("hCorrected_a%d", alphaBin));
      for (int i = 1; i <= hCorrected->GetNbinsX(); ++i) {
        double pt = hCorrected->GetBinCenter(i);
        double val = hCorrected->GetBinContent(i);
        double err = hCorrected->GetBinError(i);
        if (val > 0 && pt >= fitRangeMin && pt <= 500.) {
          double corr = fref->Eval(pt);
          hCorrected->SetBinContent(i, val / corr);
          hCorrected->SetBinError(i, err / corr);
        } else {
          hCorrected->SetBinContent(i, 0);
          hCorrected->SetBinError(i, 0);
        }
      }
      hCorrected->SetMarkerStyle(kFullSquare);
      hCorrected->SetMarkerColor(kRed);
      hCorrected->SetLineColor(kRed);
      hCorrected->Draw("PE SAME");
      TH1D* hUncorr = (TH1D*)hRatio->Clone(Form("hUncorr_a%d", alphaBin));
      hUncorr->SetMarkerStyle(kOpenCircle);
      hUncorr->SetMarkerColor(kBlue);
      hUncorr->SetLineColor(kBlue);
      hUncorr->Draw("PE SAME");
      TLegend* leg3 = new TLegend(0.55, 0.70, 0.88, 0.88);
      leg3->SetBorderSize(0);
      leg3->SetFillStyle(0);
      leg3->SetTextFont(42);
      leg3->SetTextSize(0.035);
      leg3->AddEntry(hUncorr, "Before L3Res", "PLE");
      leg3->AddEntry(hCorrected, "After L3Res", "PLE");
      leg3->Draw();
      tex->DrawLatex(0.20, 0.20, "Closure: Apply derived L3Res");
      tex->DrawLatex(0.20, 0.15, "Expect ratio #rightarrow 1.0");
      CMS_lumi(c3, 0, 0);
      c3->SaveAs(Form("%s/L3Res_%s_alpha%d_closure.png", pngFolder.c_str(), _run.c_str(), alphaBin));
      outfile->cd();
      hCorrected->Write(Form("ratio_corrected_alpha%d", alphaBin));
      delete c3;
    }

    // Counts MC
    if (counts3D_mc) {
      counts3D_mc->GetZaxis()->SetRange(1, alphaBin);
      TH2D* h2dCounts = (TH2D*)counts3D_mc->Project3D("yx");
      h2dCounts->SetName(Form("counts_pteta_mc_alpha%d", alphaBin));
      double ptMinAll = balance3D_mc->GetXaxis()->GetBinLowEdge(1);
      double ptMaxAll = balance3D_mc->GetXaxis()->GetBinLowEdge(nptbins+1);
      h2dCounts->SetTitle(Form("MC Counts (#alpha < %.2f);p_{T}^{#gamma} (GeV) [%.0f-%.0f];|#eta_{jet}| [%.3f-%.3f]", alphaCutVal, ptMinAll, ptMaxAll, etaMinAll, etaMaxAll));
      TCanvas* cCounts = new TCanvas(Form("cCounts_mc_a%d", alphaBin), Form("cCounts_mc_a%d", alphaBin), 1000, 700);
      cCounts->SetRightMargin(0.15);
      cCounts->SetLeftMargin(0.12);
      cCounts->SetLogx();
      cCounts->cd();
      gStyle->SetPaintTextFormat("%.0f");
      h2dCounts->Draw("COLZ");
      CMS_lumi(cCounts, 0, 0);
      cCounts->SaveAs(Form("%s/L3Res_%s_alpha%d_counts_mc.png", pngFolder.c_str(), _run.c_str(), alphaBin));
      outfile->cd();
      h2dCounts->Write();
      delete cCounts;
      counts3D_mc->GetZaxis()->SetRange(1, nalphabins);
    }

    // Counts Data
    if (counts3D_data) {
      counts3D_data->GetZaxis()->SetRange(1, alphaBin);
      TH2D* h2dCountsData = (TH2D*)counts3D_data->Project3D("yx");
      h2dCountsData->SetName(Form("counts_pteta_data_alpha%d", alphaBin));
      double ptMinAll = balance3D_data->GetXaxis()->GetBinLowEdge(1);
      double ptMaxAll = balance3D_data->GetXaxis()->GetBinLowEdge(nptbins+1);
      h2dCountsData->SetTitle(Form("Data Counts (#alpha < %.2f);p_{T}^{#gamma} (GeV) [%.0f-%.0f];|#eta_{jet}| [%.3f-%.3f]", alphaCutVal, ptMinAll, ptMaxAll, etaMinAll, etaMaxAll));
      TCanvas* cCountsData = new TCanvas(Form("cCounts_data_a%d", alphaBin), Form("cCounts_data_a%d", alphaBin), 1000, 700);
      cCountsData->SetRightMargin(0.15);
      cCountsData->SetLeftMargin(0.12);
      cCountsData->SetLogx();
      cCountsData->cd();
      gStyle->SetPaintTextFormat("%.0f");
      h2dCountsData->Draw("COLZ");
      CMS_lumi(cCountsData, 0, 0);
      cCountsData->SaveAs(Form("%s/L3Res_%s_alpha%d_counts_data.png", pngFolder.c_str(), _run.c_str(), alphaBin));
      outfile->cd();
      h2dCountsData->Write();
      delete cCountsData;
      counts3D_data->GetZaxis()->SetRange(1, nalphabins);
    }

    outfile->cd();
    hRatio->Write(Form("ratio_vspT_alpha%d", alphaBin));
    if (hRatioClean) hRatioClean->Write(Form("ratio_vspT_cleaned_alpha%d", alphaBin));
    f0->Write(); f1->Write(); f2->Write(); f3->Write(); f4->Write(); fref->Write();

    delete c1;
    delete c2;
  }

  ftxt.close();
  outfile->Close();
  inFile->Close();

  cout << "\n============================================" << endl;
  cout << "Output written to: " << outfolder << "/" << outfilename << ".root" << endl;
  cout << "Plots saved in: " << pngFolder << "/" << endl;
  cout << "Text file: " << txtFolder << "/" << outfilename << ".txt" << endl;
  cout << "============================================" << endl;
}
