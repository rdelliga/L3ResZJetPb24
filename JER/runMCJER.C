#include "plots.h"
#include "MCJER.C"
#include "MCJPR.C"
#include "doTxtMCJER.C"

void runMCJER() {

  // Note: for the output folders you still need to create folders by hand

  // The first parameter for the MCJER/JPR is the MC output from the histogram filler. It can be from the "forjer" version or the standard one.
  // Second: min
  
  //// Run jet pt resolution
  MCJER("../../../HIJEC_rereco_results_redoMCtruth/RERECOMC_AK4_PFTRIG_ID_forjer.root",  28, "MCJER", "MCJER-rereco.root");
  doTxtMCJER("MCJER-rereco.root", "MCjerparams.txt", false);
  
  //// Jet eta resolution - set second param to 1
  //MCJPR("../../../HIJEC_rereco_results_redoMCtruth/RERECOMC_AK4_PFTRIG_ID_forjer.root", 1, "MCJETAR", "MCJETAR-rereco.root");
  // doTxtMCJER("MCJETAR-rereco.root", "MCjetarparams.txt", true);
  
  //// Jet phi resolution - set second parem to 0
  // MCJPR("../../../HIJEC_rereco_results_redoMCtruth/RERECOMC_AK4_PFTRIG_ID_forjer.root", 0, "MCJPHIR", "MCJPHIR-rereco.root");
  //  doTxtMCJER("MCJPHIR-rereco.root", "MCjphirparams.txt", true);
}
