/* BASH COLORS */
#define RST   "[0m"
#define KRED  "[31m"
#define KGRN  "[32m"
#define KYEL  "[33m"
#define KBLU  "[34m"
#define KMAG  "[35m"
#define KCYN  "[36m"
#define KWHT  "[37m"
#define FRED(x) KRED x RST
#define FGRN(x) KGRN x RST
#define FYEL(x) KYEL x RST
#define FBLU(x) KBLU x RST
#define FMAG(x) KMAG x RST
#define FCYN(x) KCYN x RST
#define FWHT(x) KWHT x RST
#define BOLD(x) "[1m" x RST
#define UNDL(x) "[4m" x RST

#include <TFile.h>
#include <TH1F.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TLorentzVector.h>
#include "TTree.h"
#include "TString.h"
#include "TColor.h"
#include "TCut.h"
#include "TLegend.h"
#include "TLine.h"
#include "THStack.h"
#include "TString.h"
#include "TLegend.h"

#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <cmath>
#include <sstream>
#include <fstream>

#include <cassert> //Can be used to terminate program if argument is not true //Ex : assert(test > 0 && "Error message");
#include <sys/stat.h> // to be able to use mkdir

using namespace std;


//--------------------------------------------
// ##     ## ######## ##       ########  ######## ########
// ##     ## ##       ##       ##     ## ##       ##     ##
// ##     ## ##       ##       ##     ## ##       ##     ##
// ######### ######   ##       ########  ######   ########
// ##     ## ##       ##       ##        ##       ##   ##
// ##     ## ##       ##       ##        ##       ##    ##
// ##     ## ######## ######## ##        ######## ##     ##
//--------------------------------------------

//Convert a double into a TString
// precision --> can choose if TString how many digits the TString should display
TString Convert_Number_To_TString(double number, int precision/*=3*/)
{
	stringstream ss;
	ss << std::setprecision(precision) << number;
	TString ts = ss.str();
	return ts;
}

//Convert a TString into a float
double Convert_TString_To_Number(TString ts)
{
	double number = 0;
	string s = ts.Data();
	stringstream ss(s);
	ss >> number;
	return number;
}

//Can set here protections : return false if a given syst does not apply to a given sample
bool Is_Syst_Match_Sample(TString syst, TString sample)
{
	// cout<<"syst "<<syst<<endl;
	// cout<<"sample "<<sample<<endl;

    // if(syst == "WZ_HF_extrap" && sample != "WZ") {return false;}

	return true;
}

//Ask user to choose options at command line for script generation
void Choose_Arguments_From_CommandLine(TString& signal)
{
    //-- Choose whether to include shape syst or not
	cout<<endl<<FYEL("--- What is your SIGNAL ?")<<endl;
    cout<<"* 'VBFgamma'   \t<-> Signal is VBFgamma"<<endl;

	cin>>signal;
	while(signal != "VBFgamma" && signal != "vbfgamma")
	{
		cin.clear();
		cin.ignore(1000, '\n');

		cout<<" Wrong answer ! Retry :"<<endl;
		cin>>signal;
	}

	return;
}







//--------------------------------------------
// ##     ##    ###    ##    ## ########    ######## ######## ##     ## ########  ##          ###    ######## ########
// ###   ###   ## ##   ##   ##  ##             ##    ##       ###   ### ##     ## ##         ## ##      ##    ##
// #### ####  ##   ##  ##  ##   ##             ##    ##       #### #### ##     ## ##        ##   ##     ##    ##
// ## ### ## ##     ## #####    ######         ##    ######   ## ### ## ########  ##       ##     ##    ##    ######
// ##     ## ######### ##  ##   ##             ##    ##       ##     ## ##        ##       #########    ##    ##
// ##     ## ##     ## ##   ##  ##             ##    ##       ##     ## ##        ##       ##     ##    ##    ##
// ##     ## ##     ## ##    ## ########       ##    ######## ##     ## ##        ######## ##     ##    ##    ########
//--------------------------------------------

/**
 * Create output text file containing skeleton/template of COMBINE datacard
 * NB : must make sure that arguments given to function (e.g. sample names, syst names, etc.) are in sync with what is in the template !

 * @param outfile_name   output name
 * @param v_samples      list of samples in datacard
 * @param v_isSignal     "0" <-> sample is bkg, "1" <-> sample is signal
 * @param v_sampleUncert uncertainty associated to sample, in % (only for bkg)
 * @param v_normSyst         list of systematics applying to all samples in %(e.g. lumi)
 * @param v_normSystValue    value of these systematics
 * @param v_shapeSyst    list of shape systematics
 */
void Generate_Datacard(vector<TString> v_samples, vector<int> v_isSignal, vector<TString> v_sampleUncert, vector<TString> v_normSyst, vector<TString> v_normSystValue, vector<TString> v_shapeSyst, TString signal, vector<bool> v_shapeSyst_isCorrelYears, bool group_nuisances, vector<TString> v_nuis_groups, vector<TString> v_normSyst_group, vector<TString> v_shapeSyst_group, TString outfile_name="Template_Datacard.txt")
{
    //TString outfile_name = "Template_Datacard.txt";
    ofstream outfile(outfile_name.Data());

	//-- Protections
	if(v_shapeSyst.size() != v_shapeSyst_isCorrelYears.size()) {cout<<"ERROR: incorrect size for vector 'v_shapeSyst_isCorrelYears' !"<<endl; return;}


 // #    # ######   ##   #####  ###### #####
 // #    # #       #  #  #    # #      #    #
 // ###### #####  #    # #    # #####  #    #
 // #    # #      ###### #    # #      #####
 // #    # #      #    # #    # #      #   #
 // #    # ###### #    # #####  ###### #    #
//--------------------------------------------

    //--- Nof observables, bkgs, nuisance params
    outfile<<"imax"<<"\t"<<1<<"\t"<<"number of categories"<<endl;
    outfile<<"jmax"<<"\t"<<"*"<<"\t"<<"number of backgrounds"<<endl;
    outfile<<"kmax"<<"\t"<<"*"<<"\t"<<"number of nuisance parameters (sources of systematic uncertainties)"<<endl;

//--------------------------------------------
    //--- Filepath, naming convention
    outfile<<"---------------------------------------------------"<<endl;
    outfile<<"shapes * [VAR]_[CHAN]_[YEAR] filetoread $CHANNEL__$PROCESS $CHANNEL__$PROCESS__$SYSTEMATIC"<<endl;

//--------------------------------------------
    //--- Var name, get yields from templates
    outfile<<"---------------------------------------------------"<<endl;
    outfile<<"bin        "<<"\t"<<"[VAR]_[CHAN]_[YEAR]"<<endl;
    outfile<<"observation"<<"\t"<<-1<<endl;

//--------------------------------------------
    //--- Processes names & indices
    outfile<<"---------------------------------------------------"<<endl;
    outfile<<"bin    ";
    for(int isample=0; isample<v_samples.size(); isample++)
    {
        outfile<<"\t";

        outfile<<"[VAR]_[CHAN]_[YEAR]";
    }
    outfile<<endl;

    outfile<<"process";
    for(int isample=0; isample<v_samples.size(); isample++)
    {
        outfile<<"\t";
        outfile<<v_samples[isample];
    }
    outfile<<endl;

    outfile<<"process";
    for(int isample=0; isample<v_samples.size(); isample++)
    {
        int index_tmp = isample;
        if(v_isSignal[isample] == 1 && isample > 0) {index_tmp = -isample;}
        else if(v_isSignal[isample] == 0 && isample == 0) {index_tmp = v_isSignal.size();}
        else if(v_isSignal[isample] != 0 && v_isSignal[isample] != 1) {cout<<"Error ! Wrong value of v_isSignal !"<<endl; return;}

        outfile<<"\t";
        outfile<<index_tmp;
    }
    outfile<<endl;

    outfile<<"rate ";
    for(int isample=0; isample<v_samples.size(); isample++)
    {
        int index_tmp = isample;
        if(v_isSignal[isample] == 1 && isample > 0) {index_tmp = -isample;}
        else if(v_isSignal[isample] != 0 && v_isSignal[isample] != 1) {cout<<"Error ! Wrong value of v_isSignal !"<<endl; return;}

        outfile<<"\t";
        outfile<<-1;
    }
    outfile<<endl;


 //                      #     #
 // #       ####   ####  ##    #     ####  #   #  ####  #####
 // #      #    # #    # # #   #    #       # #  #        #
 // #      #    # #      #  #  #     ####    #    ####    #
 // #      #    # #  ### #   # #         #   #        #   #
 // #      #    # #    # #    ##    #    #   #   #    #   #
 // ######  ####   ####  #     #     ####    #    ####    #

  //   ##   #      #
  //  #  #  #      #
  // #    # #      #
  // ###### #      #
  // #    # #      #
  // #    # ###### ######
//--------------------------------------------

//--- Systematics applied to all processes (e.g. lumi)
    outfile<<"---------------------------------------------------"<<endl;
    for(int isyst=0; isyst<v_normSyst.size(); isyst++)
    {
        //-- Year-specific markers
        if(v_normSyst[isyst].EndsWith("161718")) {} //Special case: no need for year-specific marker (shall apply to all years)
        else if(v_normSyst[isyst].EndsWith("1617")) {outfile<<"[201617]";}
        else if(v_normSyst[isyst].EndsWith("1718")) {outfile<<"[201718]";}
        else if(v_normSyst[isyst].EndsWith("1618")) {outfile<<"[201618]";}
        else if(v_normSyst[isyst].EndsWith("16")) {outfile<<"[2016]";}
        else if(v_normSyst[isyst].EndsWith("17")) {outfile<<"[2017]";}
        else if(v_normSyst[isyst].EndsWith("18")) {outfile<<"[2018]";}

        outfile<<v_normSyst[isyst];
        // if(!v_normSyst_isCorrelYears[isyst]) {outfile<<"[YEAR]";} //Uncorrelated for each year

        outfile<<"\t"<<"lnN";

        for(int isample=0; isample<v_samples.size(); isample++)
        {
			// cout<<"sample "<<v_samples[isample]<<endl;
			// cout<<"syst "<<v_normSyst[isyst]<<endl;

            outfile<<"\t";

            if(Is_Syst_Match_Sample(v_normSyst[isyst], v_samples[isample])) //Other syst : check if applies to current samples (from name)
			{
                //-- Hard-coded special cases: e.g. if a lN syst. is correlated between years with different values, use a marker replaced with year-specific values by parsing code
                if(v_normSyst[isyst] == "Lumi1617") {outfile<<"[Lumi1617]";}
                else if(v_normSyst[isyst] == "Lumi1718") {outfile<<"[Lumi1718]";}
                else if(v_normSyst[isyst] == "Lumi161718") {outfile<<"[Lumi161718]";} //LumiXY

				else {outfile<<v_normSystValue[isyst];} //Normal cases
			}
			else {outfile<<"-";}
        }
        outfile<<endl;
    }

//                      #     #
// #       ####   ####  ##    #     ####  #   #  ####  #####
// #      #    # #    # # #   #    #       # #  #        #
// #      #    # #      #  #  #     ####    #    ####    #
// #      #    # #  ### #   # #         #   #        #   #
// #      #    # #    # #    ##    #    #   #   #    #   #
// ######  ####   ####  #     #     ####    #    ####    #

 //  ####  # #    #  ####  #      ######
 // #      # ##   # #    # #      #
 //  ####  # # #  # #      #      #####
 //      # # #  # # #  ### #      #
 // #    # # #   ## #    # #      #
  // ####  # #    #  ####  ###### ######
//--------------------------------------------

    vector<TString> v_rateParam; //Store names of samples for which we want to use a rate parameter (specified at end of datacard)

    //--- lnN Systematics applied to only 1 process (e.g. background uncert.)
    outfile<<"---------------------------------------------------"<<endl;
    for(int isample=0; isample<v_samples.size(); isample++)
    {
        if(v_isSignal[isample] == 1 && v_sampleUncert[isample] == "-1") {continue;} //Signal without norm. uncertainty
		else if(v_isSignal[isample] == 0 && v_sampleUncert[isample] == "-1") //Rate parameter for background --> Don't apply lnN uncertainty
        {
            v_rateParam.push_back(v_samples[isample]);
            continue;
        }

		// cout<<"Sample "<<v_samples[isample]<<" / Uncert = "<<v_sampleUncert[isample]<<endl;

        outfile<<v_samples[isample] + "_rate"<<"\t"<<"lnN"; //Create lnN rate parameter for current process

        for(int jsample=0; jsample<v_samples.size(); jsample++) //Apply it to current process only
        {
            outfile<<"\t";

			if(isample == jsample) //Apply to this process
            {
                if(v_sampleUncert[jsample].Contains("/")) //Ex: '-0.04/0.06' -> 4% down and 6% up
                {
                    //Break TString in 2 based on delimited '/'
                    TObjArray* keys = v_sampleUncert[jsample].Tokenize("/");
                    // keys->Print();
                    outfile<<1-abs(Convert_TString_To_Number(((TObjString *)(keys->At(0)))->String())/100.);
                    outfile<<"/";
                    outfile<<1+abs(Convert_TString_To_Number(((TObjString *)(keys->At(1)))->String())/100.);
                }
                else {outfile<<1.+(Convert_TString_To_Number(v_sampleUncert[jsample])/100.);} //Ex: '50' <-> 50% -> '1.5'
            }
            else {outfile<<"-";} //Don't apply to other processes
        }
        outfile<<endl;
    }


 //  ####  #    #   ##   #####  ######
 // #      #    #  #  #  #    # #
 //  ####  ###### #    # #    # #####
 //      # #    # ###### #####  #
 // #    # #    # #    # #      #
  // ####  #    # #    # #      ######

//--------------------------------------------

    //--- Shape systematics
    outfile<<"---------------------------------------------------"<<endl;
    for(int isyst=0; isyst<v_shapeSyst.size(); isyst++)
    {
		//-- Markers at beginning of line :
        //- [SHAPE] can be used later to easily disactivate all shape systs, at parsing
        //- [201617] can be used to disactivate the prefiring syst for 2018, etc.
        outfile<<"[SHAPE]";
        if(v_shapeSyst[isyst].EndsWith("1617") || v_shapeSyst[isyst].BeginsWith("prefir")) {outfile<<"[201617]";} //Hardcoded: prefire for 16/17 only
        else if(v_shapeSyst[isyst].EndsWith("1718")) {outfile<<"[201718]";}
        else if(v_shapeSyst[isyst].EndsWith("1618")) {outfile<<"[201618]";}
        else if(v_shapeSyst[isyst].EndsWith("16")) {outfile<<"[2016]";}
        else if(v_shapeSyst[isyst].EndsWith("17")) {outfile<<"[2017]";}
        else if(v_shapeSyst[isyst].EndsWith("18")) {outfile<<"[2018]";}

        outfile<<v_shapeSyst[isyst];
        if(!v_shapeSyst_isCorrelYears[isyst]) {outfile<<"[YEAR]";} //Uncorrelated for different year --> Modify systematic name itself
        outfile<<"\t"<<"shape";

        for(int isample=0; isample<v_samples.size(); isample++)
        {
            outfile<<"\t";

            if (Is_Syst_Match_Sample(v_shapeSyst[isyst], v_samples[isample])) {outfile<<"1";}
			else {outfile<<"-";}
        }
        outfile<<endl;
    }


 // #####    ##   ##### ######    #####    ##   #####    ##   #    #
 // #    #  #  #    #   #         #    #  #  #  #    #  #  #  ##  ##
 // #    # #    #   #   #####     #    # #    # #    # #    # # ## #
 // #####  ######   #   #         #####  ###### #####  ###### #    #
 // #   #  #    #   #   #         #      #    # #   #  #    # #    #
 // #    # #    #   #   ######    #      #    # #    # #    # #    #

//--------------------------------------------
//Modify normalization of any process from the datacard (e.g. FCNC)
//See : https://cms-hcomb.gitbook.io/combine/setting-up-the-analysis/preparing-the-datacard#rate-parameters
    outfile<<"---------------------------------------------------"<<endl;
	// outfile<<"[ratePar]rate_modif"<<"\t"<<"rateParam"<<"\t"<<"[VAR]_[CHAN]_[YEAR]"<<"\t"<<"sigPar"<<"\t"<<"rateVal";

    //-- May use rate parameters for some samples (specified at end of datacard)
    for(int isample=0; isample<v_rateParam.size(); isample++)
    {
        outfile<<"rate_"+v_rateParam[isample]<<"\t"<<"rateParam"<<"\t"<<"*"<<"\t"<<v_rateParam[isample]<<"\t"<<"1.0 [0.0,3.0]"<<endl;
    }


 //  ####  #####   ##   #####
 // #        #    #  #    #
 //  ####    #   #    #   #
 //      #   #   ######   #   ###
 // #    #   #   #    #   #   ###
 //  ####    #   #    #   #   ###

//--------------------------------------------
//--- MC statistical uncert.
//See : https://cms-hcomb.gitbooks.io/combine/content/part2/bin-wise-stats.html#usage-instructions
// Usage : [channel] autoMCStats [threshold] [include-signal = 0] [hist-mode = 1]
// [threshold] : A positive value sets the threshold on the effective number of unweighted events above which the uncertainty will be modeled with the Barlow-Beeston-lite approach described above. Below the threshold an individual uncertainty per-process will be created.

    outfile<<"---------------------------------------------------"<<endl;

    outfile<<"[STAT]"<<"\t"<<"*"<<"\t"<<"autoMCStats"<<"\t"<<"10"<<endl;


 //  ####  ##### #    # ###### #####
 // #    #   #   #    # #      #    #
 // #    #   #   ###### #####  #    #
 // #    #   #   #    # #      #####
 // #    #   #   #    # #      #   #
 //  ####    #   #    # ###### #    #

//--------------------------------------------
//-- Additional parameters

    outfile<<"---------------------------------------------------"<<endl;

    //-- Groups of nuisances (see doc: https://cms-analysis.github.io/HiggsAnalysis-CombinedLimit/part2/settinguptheanalysis/#groups-of-nuisances)
    for(int igroup=0; igroup<v_nuis_groups.size(); igroup++)
    {
        if(v_nuis_groups[igroup] == "xxx") {outfile<<v_nuis_groups[igroup]<<" group = xxx"<<endl;}
        else
        {
            TString ts_freeze = ""; //List of systs to freeze in current group
            for(int isyst=0; isyst<v_shapeSyst.size(); isyst++)
            {
                if(v_shapeSyst_group[isyst] == v_nuis_groups[igroup])
                {
                    ts_freeze+= " " + v_shapeSyst[isyst];
                    if(!v_shapeSyst_isCorrelYears[isyst]) {ts_freeze+= "[YEAR]";}
                }
            }
            for(int isyst=0; isyst<v_normSyst.size(); isyst++)
            {
                if(v_normSyst_group[isyst] == v_nuis_groups[igroup]) {ts_freeze+= " " + v_normSyst[isyst];}
            }
            if(v_nuis_groups[igroup] == "theory") {outfile<<"[THEORY]";}
            outfile<<v_nuis_groups[igroup]<<" group = "<<ts_freeze<<endl;
        }
    }

//--------------------------------------------

    cout<<endl<<endl<<"---> File ./"<<outfile_name<<" created..."<<endl<<endl<<endl;

    return;
}








//--------------------------------------------
//--------------------------------------------
//--------------------------------------------
//--------------------------------------------
//--------------------------------------------
//--------------------------------------------
//--------------------------------------------
//--------------------------------------------
//--------------------------------------------
//--------------------------------------------
//--------------------------------------------










//--------------------------------------------
// ##     ##    ###    #### ##    ##
// ###   ###   ## ##    ##  ###   ##
// #### ####  ##   ##   ##  ####  ##
// ## ### ## ##     ##  ##  ## ## ##
// ##     ## #########  ##  ##  ####
// ##     ## ##     ##  ##  ##   ###
// ##     ## ##     ## #### ##    ##
//--------------------------------------------

//Define all arguments needed by generator function (see function description for details about args)
int main()
{
    bool group_nuisances = false; //true <-> define nuisance groups (hardcoded) at the end of datacard (so that multiple nuisances can be frozen at once)

//-- Read command line arguments
//--------------------------------------------
    TString signal = "VBFgamma";
    // Choose_Arguments_From_CommandLine(signal);


//  ####    ##   #    # #####  #      ######  ####
// #       #  #  ##  ## #    # #      #      #
//  ####  #    # # ## # #    # #      #####   ####
//      # ###### #    # #####  #      #           #
// #    # #    # #    # #      #      #      #    #
//  ####  #    # #    # #      ###### ######  ####

//-- Sample / 0 = sig, 1 = bkg / -1 = rateParam, else = lnN uncertainty (possibly 2-sided, e.g. "-4.2/+8" <-> 0.96/1.08)
//--------------------------------------------
    vector<TString> v_samples; vector<int> v_isSignal; vector<TString> v_sampleUncert;
    if(signal == "VBFgamma") //Signal : VBFgamma (no norm. uncert.)
    {
        v_samples.push_back("VBFgamma"); v_isSignal.push_back(1); v_sampleUncert.push_back("-1");
    }
//--------------------------------------------
    v_samples.push_back("tX"); v_isSignal.push_back(0); v_sampleUncert.push_back("20");


 //               #     #
 // #      #    # ##    #     ####  #   #  ####  #####
 // #      ##   # # #   #    #       # #  #        #
 // #      # #  # #  #  #     ####    #    ####    #
 // #      #  # # #   # #         #   #        #   #
 // #      #   ## #    ##    #    #   #   #    #   #
 // ###### #    # #     #     ####    #    ####    #

//-- lnN systematics
//Write "(1+X)%". E.g for lnN symmetric of 10% => "1.10"
//For a 5%/10% lnN asymmetric syst, write : "1.05/1.10"
//-1 <-> values must be hardcoded (to allow for correlations with different values per year)
//--------------------------------------------
    vector<TString> v_normSyst; vector<TString> v_normSystValue; vector<TString> v_normSyst_group;
    v_normSyst.push_back("Lumi16"); v_normSystValue.push_back("1.01"); v_normSyst_group.push_back("other_exp"); //Was: 1.022
    v_normSyst.push_back("Lumi17"); v_normSystValue.push_back("1.02"); v_normSyst_group.push_back("other_exp");
    v_normSyst.push_back("Lumi18"); v_normSystValue.push_back("1.015"); v_normSyst_group.push_back("other_exp");
    v_normSyst.push_back("Lumi1718"); v_normSystValue.push_back("-1"); v_normSyst_group.push_back("other_exp"); //Value defined in parser code
    v_normSyst.push_back("Lumi161718"); v_normSystValue.push_back("-1"); v_normSyst_group.push_back("other_exp"); //Was: LumiXY //Value defined in parser code

    v_normSyst.push_back("Trigger16"); v_normSystValue.push_back("1.02"); v_normSyst_group.push_back("other_exp");
    v_normSyst.push_back("Trigger17"); v_normSystValue.push_back("1.02"); v_normSyst_group.push_back("other_exp");
    v_normSyst.push_back("Trigger18"); v_normSystValue.push_back("1.02"); v_normSyst_group.push_back("other_exp");


//  ####  #    #   ##   #####  ######     ####  #   #  ####  #####
// #      #    #  #  #  #    # #         #       # #  #        #
//  ####  ###### #    # #    # #####      ####    #    ####    #
//      # #    # ###### #####  #              #   #        #   #
// #    # #    # #    # #      #         #    #   #   #    #   #
//  ####  #    # #    # #      ######     ####    #    ####    #

//-- Name of shape systematic / Whether syst is correlated between years (single nuisance) or not (1 per year) / Nuisance group to which the syst belongs
//--------------------------------------------
    vector<TString> v_shapeSyst; vector<bool> v_shapeSyst_isCorrelYears; vector<TString> v_shapeSyst_group;
    // v_shapeSyst.push_back("PU"); v_shapeSyst_isCorrelYears.push_back(true); v_shapeSyst_group.push_back("other_exp");
    // v_shapeSyst.push_back("prefire"); v_shapeSyst_isCorrelYears.push_back(true); v_shapeSyst_group.push_back("other_exp");
    // v_shapeSyst.push_back("BtagHF"); v_shapeSyst_isCorrelYears.push_back(true); v_shapeSyst_group.push_back("btag");
    // v_shapeSyst.push_back("BtagLF"); v_shapeSyst_isCorrelYears.push_back(true); v_shapeSyst_group.push_back("btag");
    // v_shapeSyst.push_back("BtagHFstats1"); v_shapeSyst_isCorrelYears.push_back(false); v_shapeSyst_group.push_back("btag");
    // v_shapeSyst.push_back("BtagHFstats2"); v_shapeSyst_isCorrelYears.push_back(false); v_shapeSyst_group.push_back("btag");
    // v_shapeSyst.push_back("BtagLFstats1"); v_shapeSyst_isCorrelYears.push_back(false); v_shapeSyst_group.push_back("btag");
    // v_shapeSyst.push_back("BtagLFstats2"); v_shapeSyst_isCorrelYears.push_back(false); v_shapeSyst_group.push_back("btag");
    // v_shapeSyst.push_back("BtagCFerr1"); v_shapeSyst_isCorrelYears.push_back(true); v_shapeSyst_group.push_back("btag");
    // v_shapeSyst.push_back("BtagCFerr2"); v_shapeSyst_isCorrelYears.push_back(true); v_shapeSyst_group.push_back("btag");

    //-- JEC //Default: single source uncorrelated between years
    bool use_split_JEC = false;
    if(!use_split_JEC) //Total JEC
    {
        // v_shapeSyst.push_back("JES"); v_shapeSyst_isCorrelYears.push_back(false); v_shapeSyst_group.push_back("jes");
    }
    else //Split JEC
    {
        v_shapeSyst.push_back("AbsoluteStat"); v_shapeSyst_isCorrelYears.push_back(false); v_shapeSyst_group.push_back("jes");
        v_shapeSyst.push_back("AbsoluteScale"); v_shapeSyst_isCorrelYears.push_back(true); v_shapeSyst_group.push_back("jes");
        v_shapeSyst.push_back("AbsoluteMPFBias"); v_shapeSyst_isCorrelYears.push_back(true); v_shapeSyst_group.push_back("jes");
        v_shapeSyst.push_back("Fragmentation"); v_shapeSyst_isCorrelYears.push_back(true); v_shapeSyst_group.push_back("jes");
        v_shapeSyst.push_back("SinglePionECAL"); v_shapeSyst_isCorrelYears.push_back(true); v_shapeSyst_group.push_back("jes");
        v_shapeSyst.push_back("SinglePionHCAL"); v_shapeSyst_isCorrelYears.push_back(true); v_shapeSyst_group.push_back("jes");
        v_shapeSyst.push_back("FlavorQCD"); v_shapeSyst_isCorrelYears.push_back(true); v_shapeSyst_group.push_back("jes");
        v_shapeSyst.push_back("TimePtEta"); v_shapeSyst_isCorrelYears.push_back(false); v_shapeSyst_group.push_back("jes");
        v_shapeSyst.push_back("RelativeJEREC1"); v_shapeSyst_isCorrelYears.push_back(false); v_shapeSyst_group.push_back("jes");
        v_shapeSyst.push_back("RelativeJEREC2"); v_shapeSyst_isCorrelYears.push_back(false); v_shapeSyst_group.push_back("jes");
        v_shapeSyst.push_back("RelativeJERHF"); v_shapeSyst_isCorrelYears.push_back(true); v_shapeSyst_group.push_back("jes");
        v_shapeSyst.push_back("RelativePtBB"); v_shapeSyst_isCorrelYears.push_back(true); v_shapeSyst_group.push_back("jes");
        v_shapeSyst.push_back("RelativePtEC1"); v_shapeSyst_isCorrelYears.push_back(false); v_shapeSyst_group.push_back("jes");
        v_shapeSyst.push_back("RelativePtEC2"); v_shapeSyst_isCorrelYears.push_back(false); v_shapeSyst_group.push_back("jes");
        v_shapeSyst.push_back("RelativePtHF"); v_shapeSyst_isCorrelYears.push_back(true); v_shapeSyst_group.push_back("jes");
        v_shapeSyst.push_back("RelativeBal"); v_shapeSyst_isCorrelYears.push_back(true); v_shapeSyst_group.push_back("jes");
        v_shapeSyst.push_back("RelativeSample"); v_shapeSyst_isCorrelYears.push_back(false); v_shapeSyst_group.push_back("jes");
        v_shapeSyst.push_back("RelativeFSR"); v_shapeSyst_isCorrelYears.push_back(true); v_shapeSyst_group.push_back("jes");
        v_shapeSyst.push_back("RelativeStatFSR"); v_shapeSyst_isCorrelYears.push_back(false); v_shapeSyst_group.push_back("jes");
        v_shapeSyst.push_back("RelativeStatEC"); v_shapeSyst_isCorrelYears.push_back(false); v_shapeSyst_group.push_back("jes");
        v_shapeSyst.push_back("RelativeStatHF"); v_shapeSyst_isCorrelYears.push_back(false); v_shapeSyst_group.push_back("jes");
        v_shapeSyst.push_back("PileUpDataMC"); v_shapeSyst_isCorrelYears.push_back(true); v_shapeSyst_group.push_back("jes");
        v_shapeSyst.push_back("PileUpPtRef"); v_shapeSyst_isCorrelYears.push_back(true); v_shapeSyst_group.push_back("jes");
        v_shapeSyst.push_back("PileUpPtBB"); v_shapeSyst_isCorrelYears.push_back(true); v_shapeSyst_group.push_back("jes");
        v_shapeSyst.push_back("PileUpPtEC1"); v_shapeSyst_isCorrelYears.push_back(true); v_shapeSyst_group.push_back("jes");
        v_shapeSyst.push_back("PileUpPtEC2"); v_shapeSyst_isCorrelYears.push_back(true); v_shapeSyst_group.push_back("jes");
        v_shapeSyst.push_back("PileUpPtHF"); v_shapeSyst_isCorrelYears.push_back(true); v_shapeSyst_group.push_back("jes");
    }


 //  ####  #####   ####  #    # #####   ####
 // #    # #    # #    # #    # #    # #
 // #      #    # #    # #    # #    #  ####
 // #  ### #####  #    # #    # #####       #
 // #    # #   #  #    # #    # #      #    #
 //  ####  #    #  ####   ####  #       ####

//-- Groups of nuisances (to be frozen together) --> To be defined at the end of the template datacard
//--------------------------------------------
    vector<TString> v_nuis_groups;
    // v_nuis_groups.push_back("theory");


//  ####    ##   #      #       ####
// #    #  #  #  #      #      #
// #      #    # #      #       ####
// #      ###### #      #           #
// #    # #    # #      #      #    #
//  ####  #    # ###### ######  ####

//Function calls
//--------------------------------------------
    //Generate the template datacard
    Generate_Datacard(v_samples, v_isSignal, v_sampleUncert, v_normSyst, v_normSystValue, v_shapeSyst, signal, v_shapeSyst_isCorrelYears, group_nuisances, v_nuis_groups, v_normSyst_group, v_shapeSyst_group);

    return 0;
}
