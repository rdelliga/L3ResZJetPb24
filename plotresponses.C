


// can I make this function and just call somehow it several times
void plotresponses(float h1 = -1.0, float h2 = 0.0, float eta1 = -5.2, float eta2 = 5.2) {

  gStyle->SetOptStat(0);

  string input = "L2residuals_pbpbreco_from3Dlxplus_alpha03_rebin_abs.root";   // This contains both data and mc and the ratio -> should one
  //string input = "L2residuals_ppreco_from3Dlxplus_alpha02.root";   // This contains both data and mc and the ratio -> should one
 
  TFile *file = new TFile(input.c_str(),"READ");

  float alpha = 0.2;
  int pts[] = {40, 55, 80, 120, 170, 1000}; // Temporary

  for (int i = 0; i < 5; ++i) {
  int pt1 = pts[i];
  int pt2= pts[i+1];
  string namelabel = Form("pbpb_%dto%dalpha%.1f",pt1,pt2,alpha);

  auto mc = (TH1D*)file->Get(Form("mc_pt%dto%d_alpha%.1f",pt1,pt2,alpha));
  auto data = (TH1D*)file->Get(Form("dt_pt%dto%d_alpha%.1f",pt1,pt2,alpha));


  // Here histograms to get from the file

  auto leg = new TLegend(0.57,0.4,0.85,0.5); //  x, y, x, y
  leg->SetTextSize(0.03);
  leg->SetBorderSize(0);
  leg->SetFillStyle(0);
  leg->AddEntry(data, "Data");
  leg->AddEntry(mc, "MC");

  TCanvas *c1 = new TCanvas("c1","c1",800,600);

  mc->GetXaxis()->SetTitle("#eta_{probe}");
  auto rp = new TRatioPlot(mc, data);

  rp->GetLowYaxis()->SetNdivisions(505);
  
    
  rp->Draw();
  rp->GetLowerRefYaxis()->SetTitle("MC/Data");
  rp->GetLowerRefXaxis()->SetTitle("probe jet #eta");

  //  ((rp->Draw();
  // TODO: set ranges for plots 1.2, 0.75 ?
  rp->GetLowerRefGraph()->SetMaximum(1.2);
  rp->GetLowerRefGraph()->SetMinimum(0.75);
  
  leg->Draw("same");


  auto txt = new TLatex();
  txt->SetTextSize(0.03);
  txt->DrawLatex( 0.2, 0.4, Form("%d < p_{T} < %d",pt1,pt2));
  txt->DrawLatex( 0.2, 0.45, Form("#alpha < %.1f",alpha));

  
   c1->Print(Form("rebin/response_hbin_%.0f_%.0f_eta_%.1f_%.1f_%s_abs.pdf",h1,h2,eta1,eta2,namelabel.c_str()));
 
  }
}
