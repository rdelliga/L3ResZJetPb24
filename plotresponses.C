
//void plotresponses( string input = "RERUNALL/L2residuals_pbpbreco_rerunall_HP_PFTRIG_nojetid_a3.root", string plottag = "PFTRIG_nojetid") {
//void plotresponses( string input = "RERUNALL/L2residuals_pbpbreco_rerunall_HP_PFTRIG_jetid_a3.root", string plottag = "PFTRIG_jetid") {
//void plotresponses( string input = "RERUNALL/L2residuals_pbpbreco_rerunall_HP_CALOTRIG_nojetid_a3.root", string plottag = "CALOTRIG_nojetid") {
//void plotresponses( string input = "RERUNALL/L2residuals_pbpbreco_rerunall_HP_CALOTRIG_jetid_a3.root", string plottag = "CALOTRIG_jetid") {
void plotresponses( string input = "RERUNALL/L2residuals_pbpbreco_rerunall_zerobias_nojetid.root", string plottag = "ZB_nojetid") {

  float h1 = -1.0, h2 = 0.0, eta1 = -5.2, eta2 = 5.2;
  
  float rmax = 1.2, rmin = 0.75; // For initial checks
  // float rmax = 1.07, rmin = 0.93; // For closure to get better zoom

  float etalimit[] = {3.0, 3.0, 2.964-0.08, 2.5, 1.93+0.12};// for shady box, do manually before figuring out something smart
  bool drawbox = false;
  // Limit eta-range
  
  gStyle->SetOptStat(0);

 
  //string input = "L2residuals_pbpbreco_from3Dlxplus_alpha03_closure_ptlims.root ";   // This contains both data and mc and the ratio -> should one
  //string input = "L2residuals_ppreco_from3Dlxplus_alpha02.root";   // This contains both data and mc and the ratio -> should one
 
  TFile *file = new TFile(input.c_str(),"READ");

  float alpha = 0.3;
  int pts[] = {15, 25, 55, 80, 120, 170, 1000}; // Temporary

  for (int i = 0; i < 6; ++i) {
    int pt1 = pts[i];
    int pt2= pts[i+1];
    string namelabel = Form("pbpb_%dto%dalpha%.1f_%s",pt1,pt2,alpha,plottag.c_str());

    auto mc = (TH1D*)file->Get(Form("mc_pt%dto%d_alpha%.1f",pt1,pt2,alpha));
    auto data = (TH1D*)file->Get(Form("dt_pt%dto%d_alpha%.1f",pt1,pt2,alpha));

    //  mc->GetXaxis()->SetRangeUser(0,2.5);
    //  data->GetXaxis()->SetRangeUser(0,2.5);
    
    auto leg = new TLegend(0.57,0.4,0.85,0.5); //  x, y, x, y
    leg->SetTextSize(0.03);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    leg->AddEntry(data, "Data");
    leg->AddEntry(mc, "MC");

    TCanvas *c1 = new TCanvas("c1","c1",800,600);

    mc->GetXaxis()->SetTitle("|#eta_{probe}|");
    auto rp = new TRatioPlot(mc, data);
    
    rp->GetLowYaxis()->SetNdivisions(505);
   
    rp->Draw();
    rp->GetLowerRefYaxis()->SetTitle("MC/Data");
    rp->GetLowerRefXaxis()->SetTitle("probe jet #eta");
    
    rp->GetLowerRefGraph()->SetMaximum(rmax);
    rp->GetLowerRefGraph()->SetMinimum(rmin);
  
    leg->Draw("same");

    // How do we find coordinate for shade?
    auto box = new TBox((etalimit[i]/5.19), 0.09, 0.9, 0.93);
    box->SetLineColor(kRed);
    box->SetFillColorAlpha(kBlack, 0.2);
    if (drawbox) box->Draw("same");

    auto txt = new TLatex();
    txt->SetTextSize(0.03);
    txt->DrawLatex( 0.2, 0.4, Form("%d < p_{T} < %d",pt1,pt2));
    txt->DrawLatex( 0.2, 0.45, Form("#alpha < %.1f",alpha));
  
    c1->Print(Form("response_eta_%.1f_%.1f_%s.pdf",eta1,eta2,namelabel.c_str()));
 
  }
}
