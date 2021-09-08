/* BASH CUSTOM */
#define RST   "\e[0m"
#define KRED  "\e[31m"
#define KGRN  "\e[32m"
#define KYEL  "\e[33m"
#define KBLU  "\e[34m"
#define KMAG  "\e[35m"
#define KCYN  "\e[36m"
#define KWHT  "\e[37m"
#define FRED(x) KRED x RST
#define FGRN(x) KGRN x RST
#define FYEL(x) KYEL x RST
#define FBLU(x) KBLU x RST
#define FMAG(x) KMAG x RST
#define FCYN(x) KCYN x RST
#define FWHT(x) KWHT x RST
#define BOLD(x) "\e[1m" x RST
#define ITAL(x) "\e[3m" x RST
#define UNDL(x) "\e[4m" x RST
#define STRIKE(x) "\e[9m" x RST
#define DIM(x) "\e[2m" x RST
#define DOUBLEUNDERLINE(x) "\e[21m" x RST
#define CURLYUNDERLINE(x) "\e[4:3m" x RST
#define BLINK(x) "\e[5m" x RST
#define REVERSE(x) "\e[7m" x RST
#define INVISIBLE(x) "\e[8m" x RST
#define OVERLINE(x) "\e[53m" x RST
#define TURQUOISE  "\e[38;5;42m"
#define SALMON  "\e[38;2;240;143;104m"
#define FTURQUOISE(x) TURQUOISE x RST
#define FSALMON(x) SALMON x RST
#define YELBKG(x) "\e[43m" x RST
#define CYANBKG(x) "\e[46m" x RST

#include <iostream>
#include <fstream>
#include <vector>
#include "TString.h"
#include "TFile.h"
#include "TH1F.h"
#include <sys/stat.h> // to be able to check file existence
#include <sstream>
#include <iomanip>

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

//Use stat function (from library sys/stat) to check if a file exists
bool Check_File_Existence(const TString& name)
{
    struct stat buffer;
    bool found = (stat (name.Data(), &buffer) == 0); //true if file exists
    return found;
}


//Convert a double into a TString
// precision --> can choose if TString how many digits the TString should display
TString Convert_Number_To_TString(double number, int precision=3)
{
	stringstream ss;
	ss << std::setprecision(precision) << number;
	TString ts = ss.str();
	return ts;
}


//--------------------------------------------
//  ######  ########  ########    ###    ######## ########
// ##    ## ##     ## ##         ## ##      ##    ##
// ##       ##     ## ##        ##   ##     ##    ##
// ##       ########  ######   ##     ##    ##    ######
// ##       ##   ##   ##       #########    ##    ##
// ##    ## ##    ##  ##       ##     ##    ##    ##
//  ######  ##     ## ######## ##     ##    ##    ########

//  ######   ######  ########  #### ########  ########    ########  #######  ########
// ##    ## ##    ## ##     ##  ##  ##     ##    ##       ##       ##     ## ##     ##
// ##       ##       ##     ##  ##  ##     ##    ##       ##       ##     ## ##     ##
//  ######  ##       ########   ##  ########     ##       ######   ##     ## ########
//       ## ##       ##   ##    ##  ##           ##       ##       ##     ## ##   ##
// ##    ## ##    ## ##    ##   ##  ##           ##       ##       ##     ## ##    ##
//  ######   ######  ##     ## #### ##           ##       ##        #######  ##     ##

// ######## ######## ##     ## ########  ##          ###    ######## ########    ######## #### ########
//    ##    ##       ###   ### ##     ## ##         ## ##      ##    ##          ##        ##     ##
//    ##    ##       #### #### ##     ## ##        ##   ##     ##    ##          ##        ##     ##
//    ##    ######   ## ### ## ########  ##       ##     ##    ##    ######      ######    ##     ##
//    ##    ##       ##     ## ##        ##       #########    ##    ##          ##        ##     ##
//    ##    ##       ##     ## ##        ##       ##     ##    ##    ##          ##        ##     ##
//    ##    ######## ##     ## ##        ######## ##     ##    ##    ########    ##       ####    ##
//--------------------------------------------

/**
 * Produce script containing the commands to produce the datacards (single and combination) automatically
 */
void Script_Datacards_TemplateFit(char include_systematics, char include_statistical, TString template_name, TString region, vector<TString> v_templates, vector<TString> v_channel, vector<TString> v_regions, TString lumiName, TString selection, TString filename_template_suffix, TString scriptname)
{
//  ####  ###### ##### #    # #####
// #      #        #   #    # #    #
//  ####  #####    #   #    # #    #
//      # #        #   #    # #####
// #    # #        #   #    # #
//  ####  ######   #    ####  #

    //-- Check if use shape syst or not
	TString systChoice;
	if(include_systematics == 'y') {systChoice = "withShape";}
	else if(include_systematics == 'n') {systChoice = "noShape";}
    else {cout<<"Wrong arguments ! Abort !"<<endl; return;}

    //-- Check if use stat. uncert. or not
	TString statChoice;
	if(include_statistical == 'y') {statChoice = "withStat";}
	else if(include_statistical == 'n') {statChoice = "noStat";}
    else {cout<<"Wrong arguments ! Abort !"<<endl; return;}

    //-- If specific arguments were chosen at command line, modify the vectors defined in the main
    if(template_name != "0") {v_templates.resize(0); v_templates.push_back(template_name);}

    //-- Trick: in case there is ==1 template and >=1 regions defined, we want to use this template for all the regions; but to allow to then add additional pairs of regions/templates, expand the current lists so that they match 1 to 1
    if(v_templates.size() == 1 && v_regions.size() >= 1)
    {
        for(int iregion=1; iregion<v_regions.size(); iregion++)
        {
            v_templates.push_back(v_templates[0]); //Duplicate first element for each additional region
        }
    }

    vector<TString> v_lumiYears;
    if(lumiName == "2016") {v_lumiYears.push_back("2016");}
    else if(lumiName == "2017") {v_lumiYears.push_back("2017");}
    else if(lumiName == "2018") {v_lumiYears.push_back("2018");}
    else if(lumiName == "201617") {v_lumiYears.push_back("2016"); v_lumiYears.push_back("2017");}
    else if(lumiName == "201618") {v_lumiYears.push_back("2016"); v_lumiYears.push_back("2018");}
    else if(lumiName == "201718") {v_lumiYears.push_back("2017"); v_lumiYears.push_back("2018");}
    else if(lumiName == "Run2") {v_lumiYears.push_back("2016"); v_lumiYears.push_back("2017"); v_lumiYears.push_back("2018");}

	ofstream file_out; //output script

    file_out.open(scriptname.Data(), std::ofstream::out); //Overwrite

	TString dir = "./datacards_TemplateFit/";
    file_out <<"mkdir "<<dir<<endl<<endl;
    for(int iyear=0; iyear<v_lumiYears.size(); iyear++)
    {
        file_out <<"mkdir "<<dir+v_lumiYears[iyear]<<endl<<endl;
    }


//  ######  ########  ########    ###    ######## ########
// ##    ## ##     ## ##         ## ##      ##    ##
// ##       ##     ## ##        ##   ##     ##    ##
// ##       ########  ######   ##     ##    ##    ######
// ##       ##   ##   ##       #########    ##    ##
// ##    ## ##    ##  ##       ##     ##    ##    ##
//  ######  ##     ## ######## ##     ##    ##    ########

// # #    # #####  # #    # # #####  #    #   ##   #
// # ##   # #    # # #    # # #    # #    #  #  #  #
// # # #  # #    # # #    # # #    # #    # #    # #
// # #  # # #    # # #    # # #    # #    # ###### #
// # #   ## #    # #  #  #  # #    # #    # #    # #
// # #    # #####  #   ##   # #####   ####  #    # ######

// #####    ##   #####   ##    ####    ##   #####  #####   ####
// #    #  #  #    #    #  #  #    #  #  #  #    # #    # #
// #    # #    #   #   #    # #      #    # #    # #    #  ####
// #    # ######   #   ###### #      ###### #####  #    #      #
// #    # #    #   #   #    # #    # #    # #   #  #    # #    #
// #####  #    #   #   #    #  ####  #    # #    # #####   ####

//--------------------------------------------
//--- First loop over years/regions/templates
//===> Commands to produce individual datacards

    for(int iyear=0; iyear<v_lumiYears.size(); iyear++)
    {
        for(int iregion=0; iregion<v_regions.size(); iregion++) //Loop over regions
        {
            // cout<<"v_regions[iregion] "<<v_regions[iregion]<<endl;

            for(int itemplate=0; itemplate<v_templates.size(); itemplate++) //Loop over templates
            {
                if(itemplate != iregion) {continue;} //Associate templates/regions 1 to 1 for more flexibility

                // cout<<"v_templates[itemplate] "<<v_templates[itemplate]<<endl;

                TString var = v_templates[itemplate] + (v_regions[iregion]!=""? "_"+v_regions[iregion]:"");

                //-- Protection: replace '-' (hyphen) with 'm' character (hyphen in histo name causes errors at reading)
                var.ReplaceAll('-', 'm');

                TString file_histos_pathFromHere = "./../templates/Templates_"+ v_templates[0] + (filename_template_suffix != ""? "_"+filename_template_suffix:"") + (selection != ""? "_"+selection:"") + "_"; //For use within this code
                TString file_histos_pathFromHere_Run2 = file_histos_pathFromHere + "Run2.root"; //In case year-dependent file is not found, will look for Run2 file by default
                file_histos_pathFromHere+= lumiName+".root";

                cout<<DIM("Trying to open input file "<<file_histos_pathFromHere<<" ... ");
                if(Check_File_Existence(file_histos_pathFromHere)) {cout<<DIM("FOUND !")<<endl;}
        		else
                {
                    cout<<endl<<DIM("Trying to open input file "<<file_histos_pathFromHere_Run2<<" ... "); //Try Run2 file
                    if(Check_File_Existence(file_histos_pathFromHere_Run2)) {cout<<DIM("FOUND !")<<endl;}
                    else {cout<<BOLD(FRED("ERROR: input template file not found ! Abort !"))<<endl;}
                    file_histos_pathFromHere = file_histos_pathFromHere_Run2; //Update main filepath
                }

                TString file_histos = "../." + file_histos_pathFromHere; //Path to write into datacard
    			cout<<endl<<FMAG("---> Will use filepath : ")<<file_histos<<endl<<endl;

    			for(int ilepchan=0; ilepchan<v_channel.size(); ilepchan++)
    			{
                    TString var_tmp = var;
                    TString template_path = "Template_Datacard.txt";
    				file_out<<"python Parser_Datacard_Template.py "
                    + var_tmp + " "
                    + v_channel[ilepchan] + " "
                    + v_lumiYears[iyear] + " "
    				+ file_histos + " "
    				+ systChoice + " "
    				+ statChoice + " "
                    + dir + v_lumiYears[iyear] + " "
                    + template_path + " "
    				<<endl;
    			} //channel loop

    			file_out<<endl<<endl;
        	} //template loop
        } //region loop
    } //year loop


//  ####   ####  #    # #####  # #    # ###### #####
// #    # #    # ##  ## #    # # ##   # #      #    #
// #      #    # # ## # #####  # # #  # #####  #    #
// #      #    # #    # #    # # #  # # #      #    #
// #    # #    # #    # #    # # #   ## #      #    #
//  ####   ####  #    # #####  # #    # ###### #####

// #####    ##   #####   ##    ####    ##   #####  #####
// #    #  #  #    #    #  #  #    #  #  #  #    # #    #
// #    # #    #   #   #    # #      #    # #    # #    #
// #    # ######   #   ###### #      ###### #####  #    #
// #    # #    #   #   #    # #    # #    # #   #  #    #
// #####  #    #   #   #    #  ####  #    # #    # #####

//--------------------------------------------
//--- Second loop over years/regions/templates
//===> Give all the single datacards as arguments to the script, for combination

    //Command to execute the script which combines the datacards
    file_out<<"python $CMSSW_BASE/src/HiggsAnalysis/CombinedLimit/scripts/combineCards.py ";

    for(int iyear=0; iyear<v_lumiYears.size(); iyear++)
    {
        for(int iregion=0; iregion<v_regions.size(); iregion++) //Loop over regions
        {
            for(int itemplate=0; itemplate<v_templates.size(); itemplate++) //Loop over templates
            {
                if(itemplate != iregion) {continue;} //Associate templates/regions 1 to 1 for more flexibility

                TString var = v_templates[itemplate] + (v_regions[iregion]!=""? "_"+v_regions[iregion]:"");

                //-- Protection: replace '-' (hyphen) with 'm' character (hyphen in histo name causes errors at reading)
                var.ReplaceAll('-', 'm');

                for(int ilepchan=0; ilepchan<v_channel.size(); ilepchan++) //Loop over channels
                {
                    TString var_tmp = var;
                    file_out<<var_tmp;
                    if(v_channel[ilepchan] != "all") {file_out<<"_" + v_channel[ilepchan];}
                    file_out<<"_"+v_lumiYears[iyear];
                    file_out<<"=" + dir + v_lumiYears[iyear] + "/"
                    + "datacard_"+var_tmp;
                    if(v_channel[ilepchan] != "all") {file_out<<"_" + v_channel[ilepchan];}
                    file_out<<".txt ";
                } //channel loop

            } //template loop
        } //region loop
    } //years loop

	TString output_name = "COMBINED_Datacard_TemplateFit";
    if(systChoice == "noShape") output_name+= "_noShape";
    if(statChoice == "noStat") output_name+= "_noStat";
    output_name+= "_" + v_templates[0]; //-- Make name template-specific //CHANGED
    output_name+= "_" + lumiName;
    output_name+= ".txt";

	file_out<<"> "<<output_name<<endl<<endl;

    file_out<<"echo \"\""<<endl;
    file_out<<"echo \"\e[33m===> Created combined datacard: \e[0m"<<output_name<<"\""<<endl<<endl<<endl;


//  ######   #######  ##     ## ########  #### ##    ## ######## ########
// ##    ## ##     ## ###   ### ##     ##  ##  ###   ## ##       ##     ##
// ##       ##     ## #### #### ##     ##  ##  ####  ## ##       ##     ##
// ##       ##     ## ## ### ## ########   ##  ## ## ## ######   ##     ##
// ##       ##     ## ##     ## ##     ##  ##  ##  #### ##       ##     ##
// ##    ## ##     ## ##     ## ##     ##  ##  ##   ### ##       ##     ##
//  ######   #######  ##     ## ########  #### ##    ## ######## ########

// ########     ###    ########    ###     ######     ###    ########  ########   ######
// ##     ##   ## ##      ##      ## ##   ##    ##   ## ##   ##     ## ##     ## ##    ##
// ##     ##  ##   ##     ##     ##   ##  ##        ##   ##  ##     ## ##     ## ##
// ##     ## ##     ##    ##    ##     ## ##       ##     ## ########  ##     ##  ######
// ##     ## #########    ##    ######### ##       ######### ##   ##   ##     ##       ##
// ##     ## ##     ##    ##    ##     ## ##    ## ##     ## ##    ##  ##     ## ##    ##
// ########  ##     ##    ##    ##     ##  ######  ##     ## ##     ## ########   ######

// ########  ######## ########     ########  ########  ######   ####  #######  ##    ##
// ##     ## ##       ##     ##    ##     ## ##       ##    ##   ##  ##     ## ###   ##
// ##     ## ##       ##     ##    ##     ## ##       ##         ##  ##     ## ####  ##
// ########  ######   ########     ########  ######   ##   ####  ##  ##     ## ## ## ##
// ##        ##       ##   ##      ##   ##   ##       ##    ##   ##  ##     ## ##  ####
// ##        ##       ##    ##     ##    ##  ##       ##    ##   ##  ##     ## ##   ###
// ##        ######## ##     ##    ##     ## ########  ######   ####  #######  ##    ##

//-- Also create datacards for individual regions (cross-checks)
//-- NB: datacards split by years may already be created by selecting a specific year via command-line
//-- NB: these cards are stored in 'datacards_TemplateFit', but must be run from 'datacards' from relative paths to work (--> need to copy card first)
    bool create_datacards_indivRegions = true;
    if(create_datacards_indivRegions)
    {
        for(int iregion=0; iregion<v_regions.size(); iregion++) //Loop over regions
        {
            //Command to execute the script which combines the datacards
            file_out<<"python $CMSSW_BASE/src/HiggsAnalysis/CombinedLimit/scripts/combineCards.py ";

            for(int itemplate=0; itemplate<v_templates.size(); itemplate++) //Loop over templates
            {
                if(itemplate != iregion) {continue;} //-- Associate templates/regions 1 to 1 for more flexibility

                for(int iyear=0; iyear<v_lumiYears.size(); iyear++)
                {
                    TString var = v_templates[itemplate] + (v_regions[iregion]!=""? "_"+v_regions[iregion]:"");

                    //-- Protection: replace '-' (hyphen) with 'm' character (hyphen in histo name causes errors at reading)
                    var.ReplaceAll('-', 'm');

                    for(int ilepchan=0; ilepchan<v_channel.size(); ilepchan++) //Loop over channels
                    {
                        TString var_tmp = var;
                        file_out<<var_tmp;
                        if(v_channel[ilepchan] != "all") {file_out<<"_" + v_channel[ilepchan];}
                        file_out<<"_"+v_lumiYears[iyear];
                        file_out<<"=" + dir + v_lumiYears[iyear] + "/"
                        + "datacard_"+var_tmp;
                        if(v_channel[ilepchan] != "all") {file_out<<"_" + v_channel[ilepchan];}
                        file_out<<".txt ";
                    } //channel loop
                } //years loop
            } //template loop

            TString output_name = "COMBINED_Datacard_TemplateFit";
            if(systChoice == "noShape") output_name+= "_noShape";
            if(statChoice == "noStat") output_name+= "_noStat";
            output_name+= "_" + v_templates[iregion]; //-- Make region card name template-specific
            output_name+= "_" + v_regions[iregion] + "_" + lumiName + ".txt";
            file_out<<"> "<<output_name<<endl<<endl;
            file_out<<"mv "<<output_name<<" datacards_TemplateFit/"<<endl<<endl;
            cout<<"--> Generated per-region card: "<<output_name<<endl;
        } //region loop
    } //create_datacards_indivRegions

	file_out.close();
}


void Run_Commands(TString scriptname)
{
    system(("chmod 755 "+scriptname).Data());

    cout<<FGRN("... Created script ./"<<scriptname<<" !")<<endl;

    cout<<endl<<endl<<FGRN("-- Executing script ./"<<scriptname<<" !")<<endl;

    system(("./"+scriptname).Data());
}











//--------------------------------------------
//  ######   #######  ##     ## ##     ##    ###    ##    ## ########
// ##    ## ##     ## ###   ### ###   ###   ## ##   ###   ## ##     ##
// ##       ##     ## #### #### #### ####  ##   ##  ####  ## ##     ##
// ##       ##     ## ## ### ## ## ### ## ##     ## ## ## ## ##     ##
// ##       ##     ## ##     ## ##     ## ######### ##  #### ##     ##
// ##    ## ##     ## ##     ## ##     ## ##     ## ##   ### ##     ##
//  ######   #######  ##     ## ##     ## ##     ## ##    ## ########

// ##       #### ##    ## ########       ###    ########   ######    ######
// ##        ##  ###   ## ##            ## ##   ##     ## ##    ##  ##    ##
// ##        ##  ####  ## ##           ##   ##  ##     ## ##        ##
// ##        ##  ## ## ## ######      ##     ## ########  ##   ####  ######
// ##        ##  ##  #### ##          ######### ##   ##   ##    ##        ##
// ##        ##  ##   ### ##          ##     ## ##    ##  ##    ##  ##    ##
// ######## #### ##    ## ########    ##     ## ##     ##  ######    ######
//--------------------------------------------

//Ask user to choose options at command line for script generation
void Choose_Arguments_From_CommandLine(char& include_systematics, char& include_statistical, TString& template_name, TString& region, TString& lumiName, TString& selection, TString& filename_template_suffix)
{
    char chartmp = 'a'; int inttmp = -1; TString tstringtmp = ""; //Buffers

    cout<<endl<<BOLD(ITAL(DIM(<<"['0' <-> use value set in main()] ... ")))<<endl<<endl;

	//-- Choose whether to include shape syst or not
	cout<<endl<<FYEL("=== Include "<<UNDL(<<"shape"))<<FYEL(" systematics in the datacards ? ===")<<endl;
    cout<<ITAL(DIM(<<"['y'/'n'] ... "));
	cin>>chartmp;
	while(chartmp != 'y' && chartmp != 'n' && chartmp != '0')
	{
		cin.clear();
		cin.ignore(1000, '\n');

		cout<<" Wrong answer ! Need to type 'y' or 'n' ! Retry :"<<endl;
		cin>>chartmp;
	}
    if(chartmp == 'y' || chartmp == 'n') {include_systematics = chartmp;}

    //-- Choose whether to include statistic uncert. or not (e.g. for pulls of NPs, need to remove them, else too long)
    cout<<endl<<FYEL("=== Include "<<UNDL(<<"statistical"))<<FYEL(" uncertainties in the datacards ? ===")<<endl;
    cout<<ITAL(DIM(<<"['y'/'n'] ... "));
    cin>>chartmp;
    while(chartmp != 'y' && chartmp != 'n' && chartmp != '0')
    {
        cin.clear();
        cin.ignore(1000, '\n');

        cout<<" Wrong answer ! Need to type 'y' or 'n' ! Retry :"<<endl;
        cin>>chartmp;
    }
    if(chartmp == 'y' || chartmp == 'n') {include_statistical = chartmp;}

    //Set the template name (e.g.'NN') to be looked for in the rootfiles //If ignored, use the value set in the main
    cout<<endl<<FYEL("=== Set the template name (to read corresponding histograms in SRs) ===")<<endl;
    cout<<ITAL(DIM("'xxx' / ..."))<<endl;
    cout<<ITAL(DIM("(0 <-> use the template name defined in the main)"))<<endl;
    cout<<ITAL(DIM("(NB: if you set a template name, it will be used for all regions defined in the main)"))<<endl;
    cout<<ITAL(DIM(<<"..."));
    cin>>tstringtmp;
    if(tstringtmp != "0") {template_name = tstringtmp;}

	//Choose the luminosity
    cout<<endl<<FYEL("=== Choose the luminosity ===")<<endl;
    cout<<ITAL(DIM(<<"['Run2'/'2016'/'2017'/'2018'/'201617'/'201618'/'201718'] ... "));
    cin>>tstringtmp;
    while(tstringtmp != "0" && tstringtmp != "Run2" && tstringtmp != "2016" && tstringtmp != "2017" && tstringtmp != "2018" && tstringtmp != "201617" && tstringtmp != "201618" && tstringtmp != "201718")
    {
        cin.clear();
        cin.ignore(1000, '\n');

        cout<<" Wrong answer ! Retry :"<<endl;
        cin>>tstringtmp;
    }
    if(tstringtmp == "Run2" || tstringtmp == "2016" || tstringtmp == "2017" || tstringtmp == "2018" || tstringtmp == "201617" || tstringtmp == "201618" || tstringtmp == "201718") {lumiName = tstringtmp;}

    //Set a 'filename suffix' if needed (if present in the filename)
    cout<<endl<<FYEL("=== Set the template name suffix for the filename ===")<<endl;
    cout<<ITAL(DIM("'xxx' / ..."))<<endl;
    cout<<ITAL(DIM(<<"(NB: 1 <-> '')"));
    cout<<ITAL(DIM(<<"..."));
    cin>>tstringtmp;
    if(tstringtmp != "0") {filename_template_suffix = tstringtmp;}
    if(tstringtmp == "1") {filename_template_suffix = "";} //No suffix

    //Set a 'selection flag' if necessary (if present in the histo/file names)
    cout<<endl<<FYEL("=== Set the event selection flag ===")<<endl;
    cout<<ITAL(DIM("'xxx' / ..."))<<endl;
    cout<<ITAL(DIM(<<"(NB: 1 <-> '')"));
    cout<<ITAL(DIM(<<"..."));
    cin>>tstringtmp;
    if(tstringtmp != "0") {selection = tstringtmp;}
    if(tstringtmp == "1") {selection = "";} //No suffix

    return;
}










//--------------------------------------------
// ##     ##    ###    #### ##    ##
// ###   ###   ## ##    ##  ###   ##
// #### ####  ##   ##   ##  ####  ##
// ## ### ## ##     ##  ##  ## ## ##
// ##     ## #########  ##  ##  ####
// ##     ## ##     ##  ##  ##   ###
// ##     ## ##     ## #### ##    ##
//--------------------------------------------

int main()
{
// Can set options here
//--------------------------------------------
    vector<TString> v_channel; //'all', 'uuu', 'eeu', 'uue', 'eee' //Used only for leptonic channels (use v_regions for 'regions')
    v_channel.push_back("all");
    // v_channel.push_back("uuu");
    // v_channel.push_back("eeu");
    // v_channel.push_back("uue");
    // v_channel.push_back("eee");

    //-- Two ways to combine regions (e.g. 'SR') and templates (e.g. 'Zpt') together:
    // a) Define here the same number of regions & templates --> will associate them 1 to 1
    // b) Define ==1 template (possibly via command-line) and >= 1 region --> will associate the template with all considered regions
    // NB: ORDER MATTERS /!\

    vector<TString> v_templates; //'NN', 'BDT', ...
    v_templates.push_back("Zpt");

    vector<TString> v_regions; //'SR', 'CR_xx', ... (must reflect bin names)
    v_regions.push_back("SR");

    TString selection = ""; //Main event selection, before sub-categorization
    TString filename_template_suffix = ""; //Specify extension in histo filename

    TString scriptname = "makeDatacardsForTemplateFit.sh"; //Name of output script to create and run

//-- Modified at command-line
//--------------------------------------------
    TString lumiName = "Run2"; //'2016','2017','2018','201617','201618','201718','Run2'

    char include_systematics = 'y';//'y' <-> datacards will include syst. uncertainties (as specified in template datacard)
    char include_statistical = 'y';//'y' <-> datacards will include stat. uncertainty (as specified in template datacard)

//--------------------------------------------

//Automated
//--------------------------------------------
    TString template_name = "0", region = "0";
    Choose_Arguments_From_CommandLine(include_systematics, include_statistical, template_name, region, lumiName, selection, filename_template_suffix);

    Script_Datacards_TemplateFit(include_systematics, include_statistical, template_name, region, v_templates, v_channel, v_regions, lumiName, selection, filename_template_suffix, scriptname);

    Run_Commands(scriptname);
//--------------------------------------------

	return 0;
}
