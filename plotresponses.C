


// can I make this function and just call somehow it several times
void plotresponses() {

  gStyle->SetOptStat(0);

  string input = "L2residuals_pbpbreco_from3Dlpxlus.root";   // This contains both data and mc and the ratio -> should one 


  TFile *file = new TFile(input.c_str(),"READ");


  // Here histograms to get from the file

  auto leg = new TLegend(0.57,0.8,0.85,0.9); //  x, y, x, y
  leg->SetTextSize(0.03);
  leg->SetBorderSize(0);
  leg->SetFillStyle(0);
 

}
