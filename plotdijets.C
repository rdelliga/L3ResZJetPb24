
void plotdijets() {


  gStyle->SetOptStat(0);

  TFile *filein = new TFile("L2residuals_pbpbreco.root","READ");
  string plottag = "pbpbreco";
  
  //vector<string> etabins = {"eta_-5.2_-3.9", "eta_-3.9_-2.6", "eta_-2.6_-1.3", "eta_-1.3_0.0", "eta_0.0_1.3", "eta_1.3_2.6", "eta_2.6_3.9", "eta_3.9_5.2"};
  //vector<string> label = {"-5.2 < #eta < -3.9", "-3.9 < #eta < -2.6", "-2.6< #eta < -1.3","-1.3 < #eta < 0", "0 < #eta < 1.3", "1.3 < #eta < 2.6", "2.6 < #eta < 3.9", "3.9 < #eta < 5.2"};

  vector<string> etabins = {"eta_0.0_1.3", "eta_1.3_2.6", "eta_2.6_3.9", "eta_3.9_5.2"};
  vector<string> label = {"0 < #eta < 1.3", "1.3 < #eta < 2.6", "2.6 < #eta < 3.9", "3.9 < #eta < 5.2"};



  map<string,TH1D*> resps;

   /*  auto leg = new TLegend(0.57,0.8,0.85,0.9); //  x, y, x, y
   leg->SetTextSize(0.03);
   leg->SetBorderSize(0);
   leg->SetFillStyle(0);
   
   
   leg->AddEntry(data["djasymmetry"],"Data");
   leg->AddEntry(mc["djasymmetry"],"MC"); */

  
  TCanvas *c1 = new TCanvas("c1","c1",600,600);

  int minpt = 80;
  TLine* line = new TLine(minpt,1,500,1);
  line->SetLineColor(kBlack);

   
  unsigned int vecSize = etabins.size();
  for (unsigned int i = 0; i < vecSize; i++)  {
    resps[Form("response_%s",etabins[i].c_str())] = (TH1D*)filein->Get(Form("response_%s",etabins[i].c_str()));

    resps[Form("response_%s",etabins[i].c_str())]->GetXaxis()->SetRangeUser(minpt,500);

    resps[Form("response_%s",etabins[i].c_str())]->SetMinimum(0.9);
    resps[Form("response_%s",etabins[i].c_str())]->SetMaximum(1.1);
     
    resps[Form("response_%s",etabins[i].c_str())]->GetXaxis()->SetTitle("p_{T}^{avg}");
 
    /*    mc[plots[i]]->SetTitle("");
	  mc[plots[i]]->GetXaxis()->SetTitle(xaxis[i].c_str());
    // todo: draw MC has histogram
    mc[plots[i]]->SetLineColor(kRed); */
 
    /*  auto rp = new TRatioPlot(mc[plots[i]], data[plots[i]]);
   rp->SetH1DrawOpt("E");
   c1->SetTicks(0, 1);
   rp->Draw();
   rp->GetLowYaxis()->SetNdivisions(505);
   c1->Update(); 

   leg->Draw(); */

   line->SetLineStyle(2);
   resps[Form("response_%s",etabins[i].c_str())]->Draw("");
   line->Draw("same");
   
   resps[Form("response_%s",etabins[i].c_str())]->Draw("same");
  
   
   auto latex = new TLatex;
   latex->SetNDC();
   latex->SetTextSize(0.05);
   latex->DrawLatex(.6,.75,Form("%s",label[i].c_str()));
 

   // Print
   c1->Print(Form("plots/residuals_%s_%s.pdf",etabins[i].c_str(),plottag.c_str()));


  }
  
}
