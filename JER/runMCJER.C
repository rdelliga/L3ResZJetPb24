#include "MCJER.C"
#include "doTxtMCJER.C"

void runMCJER() {

  MCJER("../../../HIJEC_rereco_results/RERECOMC_AK4_PFTRIG_nojetid.root", 15, "MCJER", "testingMCjer-rereco.root");
  doTxtMCJER("testingMCjer-rereco.root", "textingMCjerparams.txt");
}
