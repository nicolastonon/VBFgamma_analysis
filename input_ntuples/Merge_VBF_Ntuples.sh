#by Nicolas Tonon @DESY

years=(2016 2017 2018) #Select data-taking years
#years=(2016)
processData=true #Hadd DATA samples
processMC=true #Hadd MC samples
mergeBySample=true #Merge samples
mergeBySampleGroup=true #Merge sample groups
force="" #"-f" <-> overwrite existing target files; "" <-> do not
tolerance="" #'If the option -k is used, hadd will not exit on corrupt or non-existant input files but skip the offending files instead'; "" <-> do not; #Warning: you may want missing files to trigger a failure !

ntupleDir="/eos/cms/store/cmst3/group/top/SMP-19-005/july20/SKimmed2_AllSamples_NewBDT" #Directory containing skimmed files
dataStr="SinglePhoton|DoubleEG|DoubleMuon" #What keywords should be considered as DATA

#outDir=$PWD/merged_ntuples
outDir="/afs/cern.ch/work/n/ntonon/public/VBFphoton/CMSSW_10_2_27/src/UserCode/input_ntuples" #Where to write merged files #FIXME
echo "mkdir $outDir"
mkdir $outDir

if [ "$mergeBySample" = true ]; then

    # --- MC ---
    if [ "$processMC" = true ]; then

        # rm /tmp/tempMC.txt
        for yearname in ${years[@]}
        do
            echo "Directory name :" $yearname
	        echo "mkdir $outDir/$yearname"
            mkdir $outDir/$yearname

            cd $ntupleDir/$yearname #Go to year-specific subdir
            liMC=$(ls | awk -F "_$yearname" '{ print $1 }' | awk '!a[$0]++' | egrep -v "$dataStr") #Define what a MC sample string looks like #List subdirs in current dir; only keep first keyword before yearname; only keep first occurence of sample name; make sure it's not a data sample #NB: need double-quotes for awk variable (single quote can't interpolate=expand) #FIXME also need suffix

            for line in $liMC #For each subdir
            do
              echo ''; echo 'Line:' $line; echo ''
              rootfiles=$(ls $line*/* | egrep -v "_f" | tr '\n' ' ') #Rootfiles to be hadded for current year/sample (ignore some empty files, and replace newlines by space)
              #echo $rootfiles

              echo "hadd $force $outDir/$yearname/merged_$line.root $rootfiles" > /tmp/ScriptMerging_$yearname_$line.sh #Merge all rootfiles matching pattern
              chmod 755 /tmp/ScriptMerging_$yearname_$line.sh #Make script executable
              /tmp/ScriptMerging_$yearname_$line.sh #Run merging script
              rm /tmp/ScriptMerging_$yearname_$line.sh #Remove script
            done
        done
    fi

    # --- DATA ---
    if [ "$processData" = true ]; then

        # rm /tmp/tempDATA.txt
        for yearname in ${years[@]}
        do
            echo "Directory name :" $yearname
	        echo "mkdir $outDir/$yearname"
            mkdir $outDir/$yearname

            cd $ntupleDir/$yearname #Go to year-specific subdir
            liDATA=$(ls | awk -F "_$yearname" '{ print $1 }' | awk '!a[$0]++' | egrep -e "$dataStr") #Define what a DATA sample string looks like #List subdirs in current dir; only keep first keyword before yearname; only keep first occurence of sample name; make sure it's a data sample #NB: need double-quotes for awk variable (single quote can't interpolate=expand) #FIXME also need suffix

            for line in $liDATA #For each subdir
            do
              echo ''; echo 'SAMPLE:' $line; echo ''
              rootfiles=$(ls $line*/* | egrep -v "_f" | tr '\n' ' ') #Rootfiles to be hadded for current year/sample (ignore some empty files, and replace newlines by space)
              #echo $rootfiles

              echo "hadd $force $outDir/$yearname/merged_$line.root $rootfiles" > /tmp/ScriptMerging_$yearname_$line.sh #Merge all rootfiles matching pattern
              chmod 755 /tmp/ScriptMerging_$yearname_$line.sh #Make script executable
              /tmp/ScriptMerging_$yearname_$line.sh #Run merging script
              rm /tmp/ScriptMerging_$yearname_$line.sh #Remove script
            done
        done
    fi
fi

# //--------------------------------------------
# //--------------------------------------------

if [ "$mergeBySampleGroup" = true ]; then

    #--- FURTHER HADDS & RENAME NTUPLES ---
    echo ''
    echo ''
    echo ''
    echo '=== FURTHER MERGING AND RENAMING ==='

    # --- MC ---
    if [ "$processMC" = true ]; then

        for yearname in ${years[@]}
        do
            echo "Directory name :" $yearname
	        echo "mkdir $outDir/$yearname"
            mkdir $outDir/$yearname

            # //--------------------------------------------
            #- Merge sample groups

            #-- GJets
            hadd $force $outDir/$yearname/GJets.root $outDir/$yearname/merged_GJets_Pt*.root
            #rm $outDir/$yearname/merged_GJets_Pt*.root

            #-- GJets #Different sample
            hadd $outDir/$yearname/GJets.root $outDir/$yearname/merged_GJets_HT*_13TeV-madgraphMLM-pythia8.root
            #rm $outDir/$yearname/merged_GJets_HT*_13TeV-madgraphMLM-pythia8.root

            #-- DY
            # hadd $force $outDir/$yearname/DY.root $outDir/$yearname/merged_DYJetsToLL_M*.root
            #rm $outDir/$yearname/merged_DYJetsToLL_M*.root

            #-- DiPhoton
            hadd $force $outDir/$yearname/DiPhoton.root $outDir/$yearname/merged_DiPhotonJetsBox*.root
            #rm $outDir/$yearname/merged_DiPhotonJetsBox*.root

            #-- WJetsToLNu
            hadd $force $outDir/$yearname/WJetsToLNu.root $outDir/$yearname/merged_WJetsToLNu_Pt*_*_13TeV-amcatnloFXFX-pythia8.root
            #rm $outDir/$yearname/merged_WJetsToLNu_Pt*_*_13TeV-amcatnloFXFX-pythia8.root

            #-- WJetsToQQ
            hadd $force $outDir/$yearname/WJetsToQQ.root $outDir/$yearname/merged_WJetsToQQ_HT*_qc19_3j_*_13TeV-madgraphMLM-pythia8.root
            #rm $outDir/$yearname/merged_WJetsToQQ_HT*_qc19_3j_*_13TeV-madgraphMLM-pythia8.root

            #-- QCD
            hadd $force $outDir/$yearname/QCD.root $outDir/$yearname/merged_QCD_Pt*_EMEnriched_*_13TeV_pythia8.root
            #rm $outDir/$yearname/merged_QCD_Pt*_EMEnriched_*_13TeV_pythia8.root

            #-- LLJJ
            hadd $force $outDir/$yearname/LLJJ.root $outDir/$yearname/merged_*LLJJ*pythia*.root
            #rm $outDir/$yearname/merged_*LLJJ*pythia*.root


            #//--------------------------------------------
            #-- Rename #NB: wildcard works for cp/mv as long as there is a single match #FIXME mv

            #-- Signal
            # cp -n $outDir/$yearname/merged_AJJ_EWK_*_13TeV_amcatnlo-pythia8.root $outDir/$yearname/VBFgamma.root
            cp -n $outDir/$yearname/merged_GJets_SM_5f_*_EWK_13TeV-madgraph-herwigpp.root $outDir/$yearname/VBFgamma.root

            #-- Backgrounds
            cp -n $outDir/$yearname/merged_TTTo2L2Nu_*_13TeV-powheg-pythia8.root $outDir/$yearname/ttbar.root
            cp -n $outDir/$yearname/merged_TTTo2L2Nu_*_PSweights_13TeV-powheg-pythia8.root $outDir/$yearname/ttbar.root #Prefer PSweights version if available
            cp -n $outDir/$yearname/merged_TTGJets_*_13TeV-amcatnloFXFX-madspin-pythia8.root $outDir/$yearname/ttGJets.root
            cp -n $outDir/$yearname/merged_ZGTo2LG_PtG-130_*_13TeV-amcatnloFXFX-pythia8.root $outDir/$yearname/ZGTo2LG.root
            cp -n $outDir/$yearname/merged_WGToLNuG_*_13TeV-madgraphMLM-pythia8.root $outDir/$yearname/WGToLNuG.root
            cp -n $outDir/$yearname/merged_DYJetsToLL_M-50_*_13TeV-amcatnloFXFX-pythia8.root $outDir/$yearname/DYJetsNLO.root

        done
    fi
    #//--------------------------------------------

    # --- DATA ---
    if [ "$processData" = true ]; then

        for yearname in ${years[@]}
        do
            echo "Directory name :" $yearname
	        echo "mkdir $outDir/$yearname"
            mkdir $outDir/$yearname

            hadd $tolerance $force $outDir/$yearname/DATA.root $outDir/$yearname/*SinglePhoton* $outDir/$yearname/*DoubleEG* $outDir/$yearname/*DoubleMuon* #If the option -k is used, hadd will not exit on corrupt or non-existant input files but skip the offending files instead
            #rm $outDir/$yearname/*SinglePhoton*.root
            #rm $outDir/$yearname/*DoubleEG*.root
            #rm $outDir/$yearname/*DoubleMuon*.root

            if [ "$yearname" = 2018 ]; then
                cp -n $outDir/$yearname/merged_DoubleMuon.root $outDir/$yearname/DATA.root #Single 2018 dataset
            fi

        done
    fi
fi
