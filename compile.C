

void compile() {

  // For local JER
  //  gROOT->ProcessLine(".L CondFormats/JetMETObjects/src/JetResolutionObject.cc+");
  //  gROOT->ProcessLine(".L JetMETCorrections/Modules/src/JetResolution.cc+");
 
  gROOT->ProcessLine(".L histograms.C+");
  gROOT->ProcessLine(".L eventhistograms.C+");
  gROOT->ProcessLine(".q");

}
