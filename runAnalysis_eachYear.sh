#-- Auto-run code in different configurations

#-- SETTINGS
#---------------------------------------
ANALYSIS=true #Run analysis code
YIELDS=false #Run yield code

#-- Select data-taking years
# years=(Run2) #Full Run2
# years=(2016 2017 2018) #Each individual year
# years=(2016 2017 2018 Run2) #Each individual year + full Run2 #NB: use only for plotting (whereas for creating histograms, simply processing 'Run2' will also produce all per-year histos) !
years=(2016)

#-- Select regions (= event categories)
regions=(SR_HighVPt SR_LowVPt CRee_HighVPt CRee_LowVPt CRmm_HighVPt CRmm_LowVPt)
# regions=(CRmm_LowVPt)

#---------------------------------------

make

# if [ "${#years[@]}" = 3 ]; then
#     years=(2016 2017 2018 Run2)
# fi
#---------------------------------------

#-- ANALYSIS

if [ "$ANALYSIS" = true ]; then
    for yearname in ${years[@]}
    do
        for region in ${regions[@]}
        do
            echo "RUNNING: [./analysis_main.exe $yearname $region]"
            ./analysis_main.exe $yearname $region
        done

    done
fi

#---------------------------------------
#-- YIELDS

if [ "$YIELDS" = true ]; then
    for yearname in ${years[@]}
    do
        for region in ${regions[@]}
        do
            echo "RUNNING: [./Yield_Table.exe $yearname $region]"
            ./Yield_Table.exe $yearname $region
        done

    done
fi
#---------------------------------------
