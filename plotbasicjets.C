// Plot basic jet properties
// Plot diff recos and data vs. mc

void plotbasicjets(float h1 = -1.0, float h2 = 0.0, float eta1 = -5.2, float eta2 = 5.2) {

  gStyle->SetOptStat(0);

  // string intputMC = "HIJEC_results/pbpbreco_MC_lxplus.root";
  // string inputData = "HIJEC_results/pbpbreco_DATA_lxplus.root";

  /* string legtxtMC = "MC";
  string legtxtData = "data";
  string namelabel = "recosMCvsdata"; */

  /*string intputMC = "HIJEC_results/ppreco_DATA_lxplus.root";
  string inputData = "HIJEC_results/pbpbreco_DATA_lxplus.root";

  string legtxtMC = "pp reco, data";
  string legtxtData = "PbPb reco, data";
  string namelabel = "recosdata"; */

  string intputMC = "HIJEC_results/ppreco_MC_lxplus.root";
  string inputData = "HIJEC_results/pbpbreco_MC_lxplus.root";

  string legtxtMC = "pp reco, MC";
  string legtxtData = "PbPb reco, MC";
  string namelabel = "recosMC";
  
  
  TFile *filedt = new TFile(inputData.c_str(),"READ");
  TFile *filemc = new TFile(intputMC.c_str(),"READ");

  // Get correct directory
  auto dirdt =  (TDirectory*)filedt->Get(Form("hibin_%.1f_%.1f/eta_%.1f_%.1f",h1,h2,eta1,eta2));
  auto dirmc =  (TDirectory*)filemc->Get(Form("hibin_%.1f_%.1f/eta_%.1f_%.1f",h1,h2,eta1,eta2));

  map<string,TH1D*> data, mc;

  data["pt"] = (TH1D*)dirdt->Get("reco jet pT");
  data["eta"] = (TH1D*)dirdt->Get("reco jet eta");
  data["phi"] = (TH1D*)dirdt->Get("reco jet phi");

  mc["pt"] = (TH1D*)dirmc->Get("reco jet pT");
  mc["eta"] = (TH1D*)dirmc->Get("reco jet eta");
  mc["phi"] = (TH1D*)dirmc->Get("reco jet phi");

  data["pt"]->GetXaxis()->SetRangeUser(20,800);
  mc["pt"]->GetXaxis()->SetRangeUser(20,800);

  auto leg = new TLegend(0.57,0.8,0.85,0.9); //  x, y, x, y
  leg->SetTextSize(0.03);
  leg->SetBorderSize(0);
  leg->SetFillStyle(0);
   
  leg->AddEntry(data["pt"],legtxtData.c_str());
  leg->AddEntry(mc["pt"],legtxtMC.c_str());

  
  vector<string> plots = {"pt", "eta", "phi"};

  unsigned int vecSize = plots.size();
  for (unsigned int i = 0; i < vecSize; i++)  {

    TCanvas *c1 = new TCanvas("c1","c1",600,600);
      
    // TODO: scale properly
    data[plots[i]]->Scale(1./data[plots[i]]->Integral(),"width");
    data[plots[i]]->SetTitle("");
  // data[plots[i]]->Scale(0.1); // TODO: to this properly

    mc[plots[i]]->Scale(1./mc[plots[i]]->Integral(),"width");
    mc[plots[i]]->SetTitle("");
    //mc[plots[i]]->Scale(0.1);


    // todo: draw MC has histogram
    mc[plots[i]]->SetLineColor(kRed);
 
    auto rp = new TRatioPlot(mc[plots[i]], data[plots[i]]);
    rp->SetH1DrawOpt("E");
    c1->SetTicks(0, 1);
    if (i == 0) c1->SetLogy();
    
    rp->Draw();
    rp->GetLowYaxis()->SetNdivisions(505);
    rp->GetLowerRefYaxis()->SetTitle("pp/PbPb");

    //if (i == 0) rp->GetLowerRefGraph()->SetMaximum(5);

    
    
    c1->Update();

    leg->Draw();

   /*   auto latex = new TLatex;
   latex->SetNDC();
   latex->SetTextSize(0.023);
   latex->DrawLatex(.6,.75,Form("%.0f < p_{T} < %.0f GeV",pt1,pt2)); */

   /*  float ce1=h1/2;
  float ce2=h2/2; 
  latex->DrawLatex(.6,.70,Form("%.0f-%.0f %%",ce1,ce2));  */

   // Print
    c1->Print(Form("plotformeeting/datavsmc_%s_hbin_%.0f_%.0f_eta_%.1f_%.1f_%s.pdf",plots[i].c_str(),h1,h2,eta1,eta2,namelabel.c_str()));
   //   gPad->RedrawAxis();
   //   c1->Print(Form("datavsmc_%s_pt_%.0f_%.0f.png",plots[i].c_str(),pt1,pt2));


  }
  
}
