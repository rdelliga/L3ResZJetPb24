// Do the fits as function of alpha
// TODO: version with trunct. RMS?
// TODO: save sigmas as function of alpha for DT and MC
// TODO: zero bias for low pT:s

#include "histograms.h"
string outfilename = "JERSF_sigmas_fits.root";


//void JERSF_fits(string inFileName = "HIJEC_results/rerunall_combinedbins/MC_AK4_jetid.root", string inFileNameDT = "HIJEC_results/rerunall_combinedbins/HP_AK4_PFTRIG_jetid_l2corr.root") {
void JERSF_fits(string inFileName = "HIJEC_results/rerunall_combinedbins/MC_AK4_jetid.root", string inFileNameZB = "HIJEC_results/rerunall_combinedbins/zerobiasall_jetid_l2corr.root", string inFileNameDT = "HIJEC_results/rerunall_combinedbins/HP_AK4_PFTRIG_jetid_l2corr.root") {

  //  int pts[] = {40, 55, 80, 120, 170, 1000}; // Temporary
  int pts[] = {15, 25, 80, 120, 1000}; // Temporary

  float etabins[] = {0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0}; // Temporary
  
  TFile *inFile = new TFile(inFileName.c_str(), "READ");
  TFile *inFileDT = new TFile(inFileNameDT.c_str(), "READ");
  TFile *inFileZB = new TFile(inFileNameZB.c_str(), "READ"); 

  map<int, TH3D*> asymmMCs, asymmDTs, asymmZBs; // _a10 to a55?
  int alphabins[] = {10, 15, 20, 25, 30, 35, 40, 45};

  TFile *outfile = new TFile(outfilename.c_str(),"RECREATE");
  TCanvas *c1 = new TCanvas("c1","c1",800,600);

  // Outputs
  map<int, map<int, TH1D*>> sigmasMCmap, sigmasDTmap;
  map<int, map<int, TGraphErrors*>> sigmasGraphMCmap, sigmasGraphDTmap;

  TH3D* asymmDT = (TH3D*)inFileDT->Get("hibin_-1.0_0.0/eta_-5.2_5.2/asymmdist3D");
  for (int ptbin = 2; ptbin <= asymmDT->GetXaxis()->GetNbins(); ++ptbin) {
    for (int etabin = 1; etabin <= asymmDT->GetYaxis()->GetNbins(); ++etabin) {
      sigmasMCmap[ptbin][etabin] = new TH1D(Form("sigmasMCpt%deta%d",ptbin,etabin),"; ;",  histograms::nalphavalues, &histograms::alphavalues[0]);
      sigmasDTmap[ptbin][etabin] = new TH1D(Form("sigmasDTpt%deta%d",ptbin,etabin),"; ;",  histograms::nalphavalues, &histograms::alphavalues[0]);
    }
  }


  
  int nalphas= 7;
  for (int i = 0; i < nalphas; ++i) {
    int alphabin = i +1;
    cout << "alpha: " << alphabins[i] << " " << Form("hibin_-1.0_0.0/eta_-5.2_5.2/asymmdist3D_a%d",alphabins[i]) <<  " file " << inFileNameDT.c_str() << endl;
    asymmMCs[i] = (TH3D*)inFile->Get(Form("hibin_-1.0_0.0/eta_-5.2_5.2/asymmdist3D_a%d",alphabins[i]));
    asymmDTs[i] = (TH3D*)inFileDT->Get(Form("hibin_-1.0_0.0/eta_-5.2_5.2/asymmdist3D_a%d",alphabins[i]));
    asymmZBs[i] = (TH3D*)inFileZB->Get(Form("hibin_-1.0_0.0/eta_-5.2_5.2/asymmdist3D_a%d",alphabins[i]));


    for (int ptbin = 2; ptbin <= asymmDTs[i]->GetXaxis()->GetNbins(); ++ptbin) {
      for (int etabin = 1; etabin <= asymmDTs[i]->GetYaxis()->GetNbins(); ++etabin) {
	//	cout << asymmDT->GetBinContent(ptbin,etabin,20) << " " <<  asymmDT->GetBinError(ptbin,etabin,20) << endl;
  
	// 	TH1D* asMC = (TH1D*)asymmMC->ProjectionZ("MC",ptbin,ptbin,etabin,etabin);
	//TH1D* asDT = (TH1D*)asymmDT->ProjectionZ("DT",ptbin,ptbin,etabin,etabin);
	TH1D* asMC = (TH1D*)asymmMCs[i]->ProjectionZ("MC",ptbin,ptbin,etabin,etabin);
	TH1D* asDT(0);

	// Take HP instead of ZB; TODO: be more flexible for 2024
	if ( ptbin > 3 ) asDT = (TH1D*)asymmDTs[i]->ProjectionZ("DT",ptbin,ptbin,etabin,etabin);
	else asDT = (TH1D*)asymmZBs[i]->ProjectionZ("DT",ptbin,ptbin,etabin,etabin);
	
	//	asDT->Scale(1./asDT->Integral(),"width"); // For some reason using "scale" leads to histograms not plotted with error bars after the first iteration
	asDT->SetTitle("");
	asDT->GetXaxis()->SetTitle("A");
	

	//	cout << asymmDT->GetBinContent(ptbin,etabin,20) << " " <<  asymmDT->GetBinError(ptbin,etabin,20) << endl;
	//      cout << asDT->GetBinContent(20) << " " <<  asDT->GetBinError(20) << endl;
	asDT->Draw("E0");
	asDT->Fit("gaus"); // TODO: fit twice?

	TF1 *fit = asDT->GetFunction("gaus");

	double sigmaDT = fit->GetParameter(2);
	double errorDT = fit->GetParError(2);
	sigmasDTmap[ptbin][etabin]->SetBinContent(alphabin,sigmaDT);
	sigmasDTmap[ptbin][etabin]->SetBinError(alphabin,errorDT);
	
	auto txt = new TLatex();
	txt->SetNDC();
	txt->SetTextSize(0.03);
	txt->DrawLatex( 0.2, 0.35, Form("%.1f < |#eta| < %.1f",etabins[etabin-1],etabins[etabin]));
	txt->DrawLatex( 0.2, 0.4, Form("%d < p_{T,avg} < %d",pts[ptbin-1],pts[ptbin]));
	txt->DrawLatex( 0.2, 0.45, Form("#sigma = %.5f",sigmaDT));
       
	c1->SetLogy();
        c1->Print(Form("jersfhists_L2_rerunall/asymm_DT_ptbin_%d_etabin_%d_a%d.png",ptbin,etabin,i));
	//c1->Print(Form("jersfhists_L2_rerunall/asymm_DT_ptbin_%d_etabin_%d_a.pdf",ptbin,etabin));
      }
      }
 


   for (int ptbin = 2; ptbin <= asymmDT->GetXaxis()->GetNbins(); ++ptbin) {
      for (int etabin = 1; etabin <= asymmDT->GetYaxis()->GetNbins(); ++etabin) {
	//	cout << asymmDT->GetBinContent(ptbin,etabin,20) << " " <<  asymmDT->GetBinError(ptbin,etabin,20) << endl;
  	TH1D* asMC = (TH1D*)asymmMCs[i]->ProjectionZ("MC",ptbin,ptbin,etabin,etabin);
	//	TH1D* asMC = (TH1D*)asymmMC->ProjectionZ("MC",ptbin,ptbin,etabin,etabin);
	
	// asMC->Scale(1./asMC->Integral(),"width");
	// Technically we'd need the lumi normalization?
	
	asMC->SetTitle("");

	asMC->GetXaxis()->SetTitle("A");

	//	cout << asymmDT->GetBinContent(ptbin,etabin,20) << " " <<  asymmDT->GetBinError(ptbin,etabin,20) << endl;
	//      cout << asDT->GetBinContent(20) << " " <<  asDT->GetBinError(20) << endl;
	asMC->Draw("");
	asMC->Fit("gaus");
	
	c1->SetLogy();

	// Put the labels in

	TF1 *fit = asMC->GetFunction("gaus");
     	//  cout << fit->GetParameter(1) << endl;    //Mean, sigma is 2

	double sigmaMC = fit->GetParameter(2);
	double errorMC = fit->GetParError(2);
	sigmasMCmap[ptbin][etabin]->SetBinContent(alphabin,sigmaMC);
	sigmasMCmap[ptbin][etabin]->SetBinError(alphabin,errorMC);


	auto txt = new TLatex();
	txt->SetNDC();
	txt->SetTextSize(0.03);
	txt->DrawLatex( 0.2, 0.35, Form("%.1f < |#eta| < %.1f",etabins[etabin-1],etabins[etabin]));
	txt->DrawLatex( 0.2, 0.4, Form("%d < p_{T,avg} < %d",pts[ptbin-1],pts[ptbin]));
	txt->DrawLatex( 0.2, 0.45, Form("#sigma = %.5f",sigmaMC));
       
	c1->Print(Form("jersfhists_L2_rerunall/asymm_MC_ptbin_%d_etabin_%d_a%d.png",ptbin,etabin,i));
      }
  }
  }

  for (int ptbin = 2; ptbin <= asymmDT->GetXaxis()->GetNbins(); ++ptbin) {
    for (int etabin = 1; etabin <= asymmDT->GetYaxis()->GetNbins(); ++etabin) {
      sigmasMCmap[ptbin][etabin]->Write(Form("sigmasMCpt%deta%d",ptbin,etabin));
      sigmasDTmap[ptbin][etabin]->Write(Form("sigmasDTpt%deta%d",ptbin,etabin));

      
      Double_t results[histograms::nalphavaluesgraphjer];
      Double_t errors[histograms::nalphavaluesgraphjer];
      Double_t errorsx[histograms::nalphavaluesgraphjer];

      Double_t resultsdt[histograms::nalphavaluesgraphjer];
      Double_t errorsdt[histograms::nalphavaluesgraphjer];
      
      for (int abin = 0; abin < histograms::nalphavaluesgraphjer; ++abin) {
	
        results[abin] =  sigmasMCmap[ptbin][etabin]->GetBinContent(abin+1);
	errors[abin] =   sigmasMCmap[ptbin][etabin]->GetBinError(abin+1);

	resultsdt[abin] =  sigmasDTmap[ptbin][etabin]->GetBinContent(abin+1);
	errorsdt[abin] =   sigmasDTmap[ptbin][etabin]->GetBinError(abin+1);
   }

      sigmasGraphMCmap[ptbin][etabin] = new TGraphErrors(histograms::nalphavaluesgraphjer, histograms::alphavaluesgraphjer, results, errorsx, errors); // TODO: do not include reference?
      sigmasGraphMCmap[ptbin][etabin]->Write(Form("gsigmasMCpt%deta%d",ptbin,etabin));

      sigmasGraphDTmap[ptbin][etabin] = new TGraphErrors(histograms::nalphavaluesgraphjer, histograms::alphavaluesgraphjer, resultsdt, errorsx, errorsdt); // TODO: do not include reference?
      sigmasGraphDTmap[ptbin][etabin]->Write(Form("gsigmasDTpt%deta%d",ptbin,etabin));

    }
  }
    
}
