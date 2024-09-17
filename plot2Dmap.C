void plot2Dmap(float h1 = -1.0, float h2 = 0.0, float eta1 = -5.2, float eta2 = 5.2) {
  gStyle->SetOptStat(0);
  
  /* string input = "HIJEC_results/pbpbreco_MC_lxplus.root";
  string title = "MC, PbPb reco";
  string namelabel = "PbPbMC"; */

  /*  string input = "HIJEC_results/ppreco_MC_lxplus.root";
  string title = "MC, pp reco";
  string namelabel = "ppMC"; */

  /* string input = "HIJEC_results/pbpbreco_DATA_lxplus.root";
  string title = "Data, PbPb reco";
  string namelabel = "PbPbdata"; */

  string input = "HIJEC_results/ppreco_DATA_lxplus.root";
  string title = "Data, pp reco";
  string namelabel = "ppMC";
 
  // string input = "HIJEC_results/pbpbreco_DATA_lxplus.root"

  TFile *file = new TFile(input.c_str(),"READ");

  auto dir =  (TDirectory*)file->Get(Form("hibin_%.1f_%.1f/eta_%.1f_%.1f",h1,h2,eta1,eta2));
  TCanvas *c1 = new TCanvas("c1","c1",800,600);
   
  TH2D* plane = (TH2D*)dir->Get("eta-phi distribution");
  plane->SetTitle(title.c_str());

  plane->Draw();

  c1->Print(Form("plotformeeting/jetmap_hbin_%.0f_%.0f_eta_%.1f_%.1f_%s.pdf",h1,h2,eta1,eta2,namelabel.c_str()));
}
