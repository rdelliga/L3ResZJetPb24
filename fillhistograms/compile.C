

void compile() {

  gROOT->ProcessLine(".L histograms.C+");
  gROOT->ProcessLine(".L eventhistograms.C+");
  gROOT->ProcessLine(".q");

}
