#-- Auto-run code in different configurations

#-- SETTINGS
#---------------------------------------
ANALYSIS=true #Run analysis code
YIELDS=true #Run yield code

years=(Run2) #Select data-taking years
# years=(2016 2017 2018) #Select data-taking years

regions=(SR_HighVPt SR_LowVPt CRee_HighVPt CRee_LowVPt CRmm_HighVPt CRmm_LowVPt) #Select regions (= event categories)

#---------------------------------------

make

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
