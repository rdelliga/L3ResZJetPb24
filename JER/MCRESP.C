#include "../histograms.h"
#include "plots.h"

// Plot MC jet response
// Same as MCJER, but just take mean from the TH3D

void MCRESP(string inFileName = "../../../HIJEC_rereco_results/RERECOMC_AK4_PFTRIG_nojetid.root", int minpt = 15, string dirname = "MCJER") {

  int pts[] = {40, 55, 80, 120, 170, 1000}; 
  float etabins[] = {0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0};

  float ptsforwidths[] = {80, 120, 170, 1000}; 
  TH1D* widths = new TH1D("widths","widths",3,&ptsforwidths[0]);

  map<int, TH1D*> mus;

  for (int ebin = 1; ebin <= histograms::netaforjer; ++ebin) {
     mus[ebin] =  new TH1D(Form("widths_%d",ebin),"",histograms::nptforJER,&histograms::ptforJER[0]);
    }

  TFile *inFile = new TFile(inFileName.c_str(), "READ");

  TH3D* responseprofile = (TH3D*)inFile->Get("hibin_-1.0_0.0/eta_-5.2_5.2/responses3D");

  TCanvas *c1 = new TCanvas("c1","c1",800,600);

  for (int ptbin = 1; ptbin <= responseprofile->GetXaxis()->GetNbins(); ++ptbin) {
      for (int etabin = 1; etabin <= responseprofile->GetYaxis()->GetNbins(); ++etabin) {
  
	//for (int ptbin = 1; ptbin <= 1; ++ptbin) {
	// for (int etabin = 1; etabin <= 1; ++etabin) {

  // Manually limit bins for 2023, old MC
      /*	if (ptbin == 8 and etabin > 11) continue;
	if (ptbin == 9 and etabin > 10) continue;
	if (ptbin == 10 and etabin > 9) continue;
	if (ptbin == 11 and etabin > 8) continue;
	if (ptbin == 12 and etabin > 8) continue;
	if (ptbin == 13 and etabin > 8) continue;
	if (ptbin == 14 and etabin > 7) continue;
	if (ptbin == 15 and etabin > 6) continue;
	if (ptbin == 16 and etabin > 5) continue;
	if (ptbin == 17 and etabin > 4) continue; */

	responseprofile->GetXaxis()->SetRange(ptbin,ptbin);    // pt axis - 2to5
	responseprofile->GetYaxis()->SetRange(etabin,etabin);    // eta axis

	TH1D* resp = (TH1D*)responseprofile->Project3D("z");
	//	resp->Draw();
	//      cout << "MEAN: " << resp->GetMean() << endl;

	
	mus[etabin]->SetBinContent(ptbin,resp->GetMean());
	mus[etabin]->SetBinError(ptbin,resp->GetRMS());

      }


  }

  // Draw all
   auto leg = new TLegend(0.57,0.6,0.85,0.9);
   c1->SetLogx();
   
   for (int ebin = 1; ebin <= histograms::netaforjer; ++ebin) {
     mus[ebin]->SetStats(0);
     mus[ebin]->SetLineColor(cols[ebin-1]);
     mus[ebin]->GetXaxis()->SetRangeUser(15,1500);
     mus[ebin]->SetMaximum(1.5);
     mus[ebin]->GetXaxis()->SetTitle("p_{T,ptcl}");
     mus[ebin]->GetYaxis()->SetTitle("p_{T,reco}/p_{T,ptcl}");
     mus[ebin]->Draw("same");
     leg->AddEntry(mus[ebin],Form("%.3f < |#eta| < %.3f",histograms::etaforjer[ebin-1],histograms::etaforjer[ebin]));
     
     mus[ebin]->GetXaxis()->SetRangeUser(minpt,1000);
     
   }
   leg->Draw();

  c1->Print(Form("%s/allmctruthclosures.png",dirname.c_str()));
 

  
}
