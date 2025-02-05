

void JERSF_printtxt(string filein = "JERSFs_fromfits.root")  {

  auto file = new TFile(filein.c_str(),"READ");
  auto SFs = (TH1D*)file->Get("SF");
  cout << " { 2 JetEta JetPt 0 None ScaleFactor } " << endl;

  // Neg eta
  for (int i = 1; i < SFs->GetXaxis()->GetNbins()-1; ++i) {
    cout << SFs->GetBinLowEdge(i) << " " <<  SFs->GetBinLowEdge(i+1)  << " 0 1000 3 " << SFs->GetBinContent(i) << " "  << SFs->GetBinContent(i) << " "  << SFs->GetBinContent(i) << endl;
  }
 
}
