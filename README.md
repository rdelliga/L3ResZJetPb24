Repository to run analysis for producind L2 residual jet energy corrections and jet energy resolution scale factor. The inputs for these macros are HiForest ntuples (at the moment for 2023 ppref).
At the moment direct balance method with tag-and-probing a dijet system is used.

## Required steps:
```
cd fillhistograms/jecfiles/
wget https://github.com/cms-jet/JECDatabase/blob/master/jet_veto_maps/Summer23BPixPrompt23/Summer23BPixPrompt23_RunD_v1.root
wget https://indico.cern.ch/event/1592138/contributions/6711482/attachments/3141457/5575822/Spring23_HI_V1_MC_L2Relative_AK4PF.txt
```

## Quick start

-> First: root -l compile.C 
-> 

Then, to run the analysis:

1. analyse.cc fills histograms you need.
Flags:

At the moment this is run with hadded ntuples per dat set (MC, HP0, HP1, HP2, ZB0, etc.)

You can do a test run with simply root -l analyse.cc

-> to run locally: run.sh

1. To fill histograms for L2residuals:

- Code is in directory L2Residual.

-> call this code separarely for different datasets. you need cmsenv to be able to call functions to apply jec (and jer sf). shouldn't really matter which cmssw you use.
-> in settings.h you can manually set which JEC files to use. The JEC (and JER SF) files are always read from the path set up in here.


When you have the output from the former
2. run deriveL2_from3D.C

3. Fit the response ratios vs. alpha: dofits.C

4. plotresponses.C can be used to plot the responses

5. Produce txt files:


JET pT/eta/phi RESOLUTION AND JER SF:

To fill histograms for JER SF (the tag-and-probe conditions are slightly different): turn on flag "" in analyse.cc
-> you need to apply the L2 residual JEC before deriving the SF, remember check that too


When you have the outputs from that, you can run MC-checks:
--- JER/MCJER.C -> pt resolution
--- JER/MCJPR.C -> eta/phi resolition (wip?)
--- JER/MCRESP.C -> this plots the MC response <pT(reco)/pT(gen)>
--- doTxtMCJER.C -> print txt files of resolution fit parameters

For SF there are a couple of scripts:
1. These are alternatives:
--- JERSF_fits.C -> Extracts resoluiton by fitting gaussian to the dijet asymmetry distributions
--- JERSF_RMS.C -> Extracts resolution from trunct RMS of dijet asymmetry distributions
2. Do fits against alpha:
--- JERSF_fits_vsalpha.C -> this needs as an input the output from the previous step
3. Produce txt files
... JERSF_printtxt.C

To look at trigger turn-ons:
(this is has been used to merge results from different datasets/triggers, for 2023 it has been zero bias and hard probes datasets)

Plot things:



