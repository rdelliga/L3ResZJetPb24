
#include "JERSF_RMS.C"
#include "JERSF_fits.C"
#include "JERSF_fits_vsalpha.C"
#include "JERSF_printtxt.C"


void runJERSF() {

  //  JERSF_RMS();
  ////////// Files in: MC, ZB, HP

  //  RERECOMC_AK4_PFTRIG_ID.root",dir+"RERECO_ZBall_AK4_PFTRIG_ID_l2res.root","L2residuals_pbpbreco_rereco_zb_jetid_l2jecclosure.root
    
      JERSF_RMS( "JERSF_sigmas_RMS.root", "/home/laura/Code/jec/HIJEC_rereco_results_redoMCtruth/RERECOMC_AK4_PFTRIG_ID_forjer.root", "/home/laura/Code/jec/HIJEC_rereco_results_redoMCtruth/RERECO_ZBall_AK4_PFTRIG_ID_l2res_forjer.root", "/home/laura/Code/jec/HIJEC_rereco_results_redoMCtruth/RERECOHP_AK4_PFTRIG_ID_l2res_forjer.root");

      JERSF_fits( "JERSF_sigmas_fits.root", "/home/laura/Code/jec/HIJEC_rereco_results_redoMCtruth/RERECOMC_AK4_PFTRIG_ID_forjer.root", "/home/laura/Code/jec/HIJEC_rereco_results_redoMCtruth/RERECO_ZBall_AK4_PFTRIG_ID_l2res_forjer.root", "/home/laura/Code/jec/HIJEC_rereco_results_redoMCtruth/RERECOHP_AK4_PFTRIG_ID_l2res_forjer.root");
  
  //    JERSF_fits( "JERSF_sigmas_fits.root", "/home/laura/Code/jec/HIJEC_rereco_results/RERECOMC_AK4_PFTRIG_jetid_l2jecclosure_forjer.root", "/home/laura/Code/jec/HIJEC_rereco_results/RERECO_ZB_AK4_PFTRIG_jetid_l2closure_forjer.root", "/home/laura/Code/jec/HIJEC_rereco_results/RERECOHP_AK4_PFTRIG_jetid_l2jecclosure_forjer.root");


    ////////////// Fit for the SF
    JERSF_fits_vsalpha("JERSF_sigmas_fits.root", "JERSFs_fromfits.root", false);
    //   JERSF_fits_vsalpha("JERSF_sigmas_RMS.root", "JERSFs_fromRMS.root", true);

    ////////////// Produce txts
    //    JERSF_printtxt("JERSFs_fromfits.root");
    JERSF_printtxt("JERSFs_fromRMS.root");

}
