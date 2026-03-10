# bools: isMC, jetID, iszb, l2res, JER SF, fillforJER


#root -l -q 'analyse.cc("RERECOHP", "AK4_PFTRIG_ID", false, true, false, false, false, false)'

#root -l -q 'analyse.cc("RERECOHP", "AK4_PFTRIG_ID_l2res", false, true, false, true, false, false)'
#root -l -q 'analyse.cc("RERECOHP", "AK4_PFTRIG_ID_l2res_forjer", false, true, false, true, false, true)'

root -l -q 'analyse.cc("RERECOMC", "AK4_PFTRIG_ID_ppJERSF", true, true, false, false, false, false)'
root -l -q 'analyse.cc("RERECOMC", "AK4_PFTRIG_ID_forjer_ppJERSF", true, true, false, false, false, true)'
#root -l -q 'analyse.cc("RERECOMC", "AK4_PFTRIG_ID_jerclosure", true, true, false, false, true, false)'



#for VARIABLE in $(seq 0 19)
#do
#    root -l -q 'analyse.cc("RERECOZB'${VARIABLE}'", "AK4_PFTRIG_ID", false, true, true, false, false, false)'
#    root -l -q 'analyse.cc("RERECOZB'${VARIABLE}'", "AK4_PFTRIG_ID_l2res", false, true, true, true, false, false)'
#    root -l -q 'analyse.cc("RERECOZB'${VARIABLE}'", "AK4_PFTRIG_ID_l2res_forjer", false, true, true, true, false, true)'
#done



