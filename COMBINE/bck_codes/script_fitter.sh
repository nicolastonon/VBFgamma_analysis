# //--------------------------------------------

#Check first argument
#if [[ $1 == "" ]] ; then
#    #echo "Missing argument ! Usage: [./script_EFTFit.sh ../datacards/datacard.txt]"
#    echo "Missing argument ! Usage: [./script_EFTFit.sh ctz]"
#    exit
#fi

# //--------------------------------------------
# TMP CMD

#python EFTFitter.py --exp -m bestfit -d ./EFTWorkspace.root --freeze --exp -P $1

# //--------------------------------------------
# BESTFITS

#python EFTFitter.py -d ./EFTWorkspace_ctz.root --freeze --exp -P ctz -m bestfit -n ctzExp
#python EFTFitter.py -d ./EFTWorkspace_ctz.root --freeze -P ctz -m bestfit -n ctzObs

#python EFTFitter.py -d ./EFTWorkspace_ctw.root --freeze --exp -P ctw -m bestfit -n ctwExp
#python EFTFitter.py -d ./EFTWorkspace_ctw.root --freeze -P ctw -m bestfit -n ctwObs

#python EFTFitter.py -d ./EFTWorkspace_cpq3.root --freeze --exp -P cpq3 -m bestfit -n cpq3Exp
#python EFTFitter.py -d ./EFTWorkspace_cpq3.root --freeze -P cpq3 -m bestfit -n cpq3Obs

#python EFTFitter.py -d ./EFTWorkspace_SM.root --freeze --exp -P cpqm -m bestfit -n cpqmExp
#python EFTFitter.py -d ./EFTWorkspace_SM.root --freeze -P cpqm -m bestfit -n cpqmObs

#python EFTFitter.py -d ./EFTWorkspace_SM.root --freeze --exp -P cpt -m bestfit -n cptExp
#python EFTFitter.py -d ./EFTWorkspace_SM.root --freeze -P cpt -m bestfit -n cptObs

#python EFTFitter.py -d ./EFTWorkspace_5D.root --freeze --exp -P ctz ctw cpq3 cpqm cpt -m bestfit -n 5DExp
#python EFTFitter.py -d ./EFTWorkspace_5D.root --freeze -P ctz ctw cpq3 cpqm cpt -m bestfit -n 5DObs

# //--------------------------------------------
# 1D SCANS
# (NB: should be done after bestfit, otherwise scan file gets overwritten(

#python EFTFitter.py -d ./EFTWorkspace_ctz.root --freeze --exp -P ctz -m scan -n ctzExp
#python EFTFitter.py -d ./EFTWorkspace_ctz.root --freeze -P ctz -m scan -n ctzObs

#python EFTFitter.py -d ./EFTWorkspace_ctw.root --freeze --exp -P ctw -m scan -n ctwExp
#python EFTFitter.py -d ./EFTWorkspace_ctw.root --freeze -P ctw -m scan -n ctwObs

#python EFTFitter.py -d ./EFTWorkspace_cpq3.root --freeze --exp -P cpq3 -m scan -n cpq3Exp
#python EFTFitter.py -d ./EFTWorkspace_cpq3.root --freeze -P cpq3 -m scan -n cpq3Obs

#python EFTFitter.py -d ./EFTWorkspace_SM.root --freeze --exp -P cpqm -m scan -n cpqmExp
#python EFTFitter.py -d ./EFTWorkspace_SM.root --freeze -P cpqm -m scan -n cpqmObs

#python EFTFitter.py -d ./EFTWorkspace_SM.root --freeze --exp -P cpt -m scan -n cptExp
#python EFTFitter.py -d ./EFTWorkspace_SM.root --freeze -P cpt -m scan -n cptObs

# //--------------------------------------------
# 2D SCANS (obs. only)

#python EFTFitter.py -d ./EFTWorkspace_5D.root --freeze -P ctz ctw -m scan -n ctzctwObs --batch #NB: need to freeze jes group ?
#python EFTFitter.py -d ./EFTWorkspace_5D.root --freeze -P cpqm cpt -m scan -n cpqmcptObs --batch

#python EFTFitter.py -d ./EFTWorkspace_5D.root --freeze -P ctz ctw -m scan -n ctzctwObs --collect
#python EFTFitter.py -d ./EFTWorkspace_5D.root --freeze -P cpqm cpt -m scan -n cpqmcptObs --collect

# //--------------------------------------------
# STAT/SYST SCAN+PLOT

#python EFTFitter.py -d ./EFTWorkspace_ctz.root --freeze --exp -P ctz -m statsyst -n ctzExp -points 40
#python EFTFitter.py -d ./EFTWorkspace_ctw.root --freeze --exp -P ctw -m statsyst -n ctwExp -points 40
#python EFTFitter.py -d ./EFTWorkspace_cpq3.root --freeze --exp -P cpq3 -m statsyst -n cpq3Exp -points 40
#python EFTFitter.py -d ./EFTWorkspace_SM.root --freeze --exp -P cpqm -m statsyst -n cpqmExp -points 40
#python EFTFitter.py -d ./EFTWorkspace_SM.root --freeze --exp -P cpt -m statsyst -n cptExp -points 40

#python EFTFitter.py -d ./EFTWorkspace_ctz.root --freeze -P ctz -m statsyst -n ctzObs -points 40
#python EFTFitter.py -d ./EFTWorkspace_ctw.root --freeze -P ctw -m statsyst -n ctwObs -points 40
#python EFTFitter.py -d ./EFTWorkspace_cpq3.root --freeze -P cpq3 -m statsyst -n cpq3Obs -points 40
#python EFTFitter.py -d ./EFTWorkspace_SM.root --freeze -P cpqm -m statsyst -n cpqmObs -points 40
#python EFTFitter.py -d ./EFTWorkspace_SM.root --freeze -P cpt -m statsyst -n cptObs -points 40

# //--------------------------------------------
# Comparison with Zpt results

#python EFTFitter.py -d ./EFTWorkspace_Zpt.root --freeze --exp -P ctz -m bestfit
#python EFTFitter.py -d ./EFTWorkspace_Zpt.root --freeze --exp -P ctw -m bestfit
#python EFTFitter.py -d ./EFTWorkspace_Zpt.root --freeze --exp -P cpq3 -m bestfit
#python EFTFitter.py -d ./EFTWorkspace_Zpt.root --freeze --exp -P cpqm -m bestfit
#python EFTFitter.py -d ./EFTWorkspace_Zpt.root --freeze --exp -P cpt -m bestfit

#python EFTFitter.py -d ./EFTWorkspace_Zpt.root --freeze -P ctz -m bestfit
#python EFTFitter.py -d ./EFTWorkspace_Zpt.root --freeze -P ctw -m bestfit
#python EFTFitter.py -d ./EFTWorkspace_Zpt.root --freeze -P cpq3 -m bestfit
#python EFTFitter.py -d ./EFTWorkspace_Zpt.root --freeze -P cpqm -m bestfit
#python EFTFitter.py -d ./EFTWorkspace_Zpt.root --freeze -P cpt -m bestfit

#python EFTFitter.py -d ./EFTWorkspace_Zpt.root --exp -P ctz ctw cpq3 cpqm cpt -m bestfit
#python EFTFitter.py -d ./EFTWorkspace_Zpt.root -P ctz ctw cpq3 cpqm cpt -m bestfit
# //--------------------------------------------

# //--------------------------------------------
# BESTFITS (LINEAR-ONLY)

#python EFTFitter.py -d ./EFTWorkspace_ctz_linear.root --freeze --exp -P ctz -m bestfit -n ctzExpLinear --onlylinear
#python EFTFitter.py -d ./EFTWorkspace_ctz_linear.root --freeze -P ctz -m bestfit -n ctzObsLinear --onlylinear

#python EFTFitter.py -d ./EFTWorkspace_ctw_linear.root --freeze --exp -P ctw -m bestfit -n ctwExpLinear --onlylinear
#python EFTFitter.py -d ./EFTWorkspace_ctw_linear.root --freeze -P ctw -m bestfit -n ctwObsLinear --onlylinear

#python EFTFitter.py -d ./EFTWorkspace_cpq3_linear.root --freeze --exp -P cpq3 -m bestfit -n cpq3ExpLinear --onlylinear
#python EFTFitter.py -d ./EFTWorkspace_cpq3_linear.root --freeze -P cpq3 -m bestfit -n cpq3ObsLinear --onlylinear

#python EFTFitter.py -d ./EFTWorkspace_SM_linear.root --freeze --exp -P cpqm -m bestfit -n cpqmExpLinear --onlylinear
#python EFTFitter.py -d ./EFTWorkspace_SM_linear.root --freeze -P cpqm -m bestfit -n cpqmObsLinear --onlylinear

#python EFTFitter.py -d ./EFTWorkspace_SM_linear.root --freeze --exp -P cpt -m bestfit -n cptExpLinear --onlylinear
#python EFTFitter.py -d ./EFTWorkspace_SM_linear.root --freeze -P cpt -m bestfit -n cptObsLinear --onlylinear
#//--------------------------------------------
