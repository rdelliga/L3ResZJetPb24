

void  run(string in = "testfiles/testtupleMC.root", string out = "test.root", bool ismc = 1) {
  //void  run() {
  // gROOT->ProcessLine(".L histograms.C+");
  // gROOT->ProcessLine(".L eventhistograms.C+");

 
  // gSystem->Load("histograms_C.so");
 //  gROOT->ProcessLine(".x analyse.cc());

 gROOT->ProcessLine(".L analyse.cc");

 gROOT->ProcessLine(Form("analyse(\"%s\",\"%s\",%i)", in.c_str(), out.c_str(), ismc));

}
