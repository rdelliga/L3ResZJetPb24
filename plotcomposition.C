// THStack to plot jet PF composition

void plotcomposition() {

  gStyle->SetOptStat(0);

  TFile *filein = new TFile("results/pbpbreco_witholdmctruth_abseta.root","READ"); // is this data or MC?
  TFile *filedata = new TFile("results/ppreco_witholdmctruth_abseta.root","READ");
  
  vector<string> etabins = {"eta_-5.2_-3.9", "eta_-3.9_-2.6", "eta_-2.6_-1.3", "eta_-1.3_0.0", "eta_0.0_1.3", "eta_1.3_2.6", "eta_2.6_3.9", "eta_3.9_5.2"};
  string plottag = "pbpbreco";
  string plottitle = "2023 ppref, PbPb reco";
  
  // Select eta bin to plot
  int i = 4;

  auto leg = new TLegend(0.1,0.1,0.4,0.3);
  
  map<string, map<string,TProfile*>> profs;
  map<string,map<string,TH1D*>> hists, histsdt;

  vector<string> histonames = {"chf", "nhf", "nef", "cef", "muf"}; // In pp plots cef+muf combined
  vector<string> histonamesdt = {"chf", "nhf", "nef", "cef", "muf"}; // In pp plots cef+muf combined
  vector<string> colours = {"kRed", "kGreen", "kBlue", "kCyan", "kYellow"};

  map<string, pair<int, int> > _style;
  _style["chf"] = make_pair<int, int>(kRed, kFullCircle);
  _style["nef"] = make_pair<int, int>(kBlue, kFullSquare);
  _style["nhf"] = make_pair<int, int>(kGreen+1, kFullDiamond);
  _style["cef"] = make_pair<int, int>(kCyan+1, kOpenTriangleUp);
  _style["muf"] = make_pair<int, int>(kMagenta+1, kOpenTriangleDown);
  _style["hhf"] = make_pair<int, int>(kViolet+2, kOpenDiamond);
  _style["hef"] = make_pair<int, int>(kOrange+2, kOpenSquare);

  map<string, string> _labels;
  _labels["chf"] = "Charged hadrons";
  _labels["nhf"] = "Neutral hadrons";
  _labels["cef"] = "Charged electromagnetic";
  _labels["nef"] = "Neutral electromagnetic";
  _labels["muf"] = "Muons";
  
  for(const string& h : histonames ) profs[Form("%s",h.c_str())][Form("%s",etabins[i].c_str())] = (TProfile*)filein->Get(Form("hibin_-1.0_0.0/%s/reco jet %s",etabins[i].c_str(),h.c_str()));
  for(const string& h : histonamesdt ) profs[Form("%s",h.c_str())][Form("%s",etabins[i].c_str())] = (TProfile*)filedata->Get(Form("hibin_-1.0_0.0/%s/reco jet %s",etabins[i].c_str(),h.c_str()));


  // Copy TProfiles into TH1Ds

  // First MC in stacked histograms
  auto hcomp = new THStack("hcomp","");
  for(const string& h : histonames )  {
    //    	 nom[etabins[i].c_str()] = asymm[etabins[i].c_str()]->ProjectionX(Form("nom_%s",etabins[i].c_str()));
    // Can one yst use pointers
    //    TH1D &his =  hists[Form("%s",h.c_str())][Form("%s",etabins[i].c_str())];
    hists[Form("%s",h.c_str())][Form("%s",etabins[i].c_str())] = profs[Form("%s",h.c_str())][Form("%s",etabins[i].c_str())]->ProjectionX(Form("proj_%s_%s",h.c_str(),etabins[i].c_str()));

    hists[Form("%s",h.c_str())][Form("%s",etabins[i].c_str())]->SetFillColor(_style[h.c_str()].first-7);
    hists[Form("%s",h.c_str())][Form("%s",etabins[i].c_str())]->SetLineColor(_style[h.c_str()].first+1);
   
    hcomp->Add(hists[Form("%s",h.c_str())][Form("%s",etabins[i].c_str())]);

    //leg->AddEntry(hists[Form("%s",h.c_str())][Form("%s",etabins[i].c_str())], _labels[h.c_str()].c_str());
  }

  // Data with markers
  
  auto hcompdt = new THStack("hcompdt","");
  for(const string& h : histonamesdt )  {
    //    	 nom[etabins[i].c_str()] = asymm[etabins[i].c_str()]->ProjectionX(Form("nom_%s",etabins[i].c_str()));
    // Can one yst use pointers
    //    TH1D &his =  hists[Form("%s",h.c_str())][Form("%s",etabins[i].c_str())];
    histsdt[Form("%s",h.c_str())][Form("%s",etabins[i].c_str())] = profs[Form("%s",h.c_str())][Form("%s",etabins[i].c_str())]->ProjectionX(Form("proj_%s_%s_dt",h.c_str(),etabins[i].c_str()));

    histsdt[Form("%s",h.c_str())][Form("%s",etabins[i].c_str())]->SetMarkerStyle(_style[h.c_str()].second); // TODO: different markers

    histsdt[Form("%s",h.c_str())][Form("%s",etabins[i].c_str())]->SetFillColor(_style[h.c_str()].first-7);
    histsdt[Form("%s",h.c_str())][Form("%s",etabins[i].c_str())]->SetLineColor(_style[h.c_str()].first+1);
   

    hcompdt->Add(histsdt[Form("%s",h.c_str())][Form("%s",etabins[i].c_str())]);

    leg->AddEntry(histsdt[Form("%s",h.c_str())][Form("%s",etabins[i].c_str())], _labels[h.c_str()].c_str());

    //    leg->AddEntry(hists[Form("%s",h.c_str())][Form("%s",etabins[i].c_str())], _labels[h.c_str()].c_str());
  }


  TCanvas *c1 = new TCanvas("c1","c1",600,600);
  hcomp->SetMaximum(1);

  hcomp->Draw("H");
  hcompdt->Draw("sameP");
    
  hcomp->GetXaxis()->SetTitle("p_{T}^{jet} (GeV)");
  hcomp->GetYaxis()->SetTitle("PF energy fraction");
  hcomp->SetTitle(plottitle.c_str()); 

  auto latex = new TLatex;
   latex->SetNDC();
   latex->SetTextSize(0.03);
   latex->DrawLatex(.15,.85,Form("|#eta| < 1.3"));
 

  leg->Draw("same");
  gPad->SetLogx();
  gPad->RedrawAxis();
  
  c1->Print(Form("plots/composition_%s_%s.pdf",etabins[i].c_str(),plottag.c_str()));
  c1->Print(Form("plots/composition_%s_%s.png",etabins[i].c_str(),plottag.c_str()));

}
