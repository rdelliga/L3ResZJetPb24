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
  // Storage for kFSR extrapolation results (per pT bin, collapsed over eta)
  map<int, double> kFSR_vsPt;       // kFSR = p0 from linear fit at alpha->0
  map<int, double> kFSR_err_vsPt;   // error on p0
  
  if (saveAlphaExtrap) {
    cout << "Saving alpha extrapolation histograms to " << alphaFolder << endl;
    TAxis* zaxis = balance3D_mc->GetZaxis();
    int nAlphaBins = zaxis->GetNbins();
    std::vector<double> alphaEdges(nAlphaBins + 1);
    for (int b = 1; b <= nAlphaBins + 1; ++b) alphaEdges[b-1] = zaxis->GetBinLowEdge(b);

    // Colors for multiple pT bins on overlay plots
    int ptColors[] = {kBlue, kRed, kGreen+2, kMagenta+2, kOrange+2, kCyan+2, kViolet+2, kTeal+2, kPink+2, kAzure+2};
    int nPtColors = sizeof(ptColors)/sizeof(ptColors[0]);

    // First loop: per-pT, per-eta alpha fits (detailed)
    for (int ptbin = 1; ptbin <= nptbins; ++ptbin) {
      for (int etabin = 1; etabin <= netabins; ++etabin) {
        TH1D* hAlphaMc = new TH1D(Form("alpha_mc_pt%d_eta%d", ptbin, etabin), "MC balance vs #alpha;#alpha;Balance", nAlphaBins, alphaEdges.data());
        TH1D* hAlphaDt = new TH1D(Form("alpha_dt_pt%d_eta%d", ptbin, etabin), "Data balance vs #alpha;#alpha;Balance", nAlphaBins, alphaEdges.data());
        TH1D* hAlphaRatio = new TH1D(Form("alpha_ratio_pt%d_eta%d", ptbin, etabin), "Balance Ratio (MC/Data) vs #alpha;#alpha;Balance Ratio", nAlphaBins, alphaEdges.data());

        int nValidBins = 0;
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
            nValidBins++;
          }
        }

        // Get readable pT and eta bin ranges for labeling
        double ptBinLo = balance3D_mc->GetXaxis()->GetBinLowEdge(ptbin);
        double ptBinHi = balance3D_mc->GetXaxis()->GetBinLowEdge(ptbin+1);
        double etaBinLo = balance3D_mc->GetYaxis()->GetBinLowEdge(etabin);
        double etaBinHi = balance3D_mc->GetYaxis()->GetBinLowEdge(etabin+1);

        // Fit ratio vs alpha with linear function to extrapolate to alpha->0
        // Use fit range that excludes potentially empty low-alpha bins
        double fitMin = 0.1;  // Start from alpha > 0.1 to avoid empty bins
        double fitMax = 0.35; // End before high-alpha bins which may have different behavior
        TF1* fAlpha = new TF1(Form("fAlpha_pt%d_eta%d", ptbin, etabin), "[0]+[1]*x", fitMin, fitMax);
        fAlpha->SetParameters(1.0, 0.0);
        
        // Only fit if we have enough valid bins
        if (nValidBins >= 3) {
          hAlphaRatio->Fit(fAlpha, "QNR");
        }

        TCanvas* cAlpha = new TCanvas(Form("cAlpha_pt%.0fto%.0f_eta%.3fto%.3f", ptBinLo, ptBinHi, etaBinLo, etaBinHi), 
                                       Form("cAlpha_pt%.0fto%.0f_eta%.3fto%.3f", ptBinLo, ptBinHi, etaBinLo, etaBinHi), 800, 600);
        cAlpha->cd();
        hAlphaRatio->SetMinimum(0.7);
        hAlphaRatio->SetMaximum(1.5);
        hAlphaRatio->SetMarkerStyle(kFullCircle);
        hAlphaRatio->SetMarkerColor(kBlue);
        hAlphaRatio->SetLineColor(kBlue);
        hAlphaRatio->GetXaxis()->SetTitle("#alpha");
        hAlphaRatio->GetYaxis()->SetTitle("Balance Ratio (MC/Data)");
        hAlphaRatio->Draw("PE1");
        
        fAlpha->SetLineColor(kRed);
        fAlpha->SetLineWidth(2);
        fAlpha->Draw("SAME");
        
        // Add reference line at 1.0
        TLine* lineRef = new TLine(alphaEdges.front(), 1.0, alphaEdges.back(), 1.0);
        lineRef->SetLineStyle(kDashed);
        lineRef->SetLineColor(kGray+1);
        lineRef->Draw("SAME");
        
        // Legend following dofits.C style
        TLegend* legAlpha = new TLegend(0.55, 0.70, 0.88, 0.88);
        legAlpha->SetBorderSize(0);
        legAlpha->SetFillStyle(0);
        legAlpha->SetTextFont(42);
        legAlpha->SetTextSize(0.030);
        legAlpha->AddEntry(hAlphaRatio, "Balance Ratio vs #alpha", "PLE");
        legAlpha->AddEntry(fAlpha, Form("Linear fit (%.2f < #alpha < %.2f)", fitMin, fitMax), "L");
        legAlpha->Draw();
        
        // Labels following dofits.C style
        TLatex* tbin = new TLatex();
        tbin->SetNDC();
        tbin->SetTextFont(42);
        tbin->SetTextSize(0.035);
        tbin->DrawLatex(0.18, 0.85, Form("%.0f < p_{T}^{#gamma} < %.0f GeV", ptBinLo, ptBinHi));
        tbin->DrawLatex(0.18, 0.80, Form("%.3f < |#eta_{jet}| < %.3f", etaBinLo, etaBinHi));
        tbin->DrawLatex(0.18, 0.75, Form("p0 = %.4f #pm %.4f", fAlpha->GetParameter(0), fAlpha->GetParError(0)));
        tbin->DrawLatex(0.18, 0.70, Form("p1 = %.4f #pm %.4f", fAlpha->GetParameter(1), fAlpha->GetParError(1)));
        
        CMS_lumi(cAlpha, 0, 0);
        
        string alphaFile = Form("%s/L3Res_%s_pt%.0fto%.0f_eta%.3fto%.3f_alpha.png", alphaFolder.c_str(), _run.c_str(), ptBinLo, ptBinHi, etaBinLo, etaBinHi);
        cAlpha->SaveAs(alphaFile.c_str());

        outfile->cd();
        hAlphaMc->Write();
        hAlphaDt->Write();
        hAlphaRatio->Write();
        fAlpha->Write();

        delete lineRef;
        delete legAlpha;
        delete tbin;
        delete cAlpha;
        delete hAlphaMc;
        delete hAlphaDt;
        delete hAlphaRatio;
        delete fAlpha;
      }
    }
    
    // Second loop: kFSR extraction per pT bin (collapsed over eta)
    // This creates the plot of extrapolated alpha->0 value vs photon pT
    cout << "\n=== Extracting kFSR vs photon pT (alpha->0 extrapolation) ===" << endl;
    
    // Create histogram for kFSR vs pT
    std::vector<double> ptEdges(nptbins + 1);
    for (int b = 1; b <= nptbins + 1; ++b) ptEdges[b-1] = balance3D_mc->GetXaxis()->GetBinLowEdge(b);
    
    TH1D* hkFSR = new TH1D("kFSR_vsPt", "k_{FSR} (Balance Ratio extrapolated to #alpha#rightarrow0) vs p_{T}^{#gamma};p_{T}^{#gamma} (GeV);k_{FSR}", 
                           nptbins, ptEdges.data());
    
    // Also create overlay plot with all pT bins on same canvas
    TCanvas* cAlphaOverlay = new TCanvas("cAlphaOverlay", "Balance Ratio vs alpha (all pT bins)", 900, 700);
    cAlphaOverlay->cd();
    TH1D* hFrameAlpha = new TH1D("hFrameAlpha", ";#alpha;Balance Ratio (MC/Data)", 100, 0, 0.5);
    hFrameAlpha->SetMinimum(0.7);
    hFrameAlpha->SetMaximum(1.5);
    hFrameAlpha->Draw();
    TLine* lineRefOverlay = new TLine(0, 1.0, 0.5, 1.0);
    lineRefOverlay->SetLineStyle(kDashed);
    lineRefOverlay->SetLineColor(kGray+1);
    lineRefOverlay->Draw("SAME");
    
    TLegend* legOverlay = new TLegend(0.55, 0.55, 0.88, 0.88);
    legOverlay->SetBorderSize(0);
    legOverlay->SetFillStyle(0);
    legOverlay->SetTextFont(42);
    legOverlay->SetTextSize(0.025);
    
    for (int ptbin = 1; ptbin <= nptbins; ++ptbin) {
      double ptBinLo = balance3D_mc->GetXaxis()->GetBinLowEdge(ptbin);
      double ptBinHi = balance3D_mc->GetXaxis()->GetBinLowEdge(ptbin+1);
      double ptCenter = balance3D_mc->GetXaxis()->GetBinCenter(ptbin);
      
      // Create ratio histogram collapsed over eta
      TH1D* hAlphaRatioCol = new TH1D(Form("alpha_ratio_pt%d_collapsed", ptbin), 
                                       Form("Balance Ratio vs #alpha (%.0f-%.0f GeV)", ptBinLo, ptBinHi),
                                       nAlphaBins, alphaEdges.data());
      
      for (int abin = 1; abin <= nAlphaBins; ++abin) {
        double sum_mc = 0., entries_mc = 0.;
        double sum_dt = 0., entries_dt = 0.;
        
        for (int etabin = 1; etabin <= netabins; ++etabin) {
          double mc = balance3D_mc->GetBinContent(ptbin, etabin, abin);
          double ent_mc = balance3D_mc->GetBinEntries(balance3D_mc->GetBin(ptbin, etabin, abin));
          double dt = balance3D_data->GetBinContent(ptbin, etabin, abin);
          double ent_dt = balance3D_data->GetBinEntries(balance3D_data->GetBin(ptbin, etabin, abin));
          
          if (mc > 0 && ent_mc > 0 && !TMath::IsNaN(mc)) {
            sum_mc += mc * ent_mc;
            entries_mc += ent_mc;
          }
          if (dt > 0 && ent_dt > 0 && !TMath::IsNaN(dt)) {
            sum_dt += dt * ent_dt;
            entries_dt += ent_dt;
          }
        }
        
        if (entries_mc > 0 && entries_dt > 0) {
          double mc_avg = sum_mc / entries_mc;
          double dt_avg = sum_dt / entries_dt;
          double ratio = mc_avg / dt_avg;
          double err_mc = mc_avg / sqrt(entries_mc);
          double err_dt = dt_avg / sqrt(entries_dt);
          double rel_err = sqrt(pow(err_mc/mc_avg, 2) + pow(err_dt/dt_avg, 2));
          
          hAlphaRatioCol->SetBinContent(abin, ratio);
          hAlphaRatioCol->SetBinError(abin, ratio * rel_err);
        }
      }
      
      // Fit to extract kFSR (alpha -> 0 extrapolation)
      double fitMin = 0.1;
      double fitMax = 0.35;
      TF1* fAlphaCol = new TF1(Form("fAlpha_pt%d_col", ptbin), "[0]+[1]*x", fitMin, fitMax);
      fAlphaCol->SetParameters(1.0, 0.0);
      hAlphaRatioCol->Fit(fAlphaCol, "QNR");
      
      // Store kFSR value (p0 = value at alpha=0)
      double kfsr = fAlphaCol->GetParameter(0);
      double kfsr_err = fAlphaCol->GetParError(0);
      kFSR_vsPt[ptbin] = kfsr;
      kFSR_err_vsPt[ptbin] = kfsr_err;
      
      hkFSR->SetBinContent(ptbin, kfsr);
      hkFSR->SetBinError(ptbin, kfsr_err);
      
      cout << "  pT [" << ptBinLo << "-" << ptBinHi << "]: kFSR = " << kfsr << " +/- " << kfsr_err << endl;
      
      // Add to overlay plot
      int colorIdx = (ptbin - 1) % nPtColors;
      hAlphaRatioCol->SetMarkerStyle(kFullCircle);
      hAlphaRatioCol->SetMarkerColor(ptColors[colorIdx]);
      hAlphaRatioCol->SetLineColor(ptColors[colorIdx]);
      hAlphaRatioCol->Draw("PE1 SAME");
      
      fAlphaCol->SetLineColor(ptColors[colorIdx]);
      fAlphaCol->SetLineWidth(2);
      fAlphaCol->Draw("SAME");
      
      legOverlay->AddEntry(hAlphaRatioCol, Form("%.0f-%.0f GeV (k_{FSR}=%.3f)", ptBinLo, ptBinHi, kfsr), "PLE");
      
      outfile->cd();
      hAlphaRatioCol->Write();
      fAlphaCol->Write();
    }
    
    legOverlay->Draw();
    CMS_lumi(cAlphaOverlay, 0, 0);
    cAlphaOverlay->SaveAs(Form("%s/L3Res_%s_alpha_overlay.png", alphaFolder.c_str(), _run.c_str()));
    delete cAlphaOverlay;
    delete hFrameAlpha;
    delete lineRefOverlay;
    delete legOverlay;
    
    // Create kFSR vs pT plot with fit
    cout << "\n=== Fitting kFSR vs photon pT ===" << endl;
    
    TCanvas* ckFSR = new TCanvas("ckFSR", "kFSR vs photon pT", 800, 600);
    ckFSR->SetLogx();
    ckFSR->cd();
    
    TH1D* hFramekFSR = new TH1D("hFramekFSR", ";p_{T}^{#gamma} (GeV);k_{FSR} (Balance Ratio at #alpha#rightarrow0)", 100, 20, 600);
    hFramekFSR->SetMinimum(0.7);
    hFramekFSR->SetMaximum(1.5);
    hFramekFSR->Draw();
    
    TLine* linekFSR = new TLine(20, 1.0, 600, 1.0);
    linekFSR->SetLineStyle(kDashed);
    linekFSR->SetLineColor(kGray+1);
    linekFSR->Draw("SAME");
    
    hkFSR->SetMarkerStyle(kFullCircle);
    hkFSR->SetMarkerColor(kBlue);
    hkFSR->SetLineColor(kBlue);
    hkFSR->Draw("PE1 SAME");
    
    // Fit kFSR vs pT with various functions
    TF1* fkFSR_const = new TF1("fkFSR_const", "[0]", ptEdges.front(), ptEdges.back());
    TF1* fkFSR_log = new TF1("fkFSR_log", "[0]+[1]*log10(x)", ptEdges.front(), ptEdges.back());
    TF1* fkFSR_invpt = new TF1("fkFSR_invpt", "[0]+[1]/x", ptEdges.front(), ptEdges.back());
    
    fkFSR_const->SetParameter(0, 1.0);
    hkFSR->Fit(fkFSR_const, "QNR");
    
    fkFSR_log->SetParameters(fkFSR_const->GetParameter(0), 0.01);
    hkFSR->Fit(fkFSR_log, "QNR");
    
    fkFSR_invpt->SetParameters(fkFSR_const->GetParameter(0), 0.0);
    hkFSR->Fit(fkFSR_invpt, "QNR");
    
    fkFSR_const->SetLineColor(kMagenta+2);
    fkFSR_const->SetLineStyle(kDotted);
    fkFSR_const->Draw("SAME");
    
    fkFSR_log->SetLineColor(kGreen+2);
    fkFSR_log->SetLineWidth(2);
    fkFSR_log->Draw("SAME");
    
    fkFSR_invpt->SetLineColor(kRed);
    fkFSR_invpt->SetLineWidth(2);
    fkFSR_invpt->Draw("SAME");
    
    TLegend* legkFSR = new TLegend(0.50, 0.65, 0.88, 0.88);
    legkFSR->SetBorderSize(0);
    legkFSR->SetFillStyle(0);
    legkFSR->SetTextFont(42);
    legkFSR->SetTextSize(0.030);
    legkFSR->AddEntry(hkFSR, "k_{FSR} (#alpha#rightarrow0 extrap.)", "PLE");
    legkFSR->AddEntry(fkFSR_const, Form("Const: %.4f", fkFSR_const->GetParameter(0)), "L");
    legkFSR->AddEntry(fkFSR_log, Form("Log: %.4f + %.4f*log_{10}(p_{T})", fkFSR_log->GetParameter(0), fkFSR_log->GetParameter(1)), "L");
    legkFSR->AddEntry(fkFSR_invpt, Form("1/p_{T}: %.4f + %.2f/p_{T}", fkFSR_invpt->GetParameter(0), fkFSR_invpt->GetParameter(1)), "L");
    legkFSR->Draw();
    
    TLatex* texkFSR = new TLatex();
    texkFSR->SetNDC();
    texkFSR->SetTextFont(42);
    texkFSR->SetTextSize(0.035);
    texkFSR->DrawLatex(0.18, 0.25, Form("#chi^{2}/ndf (const) = %.1f/%d", fkFSR_const->GetChisquare(), fkFSR_const->GetNDF()));
    texkFSR->DrawLatex(0.18, 0.20, Form("#chi^{2}/ndf (log) = %.1f/%d", fkFSR_log->GetChisquare(), fkFSR_log->GetNDF()));
    texkFSR->DrawLatex(0.18, 0.15, Form("#chi^{2}/ndf (1/p_{T}) = %.1f/%d", fkFSR_invpt->GetChisquare(), fkFSR_invpt->GetNDF()));
    
    CMS_lumi(ckFSR, 0, 0);
    ckFSR->SaveAs(Form("%s/L3Res_%s_kFSR_vspT.png", alphaFolder.c_str(), _run.c_str()));
    
    outfile->cd();
    hkFSR->Write();
    fkFSR_const->Write();
    fkFSR_log->Write();
    fkFSR_invpt->Write();
    
    delete ckFSR;
    delete hFramekFSR;
    delete linekFSR;
    delete legkFSR;
    delete texkFSR;
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
    hFrame1->SetMinimum(0.7);
    hFrame1->SetMaximum(1.5);
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
    hRatio->Draw("PE1 SAME");
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
        frameRaw->SetMinimum(0.7);
        frameRaw->SetMaximum(1.5);
        frameRaw->Draw();
        hMcRaw->SetMarkerStyle(kFullCircle);
        hMcRaw->SetMarkerColor(kBlue);
        hMcRaw->SetLineColor(kBlue);
        hMcRaw->Draw("PE1 SAME");
        hDtRaw->SetMarkerStyle(kFullSquare);
        hDtRaw->SetMarkerColor(kRed);
        hDtRaw->SetLineColor(kRed);
        hDtRaw->Draw("PE1 SAME");
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
    hFrame2->SetMinimum(0.7);
    hFrame2->SetMaximum(1.5);
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
    hRatioFull->Draw("PE1 SAME");
    TH1D* hRatioClean = drawCleaned(hRatio, "G", ptminG, ptmaxG, kFullCircle, kBlue);
    hRatioClean->Draw("PE1 SAME");
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
      hCorrected->Draw("PE1 SAME");
      TH1D* hUncorr = (TH1D*)hRatio->Clone(Form("hUncorr_a%d", alphaBin));
      hUncorr->SetMarkerStyle(kOpenCircle);
      hUncorr->SetMarkerColor(kBlue);
      hUncorr->SetLineColor(kBlue);
      hUncorr->Draw("PE1 SAME");
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
      // draw as color map (no overlaid text) and use log-x for pT readability
      gStyle->SetPaintTextFormat("0.0f");
      h2dCounts->SetMarkerSize(1.4);
      h2dCounts->Draw("TEXTCOLZ");
      CMS_lumi(cCounts, 0, 0);
      cCounts->SaveAs(Form("%s/L3Res_%s_alpha%d_counts_mc_pt%.0fto%.0f_eta%.3fto%.3f.png", pngFolder.c_str(), _run.c_str(), alphaBin, ptMinAll, ptMaxAll, etaMinAll, etaMaxAll));
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
      gStyle->SetPaintTextFormat("0.0f");
      h2dCountsData->SetMarkerSize(1.4);
      h2dCountsData->Draw("TEXTCOLZ");
      CMS_lumi(cCountsData, 0, 0);
      cCountsData->SaveAs(Form("%s/L3Res_%s_alpha%d_counts_data_pt%.0fto%.0f_eta%.3fto%.3f.png", pngFolder.c_str(), _run.c_str(), alphaBin, ptMinAll, ptMaxAll, etaMinAll, etaMaxAll));
      outfile->cd();
      h2dCountsData->Write();
      delete cCountsData;
      counts3D_data->GetZaxis()->SetRange(1, nalphabins);
    }

    // L3 Residual Map (MC/Data ratio) as 2D TEXTCOLZ plot
    {
      double ptMinAll = balance3D_mc->GetXaxis()->GetBinLowEdge(1);
      double ptMaxAll = balance3D_mc->GetXaxis()->GetBinLowEdge(nptbins+1);
      
      // Create 2D histogram for L3Res map
      std::vector<double> ptEdges(nptbins + 1), etaEdges(netabins + 1);
      for (int b = 1; b <= nptbins + 1; ++b) ptEdges[b-1] = balance3D_mc->GetXaxis()->GetBinLowEdge(b);
      for (int b = 1; b <= netabins + 1; ++b) etaEdges[b-1] = balance3D_mc->GetYaxis()->GetBinLowEdge(b);
      
      TH2D* h2dL3Res = new TH2D(Form("l3res_pteta_alpha%d", alphaBin),
                                 Form("L3 Residual (MC/Data) (#alpha < %.2f);p_{T}^{#gamma} (GeV) [%.0f-%.0f];|#eta_{jet}| [%.3f-%.3f]",
                                      alphaCutVal, ptMinAll, ptMaxAll, etaMinAll, etaMaxAll),
                                 nptbins, ptEdges.data(), netabins, etaEdges.data());
      
      // Fill from 3D profiles with cumulative alpha cut
      balance3D_mc->GetZaxis()->SetRange(1, alphaBin);
      balance3D_data->GetZaxis()->SetRange(1, alphaBin);
      
      for (int ptbin = 1; ptbin <= nptbins; ++ptbin) {
        for (int etabin = 1; etabin <= netabins; ++etabin) {
          double sum_mc = 0., entries_mc = 0.;
          double sum_dt = 0., entries_dt = 0.;
          
          for (int abin = 1; abin <= alphaBin; ++abin) {
            double val_mc = balance3D_mc->GetBinContent(ptbin, etabin, abin);
            double ent_mc = balance3D_mc->GetBinEntries(balance3D_mc->GetBin(ptbin, etabin, abin));
            double val_dt = balance3D_data->GetBinContent(ptbin, etabin, abin);
            double ent_dt = balance3D_data->GetBinEntries(balance3D_data->GetBin(ptbin, etabin, abin));
            
            if (val_mc > 0 && ent_mc > 0 && !TMath::IsNaN(val_mc)) {
              sum_mc += val_mc * ent_mc;
              entries_mc += ent_mc;
            }
            if (val_dt > 0 && ent_dt > 0 && !TMath::IsNaN(val_dt)) {
              sum_dt += val_dt * ent_dt;
              entries_dt += ent_dt;
            }
          }
          
          double mc_val = (entries_mc > 0) ? sum_mc / entries_mc : 0.;
          double dt_val = (entries_dt > 0) ? sum_dt / entries_dt : 0.;
          double l3res = (dt_val > 0 && mc_val > 0) ? mc_val / dt_val : 1.0;
          
          h2dL3Res->SetBinContent(ptbin, etabin, l3res);
        }
      }
      
      balance3D_mc->GetZaxis()->SetRange(1, nalphabins);
      balance3D_data->GetZaxis()->SetRange(1, nalphabins);
      
      TCanvas* cL3Res = new TCanvas(Form("cL3Res_a%d", alphaBin), Form("cL3Res_a%d", alphaBin), 1000, 700);
      cL3Res->SetRightMargin(0.15);
      cL3Res->SetLeftMargin(0.12);
      cL3Res->SetLogx();
      cL3Res->cd();
      gStyle->SetPaintTextFormat("0.3f");
      h2dL3Res->SetMarkerSize(1.4);
      h2dL3Res->SetMinimum(0.7);
      h2dL3Res->SetMaximum(1.5);
      h2dL3Res->Draw("TEXTCOLZ");
      CMS_lumi(cL3Res, 0, 0);
      cL3Res->SaveAs(Form("%s/L3Res_%s_alpha%d_l3resmap_pt%.0fto%.0f_eta%.3fto%.3f.png", pngFolder.c_str(), _run.c_str(), alphaBin, ptMinAll, ptMaxAll, etaMinAll, etaMaxAll));
      outfile->cd();
      h2dL3Res->Write();
      delete cL3Res;
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
