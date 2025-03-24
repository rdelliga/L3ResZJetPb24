# bools: isMC, jetID, iszb, l2res, JER
 
root -l -q 'analyse_JER.cc("HP", "AK4_PFTRIG_jetid_l2corr_forjer_wideeta", false, true, false, true, false)'
root -l -q 'analyse_JER.cc("HP", "AK4_PFTRIG_nojetid_l2corr_forjer_wideeta", false, false, false, true, false)'
root -l -q 'analyse_JER.cc("MC", "AK4_PFTRIG_jetid_l2corr_forjer_wideeta", true, true, false, false, false)'
root -l -q 'analyse_JER.cc("MC", "AK4_PFTRIG_nojetid_l2corr_forjer_wideeta", true, false, false, false, false)'

for VARIABLE in $(seq 0 19)
do
    root -l -q 'analyse_JER.cc("zb'${VARIABLE}'", "AK4_PFTRIG_jetid_l2corr_forjer_wideeta", false, true, false, true, false)'
    root -l -q 'analyse_JER.cc("zb'${VARIABLE}'", "AK4_PFTRIG_nojetid_l2corr_forjer_wideeta", false, false, false, true, false)'
done


