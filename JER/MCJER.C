#include "../fillhistograms/histograms.h"
#include "TMath.h"
//#include "plots.h"

void MCJER(TString inFileName = "../../../HIJEC_rereco_results/RERECOMC_AK4_PFTRIG_nojetid.root", int minpt = 28, string dirname = "MCJER", TString outFileName = "testingMCjer-rereco.root") {
     
  map<int, TH1D*> ws;

  for (int ebin = 1; ebin <= histograms::netaforjer; ++ebin) {
     ws[ebin] =  new TH1D(Form("widths_%d",ebin),"",histograms::nptforJER,&histograms::ptforJER[0]);
    }

  TFile *inFile = new TFile(inFileName, "READ");
  TFile *outfile = new TFile(outFileName,"RECREATE");

  TH3D* responseprofile = (TH3D*)inFile->Get("hibin_-1.0_0.0/eta_-5.2_5.2/responses3D");

  TCanvas *c1 = new TCanvas("c1","c1",800,600);

  for (int ptbin = 1; ptbin <= responseprofile->GetXaxis()->GetNbins(); ++ptbin) {
      for (int etabin = 1; etabin <= responseprofile->GetYaxis()->GetNbins(); ++etabin) {
	// Manually limit bins for 2023, old MC
	if (ptbin == 8 and etabin > 11) continue;
	if (ptbin == 9 and etabin > 10) continue;
	if (ptbin == 10 and etabin > 9) continue;
	if (ptbin == 11 and etabin > 8) continue;
	if (ptbin == 12 and etabin > 8) continue;
	if (ptbin == 13 and etabin > 8) continue;
	if (ptbin == 14 and etabin > 7) continue;
	if (ptbin == 15 and etabin > 6) continue;
	if (ptbin == 16 and etabin > 5) continue;
	if (ptbin == 17 and etabin > 4) continue;

	
	cout << "etabin " << etabin << endl;
	responseprofile->GetXaxis()->SetRange(ptbin,ptbin);    // pt axis - 2to5
	responseprofile->GetYaxis()->SetRange(etabin,etabin);    // eta axis

	TH1D* resp = (TH1D*)responseprofile->Project3D("z");

	resp->Scale(1./resp->Integral(),"width");
	resp->SetTitle("");

	resp->GetXaxis()->SetTitle("p_{T,reco}/p_{T,gen}");

	// JER should come from a gaussian fit here
	resp->Draw();

	// Fit twice
	TF1 *f1 = new TF1("f1", "gaus");
	f1->SetParameter(0,1.5);
	resp->Fit("f1");


	double c = f1->GetParameter(0);
	double m = f1->GetParameter(1);
	double s = f1->GetParameter(2);

	TF1 *f2 = new TF1("f2", "gaus", m-2*s, m+2*s );
	f2->SetParameters(c,m,s);

	resp->Fit("f2","R");

	
	c1->SetLogy();
	auto txt = new TLatex();
	txt->SetNDC();
	txt->SetTextSize(0.03);
	txt->DrawLatex( 0.2, 0.35, Form("%.1f < |#eta| < %.1f",histograms::etaforjer[etabin-1],histograms::etaforjer[etabin]));
	txt->DrawLatex( 0.2, 0.4, Form("%g < p_{T,gen} < %g",histograms::ptforJER[ptbin-1],histograms::ptforJER[ptbin]));
	txt->DrawLatex( 0.2, 0.45, Form("#sigma = %.5f #pm %.5f",f2->GetParameter(2), f2->GetParError(2)));

	ws[etabin]->SetBinContent(ptbin,f2->GetParameter(2));
	ws[etabin]->SetBinError(ptbin,f2->GetParError(2));
	//	ws[etabin]->SetMaximum(0.2);       

	c1->Print(Form("%s/resp_ptbin_%d_etabin_%d.png",dirname.c_str(),ptbin,etabin));
      }
  }

  //  

  auto leg = new TLegend(0.57,0.6,0.85,0.9);
  auto legb = new TLegend(0.57,0.6,0.85,0.9); //barrel(ish) bins
  auto legf = new TLegend(0.57,0.6,0.85,0.9); //forward(ish) bins

  TCanvas *c2 = new TCanvas("c2","c2",800,600);
  c2->SetLogx();
  gStyle->SetOptStat(0);

  TCanvas *c2b = new TCanvas("c2b","c2b",800,600);
  c2b->SetLogx();
  gStyle->SetOptStat(0);

  TCanvas *c2f = new TCanvas("c2f","c2f",800,600);
  c2f->SetLogx();
  gStyle->SetOptStat(0);
  
  for (int ebin = 1; ebin <= histograms::netaforjer; ++ebin) {
    //for (int ebin = 1; ebin <= 5; ++ebin) {

    c2->cd();
    
    ws[ebin]->SetLineColor(cols[ebin-1]);
    ws[ebin]->GetXaxis()->SetRangeUser(15,1500);
    ws[ebin]->SetMaximum(0.5);
    ws[ebin]->GetXaxis()->SetTitle("p_{T,ptcl}");
    ws[ebin]->GetYaxis()->SetTitle("#sigma");
    
    ws[ebin]->Draw("same");

    leg->AddEntry(ws[ebin],Form("%.3f < |#eta| < %.3f",histograms::etaforjer[ebin-1],histograms::etaforjer[ebin]));

    ws[ebin]->GetXaxis()->SetRangeUser(minpt,1000);
    ws[ebin]->Write();

    // Fit NSC: sqrt([0]*abs([0])/(x*x)+[1]*[1]*pow(x,[3])+[2]*[2])
    //TF1 *nsc = new TF1("nsc", "sqrt([0]*abs([0])/(x*x)+[1]*[1]*pow(x,[3])+[2]*[2])");

   TF1 *nsc = new TF1("nsc", "sqrt([0]*([0])/(x*x)+[1]*[1]*pow(x,[3])+[2]*[2])");
    nsc->SetLineColor(cols[ebin-1]);
    if (ebin ==  histograms::netaforjer) ws[ebin]->GetXaxis()->SetRangeUser(minpt,100);
    // nsc->SetLineColor(kBlue-8+ebin);
    ws[ebin]->Fit("nsc");

    nsc->Write(Form("fit_eta_%.3fto%.3f",histograms::etaforjer[ebin-1],histograms::etaforjer[ebin]));
    
    nsc->Draw("same");
    if (ebin <= 8) {
      c2b->cd();
      ws[ebin]->Draw("same");
      nsc->Draw("same");
      legb->AddEntry(ws[ebin],Form("%.3f < |#eta| < %.3f",histograms::etaforjer[ebin-1],histograms::etaforjer[ebin]));
    }

    else {
      c2f->cd();
       ws[ebin]->Draw("same");
       nsc->Draw("same");
       legf->AddEntry(ws[ebin],Form("%.3f < |#eta| < %.3f",histograms::etaforjer[ebin-1],histograms::etaforjer[ebin]));
    }
    
    
  } 
  leg->Draw();
  c2b->cd();
  legb->Draw();
  
  c2f->cd();
  legf->Draw();

  c2->Print(Form("%s/allmcjers.png",dirname.c_str()));
  c2b->Print(Form("%s/allmcjers_barrel.png",dirname.c_str()));
  c2f->Print(Form("%s/allmcjers_fwd.png",dirname.c_str()));
  
  outfile->Close();
}
