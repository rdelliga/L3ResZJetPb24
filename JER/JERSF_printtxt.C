

void JERSF_printtxt(string filein = "JERSFs_fromfits.root")  {

  auto file = new TFile(filein.c_str(),"READ");
  auto SFs = (TH1D*)file->Get("SF");
  string header = "{ 2 JetEta JetPt 0 None ScaleFactor }";
  string filename = "JERSF.txt";
  float minpt = 0, maxpt = 7000;
  
  cout << header.c_str() << endl;

  ofstream txtfile;
  txtfile.open(filename.c_str());

  std::cout << "Producing txt file " << filename.c_str() << " with header " << header.c_str() << std::endl;
  txtfile << header.c_str() << std::endl;

  
  // Neg eta
  for (int i = SFs->GetXaxis()->GetNbins()-1; i > 1 ; i--) {
    cout << -SFs->GetBinLowEdge(i) << " " <<  (i == 1 ? -SFs->GetBinLowEdge(i-1) : SFs->GetBinLowEdge(i-1))  << " " << minpt << " " << maxpt << " 3 " << SFs->GetBinContent(i-1) << " "  << SFs->GetBinContent(i-1) << " "  << SFs->GetBinContent(i-1) << endl;
    txtfile << -SFs->GetBinLowEdge(i) << " " <<  (i == 1 ? -SFs->GetBinLowEdge(i-1) : SFs->GetBinLowEdge(i-1))  << " " << minpt << " " << maxpt << " 3 " << SFs->GetBinContent(i-1) << " "  << SFs->GetBinContent(i-1) << " "  << SFs->GetBinContent(i-1) << endl;
  }

   for (int i = 1; i < SFs->GetXaxis()->GetNbins()-1; ++i) {
    cout << SFs->GetBinLowEdge(i) << " " <<  SFs->GetBinLowEdge(i+1)  << " " << minpt << " " << maxpt << " 3 " << SFs->GetBinContent(i) << " "  << SFs->GetBinContent(i) << " "  << SFs->GetBinContent(i) << endl;
    txtfile << SFs->GetBinLowEdge(i) << " " <<  SFs->GetBinLowEdge(i+1)  << " " << minpt << " " << maxpt << " 3 " << SFs->GetBinContent(i) << " "  << SFs->GetBinContent(i) << " "  << SFs->GetBinContent(i) << endl;
  }
 
}
