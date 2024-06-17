#include "eventhistograms.h"

eventhistograms::eventhistograms(TDirectory *dir, bool ismc) {

  TDirectory *curdir = gDirectory;
  bool enter = dir->cd();
  assert(enter);
  this->dir = dir;
  
  this->isMC = ismc;
  
// Event quality histograms
  event_vz = new TH1D("vz", "vz;;", 100, -20, 20);
  event_hiBin = new TH1D("hiBin", "hiBin;;", 120, 0, 120);

  
// Weights etc?

  if (isMC)  event_pthatwsgenweight = new TH2D("pthatvsgenwt","",200,0,1500,1000,0,0.1);
  //  curdir->cd(); // Not sure if this is needed...
  
}


void eventhistograms::Write() {
  dir->cd();
  dir->Write();
  }
