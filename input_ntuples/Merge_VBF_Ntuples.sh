#by Nicolas Tonon @DESY

# //--------------------------------------------
# //--------------------------------------------

#-- USER OPTIONS
years=(2016 2017 2018) #Select data-taking years #(2016) <-> 2016 only #(2016 2017 2018) <-> all 3 years
processData=false #Hadd DATA samples
processMC=false #Hadd MC samples
mergeBySample=false #Merge samples
mergeBySampleGroup=false #Merge sample groups
haddOptions="-k -O" #'-f' <-> overwrite existing target file; '-k' <-> do not exist on corrupt/missing input '-O' <-> reoptimize TTree compression; '-f6' <-> set maximal compression level (conflicts with -O ?) #NB: do NOT parallelize with 'j N' (mess with lumiWeight vector, etc.)
removeTempPerSampleMergedFiles=false #true <-> delete temporary 'merged_xxx' per-process files (after they have been hadded by sample groups)

#-- Samples to process
dataStr="SinglePhoton|DoubleEG|DoubleMuon|EGamma" #What keywords should be considered as DATA
tmpStr="" #If not "", will only consider the subset of MC samples containing the corresponding keyword

#-- Input files
# ntupleDir="/eos/user/n/ntonon/condor_outputs/july20new/xxx"
ntupleDir="/eos/cms/store/group/phys_smp/vbfA/skimmed_ntuples/july20new/8oct21"

#-- Where to write merged files
outDir="/eos/cms/store/group/phys_smp/vbfA/merged_ntuples"

# //--------------------------------------------
# //--------------------------------------------

mv_cp='cp -n' #By default, copy (duplicate) single merged files
if [ "$removeTempPerSampleMergedFiles" = true ]; then mv_cp='mv'; fi #Move (rename) single merged files

echo "mkdir $outDir"
mkdir $outDir

if [ "$mergeBySample" = true ]; then

    # --- MC ---
    if [ "$processMC" = true ]; then

        # rm /tmp/tempMC.txt
        for yearname in ${years[@]}
        do
            if [ ! -d "$ntupleDir/$yearname" ]; then #If the input directory does not contain the subdir for a given year, skip it
                continue
            fi

            echo "Directory name :" $yearname
	        echo "mkdir $outDir/$yearname"
            mkdir $outDir/$yearname

            cd $ntupleDir/$yearname #Go to year-specific subdir
            liMC=$(ls | awk -F "_$yearname" '{ print $1 }' | awk '!a[$0]++' | egrep -v "$dataStr" | egrep -e "$tmpStr") #Define what a MC sample string looks like #List subdirs in current dir; only keep first keyword before yearname; only keep first occurence of sample name; make sure it's not a data sample #NB: need double-quotes for awk variable (single quote can't interpolate=expand)
            #echo $liMC; continue #Printout

            for line in $liMC #For each subdir
            do
              echo ''; echo 'Line:' $line; echo ''
              rootfiles=$(ls $line*/*.root | tr '\n' ' ') #Rootfiles to be hadded for current year/sample (replace newlines by space)
              #rootfiles=$(ls $line*/*.root | tr " " "\n" | sed 's/$/ \\/'); rootfiles=${rootfiles::-2}
              #echo $rootfiles

              echo "hadd $haddOptions $outDir/$yearname/merged_$line.root $rootfiles" > /tmp/ScriptMerging_$yearname_$line.sh #Merge all rootfiles matching pattern
              chmod 755 /tmp/ScriptMerging_$yearname_$line.sh #Make script executable
              /tmp/ScriptMerging_$yearname_$line.sh #Run merging script
              #echo "Script: /tmp/ScriptMerging_$yearname_$line.sh"; cat /tmp/ScriptMerging_$yearname_$line.sh
              rm /tmp/ScriptMerging_$yearname_$line.sh #Remove script
            done
        done
    fi

    # --- DATA ---
    if [ "$processData" = true ]; then

        # rm /tmp/tempDATA.txt
        for yearname in ${years[@]}
        do
            if [ ! -d "$ntupleDir/$yearname" ]; then #If the input directory does not contain the subdir for a given year, skip it
                continue
            fi

            echo "Directory name :" $yearname
	        echo "mkdir $outDir/$yearname"
            mkdir $outDir/$yearname

            cd $ntupleDir/$yearname #Go to year-specific subdir
            liDATA=$(ls | awk -F "_$yearname" '{ print $1 }' | awk '!a[$0]++' | egrep -e "$dataStr") #Define what a DATA sample string looks like #List subdirs in current dir; only keep first keyword before yearname; only keep first occurence of sample name; make sure it's a data sample #NB: need double-quotes for awk variable (single quote can't interpolate=expand)

            for line in $liDATA #For each subdir
            do
              echo ''; echo 'SAMPLE:' $line; echo ''
              rootfiles=$(ls $line*/*.root | tr '\n' ' ') #Rootfiles to be hadded for current year/sample (replace newlines by space)
              #echo $rootfiles

              echo "hadd $haddOptions $outDir/$yearname/merged_$line.root $rootfiles" > /tmp/ScriptMerging_$yearname_$line.sh #Merge all rootfiles matching pattern
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
            if [ ! -d "$ntupleDir/$yearname" ]; then #If the input directory does not contain the subdir for a given year, skip it
                continue
            fi

            #-- Store grouped-merged ntuples in dedicated subdir
            groupedDir=$outDir/$yearname/merged
            echo "Directory name : $groupedDir"
            echo "mkdir $groupedDir"
            mkdir $groupedDir

            # //--------------------------------------------
            #- Merge sample groups

            if [ "$yearname" = 2016 ]; then
                #-- GJets NLO sample for 2016 //High-Stat. samples
                hadd $haddOptions $groupedDir/GJets.root $outDir/$yearname/merged_GJets_Pt-5To50*.root $outDir/$yearname/merged_GJets_Pt-50To100*.root $outDir/$yearname/merged_GJets_Pt-100To250*.root $outDir/$yearname/merged_GJets_Pt-250To400*.root $outDir/$yearname/merged_GJets_Pt-400To650*.root $outDir/$yearname/merged_GJets_Pt-650ToInf*.root
                if [ "$removeTempPerSampleMergedFiles" = true ]; then rm $outDir/$yearname/merged_GJets_Pt-5To50*.root $outDir/$yearname/merged_GJets_Pt-50To100*.root $outDir/$yearname/merged_GJets_Pt-100To250*.root $outDir/$yearname/merged_GJets_Pt-250To400*.root $outDir/$yearname/merged_GJets_Pt-400To650*.root $outDir/$yearname/merged_GJets_Pt-650ToInf*.root; fi

                #-- GJets NLO sample for 2016 //Different binning, lower stat.
                hadd $haddOptions $groupedDir/GJetsLowStat.root $outDir/$yearname/merged_GJets_Pt-20To100*.root $outDir/$yearname/merged_GJets_Pt-100To200*.root $outDir/$yearname/merged_GJets_Pt-200To500*.root $outDir/$yearname/merged_GJets_Pt-500To1000*.root $outDir/$yearname/merged_GJets_Pt-1000To2000*.root $outDir/$yearname/merged_GJets_Pt-2000To5000*.root
                if [ "$removeTempPerSampleMergedFiles" = true ]; then rm $outDir/$yearname/merged_GJets_Pt-20To100*.root $outDir/$yearname/merged_GJets_Pt-100To200*.root $outDir/$yearname/merged_GJets_Pt-200To500*.root $outDir/$yearname/merged_GJets_Pt-500To1000*.root $outDir/$yearname/merged_GJets_Pt-1000To2000*.root $outDir/$yearname/merged_GJets_Pt-2000To5000*.root; fi
            fi

            #-- GJets #LO HT-binned samples for 2017/18
            hadd $groupedDir/GJetsLO.root $outDir/$yearname/merged_GJets_HT*_13TeV-madgraphMLM-pythia8.root
            if [ "$removeTempPerSampleMergedFiles" = true ]; then rm $outDir/$yearname/merged_GJets_HT*_13TeV-madgraphMLM-pythia8.root; fi

            #-- DYJetsLOJetBins = LO jet-binned #USED
            hadd $haddOptions $groupedDir/DYJetsLOJetBins.root $outDir/$yearname/merged_DY1JetsToLL* $outDir/$yearname/merged_DY2JetsToLL* $outDir/$yearname/merged_DY3JetsToLL* $outDir/$yearname/merged_DY4JetsToLL*
            if [ "$removeTempPerSampleMergedFiles" = true ]; then rm $outDir/$yearname/merged_DY1JetsToLL* $outDir/$yearname/merged_DY2JetsToLL* $outDir/$yearname/merged_DY3JetsToLL* $outDir/$yearname/merged_DY4JetsToLL*; fi

            #-- DYJetsJetBins = NLO jet-binned #Matches 'DYToLL_nJ' (2016) & 'DYJetsToLL_nJ' (2017/18) #USED
            hadd $haddOptions $groupedDir/DYJetsNLOJetBins.root $outDir/$yearname/merged_DY*ToLL_*J*_13TeV-amcatnloFXFX-pythia8.root
            if [ "$removeTempPerSampleMergedFiles" = true ]; then rm $outDir/$yearname/$outDir/$yearname/merged_DY*ToLL_*J*_13TeV-amcatnloFXFX-pythia8.root; fi

            #-- DYJetsLO (HT-binned) #Not used
            #hadd $haddOptions $groupedDir/DYJetsLO.root $outDir/$yearname/merged_DYJetsToLL_M-50_HT*.root
            #if [ "$removeTempPerSampleMergedFiles" = true ]; then rm $outDir/$yearname/$outDir/$yearname/merged_DYJetsToLL_M-50_HT*.root; fi

            #-- DiPhoton
            hadd $haddOptions $groupedDir/DiPhoton.root $outDir/$yearname/merged_DiPhotonJetsBox*.root
            if [ "$removeTempPerSampleMergedFiles" = true ]; then rm $outDir/$yearname/merged_DiPhotonJetsBox*.root; fi

            #-- WJetsToLNu
            hadd $haddOptions $groupedDir/WJetsToLNu.root $outDir/$yearname/merged_WJetsToLNu_Pt*_*_13TeV-amcatnloFXFX-pythia8.root
            if [ "$removeTempPerSampleMergedFiles" = true ]; then rm $outDir/$yearname/merged_WJetsToLNu_Pt*_*_13TeV-amcatnloFXFX-pythia8.root; fi

            #-- WJetsToQQ
            hadd $haddOptions $groupedDir/WJetsToQQ.root $outDir/$yearname/merged_WJetsToQQ_HT*_qc19_3j_*_13TeV-madgraphMLM-pythia8.root
            if [ "$removeTempPerSampleMergedFiles" = true ]; then rm $outDir/$yearname/merged_WJetsToQQ_HT*_qc19_3j_*_13TeV-madgraphMLM-pythia8.root; fi

            #-- QCD
            hadd $haddOptions $groupedDir/QCD.root $outDir/$yearname/merged_QCD_Pt*_EMEnriched_*_13TeV_pythia8.root
            if [ "$removeTempPerSampleMergedFiles" = true ]; then rm $outDir/$yearname/merged_QCD_Pt*_EMEnriched_*_13TeV_pythia8.root; fi

            #-- LLJJPythia #USED
            if [ "$yearname" = 2016 ]
            then
                hadd $haddOptions $groupedDir/LLJJ.root $outDir/$yearname/merged_*LLJJ_MLL-50_MJJ-120_13TeV-madgraph-pythia8.root $outDir/$yearname/merged_*LLJJ_INT_SM_5f_LO*.root
                if [ "$removeTempPerSampleMergedFiles" = true ]; then rm $outDir/$yearname/merged_*EWK_LLJJ_MLL-50_MJJ-120_13TeV-madgraph-pythia8.root $outDir/$yearname/merged_*LLJJ_INT_SM_5f_LO*.root; fi
            else #2017/18
                hadd $haddOptions $groupedDir/LLJJ.root $outDir/$yearname/merged_*LLJJ_MLL-50_MJJ-120_TuneCP5*.root $outDir/$yearname/merged_*LLJJ_INT_SM_5f_LO*.root
                if [ "$removeTempPerSampleMergedFiles" = true ]; then rm $outDir/$yearname/merged_*LLJJ_MLL-50_MJJ-120_TuneCP5*.root $outDir/$yearname/merged_*LLJJ_INT_SM_5f_LO*.root; fi
            fi

            #-- LLJJherwig #Not used (no ee events)
            #hadd $haddOptions $groupedDir/LLJJ_herwig.root $outDir/$yearname/merged_*LLJJ*herwig*.root
            #if [ "$removeTempPerSampleMergedFiles" = true ]; then rm $outDir/$yearname/merged_*LLJJ*herwig*.root; fi

            #-- TTTo2L2Nu
            if [ "$yearname" = 2017 ] #Combine with PSweights sample
            then
                hadd $haddOptions $groupedDir/ttbar.root $outDir/$yearname/merged_TTTo2L2Nu*.root
                if [ "$removeTempPerSampleMergedFiles" = true ]; then rm $outDir/$yearname/merged_TTTo2L2Nu*.root; fi
            else #Single sample
                 $mv_cp $outDir/$yearname/merged_TTTo2L2Nu*.root $groupedDir/ttbar.root
            fi

            #-- TTGJets
            if [ "$yearname" = 2016 ] #Combine with ext
            then
                hadd $haddOptions $groupedDir/ttGJets.root $outDir/$yearname/merged_TTGJets*.root
                if [ "$removeTempPerSampleMergedFiles" = true ]; then rm $outDir/$yearname/merged_TTGJets*.root; fi
            else #Single sample
                 $mv_cp $outDir/$yearname/merged_TTGJets*.root $groupedDir/ttGJets.root
            fi

            #//--------------------------------------------
            #//--------------------------------------------
            #-- Rename #NB: wildcard works for cp/mv as long as there is a single match #NB: replace 'mv' with 'cp' to copy instead of replacing (and '-n' avoid overwriting existing files)

            #-- Signal
            $mv_cp $outDir/$yearname/merged_GJets_SM_5f_*_EWK_13TeV-madgraph-herwigpp.root $groupedDir/VBFgamma.root
            # mv_cp $outDir/$yearname/merged_AJJ_EWK_*_13TeV_amcatnlo-pythia8.root $groupedDir/VBFgamma.root

            #-- Backgrounds
            $mv_cp $outDir/$yearname/merged_ZGTo2LG_PtG-130_*_13TeV-amcatnloFXFX-pythia8.root $groupedDir/ZGTo2LG.root
            $mv_cp $outDir/$yearname/merged_WGToLNuG_*_13TeV-madgraphMLM-pythia8.root $groupedDir/WGToLNuG.root
            $mv_cp $outDir/$yearname/merged_DYJetsToLL_M-50_*_13TeV-amcatnloFXFX-pythia8.root $groupedDir/DYJetsNLO.root #USED

            #if [ "$yearname" = 2016 ]
            #then
            #    $mv_cp $outDir/$yearname/merged_DYJetsToLL_M-50_TuneCUETP8M1_13TeV-madgraphMLM-pythia8.root $groupedDir/DYJetsLOInclusive.root #2016 #Specify full string to not match HT samples
            #else
            #    $mv_cp $outDir/$yearname/merged_DYJetsToLL_M-50_TuneCP5_13TeV-madgraphMLM-pythia8.root $groupedDir/DYJetsLOInclusive.root #2017/18 #Specify full string to not match HT samples
            #fi

        done
    fi
    #//--------------------------------------------

    # --- DATA ---
    if [ "$processData" = true ]; then

        for yearname in ${years[@]}
        do
            if [ ! -d "$ntupleDir/$yearname" ]; then #If the input directory does not contain the subdir for a given year, skip it
                continue
            fi

            #-- Store grouped-merged ntuples in dedicated subdir
            groupedDir=$outDir/$yearname/merged
            echo "Directory name : $groupedDir"
            echo "mkdir $groupedDir"
            mkdir $groupedDir

            if [ "$yearname" = 2018 ]; then
                hadd $haddOptions $groupedDir/DATA.root $outDir/$yearname/*EGamma* $outDir/$yearname/*DoubleMuon*
            else
                hadd $haddOptions $groupedDir/DATA.root $outDir/$yearname/*SinglePhoton* $outDir/$yearname/*DoubleEG* $outDir/$yearname/*DoubleMuon*
            fi
            if [ "$removeTempPerSampleMergedFiles" = true ]; then
                rm $outDir/$yearname/*SinglePhoton*.root
                rm $outDir/$yearname/*DoubleEG*.root
                rm $outDir/$yearname/*DoubleMuon*.root
                rm $outDir/$yearname/*EGamma*.root
            fi

        done
    fi
fi
