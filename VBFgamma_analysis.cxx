//-- by Nicolas Tonon (DESY) --//

//--- LIST OF FUNCTIONS (for quick search) :
//--------------------------------------------
// Train_BDT

// Produce_Templates

// Draw_Templates

// Compare_TemplateShapes_Processes

// MergeSplit_Templates
// SetBranchAddress_SystVariationArray
//--------------------------------------------

#include "VBFgamma_analysis.h"

#define MYDEBUG(msg) cout<<endl<<ITAL("-- DEBUG: " << __FILE__ << ":" << __LINE__ <<":")<<FRED(" " << msg  <<"")<<endl

using namespace std;


//---------------------------------------------------------------------------
// ####    ##    ##    ####    ########
//  ##     ###   ##     ##        ##
//  ##     ####  ##     ##        ##
//  ##     ## ## ##     ##        ##
//  ##     ##  ####     ##        ##
//  ##     ##   ###     ##        ##
// ####    ##    ##    ####       ##
//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

//Overloaded constructor
VBFgamma_analysis::VBFgamma_analysis(vector<TString> thesamplelist, vector<TString> thesamplegroups, vector<TString> thesystlist, vector<TString> thesystTreelist, vector<TString> thechannellist, vector<TString> thevarlist, vector<TString> set_v_cut_name, vector<TString> set_v_cut_def, vector<bool> set_v_cut_IsUsedForBDT, vector<TString> set_v_add_var_names, TString theplotextension, vector<TString> set_lumi_years, bool show_pulls, TString region, TString signal_process, TString classifier_name, bool is_blind, bool use_specificMVA_eachYear, TString nominal_tree_name, bool process_samples_byGroup, bool use_paperStyle)
{
    //Canvas definition
    Load_Canvas_Style();

    TH1::SetDefaultSumw2();
    gStyle->SetErrorX(0.);

    nbins = 10; //default

	mkdir("outputs", 0777);
	mkdir("plots", 0777);

    if(nominal_tree_name != "") {this->nominal_tree_name = nominal_tree_name;}

	stop_program = false;

	this->region = region;

    this->use_paperStyle = use_paperStyle;

    this->signal_process = signal_process;
    // if(signal_process != "tZq" && signal_process != "ttZ" && signal_process != "tWZ") {cout<<BOLD(FRED("ERROR ! [signal_process] option not recognized ! Abort..."))<<endl; stop_program = true;}

    this->use_specificMVA_eachYear = use_specificMVA_eachYear;

	this->is_blind = is_blind;

    sample_list.resize(thesamplelist.size());
    sample_groups.resize(thesamplelist.size());
	for(int i=0; i<thesamplelist.size(); i++)
	{
        sample_list[i] = thesamplelist[i];
        sample_groups[i] = thesamplegroups[i];
        // cout<<"- Added sample : "<<sample_list[i]<<endl;
	}

    //Count nof different sample groups
    nSampleGroups=1; //minimum is 1 sample group
    for(int i=1; i<sample_groups.size(); i++)
    {
        if(sample_groups[i] != sample_groups[i-1]) {nSampleGroups++;}
    }
    if(region=="twz") {nSampleGroups++;} //In tWZ region, single out tWZ process from tX

//=== LUMI ===//
/*
# Luminosity conventions #
- The 'v_lumiYears' vector lists the years which are to be considered (its elements can be '2016', '2017' and '2018' - in correct order)
- Depending on the elements of 'v_lumiYears', we define a unique identifier 'lumiName' (e.g. "201617" for 2016+2017, or "Run2" for the sum of all 3 years)
- From there, we can compute the corresponding integrated luminosity and store it in the 'lumiValue' variable (for plots)

- Input ntuples are stored in separe "2016"/"2017"/"2018" sub-directories ; hence, to read them, we loop on the elements of v_lumiYears. Also specify in histo name the exact year it corresponds to.
- The 'lumiName' identifier is instead used for outputs (root files, histograms, ...) to differentiate them and chain them to subsequent codes (Combine, etc.)
 */

    //Lumi choice determines which ntuples are read
    v_lumiYears = set_lumi_years; //Vector containing the names of the years to process

    //Set a unique name to each combination of year(s)
    if(v_lumiYears.size() == 1) {lumiName = v_lumiYears[0];}
    else if(v_lumiYears.size() == 2)
    {
        if(v_lumiYears[0] == "2016" && v_lumiYears[1] == "2017") {lumiName = "201617";}
        else if(v_lumiYears[0] == "2016" && v_lumiYears[1] == "2018") {lumiName = "201618";}
        else if(v_lumiYears[0] == "2017" && v_lumiYears[1] == "2018") {lumiName = "201718";}
    }
    else if(v_lumiYears.size() == 3) {lumiName = "Run2";}
    else {cout<<BOLD(FRED("ERROR ! I don't understand the list of years to process, please stick to the conventions ! Abort..."))<<endl; stop_program = true;}
    // this->lumiYear = lumiYear;

    Set_Luminosity(lumiName); //Compute the corresponding integrated luminosity

//-------------

    dir_ntuples = NTUPLEDIR; //Defined in Utils/Helper
    TString dir_ntuples_tmp = dir_ntuples;

    this->use_optimized_ntuples = false; //Obsolete
    /* //Obsolete
    dir_ntuples_SR = dir_ntuples + "SR/";
    if(region=="tZq" || region=="ttZ" || region=="signal") {dir_ntuples = dir_ntuples_SR; cout<<endl<<FMAG("NB: will use SR sub-samples ! Make sure they are up-to-date !")<<endl<<endl;}
    if(dir_ntuples == dir_ntuples_SR && !Check_File_Existence((dir_ntuples_SR + v_lumiYears[0] + "/DATA.root")))
    {
        cout<<endl<<FMAG("Warning : SR sub-sample (produced with code input_ntuples/Split_FullSamples) "<<(dir_ntuples_SR + v_lumiYears[0] + "/DATA.root")<<" not found ! Assuming that split ntuples are not available --> Will use full ntuples ! (slower)")<<endl<<endl;
        dir_ntuples = NTUPLEDIR;
    }
    // cout<<"dir_ntuples : "<<dir_ntuples<<endl;

    //-- To considerably speed-up the usual AN workflow, use [Split_AllNtuples_ByCategory] code to 1) split full ntuples by sub-categories, 2) merge all samples which belong to same sample groups
    //---> if found, will only use these 'optimized' ntuples <-> update the member lists accordingly
    if(dir_ntuples == dir_ntuples_SR && Check_File_Existence((dir_ntuples_SR + v_lumiYears[0] + "/NPL.root")) ) //Hard-coded check: if 'xx/SR/xx/NPL.root' sample is present, it must mean that we have split the ntuples by categ. and merged them by groups (here: NPL group)
    {
        this->use_optimized_ntuples = true;
        cout<<BOLD(FMAG("=== Optimized sub-ntuples found ! Will use ntuples split by categories & merged by groups (++ performance) ==="))<<endl;
        cout<<DIM("--> Set sample_list = sample_groups")<<endl<<endl;

        //Update member lists
        sample_list.clear();
        for(int igroup=0; igroup<sample_groups.size(); igroup++)
        {
            if(igroup > 0 && sample_groups[igroup]==sample_groups[igroup-1]) {continue;} //Group already considered --> Skip
            sample_list.push_back(sample_groups[igroup]); //Add sample group
        }
        sample_groups = sample_list;
    }
    */

    this->process_samples_byGroup = process_samples_byGroup;
    if(this->process_samples_byGroup) //Run on group ntuples to reduce the number of files to read
    {
        bool found_allGroupNtuples = true;
        for(int iyear=0; iyear<v_lumiYears.size(); iyear++)
        {
            for(int igroup=0; igroup<sample_groups.size(); igroup++)
            {
                if(igroup > 0 && sample_groups[igroup] == sample_groups[igroup-1]) {continue;}
                if(!Check_File_Existence(dir_ntuples + v_lumiYears[iyear] + "/" + sample_groups[igroup] + ".root")) //If some group ntuple is missing, will read all individual ntuples instead (default)
                {
                    cout<<FRED("WARNING: you have set [process_samples_byGroup=true] but I could not find group ntuple ["<<(dir_ntuples + v_lumiYears[iyear] + "/" + sample_groups[igroup] + ".root")<<"]... !")<<endl;
                    // usleep(100000); //Pause

                    // cout<<BOLD(FRED("WARNING: you have set [process_samples_byGroup=true] but I could not find group ntuple ["<<(dir_ntuples + v_lumiYears[iyear] + "/" + sample_groups[igroup] + ".root")<<"]... Setting [process_samples_byGroup=false] and processing individual ntuples instead !"))<<endl;
                    // found_allGroupNtuples = false;
                }
            }
        }
        if(found_allGroupNtuples)
        {
           sample_groups.erase( unique( sample_groups.begin(), sample_groups.end() ), sample_groups.end() ); //Remove all duplicates in 'sample_groups' (<-> only keep 1 occurence for each group)
           sample_list = sample_groups; //Samples will now refer to sample groups

           // for(int igroup=0; igroup<sample_groups.size(); igroup++) {cout<<"sample_groups[igroup] "<<sample_groups[igroup]<<endl;}
           // for(int isample=0; isample<sample_list.size(); isample++) {cout<<"sample_list[isample] "<<sample_list[isample]<<endl;}
        }
    }

	//-- Get colors
    int color_scheme = 2; //Check color scheme definitions directly in Get_Samples_Colors() //FIXME
	color_list.resize(sample_list.size());
	Get_Samples_Colors(color_list, v_custom_colors, sample_list, sample_groups, color_scheme); //Read hard-coded sample colors

	this->classifier_name = classifier_name;
    if(classifier_name != "BDT" && classifier_name != "NN") {cout<<BOLD(FRED("Warning : classifier_name value ["<<classifier_name<<"] not supported !"))<<endl;}

	plot_extension = theplotextension;

	show_pulls_ratio = show_pulls;

	syst_list.resize(thesystlist.size());
	for(int i=0; i<thesystlist.size(); i++)
	{
		syst_list[i] = thesystlist[i];
	}
	if(syst_list.size() == 0 || syst_list[0] != "") {cout<<"ERROR : first element of 'syst_list' is not empty (<-> nominal event weight) ! If that's what you want, remove this protection !"<<endl; stop_program = true;}

	systTree_list.resize(thesystTreelist.size());
	for(int i=0; i<thesystTreelist.size(); i++)
	{
		systTree_list[i] = thesystTreelist[i];
	}
	if(systTree_list.size() == 0 || systTree_list[0] != "") {cout<<"ERROR : first element of 'systTree_list' is not empty (<-> nominal TTree) ! If that's what you want, remove this protection !"<<endl; stop_program = true;}

	channel_list.resize(thechannellist.size());
	for(int i=0; i<thechannellist.size(); i++)
	{
		channel_list[i] = thechannellist[i];
	}
	if(channel_list.size() == 0 || channel_list[0] != "") {cout<<"ERROR : first element of 'channel_list' is not empty (<-> no subcat.) or vector is empty ! If that's what you want, remove this protection !"<<endl; stop_program = true;}

	for(int i=0; i<set_v_cut_name.size(); i++) //Region cuts vars (e.g. NJets)
	{
		v_cut_name.push_back(set_v_cut_name[i]);
		v_cut_def.push_back(set_v_cut_def[i]);
		v_cut_IsUsedForBDT.push_back(set_v_cut_IsUsedForBDT[i]);
		v_cut_float.push_back(-999);
		v_cut_char.push_back(1); //flags set to true by default

		//NOTE : it is a problem if a variable is present in more than 1 list, because it will cause SetBranchAddress conflicts (only the last SetBranchAddress to a branch will work)
		//---> If a variable is present in 2 lists, erase it from other lists !
		for(int ivar=0; ivar<thevarlist.size(); ivar++)
		{
			if(thevarlist[ivar].BeginsWith("is") || thevarlist[ivar].BeginsWith("passed") )
			{
				cout<<BOLD(FRED("## Warning : categories should not been used as input/spectator variables ! Are you sure ? "))<<endl;
			}
			if(thevarlist[ivar] == set_v_cut_name[i])
			{
				cout<<FGRN("** Constructor")<<" : erased variable "<<thevarlist[ivar]<<" from vector thevarlist (possible conflict) !"<<endl;
				thevarlist.erase(thevarlist.begin() + ivar);
				ivar--; //modify index accordingly
			}

		}
		for(int ivar=0; ivar<set_v_add_var_names.size(); ivar++)
		{
			if(set_v_add_var_names[ivar].BeginsWith("is") || set_v_add_var_names[ivar].BeginsWith("passed") )
			{
				cout<<BOLD(FRED("## Warning : categories should not been used as input/spectator variables ! Are you sure ? "))<<endl;
			}
			if(set_v_add_var_names[ivar] == set_v_cut_name[i])
			{
				cout<<FGRN("** Constructor")<<" : erased variable "<<set_v_add_var_names[ivar]<<" from vector set_v_add_var_names (possible conflict) !"<<endl;
				set_v_add_var_names.erase(set_v_add_var_names.begin() + ivar);
				ivar--; //modify index accordingly
			}
		}

		// cout<<"Cuts : name = "<<v_cut_name[i]<<" / def = "<<v_cut_def[i]<<endl;
	}
	for(int i=0; i<thevarlist.size(); i++) //TMVA vars
	{
		var_list.push_back(thevarlist[i]);
		// var_list_floats.push_back(-999);
        var_list_pfloats.push_back(NULL);

		for(int ivar=0; ivar<set_v_add_var_names.size(); ivar++)
		{
			if(set_v_add_var_names[ivar] == thevarlist[i])
			{
				cout<<FGRN("** Constructor")<<" : erased variable "<<set_v_add_var_names[ivar]<<" from vector set_v_add_var_names (possible conflict) !"<<endl;
				set_v_add_var_names.erase(set_v_add_var_names.begin() + ivar);
				ivar--; //modify index accordingly
			}
		}
	}

	for(int i=0; i<set_v_add_var_names.size(); i++) //Additional vars, only for CR plots
	{
		v_add_var_names.push_back(set_v_add_var_names[i]);
		v_add_var_floats.push_back(-999);
	}

	//Make sure that the "==" sign is written properly, or rewrite it
	for(int ivar=0; ivar<v_cut_name.size(); ivar++)
	{
		if( v_cut_def[ivar].Contains("=") && !v_cut_def[ivar].Contains("!") && !v_cut_def[ivar].Contains("==") && !v_cut_def[ivar].Contains("<") && !v_cut_def[ivar].Contains(">") )
		{
			v_cut_def[ivar] = "==" + Convert_Number_To_TString(Find_Number_In_TString(v_cut_def[ivar]));

			cout<<endl<<BOLD(FBLU("##################################"))<<endl;
			cout<<"--- Changed cut on "<<v_cut_name[ivar]<<" to: "<<v_cut_def[ivar]<<" ---"<<endl;
			cout<<BOLD(FBLU("##################################"))<<endl<<endl;
		}
	}

    array_PU = NULL;
    array_prefiringWeight = NULL;
    array_Btag = NULL;
    array_jetPileupID = NULL;
    array_fakeFactor = NULL;
    array_ME = NULL;
    array_alphaS = NULL;
    array_PDFtotal = NULL;
    array_partonShower = NULL;
    array_LepEffLoose_mu = NULL;
    array_LepEffTight_mu = NULL;
    array_LepEffLoose_el = NULL;
    array_LepEffTight_el = NULL;

	//Store the "cut name" that will be written as a suffix in the name of each output file
	this->filename_suffix = "";
	TString tmp = "";
	for(int ivar=0; ivar<v_cut_name.size(); ivar++)
	{
		if(v_cut_name[ivar].BeginsWith("is") || v_cut_name[ivar].BeginsWith("passed") ) {continue;} //No need to appear in filename

		if(v_cut_def[ivar] != "")
		{
            if(!v_cut_def[ivar].Contains("&&") && !v_cut_def[ivar].Contains("||") ) //Single condition
            {
                tmp+= "_" + v_cut_name[ivar] + Convert_Sign_To_Word(v_cut_def[ivar]) + Convert_Number_To_TString(Find_Number_In_TString(v_cut_def[ivar]));
            }
            else if(v_cut_def[ivar].Contains("&&")) //Double '&&' condition
            {
                TString cut1 = Break_Cuts_In_Two(v_cut_def[ivar]).first, cut2 = Break_Cuts_In_Two(v_cut_def[ivar]).second;
                tmp = "_" + v_cut_name[ivar] + Convert_Sign_To_Word(cut1) + Convert_Number_To_TString(Find_Number_In_TString(cut1));
                tmp+= Convert_Sign_To_Word(cut2) + Convert_Number_To_TString(Find_Number_In_TString(cut2));
            }
            else if(v_cut_def[ivar].Contains("||") )
            {
                TString cut1 = Break_Cuts_In_Two(v_cut_def[ivar]).first, cut2 = Break_Cuts_In_Two(v_cut_def[ivar]).second;
                tmp = "_" + v_cut_name[ivar] + Convert_Sign_To_Word(cut1) + Convert_Number_To_TString(Find_Number_In_TString(cut1));
                tmp+= "OR" + Convert_Sign_To_Word(cut2) + Convert_Number_To_TString(Find_Number_In_TString(cut2));
            }

			this->filename_suffix+= tmp;
		}
	}

    cout<<endl<<endl<<BLINK(BOLD(FBLU("[Region : "<<region<<"]")))<<endl;
    cout<<endl<<BLINK(BOLD(FBLU("[Luminosity : "<<lumiName<<"]")))<<endl<<endl<<endl;

//--------------------------------------------

    usleep(1000000); //Pause for 1s (in microsec)
}


VBFgamma_analysis::~VBFgamma_analysis()
{
    // cout<<"~VBFgamma_analysis()"<<endl;

    for(int icol=0; icol<v_custom_colors.size(); icol++)
    {
        if(v_custom_colors[icol] != 0) {delete v_custom_colors[icol];}
    }
}


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/**
 * Compute the luminosity re-scaling factor (MC),  to be used thoughout the code
 * @param desired_luminosity [Value of the desired lumi in fb-1]
 */
void VBFgamma_analysis::Set_Luminosity(TString luminame)
{
    if(luminame == "2016") {lumiValue = 36.33;} //Preliminary was: 35.92
	else if(luminame == "2017") {lumiValue = 41.53;}
    else if(luminame == "2018") {lumiValue = 59.74;}
    else if(luminame == "201617") {lumiValue = 36.33+41.53;}
    else if(luminame == "201618") {lumiValue = 36.33+59.74;}
    else if(luminame == "201718") {lumiValue = 41.53+59.74;}
    else if(luminame == "Run2") {lumiValue = 36.33+41.53+59.74;}

	assert(lumiValue > 0 && "Using wrong lumi reference -- FIX IT !"); //Make sure we use a sensible lumi value

    // if(luminosity_rescale != 1)
	// {
	// 	cout<<endl<<BOLD(FBLU("##################################"))<<endl;
	// 	cout<<"--- Using luminosity scale factor : "<<desired_luminosity<<" / "<<lumiValue<<" = "<<luminosity_rescale<<" ! ---"<<endl;
	// 	cout<<BOLD(FBLU("##################################"))<<endl<<endl;
	// }

    return;
}













//---------------------------------------------------------------------------
// ########    ########        ###       ####    ##    ##    ####    ##    ##     ######
//    ##       ##     ##      ## ##       ##     ###   ##     ##     ###   ##    ##    ##
//    ##       ##     ##     ##   ##      ##     ####  ##     ##     ####  ##    ##
//    ##       ########     ##     ##     ##     ## ## ##     ##     ## ## ##    ##   ####
//    ##       ##   ##      #########     ##     ##  ####     ##     ##  ####    ##    ##
//    ##       ##    ##     ##     ##     ##     ##   ###     ##     ##   ###    ##    ##
//    ##       ##     ##    ##     ##    ####    ##    ##    ####    ##    ##     ######
//---------------------------------------------------------------------------


/**
 * Perform BDT training. Uses relevant samples, enforces cuts, etc. Modify training parameters/nof events/... in this function.
 * NB : Will sum all years given to member vector 'v_lumiYears' for the training ! If want separate training per year, run this func individually for each one!
 */
void VBFgamma_analysis::Train_BDT(TString channel)
{
//--- OPTIONS --------------------------------
//--------------------------------------------
	bool use_relative_weights = false; //false <-> use abs(weight), much faster if there are many negative weights
    int nmaxEv = 100000; //max nof events for train or test; -1 <-> use all available events //NB: full Run 2 is too much
    float trainingEv_proportion = 0.7;
//--------------------------------------------
//--------------------------------------------

    cout<<endl<<BYEL("                          ")<<endl<<endl;
    cout<<FYEL("--- TRAINING ---")<<endl;
    cout<<endl<<BYEL("                          ")<<endl<<endl;

	if(use_relative_weights) {cout<<"-- Using "<<BOLD(FGRN("*RELATIVE weights*"))<<" --"<<endl<<endl<<endl;}
	else {cout<<"-- Using "<<BOLD(FGRN("*ABSOLUTE weights*"))<<" --"<<endl<<endl<<endl;}

    if(classifier_name != "BDT") {cout<<BOLD(FRED("ERROR : can only train BDTs within TMVA for now... Abort ! (classifier_name = "<<classifier_name<<")"))<<endl; return;}
    if(signal_process == "") {cout<<BOLD(FRED("ERROR: can only train a BDT if [signal_process] is specified !"))<<endl; return;}

	mkdir("weightsMVA", 0777);
    mkdir("weightsMVA/BDT", 0777);
    mkdir(("weightsMVA/BDT/"+lumiName).Data(), 0777);
    mkdir(("weightsMVA/BDT/"+lumiName+"/"+signal_process).Data(), 0777);

	usleep(1000000); //Pause for 1s (in microsec)

//--------------------------------
//  ####  #    # #####  ####
// #    # #    #   #   #
// #      #    #   #    ####
// #      #    #   #        #
// #    # #    #   #   #    #
//  ####   ####    #    ####
//--------------------------------------------

	//---Apply additional cuts on the signal and background samples (can be different)
	TCut mycut = "";
	TString tmp = "";

	//--- CHOOSE TRAINING EVENTS <--> cut on corresponding category
	TString cat_tmp = "";
	cat_tmp = Get_Category_Boolean_Name(region); //NB: does not work for NPL samples (different flag)... would need workaround

	//Even if ask templates in the SR, need to use training (looser) category for training !
	// if(cat_tmp.Contains("_SR") )
	// {
	// 	int i = cat_tmp.Index("_SR"); //Find index of substrin g
	// 	cat_tmp.Remove(i); //Remove substring
	// }
    // tmp+= cat_tmp + "==1";

    if(cat_tmp == "") {cat_tmp = "1";}

    tmp = cat_tmp;
    // tmp = cat_tmp + " && !TMath::IsNaN(mTW) && !std::isinf(abs(TopZsystem_M))"; //TMP fix... because trained on 0jet events ?

	//--- Define additionnal cuts
	for(int ivar=0; ivar<v_cut_name.size(); ivar++)
	{
		if(v_cut_def[ivar] != "")
		{
			if(tmp != "") {tmp+= " && ";}

			if(!v_cut_def[ivar].Contains("&&") && !v_cut_def[ivar].Contains("||")) {tmp+= v_cut_name[ivar] + v_cut_def[ivar];} //If cut contains only 1 condition
			else if(v_cut_def[ivar].Contains("&&") && v_cut_def[ivar].Contains("||")) {cout<<BOLD(FRED("ERROR ! Wrong cut definition !"))<<endl;}
			else if(v_cut_def[ivar].Contains("&&") )//If '&&' in the cut, break it in 2
			{
				tmp+= v_cut_name[ivar] + Break_Cuts_In_Two(v_cut_def[ivar]).first;
				tmp+= " && ";
				tmp+= v_cut_name[ivar] + Break_Cuts_In_Two(v_cut_def[ivar]).second;
			}
			else if(v_cut_def[ivar].Contains("||") )//If '||' in the cut, break it in 2
			{
				tmp+= v_cut_name[ivar] + Break_Cuts_In_Two(v_cut_def[ivar]).first;
				tmp+= " || ";
				tmp+= v_cut_name[ivar] + Break_Cuts_In_Two(v_cut_def[ivar]).second;
			}
		}
	}

	bool split_by_leptonChan = false;
	if(split_by_leptonChan && (channel != "all" && channel != ""))
	{
		if(channel == "uuu" || channel == "uu")	{mycut = "channel==0";}
		else if(channel == "uue" || channel == "ue") {mycut = "channel==1";}
		else if(channel == "eeu" || channel == "ee") {mycut = "channel==2";}
		else if(channel == "eee") {mycut = "channel==3";}
		else {cout << "WARNING : wrong channel name while training " << endl;}
	}

	cout<<"-- Will apply the following cut(s) : "<<BOLD(FGRN(""<<tmp<<""))<<endl<<endl<<endl<<endl;
	usleep(2000000); //Pause for 2s (in microsec)

	if(tmp != "") {mycut+= tmp;}

	//--------------------------------------------
	//---------------------------------------------------------------
    // This loads the TMVA libraries
    TMVA::Tools::Instance();

	//Allows to bypass a protection in TMVA::Transplot_extensionionHandler, cf. description in source file:
	// if there are too many input variables, the creation of correlations plots blows up memory and basically kills the TMVA execution --> avoid above critical number (which can be user defined)
	(TMVA::gConfig().GetVariablePlotting()).fMaxNumOfAllowedVariablesForScatterPlots = 300;

	TString weights_dir = "weightsMVA";

    //The TMVA::DataLoader object will hold the training and test data, and is later passed to the TMVA::Factory
	TMVA::DataLoader* myDataLoader = new TMVA::DataLoader(weights_dir); //If no TString given in arg, path of weightdir *in TTree* will be : default/weights/...

	//--- Could modify here the name of local dir. storing the BDT weights (default = "weights")
	//By setting it to "", weight files will be stored directly at the path given to myDataLoader
	//Complete path for weight files is : [path_given_toDataloader]/[fWeightFileDir]
	//Apparently, TMVAGui can't handle nested repos in path given to myDataLoader... so split path in 2 here
	TMVA::gConfig().GetIONames().fWeightFileDir = "BDT/"+lumiName+"/"+signal_process;
	// if(classifier_name == "NN" && NN_type == "TMVA") {TMVA::gConfig().GetIONames().fWeightFileDir = "NN/TMVA/"+lumiName;}

//--------------------------------------------
 // #    #   ##   #####  #   ##   #####  #      ######  ####
 // #    #  #  #  #    # #  #  #  #    # #      #      #
 // #    # #    # #    # # #    # #####  #      #####   ####
 // #    # ###### #####  # ###### #    # #      #           #
 //  #  #  #    # #   #  # #    # #    # #      #      #    #
 //   ##   #    # #    # # #    # #####  ###### ######  ####
//--------------------------------------------

	// Define the input variables that shall be used for the MVA training
	for(int i=0; i<var_list.size(); i++)
	{
		myDataLoader->AddVariable(var_list[i].Data(), 'F');
	}

	//Choose if the cut variables are used in BDT or not
	//Spectator vars are not used for training/evalution, but possible to check their correlations, etc.
    //NB: if a cut requires "var == x", all the selected events will be equal to x, so can't use it as discriminant variable !
	for(int i=0; i<v_cut_name.size(); i++)
	{
		if(v_cut_IsUsedForBDT[i] && !v_cut_def[i].Contains("==")) {myDataLoader->AddVariable(v_cut_name[i].Data(), 'F');}
	}

	double nEvents_sig = 0;
	double nEvents_bkg = 0;


//--------------------------------------------
//                       #
//  ####  #  ####       #  #####  #    #  ####      ####    ##   #    # #####  #      ######  ####
// #      # #    #     #   #    # #   #  #    #    #       #  #  ##  ## #    # #      #      #
//  ####  # #         #    #####  ####   #          ####  #    # # ## # #    # #      #####   ####
//      # # #  ###   #     #    # #  #   #  ###         # ###### #    # #####  #      #           #
// #    # # #    #  #      #    # #   #  #    #    #    # #    # #    # #      #      #      #    #
//  ####  #  ####  #       #####  #    #  ####      ####  #    # #    # #      ###### ######  ####
//--------------------------------------------

	//--- Only select few samples for training
	std::vector<TFile*> files_to_close;

    for(int iyear=0; iyear<v_lumiYears.size(); iyear++)
    {
    	for(int isample=0; isample<sample_list.size(); isample++)
        {
            TString samplename_tmp = sample_list[isample];

            TString mycut_string_tmp = mycut.GetTitle();
            if(samplename_tmp.Contains("NPL", TString::kIgnoreCase) || samplename_tmp.Contains("DY", TString::kIgnoreCase) || samplename_tmp.Contains("ttbar", TString::kIgnoreCase)) {mycut_string_tmp = mycut_string_tmp.ReplaceAll("_SR", "_SRFake"); mycut_string_tmp = mycut_string_tmp.ReplaceAll("_CR", "_CRFake");} //Modify flag for NPL samples only
            TCut mycut_tmp = mycut_string_tmp.Data();

            //-- Protections
            if(sample_list[isample] == "DATA") {continue;} //Don't use data for training

            //Can hardcode here the backgrounds against which to train, instead of considering full list of samples
            // if(signal_process == "tZq")
            // {
            //     if(!samplename_tmp.Contains("tZq") && !samplename_tmp.EndsWith("ttZ") && !samplename_tmp.Contains("ttH") && samplename_tmp.Contains("ttW") && samplename_tmp.Contains("WZ") && samplename_tmp.Contains("ZZ4l") && samplename_tmp.Contains("TTbar_DiLep") ) {continue;}
            // }

            //-- TMP sample list
            if(samplename_tmp != "tZq" && samplename_tmp != "ttZ" && samplename_tmp != "ttH" && samplename_tmp != "ttW" && samplename_tmp != "WZ" && samplename_tmp != "ZZ4l" && samplename_tmp != "TTbar_DiLep" ) {continue;}

    		cout<<endl<<"-- Sample : "<<sample_list[isample]<<endl;

            // --- Register the training and test trees
            TString inputfile = dir_ntuples + v_lumiYears[iyear] + "/" + sample_list[isample] + ".root";


//--------------------------------------------
// ##### ##### #####  ###### ######  ####
//   #     #   #    # #      #      #
//   #     #   #    # #####  #####   ####
//   #     #   #####  #      #           #
//   #     #   #   #  #      #      #    #
//   #     #   #    # ###### ######  ####
//--------------------------------------------

    	    TFile *file_input = NULL, *file_input_train = NULL, *file_input_test = NULL;
    		TTree *tree = NULL, *tree_train = NULL, *tree_test = NULL;

            file_input = TFile::Open(inputfile);
            if(!file_input) {cout<<BOLD(FRED(<<inputfile<<" not found!"))<<endl; continue;}
            files_to_close.push_back(file_input); //store pointer to file, to close it after its events will have been read for training

            tree = (TTree*) file_input->Get(nominal_tree_name);
            if(tree==0) {cout<<BOLD(FRED("ERROR :"))<<" file "<<inputfile<<" --> *tree = 0 !"<<endl; continue;}
            else if(tree->GetEntries()==0) {cout<<"Empty ! "<<endl; continue;}
            else {cout<<FMAG("=== Opened file : ")<<inputfile<<endl<<endl;}

            //-- Global event weights per tree (see below for setting event-wise weights)
            Double_t signalWeight     = 1.0;
            Double_t backgroundWeight = 1.0;

            //-- Choose between absolute/relative weights for training
    		if(samplename_tmp.Contains(signal_process) )
    		{
                nEvents_sig+= tree->GetEntries(mycut); myDataLoader->AddSignalTree(tree, signalWeight);

    			if(use_relative_weights)
    			{
                    // TString weightExp = "weight";
                    TString weightExp = "eventWeight*eventMCFactor";
    				myDataLoader->SetSignalWeightExpression(weightExp);
    				cout<<"Signal sample : "<<samplename_tmp<<" / Weight expression : "<<weightExp<<endl<<endl;
    			}
    			else
    			{
                    // TString weightExp = "fabs(weight)";
                    TString weightExp = "fabs(eventWeight*eventMCFactor)";
    				myDataLoader->SetSignalWeightExpression(weightExp);
    				cout<<"Signal sample : "<<samplename_tmp<<" / Weight expression : "<<weightExp<<endl<<endl;
    			}
    		}
    		else
    		{
                nEvents_bkg+= tree->GetEntries(mycut); myDataLoader->AddBackgroundTree(tree, backgroundWeight);

                if(use_relative_weights)
    			{
                    // TString weightExp = "weight";
                    TString weightExp = "eventWeight*eventMCFactor";
    				myDataLoader->SetBackgroundWeightExpression(weightExp);
    				cout<<"Background sample : "<<samplename_tmp<<" / Weight expression : "<<weightExp<<endl;
    			}
    			else
    			{
                    // TString weightExp = "fabs(weight)";
                    TString weightExp = "fabs(eventWeight*eventMCFactor)";
    				myDataLoader->SetBackgroundWeightExpression(weightExp);
    				cout<<"Background sample : "<<samplename_tmp<<" / Weight expression : "<<weightExp<<endl;
    			}
    		}

        } //samples loop
    } //year loop


//--------------------------------
// #####  #####  ###### #####    ##   #####  ######    ##### #####  ###### ######  ####
// #    # #    # #      #    #  #  #  #    # #           #   #    # #      #      #
// #    # #    # #####  #    # #    # #    # #####       #   #    # #####  #####   ####
// #####  #####  #      #####  ###### #####  #           #   #####  #      #           #
// #      #   #  #      #      #    # #   #  #           #   #   #  #      #      #    #
// #      #    # ###### #      #    # #    # ######      #   #    # ###### ######  ####
//--------------------------------------------

	// if(mycuts != mycutb) {cout<<__LINE__<<FRED("PROBLEM : cuts are different for signal and background ! If this is normal, modify code -- Abort")<<endl; delete myDataLoader; return;}

	// If nTraining_Events=nTesting_Events="0", half of the events in the tree are used for training, and the other half for testing
	//NB : when converting nEvents to TString, make sure to ask for sufficient precision !

	//-- Choose dataset splitting
	TString nTraining_Events_sig = "", nTraining_Events_bkg = "", nTesting_Events_sig = "", nTesting_Events_bkg = "";

    int nTrainEvSig = ((nmaxEv == -1 || nEvents_sig * trainingEv_proportion     < nmaxEv)) ? nEvents_sig * trainingEv_proportion     : nmaxEv;
    int nTrainEvBkg = ((nmaxEv == -1 || nEvents_bkg * trainingEv_proportion     < nmaxEv)) ? nEvents_bkg * trainingEv_proportion     : nmaxEv;
    int nTestEvSig  = ((nmaxEv == -1 || nEvents_sig * (1-trainingEv_proportion) < nmaxEv)) ? nEvents_sig * (1-trainingEv_proportion) : nmaxEv;
    int nTestEvBkg  = ((nmaxEv == -1 || nEvents_bkg * (1-trainingEv_proportion) < nmaxEv)) ? nEvents_bkg * (1-trainingEv_proportion) : nmaxEv;

    nTraining_Events_sig = Convert_Number_To_TString(nTrainEvSig, 12);
    nTraining_Events_bkg = Convert_Number_To_TString(nTrainEvBkg, 12);
    nTesting_Events_sig = Convert_Number_To_TString(nTestEvSig, 12);
    nTesting_Events_bkg = Convert_Number_To_TString(nTestEvBkg, 12);

	cout<<endl<<endl<<endl<<BOLD(FBLU("==================================="))<<endl;
	cout<<BOLD(FBLU("-- Requesting "<<nTraining_Events_sig<<" Training events [SIGNAL]"))<<endl;
	cout<<BOLD(FBLU("-- Requesting "<<nTesting_Events_sig<<" Testing events [SIGNAL]"))<<endl<<endl;
	cout<<BOLD(FBLU("-- Requesting "<<nTraining_Events_bkg<<" Training events [BACKGROUND]"))<<endl;
	cout<<BOLD(FBLU("-- Requesting "<<nTesting_Events_bkg<<" Testing events [BACKGROUND]"))<<endl;
	cout<<BOLD(FBLU("==================================="))<<endl<<endl<<endl<<endl;

    myDataLoader->PrepareTrainingAndTestTree(mycut, mycut, "nTrain_Signal="+nTraining_Events_sig+":nTrain_Background="+nTraining_Events_bkg+":nTest_Signal="+nTesting_Events_sig+":nTest_Background="+nTesting_Events_bkg+":SplitMode=Random:EqualTrainSample:!V"); //EqualTrainSample = ?

	//-- for quick testing -- few events
	// myDataLoader->PrepareTrainingAndTestTree(mycut, mycut, "nTrain_Signal=10:nTrain_Background=10:nTest_Signal=10:nTest_Background=10:SplitMode=Random:NormMode=NumEvents:!V");

    //Output rootfile containing TMVAGui infos, ROCS, ... for control
    TString basename = classifier_name;
    if(channel != "") {basename+= "_" + channel;}
    if(region != "") {basename+= "_" + region;}
    basename+= "_" + lumiName + this->filename_suffix;
    TString output_file_name = "outputs/" + basename + ".root";

	TFile* output_file = TFile::Open(output_file_name, "RECREATE");

    //The TMVA::Factory handles the training/testing/evaluation phases
	TMVA::Factory* factory = new TMVA::Factory(classifier_name, output_file, "V:!Silent:Color:DrawProgressBar:Correlations=True:AnalysisType=Classification");

	//So that the output weights are labelled differently
	TString method_title = signal_process;
	if(channel != "") {method_title = "_" + channel;}


//--------------------------------------------
//  ####  #####  ##### #  ####  #    #  ####     #    # ###### ##### #    #  ####  #####
// #    # #    #   #   # #    # ##   # #         ##  ## #        #   #    # #    # #    #
// #    # #    #   #   # #    # # #  #  ####     # ## # #####    #   ###### #    # #    #
// #    # #####    #   # #    # #  # #      #    #    # #        #   #    # #    # #    #
// #    # #        #   # #    # #   ## #    #    #    # #        #   #    # #    # #    #
//  ####  #        #   #  ####  #    #  ####     #    # ######   #   #    #  ####  #####
//--------------------------------------------

    TString method_options = "";

    //~ttH2017
    // method_options= "!H:!V:NTrees=200:BoostType=Grad:Shrinkage=0.10:!UseBaggedBoost:nCuts=200:MinNodeSize=5%:NNodesMax=5:MaxDepth=8:NegWeightTreatment=PairNegWeightsGlobal:CreateMVAPdfs:DoBoostMonitor=True";

    //tHq2017
    // method_options = "!H:!V:NTrees=200:nCuts=40:MaxDepth=4:BoostType=Grad:Shrinkage=0.10:!UseBaggedGrad:NegWeightTreatment=PairNegWeightsGlobal:CreateMVAPdfs";

    //Testing
    // method_options = "!H:!V:NTrees=200:nCuts=40:MaxDepth=4:MinNodeSize=5%:UseBaggedBoost=True:BaggedSampleFraction=0.5:BoostType=Grad:Shrinkage=0.10";

    //Quick test
    // method_options = "!H:!V:NTrees=20:nCuts=5:MaxDepth=1:MinNodeSize=10%:UseBaggedBoost=True:BaggedSampleFraction=0.5:BoostType=Grad:Shrinkage=0.10";


//--------------------------------------------
 // ##### #####    ##   # #    #       ##### ######  ####  #####       ###### #    #   ##   #
 //   #   #    #  #  #  # ##   #         #   #      #        #         #      #    #  #  #  #
 //   #   #    # #    # # # #  #         #   #####   ####    #         #####  #    # #    # #
 //   #   #####  ###### # #  # #         #   #           #   #         #      #    # ###### #
 //   #   #   #  #    # # #   ##         #   #      #    #   #         #       #  #  #    # #
 //   #   #    # #    # # #    #         #   ######  ####    #         ######   ##   #    # ######
//--------------------------------------------

	if(classifier_name == "BDT") {factory->BookMethod(myDataLoader, TMVA::Types::kBDT, method_title, method_options);} //Book BDT -- pass dataLoader and options as arg

	output_file->cd();

    //-- Save TMVA ranking info
    mkdir("outputs/Rankings", 0777);
    mkdir(("outputs/Rankings/"+lumiName).Data(), 0777); //Dir. containing variable ranking infos
	TString ranking_file_path = "outputs/Rankings/"+lumiName+"/rank_"+basename+".txt";
    cout<<endl<<endl<<endl<<FBLU("NB : Temporarily redirecting standard output to file '"<<ranking_file_path<<"' in order to save Ranking Info !!")<<endl<<endl<<endl;
	std::ofstream out("ranking_info_tmp.txt"); //Temporary name
    out<<endl;
	std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
	std::cout.rdbuf(out.rdbuf()); //redirect std::cout to text file --> Ranking info will be saved !

    // Train MVAs using the set of training events
    factory->TrainAllMethods();

	std::cout.rdbuf(coutbuf); //reset to standard output again

	//-- NB : Test & Evaluation recap in the output files
    factory->TestAllMethods(); // ---- Evaluate all MVAs using the set of test events
    factory->EvaluateAllMethods(); // ----- Evaluate and compare performance of all configured MVAs

	//Could retrieve ROC graph directly
	// TMultiGraph* rocgraph = f.GetROCCurveAsMultiGraph("<datasetname>");

    // --------------------------------------------------------------
    // Save the output
    output_file->Close();
    std::cout << "==> Wrote root file: " << output_file_name << std::endl;
    std::cout << "==> TMVA is done!" << std::endl;

    MoveFile("./ranking_info_tmp.txt", ranking_file_path);
    Extract_Ranking_Info(ranking_file_path, channel); //Extract only ranking info from TMVA output

	for(unsigned int i=0; i<files_to_close.size(); i++) {files_to_close[i]->Close(); delete files_to_close[i];}

	delete myDataLoader; myDataLoader = NULL;
	delete factory; factory = NULL;
	output_file->Close(); output_file = NULL;

	return;
}
































//---------------------------------------------------------------------------
//  ######  ########  ########    ###    ######## ########       ######## ######## ##     ## ########  ##          ###    ######## ########  ######
// ##    ## ##     ## ##         ## ##      ##    ##                ##    ##       ###   ### ##     ## ##         ## ##      ##    ##       ##    ##
// ##       ##     ## ##        ##   ##     ##    ##                ##    ##       #### #### ##     ## ##        ##   ##     ##    ##       ##
// ##       ########  ######   ##     ##    ##    ######            ##    ######   ## ### ## ########  ##       ##     ##    ##    ######    ######
// ##       ##   ##   ##       #########    ##    ##                ##    ##       ##     ## ##        ##       #########    ##    ##             ##
// ##    ## ##    ##  ##       ##     ##    ##    ##                ##    ##       ##     ## ##        ##       ##     ##    ##    ##       ##    ##
//  ######  ##     ## ######## ##     ##    ##    ########          ##    ######## ##     ## ##        ######## ##     ##    ##    ########  ######
//---------------------------------------------------------------------------

void VBFgamma_analysis::Produce_Templates(TString template_name, bool makeHisto_inputVars, bool plot_onlyMaxNodeEvents)
{
//--- OPTIONS --------------------------------
//--------------------------------------------
    int nentries_max = -1; //-1 <-> use all entries; N <-> use at most N entries per sample (for testing)

    bool noSysts_inputVars = true; //true <-> don't compute syst weights for histos of input variables (not worth the CPU-time)

//--------------------------------------------
//--------------------------------------------

    if(template_name == "" && classifier_name != "BDT" && classifier_name != "NN") {cout<<BOLD(FRED("Error : classifier_name value ["<<classifier_name<<"] not supported !"))<<endl; return;}
    if(template_name=="") {template_name = classifier_name;}

    cout<<endl<<BYEL("                          ")<<endl<<endl;
	if(makeHisto_inputVars) {cout<<FYEL("--- Producing Input variables histograms ---")<<endl;}
	else {cout<<FYEL("--- Producing "<<template_name<<" Templates ---")<<endl;}
    cout<<endl<<BYEL("                          ")<<endl<<endl;


//  ####  ###### ##### #    # #####
// #      #        #   #    # #    #
//  ####  #####    #   #    # #    #
//      # #        #   #    # #####
// #    # #        #   #    # #
//  ####  ######   #    ####  #

	TH1::SetDefaultSumw2();

    TString restore_classifier_name = classifier_name;
    if(makeHisto_inputVars) {classifier_name = "";} //For naming conventions

    //-- Define category of interest
    TString cat_tmp = region;

    //-- Hard-coded: determine whether we are producing EFT templates using a 'predefined strategy' (cf. definitions in main --> different strategies to apply automatic cuts)
    bool use_maxNode_events = false;

    //-- Don't make systematics shifted histos for input vars (too long)
    //Force removal of systematics ; restore values at end of this func
    vector<TString> restore_syst_list = syst_list;
    vector<TString> restore_systTree_list = systTree_list;
    if(makeHisto_inputVars && noSysts_inputVars)
    {
        syst_list.resize(1);
        syst_list[0] = "";
        systTree_list.resize(1);
        systTree_list[0] = "";
    }
    else
    {
        //Must allocate memory to these arrays, which will hold the values of the systematics variations weights
        //NB : the sizes of the arrays are hardcoded, depends on Potato implementation (could also set same large size for all)
        array_PU = new double[2];
        array_prefiringWeight = new double[2];
        array_Btag = new double[16];
        array_jetPileupID = new double[4];
        // array_fakeFactor = new double[2]; //David's FR
        array_fakeFactor = new double[12]; //ttH FR
        array_ME = new double[6];
        array_alphaS = new double[2];
        array_PDFtotal = new double[2];
        array_partonShower = new double[4];
        array_LepEffLoose_mu = new double[2];
        array_LepEffTight_mu = new double[2];
        array_LepEffLoose_el = new double[2];
        array_LepEffTight_el = new double[2];
    }

    if(!makeHisto_inputVars)
    {
        if(classifier_name == "BDT") //BDT //NB: only consider 1 single list of variables (although training weight file may differ between years, read later)
        {
            //NB : TMVA requires floats, and nothing else, to ensure reproducibility of results (training done with floats) => Need to recast e.g. doubles as floats //See: https://sourceforge.net/p/tmva/mailman/message/836453/
            reader = new TMVA::Reader("!Color:!Silent");

            // Name & adress of local variables which carry the updated input values during the event loop
            // NB: the variable names MUST corresponds in name and type to those given in the weight file(s) used -- same order
            // NB: if booking 2 BDTs, must make sure that they use the same input variables... or else, find some way to make it work in the code)
            for(int i=0; i<var_list.size(); i++)
            {
                //cout<<"Added variable "<<var_list[i]<<endl;
                var_list_pfloats[i] = new float; reader->AddVariable(var_list[i], var_list_pfloats[i]);
            }
            for(int i=0; i<v_cut_name.size(); i++)
            {
                if(v_cut_IsUsedForBDT[i] && !v_cut_def[i].Contains("==")) {reader->AddVariable(v_cut_name[i], &v_cut_float[i]);}
            }
        }
    }

    //-- Default template binning (modified in 'Get_Template_Range()')
    nbins = 15;
    float xmin = -1, xmax = 1; //BDT: [-1,1]

    //-- Define ranges of jet/bjets multiplicities -- for 'categ' templates only (modified in 'Get_Template_Range()')
    int nbjets_min = 1, nbjets_max=2, njets_min=2, njets_max=6;

    //-- Create output file //NB: the same conventions must be enforced in function 'Get_HistoFile_InputPath' (to automatically find/read this output file later)
    TString output_file_name = "";
    if(makeHisto_inputVars) {output_file_name = (TString) "outputs/ControlHistograms" + (region == ""? "":"_"+region) + "_" + lumiName + filename_suffix +".root";}
    else {output_file_name = (TString) "outputs/Templates" + (template_name == ""? "":"_"+template_name) + "_" + lumiName + filename_suffix + ".root";}
    TFile* file_output = TFile::Open(output_file_name, "RECREATE");

    TFile* file_input = NULL;

    int nhistos = 0; //Count nof histos written to the output file

    vector<vector<vector<vector<int>>>> v4_nEntries_year_proc_var_bin(v_lumiYears.size()); //Count nof actual entries (events) entering each bin of each nominal histo (info not stored in TH1Fs)


// #    #      ##      #    #    #
// ##  ##     #  #     #    ##   #
// # ## #    #    #    #    # #  #
// #    #    ######    #    #  # #
// #    #    #    #    #    #   ##
// #    #    #    #    #    #    #

// #       ####   ####  #####   ####
// #      #    # #    # #    # #
// #      #    # #    # #    #  ####
// #      #    # #    # #####       #
// #      #    # #    # #      #    #
// ######  ####   ####  #       ####

	// cout<<endl<<ITAL("-- Ntuples directory : "<<dir_ntuples<<"")<<endl<<endl;

    // float tmp_compare = 0;
    float total_nentries_toRead = Count_Total_Nof_Entries(dir_ntuples, nominal_tree_name, sample_list, systTree_list, v_cut_name, v_cut_def, v_lumiYears, makeHisto_inputVars, noSysts_inputVars);

    cout<<endl<<FBLU(OVERLINE("                           "))<<endl;
    cout<<FBLU(BOLD("...Will READ (not necessarily process) a total of "<<std::setprecision(12)<<total_nentries_toRead<<" entries..."))<<endl;
    cout<<FBLU(UNDL("                           "))<<endl<<endl<<endl;

    //-- Draw progress bar
    bool draw_progress_bar = false;
    if(total_nentries_toRead < 200000) {draw_progress_bar = false;}
    Int_t ibar = 0; //event counter
    TMVA::Timer timer(total_nentries_toRead, "", true);
    TMVA::gConfig().SetDrawProgressBar(1);
    TMVA::gConfig().SetUseColor(1);

    bool MVA_alreadyLoaded = false; //If reading same MVA for multiple years, need to load weights only once
    TString BDT_method_name = "BDT"; //Need to store BDT method name for evaluation

    //-- YEARS LOOP
    vector<TString> total_var_list; vector<float> total_var_floats; vector<float*> total_var_pfloats;
    for(int iyear=0; iyear<v_lumiYears.size(); iyear++)
    {
        cout<<endl<<UNDL(FMAG("=== YEAR : "<<v_lumiYears[iyear]<<""))<<endl<<endl;

        v4_nEntries_year_proc_var_bin[iyear].resize(sample_list.size());


// #    # #    #   ##
// ##  ## #    #  #  #
// # ## # #    # #    #
// #    # #    # ######
// #    #  #  #  #    #
// #    #   ##   #    #

        int inode_clfy2 = 0; //When reading 2 NNs, may either need to read first (NN-EFTs) or second (NN-SM) node of clfy2 //Obsolete?

        //-- BDT
        if(!makeHisto_inputVars && (template_name == "BDT" || template_name.Contains("NN")) && (use_specificMVA_eachYear || !MVA_alreadyLoaded)) //Read MVA input file
        {
            MVA_alreadyLoaded = true;
            TString MVA_input_path = Get_MVAFile_InputPath(template_name, v_lumiYears[iyear], use_specificMVA_eachYear, false);
            if(MVA_input_path == "") {return;} //MVA file not found

            if(template_name == "BDT") //Book TMVA reader from .xml weight file
            {
                reader->BookMVA(BDT_method_name, MVA_input_path);
            }
            else if(template_name.Contains("NN")) //Read NN info and weights
            {
                //-- Get path of NN info text file --> Read list of input variables (and more)
                var_list_NN.resize(0); NN_iMaxNode = -1; NN_inputLayerName = ""; NN_outputLayerName = ""; NN_nNodes = -1; minmax_bounds.clear();
                TString NNinfo_input_path = Get_MVAFile_InputPath(template_name, v_lumiYears[iyear], use_specificMVA_eachYear, true);
                if(NNinfo_input_path == "") {return;} //MVA file not found

                if(Extract_Values_From_NNInfoFile(NNinfo_input_path, var_list_NN, v_NN_nodeLabels, NN_inputLayerName, NN_outputLayerName, NN_iMaxNode, NN_nNodes, minmax_bounds)) {clfy1 = new TFModel(MVA_input_path.Data(), var_list_NN.size(), NN_inputLayerName.Data(), NN_nNodes, NN_outputLayerName.Data());} //Load neural network model
                else {return;} //Error: missing NN infos

                var_list = var_list_NN; //Use NN input features
                // var_list_floats.resize(var_list.size());
                var_list_pfloats.resize(var_list.size()); for(int ivar=0; ivar<var_list_pfloats.size(); ivar++) {var_list_pfloats[ivar] = new float;} //Allocate necessary memory
            } //NN
        } //Read MVA


 // #    #   ##   #####  #   ##   #####  #      ######  ####
 // #    #  #  #  #    # #  #  #  #    # #      #      #
 // #    # #    # #    # # #    # #####  #      #####   ####
 // #    # ###### #####  # ###### #    # #      #           #
 //  #  #  #    # #   #  # #    # #    # #      #      #    #
 //   ##   #    # #    # # #    # #####  ###### ######  ####

        //-- Define the list of variables there, because may need to first read a MVA info file (year-dependent) //Define list only for first year, assume it's identical for all years
        //-- NB: for NN templates, list of variables may be hard-coded here based on the info found in the MVA file --> Make sure the correct MVA files are found
        if(!iyear)
        {
        	//-- Vector of variables --> Loop
        	if(makeHisto_inputVars) //Input variables
        	{
        		for(int i=0; i<var_list.size(); i++) {total_var_list.push_back(var_list[i]);}
        		for(int i=0; i<v_add_var_names.size(); i++) {total_var_list.push_back(v_add_var_names[i]);}
        	}
        	else //Templates
        	{
                //-- Could use this helper function to fill the (hardcoded) list of variables to consider, based on user-options
                Fill_Variables_List(total_var_list, template_name);
        	} //Templates

            // total_var_floats.resize(total_var_list.size());
            total_var_pfloats.resize(total_var_list.size()); for(int ivar=0; ivar<total_var_pfloats.size(); ivar++) {total_var_pfloats[ivar] = new float;} //Reserve necessary memory
        }
        // for(int ivar=0; ivar<total_var_list.size(); ivar++) {cout<<"total_var_list[ivar] "<<total_var_list[ivar]<<endl;}


//  ####    ##   #    # #####  #      ######    #       ####   ####  #####
// #       #  #  ##  ## #    # #      #         #      #    # #    # #    #
//  ####  #    # # ## # #    # #      #####     #      #    # #    # #    #
//      # ###### #    # #####  #      #         #      #    # #    # #####
// #    # #    # #    # #      #      #         #      #    # #    # #
//  ####  #    # #    # #      ###### ######    ######  ####   ####  #

    	//-- SAMPLE LOOP
    	for(int isample=0; isample<sample_list.size(); isample++)
    	{
            //-- Input TFile path
            TString inputfile = dir_ntuples + v_lumiYears[iyear] + "/" + sample_list[isample] + ".root";

            cout<<endl<<endl<<UNDL(FBLU("Sample : "<<sample_list[isample]<<""))<<endl;
            cout<<DIM("(path: "<<inputfile<<")")<<endl;

            bool isFake = false; //Identify NPL samples (different flags)
            if(sample_list[isample].Contains("NPL", TString::kIgnoreCase) || sample_list[isample].Contains("DY", TString::kIgnoreCase) || sample_list[isample].Contains("ttbar", TString::kIgnoreCase)) {isFake = true;}
            bool isData = false;
            if(sample_list[isample] == "DATA") {isData = true;}

            //-- For current year/sample, store the nominal integral(s) per channel and variable (for nominal TTree only) --> Can use it to rescale some systematics later (normalize to nominal)
            // vector<vector<float>> v_storeNominalIntegral_chan_var(channel_list.size());
            // for(int ichan=0; ichan<v_storeNominalIntegral_chan_var.size(); ichan++) {v_storeNominalIntegral_chan_var[ichan].resize(total_var_list.size());}

            //-- Store nominal bin contents/errors for all channel/variable/histo bin --> for systematics, can then adjust the bin content to avoid that either nominal or variation is set to ~0, but not the other (causes combine failures)
            //-- For current year/sample, store the nominal integral(s) per channel and variable (for nominal TTree only) --> Can use it to rescale some systematics later (normalize to nominal)
            vector<vector<vector<float>>> v_storeNominalBinContents_chan_var_bin(channel_list.size()), v_storeNominalBinErrors_chan_var_bin(channel_list.size());
            for(int ichan=0; ichan<v_storeNominalBinContents_chan_var_bin.size(); ichan++) {v_storeNominalBinContents_chan_var_bin[ichan].resize(total_var_list.size()); v_storeNominalBinErrors_chan_var_bin[ichan].resize(total_var_list.size());}

            //-- Apply ad-hoc scale factor to private ttZ sample so that SM yield matches that of central ttZ sample
            //-- NB: should correct the xsec at potato level instead !
            // float SF_SMEFT_ttZ = 0.86;

    		// cout<<"inputfile "<<inputfile<<endl;
    		if(!Check_File_Existence(inputfile))
    		{
    			cout<<endl<<"File "<<inputfile<<FRED(" not found!")<<endl;
    			continue;
    		}

    		//-- Loop on TTrees : first empty element corresponds to nominal TTree ; additional TTrees may correspond to JES/JER TTrees (defined in main)
    		//NB : only nominal TTree contains systematic weights ; others only contain the nominal weight (but variables have different values)
            for(int itree=0; itree<systTree_list.size(); itree++)
    		{
                if(systTree_list[itree] != "" && systTree_list[itree] != nominal_tree_name && (sample_list[isample] == "DATA" || sample_list[isample] == "DY" || sample_list[isample].Contains("NPL") || sample_list[isample].Contains("TTbar") ) ) {continue;} //Special cases for which only nominal tree must be considered

                TString treename_tmp = systTree_list[itree];
    			if(treename_tmp == "") {treename_tmp = nominal_tree_name;}

                //-- Re-open the input file for each individual TTree (easier to cleanly delete all associated objects, prevents conflicts)
                file_input = TFile::Open(inputfile, "READ");

                //-- Read TTree
                TTree* tree = (TTree*) file_input->Get(treename_tmp);
                // tree->SetDirectory(0); //Need to dis-associated the TTree from the TFile, else may get (rare) crashes

    			if(!tree || !tree->GetEntries())
    			{
    				cout<<BOLD(FRED("ERROR : tree '"<<treename_tmp<<"' not found in file : "<<inputfile<<" ! Skip !"))<<endl;
                    if(!itree) {break;} //If we did not find the nominal TTree, no need to look for the others
                    continue; //Skip TTree
    			}

                v4_nEntries_year_proc_var_bin[iyear][isample].resize(total_var_list.size());


//   ##   #####  #####  #####  ######  ####   ####  ######  ####
//  #  #  #    # #    # #    # #      #      #      #      #
// #    # #    # #    # #    # #####   ####   ####  #####   ####
// ###### #    # #    # #####  #           #      # #           #
// #    # #    # #    # #   #  #      #    # #    # #      #    #
// #    # #####  #####  #    # ######  ####   ####  ######  ####

    			//Disactivate all un-necessary branches ; below, activate only needed ones
    			tree->SetBranchStatus("*", 0); //disable all branches by default, speed up

                //-- Always keep track of these vars (needed for different templates, syst, etc.)
                float* channel = new float;
                // tree->SetBranchStatus("channel", 1); tree->SetBranchAddress("channel", channel);

                if(makeHisto_inputVars)
    			{
    				for(int i=0; i<total_var_list.size(); i++)
    				{
                        // if(total_var_list[i] == "mTW") {total_var_pfloats[i] = mTW; continue;}
                        // if(total_var_list[i] == "njets") {total_var_pfloats[i] = njets; continue;}
                        // if(total_var_list[i] == "channel") {total_var_pfloats[i] = channel; continue;}

                        tree->SetBranchStatus(total_var_list[i], 1);
                        tree->SetBranchAddress(total_var_list[i], total_var_pfloats[i]);
    				}

    			}
    			else //Templates
    			{
                    if(template_name=="BDT" || template_name.Contains("NN")) //Book input variables in same order as for MVA training //Activate input features needed for MVA evaluation (same as used for training)
                    {
                        for(int i=0; i<var_list.size(); i++)
                        {
                            if(!isample) {cout<<DIM("MVA 1 -- "<<i<<" -- Activate variable '"<<var_list[i]<<"'")<<endl;}

                            // if(var_list[i] == "mTW") {var_list_pfloats[i] = mTW; continue;}
                            // if(var_list[i] == "njets") {var_list_pfloats[i] = njets; continue;}
                            // if(var_list[i] == "channel") {var_list_pfloats[i] = channel; continue;}

                            // cout<<"Activate branch "<<var_list[i]<<"("<<var_list_pfloats[i]<<")"<<endl;
                            tree->SetBranchStatus(var_list[i], 1);
                            tree->SetBranchAddress(var_list[i], var_list_pfloats[i]);
                        }
                    } //MVA templates
                } //Templates

    			for(int i=0; i<v_cut_name.size(); i++)
    			{
                    // cout<<"v_cut_name[i] "<<v_cut_name[i]<<endl;

    				tree->SetBranchStatus(v_cut_name[i], 1);
                    if(v_cut_name[i].BeginsWith("is") || v_cut_name[i].BeginsWith("passed") ) //Categories are encoded into Char_t, not float
    				{
                        if(!isFake) {tree->SetBranchAddress(v_cut_name[i], &v_cut_char[i]);}
                        else {tree->SetBranchStatus(v_cut_name[i]+"Fake", 1); tree->SetBranchAddress(v_cut_name[i]+"Fake", &v_cut_char[i]);} //Fake <-> separate flag
    				}
    				else //All others are floats
    				{
    					tree->SetBranchAddress(v_cut_name[i], &v_cut_float[i]);
    				}
    			}

    			//--- Cut on relevant event selection stored as flag (Char_t)
                //NB: don't 'call' the flag directly, use function Get_Category_Boolean_Name() instead (need different flags for NPL samples)
    			Char_t is_goodCategory = true; //Cut on event category flag
                // vector<Char_t> v_is_goodCategory(4, 1);
                if(cat_tmp != "")
                {
                    TString cat_name = Get_Category_Boolean_Name(cat_tmp, isFake);

                    bool alreadyCutOnFlag = false; //Check if address of category flag is already set and cut on (don't cut again)
                    for(int icut=0; icut<v_cut_name.size(); icut++)
                    {
                        if(cat_name == v_cut_name[icut]) {alreadyCutOnFlag = true;}
                    }

                    if(!alreadyCutOnFlag)
                    {
                        tree->SetBranchStatus(cat_name, 1);
                        tree->SetBranchAddress(cat_name, &is_goodCategory);
                        // cout<<"Will apply cut on flag ["<<cat_name<<"] !"<<endl;
                    }
                }

                //--- Event weights //FIXME
                Int_t vjj_nlumiWeights = 113;
                Float_t vjj_photon_effWgt, vjj_weight, vjj_mu_effWgt, vjj_lumiWeights[vjj_nlumiWeights];
                if(!isData)
                {
                    tree->SetBranchStatus("vjj_photon_effWgt", 1); tree->SetBranchAddress("vjj_photon_effWgt", &vjj_photon_effWgt);
                    tree->SetBranchStatus("vjj_weight", 1); tree->SetBranchAddress("vjj_weight", &vjj_weight);
                    tree->SetBranchStatus("vjj_lumiWeights", 1); tree->SetBranchAddress("vjj_lumiWeights", vjj_lumiWeights);
                    tree->SetBranchStatus("vjj_mu_effWgt", 1); tree->SetBranchAddress("vjj_mu_effWgt", &vjj_mu_effWgt);
                }

                Bool_t vjj_isGood;
                Int_t vjj_fs, vjj_trig;
                Float_t vjj_v_eta, vjj_jj_deta, vjj_jj_m, vjj_v_pt, vjj_lead_pt, vjj_sublead_pt;
                tree->SetBranchStatus("vjj_v_eta", 1); tree->SetBranchAddress("vjj_v_eta", &vjj_v_eta);
                tree->SetBranchStatus("vjj_jj_deta", 1); tree->SetBranchAddress("vjj_jj_deta", &vjj_jj_deta);
                tree->SetBranchStatus("vjj_jj_m", 1); tree->SetBranchAddress("vjj_jj_m", &vjj_jj_m);
                tree->SetBranchStatus("vjj_v_pt", 1); tree->SetBranchAddress("vjj_v_pt", &vjj_v_pt);
                tree->SetBranchStatus("vjj_isGood", 1); tree->SetBranchAddress("vjj_isGood", &vjj_isGood);
                tree->SetBranchStatus("vjj_fs", 1); tree->SetBranchAddress("vjj_fs", &vjj_fs);
                tree->SetBranchStatus("vjj_jj_m", 1); tree->SetBranchAddress("vjj_jj_m", &vjj_jj_m);
                tree->SetBranchStatus("vjj_lead_pt", 1); tree->SetBranchAddress("vjj_lead_pt", &vjj_lead_pt);
                tree->SetBranchStatus("vjj_sublead_pt", 1); tree->SetBranchAddress("vjj_sublead_pt", &vjj_sublead_pt);
                tree->SetBranchStatus("vjj_trig", 1); tree->SetBranchAddress("vjj_trig", &vjj_trig);

                //-- Reserve 1 float for each systematic weight (also for nominal to keep ordering, but not used)
    			vector<Double_t*> v_double_systWeights(syst_list.size(), NULL);
    			for(int isyst=0; isyst<syst_list.size(); isyst++)
    			{
    				//-- Protections : not all syst weights apply to all samples, etc.
    				if(sample_list[isample] == "DATA") {break;}
    				else if(systTree_list[itree] != "" && systTree_list[itree] != nominal_tree_name) {break;} //Syst event weights only stored in nominal TTree
                    else if((sample_list[isample].Contains("NPL") && syst_list[isyst] != "" && !syst_list[isyst].BeginsWith("FR")) || (!sample_list[isample].Contains("NPL") && syst_list[isyst].BeginsWith("FR"))) {continue;} //NPL <-> only fakes sytematics; all others <-> no fakes systematics
                    else if(v_lumiYears[iyear] == "2018" && syst_list[isyst].BeginsWith("prefir") ) {continue;} //no prefire in 2018

                    //Set proper branch address (hard-coded mapping)
                    SetBranchAddress_SystVariationArray(tree, syst_list[isyst], v_double_systWeights, isyst);
    			}


 //                                    #
 // # #    # # #####    ##### #    #  ##
 // # ##   # #   #        #   #    # # #
 // # # #  # #   #        #   ######   #
 // # #  # # #   #        #   #    #   #
 // # #   ## #   #        #   #    #   #
 // # #    # #   #        #   #    # #####

    			//-- Reserve memory for 1 TH1F per category, per systematic, per variable //v3 <-> vec of vec of vec
    			vector<vector<vector<TH1F*>>> v3_histo_chan_syst_var(channel_list.size());

    			for(int ichan=0; ichan<channel_list.size(); ichan++)
    			{
                    //-- Cases for which we only need to store the nominal weight
    				if((channel_list.size() > 1 && channel_list[ichan] == "") || sample_list[isample] == "DATA"  || (systTree_list[itree] != "" && systTree_list[itree] != nominal_tree_name))
                    {
                        v3_histo_chan_syst_var[ichan].resize(1);
                    }

                    //-- Reserve memory for TH1Fs
    				if(sample_list[isample] == "DATA" || (systTree_list[itree] != "" && systTree_list[itree] != nominal_tree_name)) //1 single weight
    				{
                        v3_histo_chan_syst_var[ichan].resize(1); //Cases for which we only need to store the nominal weight
    				}
    				else //Need 1 histo for nominal + 1 histo per systematic
    				{
                        v3_histo_chan_syst_var[ichan].resize(syst_list.size());
    				}

    				//-- Init TH1Fs
    				for(int isyst=0; isyst<v3_histo_chan_syst_var[ichan].size(); isyst++)
    				{
                        // cout<<"syst_list[isyst] "<<syst_list[isyst]<<endl;

                        v3_histo_chan_syst_var[ichan][isyst].resize(total_var_list.size());

    					for(int ivar=0; ivar<total_var_list.size(); ivar++)
    					{
                            // cout<<"total_var_list[ivar] "<<total_var_list[ivar]<<endl;

                            //-- Init to null
                            v3_histo_chan_syst_var[ichan][isyst][ivar] = NULL;

    						if(makeHisto_inputVars) //Kinematic variables
                            {
                                if(!Get_Variable_Range(total_var_list[ivar], nbins, xmin, xmax)) {cout<<FRED("Unknown variable name : "<<total_var_list[ivar]<<"! (include it in function Get_Variable_Range() in Helper.cxx)")<<endl; continue;} //Get binning for this input variable
                                if(region.Contains("ttz4l", TString::kIgnoreCase) && !total_var_list[ivar].Contains("njets") && !total_var_list[ivar].Contains("nbjets") && !total_var_list[ivar].Contains("channel")) {nbins = (int) (nbins / 2);} //Need tighter binning in low-stat regions
                            }
                            else //Templates
                            {
                                Get_Template_Range(nbins, xmin, xmax, total_var_list[ivar], plot_onlyMaxNodeEvents, nbjets_min, nbjets_max, njets_min, njets_max, minmax_bounds);
                            }
                            // cout<<"nbins "<<nbins<<" / xmin "<<xmin<<" / xmax "<<xmax<<endl;

                            v3_histo_chan_syst_var[ichan][isyst][ivar] = new TH1F("", "", nbins, xmin, xmax);

                            v4_nEntries_year_proc_var_bin[iyear][isample][ivar].resize(nbins);
    					} //var
    				} //syst
    			} //chan

                //-- For private EFT samples, get and store index of SM reweight
                int idx_sm = 0; //Hardcoded: SM index = 0 !
                int nweights = 25; //For my SMEFT samples, only need >= 21 EFT weights for parameterization


// ###### #    # ###### #    # #####    #       ####   ####  #####
// #      #    # #      ##   #   #      #      #    # #    # #    #
// #####  #    # #####  # #  #   #      #      #    # #    # #    #
// #      #    # #      #  # #   #      #      #    # #    # #####
// #       #  #  #      #   ##   #      #      #    # #    # #
// ######   ##   ###### #    #   #      ######  ####   ####  #

    			// cout<<"* Tree '"<<systTree_list[itree]<<"' :"<<endl;

    			// int nentries = 1000;
    			int nentries = tree->GetEntries();
                if(nentries_max > 0 && nentries > nentries_max)
                {
                    nentries = nentries_max;
                    cout<<FRED("Warning: you have set the limit nentries_max = "<<nentries_max<<"")<<endl;
                }

                // if(!draw_progress_bar) {cout<<endl<< "--- "<<sample_list[isample]<<" : Processing: " << tree->GetEntries() << " events" << std::endl;}

                cout<<"--- Tree "<<treename_tmp<<" // Reading " << nentries << " events" <<endl;
                // cout<< "--- "<<sample_list[isample]<<" : Processing " << nentries << " events" << std::endl;

    			for(int ientry=0; ientry<nentries; ientry++)
    			{
    				// cout<<FGRN("ientry "<<ientry<<"")<<endl;
                    if(ientry%5000==0) {cout<<DIM("Entry "<<ientry<<"")<<endl;} //Very slow, print progress

                    // std::fill(var_list_floats.begin(), var_list_floats.end(), 0); //Reset vectors reading inputs to 0

    				tree->GetEntry(ientry);

    				// if(std::isnan(eventWeight*eventMCFactor) || std::isinf(eventWeight*eventMCFactor))
    				// {
    				// 	cout<<BOLD(FRED("* Found event with eventWeight*eventMCFactor = "<<eventWeight*eventMCFactor<<" ; remove it..."))<<endl; continue;
    				// }

//---- APPLY EVENT CUTS HERE ------------------------- //FIXME

                    float ptCut = 200, fs = 22, mjj=200;
                    bool lowPtCut= (abs(vjj_v_eta)<1.442 && abs(vjj_jj_deta) > 3.0 && vjj_jj_m > 500 && vjj_v_pt > 75);
                    bool generalCuts = ((vjj_isGood) && (vjj_fs==fs) && (vjj_jj_m>mjj) && (vjj_lead_pt>50) && (vjj_sublead_pt>50));
                    bool pass = ( (vjj_trig == 2) || (vjj_trig==3 && !lowPtCut) ) && (generalCuts && vjj_v_pt > ptCut);

                    //--- Cut on category flag
                    if(!is_goodCategory) {continue;}
                    // if(std::accumulate(v_is_goodCategory.begin(), v_is_goodCategory.end(), 0) != 1) {continue;} //Check whether at least 1 cat. flag is true

    				bool pass_all_cuts = true;
    				for(int icut=0; icut<v_cut_name.size(); icut++)
    				{
    					if(v_cut_def[icut] == "") {continue;}

    					//Categories are encoded into Char_t. Convert them to float for code automation
    					if(v_cut_name[icut].BeginsWith("is") || v_cut_name[icut].BeginsWith("passed") ) {v_cut_float[icut] = (float) v_cut_char[icut];}
    					// cout<<"Cut : name="<<v_cut_name[icut]<<" / def="<<v_cut_def[icut]<<" / value="<<v_cut_float[icut]<<" / pass ? "<<Is_Event_Passing_Cut(v_cut_def[icut], v_cut_float[icut])<<endl;
    					if(!Is_Event_Passing_Cut(v_cut_def[icut], v_cut_float[icut])) {pass_all_cuts = false; break;}
    				}
    				if(!pass_all_cuts) {continue;}

//--------------------------------------------

                    ibar++;
                    if(draw_progress_bar && ibar%50000==0) {timer.DrawProgressBar(ibar, ""); cout<<DIM("Processed "<<ibar<<" / "<<total_nentries_toRead<<"")<<endl; }

                    // cout<<"//-------------------------------------------- "<<endl;
                    // cout<<"eventWeight "<<eventWeight<<endl;
                    // cout<<"eventMCFactor "<<eventMCFactor<<endl;
                    // for(int ivar=0; ivar<var_list_floats.size(); ivar++) {cout<<"var_list_floats "<<ivar<<" = "<<var_list_floats[ivar]<<endl;} //Debug
                    // for(int ivar=0; ivar<var_list_floats.size(); ivar++) {cout<<"var_list_floats "<<ivar<<" "<<var_list[ivar]<<" / "<<var_list_pfloats[ivar]<<" = "<<*var_list_pfloats[ivar]<<endl;} //Debug
                    // cout<<"njets "<<*njets<<endl;
                    // cout<<"mTW "<<*mTW<<endl;

    				//-- S i n g l e    t e m p l a t e     v a l u e (<-> only consider 'template_name')
                    std::vector<float> clfy1_outputs, clfy2_outputs; //Compute NN responses only once per event
                    if(!makeHisto_inputVars) //Templates
                    {
                        if(template_name == "BDT" || template_name.Contains("NN")) {*total_var_pfloats[0] = reader->EvaluateMVA(BDT_method_name);} //BDT output value
                        else if(template_name == "countExp")
                        {
                            *total_var_pfloats[0] = 0.5;
                        }
                        else if(template_name == "channel") //Hard-coded from main SM analysis
                        {
                            *total_var_pfloats[0] = (*channel==1 || *channel==3)? 0.5:1.5;
                            // total_var_floats[0] = (*channel==1 || *channel==3)? 0.5:1.5;
                        }
                    } //Templates

                    double weight_tmp = 1.;
                    if(!isData) {weight_tmp = vjj_photon_effWgt * vjj_weight * vjj_lumiWeights[0] / vjj_mu_effWgt;} //FIXME

                    // double weight_tmp = eventWeight * eventMCFactor; //Fill histo with this weight ; manipulate differently depending on syst
                    // cout<<"eventWeight "<<eventWeight<<" / eventMCFactor "<<eventMCFactor<<" / weight_tmp "<<weight_tmp<<endl;

                    //-- F i l l     a l l      v a r i a b l e s (<-> consider full list of variables 'total_var_list')
                    //-- Can choose to fill event only into histogram corresponding to relevant leptonic channel
    				for(int ichan=0; ichan<channel_list.size(); ichan++)
    				{
                        //-- Fill histos for sub-channels with corresponding events
                        if(channel_list[ichan] == "uuu" && *channel != 0) {continue;}
                        if(channel_list[ichan] == "uue" && *channel != 1) {continue;}
                        if(channel_list[ichan] == "eeu" && *channel != 2) {continue;}
                        if(channel_list[ichan] == "eee" && *channel != 3) {continue;}

                        bool cat_alreadyFound = false; //For pre-defined strategies, fill orthogonal categories
                        for(int ivar=0; ivar<total_var_list.size(); ivar++)
                        {
                            //-- TEMPLATES
                            if(!makeHisto_inputVars)
                            {
                                *total_var_pfloats[ivar] = *total_var_pfloats[0]; //Force use of first variable //FIXME
                            }
                            // cout<<total_var_list[ivar]<<" = "<<*total_var_pfloats[ivar]<<endl; //Debug printout

                            //-- S y s t  e m a t i c    w e i g h t s
        					for(int isyst=0; isyst<syst_list.size(); isyst++)
        					{
                                //-- Protections : not all syst weights apply to all samples, etc.
                                if((sample_list[isample] == "DATA") && syst_list[isyst] != "") {break;}
                                else if(systTree_list[itree] != "" && systTree_list[itree] != nominal_tree_name && syst_list[isyst] != "") {break;} //Syst event weights only stored in nominal TTree
                                else if(v_lumiYears[iyear] == "2018" && syst_list[isyst].BeginsWith("prefir") ) {continue;} //no prefire in 2018
                                else if((sample_list[isample].Contains("NPL") && syst_list[isyst] != "" && !syst_list[isyst].BeginsWith("FR")) || (!sample_list[isample].Contains("NPL") && syst_list[isyst].BeginsWith("FR"))) {continue;} //NPL <-> only fakes sytematics; all others <-> no fakes systematics

        						// cout<<"-- sample "<<sample_list[isample]<<" / channel "<<channel_list[ichan]<<" / syst "<<syst_list[isyst]<<endl;

                                //-- Start from nominal weight (no syst) //Re-initialize for each syst
                                // weight_tmp = eventWeight*eventMCFactor;

                                //No prefiring uncertainty for 2018 samples ; still write the histo (=nominal) to avoid troubles down the way... ?
                                // if(v_lumiYears[iyear] == "2018" && syst_list[isyst].BeginsWith("prefir")) {*(v_double_systWeights[isyst]) = 1;}

                                // if(isyst>0) cout<<"syst_list[isyst] "<<syst_list[isyst]<<" : "<<*(v_double_systWeights[isyst])<<endl;
                                // cout<<"nominal : "<<weight_tmp<<endl;
                                if(syst_list[isyst] != "") //Syst weights were already divided by nominal weight
                                {
                                    if(abs(*(v_double_systWeights[isyst])) < 100) //Safe protection against huge, non-physical PDF/PSweights/...
                                    {
                                        weight_tmp*= *(v_double_systWeights[isyst]);
                                    }
                                }
                                // cout<<"syst : "<<weight_tmp<<endl;

        						if(std::isnan(weight_tmp) || std::isinf(weight_tmp))
        						{
        							cout<<BOLD(FRED("* Found event with syst. weight = "<<weight_tmp<<" ; remove it..."))<<endl;
        							cout<<"(sample "<<sample_list[isample]<<" / channel "<<channel_list[ichan]<<" / year "<<v_lumiYears[iyear]<<" / syst "<<syst_list[isyst]<<")"<<endl;
        							continue;
        						}

                                //-- F i l l     h i s t o
                                Fill_TH1F_UnderOverflow(v3_histo_chan_syst_var[ichan][isyst][ivar], *total_var_pfloats[ivar], weight_tmp);
                            } //syst loop
    					} //var loop

    					if(channel_list[ichan] != "") {break;} //subcategories are orthogonal ; if already found, can break subcat. loop
    				} //subcat/chan loop
    			} //TTree entries loop
//--------------------------------------------


// #    # #####  # ##### ######
// #    # #    # #   #   #
// #    # #    # #   #   #####
// # ## # #####  #   #   #
// ##  ## #   #  #   #   #
// #    # #    # #   #   ######

    			// --- Write histograms
    			TString samplename = sample_list[isample];
    			if(sample_list[isample] == "DATA") {samplename = "data_obs";}

    			for(int ichan=0; ichan<channel_list.size(); ichan++)
    			{
    				for(int isyst=0; isyst<syst_list.size(); isyst++)
    				{
                        //-- Protections : not all syst weights apply to all samples, etc.
                        if(sample_list[isample] == "DATA" && syst_list[isyst] != "") {break;}
                        else if(systTree_list[itree] != "" && systTree_list[itree] != nominal_tree_name && syst_list[isyst] != "") {break;} //Syst event weights only stored in nominal TTree
                        else if(v_lumiYears[iyear] == "2018" && syst_list[isyst].BeginsWith("prefir") ) {continue;} //no prefire in 2018
                        else if((sample_list[isample].Contains("NPL") && syst_list[isyst] != "" && !syst_list[isyst].BeginsWith("FR")) || (!sample_list[isample].Contains("NPL") && syst_list[isyst].BeginsWith("FR"))) {continue;} //NPL <-> only fakes sytematics; all others <-> no fakes systematics

    					for(int ivar=0; ivar<total_var_list.size(); ivar++)
    					{
                            // cout<<"channel "<<channel_list[ichan]<<" / systTree "<<systTree_list[isyst]<<" / syst "<<syst_list[isyst]<<" / var "<<total_var_list[ivar]<<endl;

    						TString output_histo_name = total_var_list[ivar];
                            if(channel_list[ichan] != "") {output_histo_name+= "_" + channel_list[ichan];}
                            if(region != "" && !makeHisto_inputVars) {output_histo_name+= "_" + region;}
                            output_histo_name+= "_" + v_lumiYears[iyear] + "__" + samplename;
							if(syst_list[isyst] != "" || (systTree_list[itree] != "" && systTree_list[itree] != nominal_tree_name)) {output_histo_name+= "__" + Get_Modified_SystName(syst_list[isyst]+systTree_list[itree], v_lumiYears[iyear], samplename);}
							else if(systTree_list[itree] != "" && systTree_list[itree] != nominal_tree_name) {output_histo_name+= "__" + systTree_list[itree];}

                            //-- Protection: replace '-' (hyphen) with 'm' character (hyphen in histo name causes errors at reading)
                            output_histo_name.ReplaceAll('-', 'm');

    						file_output->cd();

                            if(sample_list[isample] != "NPL_MC" && sample_list[isample] != "DATA") //NB: NPL_MC *should* have negative bins (substraction); and allow empty data
                            {
                                //-- Nominal: prevent negative bins, and error>content
                                if(syst_list[isyst] == "" && systTree_list[itree] == "")
                                {
                                    Avoid_Histogram_EmptyOrNegativeBins(v3_histo_chan_syst_var[ichan][isyst][ivar]);

                                    v_storeNominalBinContents_chan_var_bin[ichan][ivar].resize(v3_histo_chan_syst_var[ichan][isyst][ivar]->GetNbinsX());
                                    v_storeNominalBinErrors_chan_var_bin[ichan][ivar].resize(v3_histo_chan_syst_var[ichan][isyst][ivar]->GetNbinsX());
                                    for(int ibin=0; ibin<v3_histo_chan_syst_var[ichan][isyst][ivar]->GetNbinsX(); ibin++)
                                    {
                                        v_storeNominalBinContents_chan_var_bin[ichan][ivar][ibin] = v3_histo_chan_syst_var[ichan][isyst][ivar]->GetBinContent(ibin+1);
                                        v_storeNominalBinErrors_chan_var_bin[ichan][ivar][ibin] = v3_histo_chan_syst_var[ichan][isyst][ivar]->GetBinError(ibin+1);
                                        // cout<<"v_storeNominalBinContents_chan_var_bin["<<ichan<<"]["<<ivar<<"]["<<ibin<<"] "<<v_storeNominalBinContents_chan_var_bin[ichan][ivar][ibin]<<endl;
                                        // cout<<"v_storeNominalBinErrors_chan_var_bin["<<ichan<<"]["<<ivar<<"]["<<ibin<<"] "<<v_storeNominalBinErrors_chan_var_bin[ichan][ivar][ibin]<<endl;
                                    }
                                }
                                //-- Others: if bin<0, or nominal~0, or (nom-var)<0, set bin to nominal; also adapt error
                                else
                                {
                                    for(int ibin=0; ibin<v3_histo_chan_syst_var[ichan][isyst][ivar]->GetNbinsX(); ibin++)
                                    {
                                        //-- bin<0, or nominal already set to ~0 --> set variation to nominal //Don't call 'Avoid_Histogram_EmptyOrNegativeBins' because we need to check case-by-case depending on the original bin contents
                                        if(v3_histo_chan_syst_var[ichan][isyst][ivar]->GetBinContent(ibin+1) <= 0 || v_storeNominalBinContents_chan_var_bin[ichan][ivar][ibin] < pow(10, -4))
                                        {
                                            v3_histo_chan_syst_var[ichan][isyst][ivar]->SetBinContent(ibin+1, v_storeNominalBinContents_chan_var_bin[ichan][ivar][ibin]);
                                            v3_histo_chan_syst_var[ichan][isyst][ivar]->SetBinError(ibin+1, pow(10,-6)); //Don't need MCstat error for shapeSysts
                                        }

                                        //-- Bin error > content --> set error=content
                                        // if(v3_histo_chan_syst_var[ichan][isyst][ivar]->GetBinError(ibin+1) > v3_histo_chan_syst_var[ichan][isyst][ivar]->GetBinContent(ibin+1) / 2.)
                                        if(v3_histo_chan_syst_var[ichan][isyst][ivar]->GetBinError(ibin+1) > v3_histo_chan_syst_var[ichan][isyst][ivar]->GetBinContent(ibin+1))
                                        {
                                            v3_histo_chan_syst_var[ichan][isyst][ivar]->SetBinError(ibin+1, v3_histo_chan_syst_var[ichan][isyst][ivar]->GetBinContent(ibin+1)-pow(10, -3));
                                            // v3_histo_chan_syst_var[ichan][isyst][ivar]->SetBinError(ibin+1, v3_histo_chan_syst_var[ichan][isyst][ivar]->GetBinContent(ibin+1)/2.);
                                        }

                                        //-- Careful ! Make sure not to remove any genuine shape effect here... (should be safe)
                                        //-- var/nom > truncation_factor or < (1/truncation_factor) --> indicates very low stat, and can cause problems when splitting per bin --> set var==nom //cf. https://hypernews.cern.ch/HyperNews/CMS/get/higgs-combination/1086/1.html //Again verified that even when got up/nom>5, this led to unphysical postfit uncertainties in a bin !
                                        float truncation_factor = 2.;
                                        if(v3_histo_chan_syst_var[ichan][isyst][ivar]->GetBinContent(ibin+1) / v_storeNominalBinContents_chan_var_bin[ichan][ivar][ibin] > truncation_factor || v3_histo_chan_syst_var[ichan][isyst][ivar]->GetBinContent(ibin+1) / v_storeNominalBinContents_chan_var_bin[ichan][ivar][ibin] < 1/truncation_factor)
                                        // if(v3_histo_chan_syst_var[ichan][isyst][ivar]->GetBinContent(ibin+1) / v_storeNominalBinContents_chan_var_bin[ichan][ivar][ibin] > 10 || v3_histo_chan_syst_var[ichan][isyst][ivar]->GetBinContent(ibin+1) / v_storeNominalBinContents_chan_var_bin[ichan][ivar][ibin] < 0.1)
                                        {
                                            v3_histo_chan_syst_var[ichan][isyst][ivar]->SetBinContent(ibin+1, v_storeNominalBinContents_chan_var_bin[ichan][ivar][ibin]);
                                            v3_histo_chan_syst_var[ichan][isyst][ivar]->SetBinError(ibin+1, pow(10,-6)); //Don't need MCstat error for shapeSysts
                                        }
                                    }
                                }
                            } //end protections for combine

                            file_output->cd();
                            v3_histo_chan_syst_var[ichan][isyst][ivar]->Write(output_histo_name);
                            nhistos++; //Count nof histos written to output file

                            // cout<<"Wrote histo : "<<output_histo_name<<" (Integral: "<<v3_histo_chan_syst_var[ichan][isyst][ivar]->Integral()<<")"<<endl;

                            //-- Delete TH1F objects, if memory was allocated
    						if(v3_histo_chan_syst_var[ichan][isyst][ivar]) {delete v3_histo_chan_syst_var[ichan][isyst][ivar]; v3_histo_chan_syst_var[ichan][isyst][ivar] = NULL;}
    					} //var loop
    				} //syst loop
    			} //chan loop

    			// cout<<"Done with "<<sample_list[isample]<<" sample"<<endl;

                // if(njets) {delete njets; njets = NULL;}
                // if(channel) {delete channel; channel = NULL;}
                // if(mTW) {delete mTW; mTW = NULL;} //Careful not to delete twice, can cause nasty segfaults

                file_input->Close(); file_input = NULL; //Also deletes associated TTrees, histos, etc.
            } //TTree loop
    	} //sample loop

        if(!makeHisto_inputVars)
        {
            if(classifier_name == "BDT")  {delete reader; reader = NULL;}
            else
            {
                if(clfy1 && use_specificMVA_eachYear) {delete clfy1; clfy1 = NULL; MVA_alreadyLoaded = false;} //Only delete if not going to be reused for another iteration
                //-- NB: do not delete clfy2 //Causes crash (because it uses the same TF session as clfy1, hence double-free... ?)
                // if(clfy2) {delete clfy2; clfy2 = NULL;}
            }
        }

    } //years loop


//  ####  #       ####   ####  ######
// #    # #      #    # #      #
// #      #      #    #  ####  #####
// #      #      #    #      # #
// #    # #      #    # #    # #
//  ####  ######  ####   ####  ######

	cout<<endl<<FYEL("==> Created root file: ")<<file_output->GetName()<<endl;
	cout<<FYEL("containing the "<<classifier_name<<" templates as histograms for : all samples / all channels")<<endl;
    cout<<DIM("("<<nhistos<<" histograms)")<<endl<<endl;
	file_output->Close(); file_output = NULL;

    //-- Can verify that the total nof processed entries computed from Count_Total_Nof_Entries() was effectively the nof processed entries
    // cout<<"total_nentries_toRead --> "<<total_nentries_toRead<<endl;
    // cout<<"tmp_compare --> "<<tmp_compare<<endl;

    //Restore potfile_outputentially modified variables
    classifier_name = restore_classifier_name;
    syst_list = restore_syst_list;
    systTree_list = restore_systTree_list;

    //NB: if get segfault when deleting a given array, may have allocated wrong size
    if(!makeHisto_inputVars || !noSysts_inputVars)
    {
        if(array_PU) {delete[] array_PU; array_PU = NULL;}
        if(array_prefiringWeight) {delete[] array_prefiringWeight; array_prefiringWeight = NULL;}
        if(array_Btag) {delete[] array_Btag; array_Btag = NULL;}
        if(array_jetPileupID) {delete[] array_jetPileupID; array_jetPileupID = NULL;}
        if(array_fakeFactor) {delete[] array_fakeFactor; array_fakeFactor = NULL;}
        if(array_ME) {delete[] array_ME; array_ME = NULL;}
        if(array_alphaS) {delete[] array_alphaS; array_alphaS = NULL;}
        if(array_PDFtotal) {delete[] array_PDFtotal; array_PDFtotal = NULL;}
        if(array_LepEffLoose_mu) {delete[] array_LepEffLoose_mu; array_LepEffLoose_mu = NULL;}
        if(array_partonShower) {delete[] array_partonShower; array_partonShower = NULL;}
        if(array_LepEffTight_mu) {delete[] array_LepEffTight_mu; array_LepEffTight_mu = NULL;}
        if(array_LepEffLoose_el) {delete[] array_LepEffLoose_el; array_LepEffLoose_el = NULL;}
        if(array_LepEffTight_el) {delete[] array_LepEffTight_el; array_LepEffTight_el = NULL;}
    }
    if(clfy1) {delete clfy1; clfy1 = NULL;}
    for(int ivar=0; ivar<var_list_pfloats.size(); ivar++) //Delete input vars
    {
        if(var_list_pfloats[ivar] && var_list[ivar] != "mTW" && var_list[ivar] != "njets" && var_list[ivar] != "channel") //Exception: mTW/njets/... are sepcial variables already deleted above //Ideally, use smart pointers (auto-delete shared mem, only once)
        {
            // cout<<"del var "<<var_list[ivar]<<" = "<<var_list_pfloats[ivar]<<endl;
            delete var_list_pfloats[ivar]; var_list_pfloats[ivar] = NULL;
        }
    }
    for(int ivar=0; ivar<var_list_pfloats_2.size(); ivar++)
    {
        if(var_list_pfloats_2[ivar] && v_varIndices_inMVA1[ivar]==-1 && var_list_NN2[ivar] != "mTW" && var_list_NN2[ivar] != "njets" && var_list_NN2[ivar] != "channel") //Be careful not to delete any previously-deleted variable ! //Ideally, use smart pointers (auto-delete shared mem, only once)
        {
            /*cout<<"del var "<<var_list_NN2[ivar]<<endl;*/
            delete var_list_pfloats_2[ivar]; var_list_pfloats_2[ivar] = NULL;
        }
    }

    //-- Not needed ? //Careful not to delete any variable twice
    for(int ivar=0; ivar<total_var_pfloats.size(); ivar++)
    {
        if(!total_var_list[ivar].Contains("mTW") && !total_var_list[ivar].Contains("njets") && !total_var_list[ivar].Contains("channel"))
        {
            /*cout<<"del var "<<total_var_list[ivar]<<endl;*/
            delete total_var_pfloats[ivar]; total_var_pfloats[ivar] = NULL;
        }
    }

    //-- Printout: count nof signal events in each bin //To understand/prevent low-stat issues
    /*
    for(int iyear=0; iyear<v_lumiYears.size(); iyear++)
    {
        cout<<"year "<<v_lumiYears[iyear]<<endl;
        for(int isample=0; isample<sample_list.size(); isample++)
        {
            cout<<"sample "<<sample_list[isample]<<endl;
            for(int ivar=0; ivar<total_var_list.size(); ivar++)
            {
                cout<<"var "<<total_var_list[ivar]<<endl;
                for(int ibin=0; ibin<v4_nEntries_year_proc_var_bin[iyear][isample][ivar].size(); ibin++)
                {
                    cout<<"Bin "<<ibin+1<<" ---> Entries "<<v4_nEntries_year_proc_var_bin[iyear][isample][ivar][ibin]<<endl;
                }
            }
        }
    }
    */


// #    # ###### #####   ####  ######
// ##  ## #      #    # #    # #
// # ## # #####  #    # #      #####
// #    # #      #####  #  ### #
// #    # #      #   #  #    # #
// #    # ###### #    #  ####  ######

    //-- For COMBINE fit, want to directly merge contributions from different processes into single histograms
    //-- For control histograms, only need to substract MC NPL from data-driven NPL
    if(!this->process_samples_byGroup) {MergeSplit_Templates(makeHisto_inputVars, output_file_name, total_var_list, template_name, region, true);} //If already considering group ntuples, no need to group anything more

	return;
}
































//-----------------------------------------------------------------------------------------
// ########  ########     ###    ##      ##
// ##     ## ##     ##   ## ##   ##  ##  ##
// ##     ## ##     ##  ##   ##  ##  ##  ##
// ##     ## ########  ##     ## ##  ##  ##
// ##     ## ##   ##   ######### ##  ##  ##
// ##     ## ##    ##  ##     ## ##  ##  ##
// ########  ##     ## ##     ##  ###  ###

// ######## ######## ##     ## ########  ##          ###    ######## ########  ######
//    ##    ##       ###   ### ##     ## ##         ## ##      ##    ##       ##    ##
//    ##    ##       #### #### ##     ## ##        ##   ##     ##    ##       ##
//    ##    ######   ## ### ## ########  ##       ##     ##    ##    ######    ######
//    ##    ##       ##     ## ##        ##       #########    ##    ##             ##
//    ##    ##       ##     ## ##        ##       ##     ##    ##    ##       ##    ##
//    ##    ######## ##     ## ##        ######## ##     ##    ##    ########  ######
//-----------------------------------------------------------------------------------------

//Create plots from histograms representing templates (used for signal extraction) or input variables (for validation).
//NB: sums histograms from all years included in 'v_lumiYears' ! If you want to plot for a single year, activate only that specific year !
/**
 * Create plots from histograms representing templates (used for signal extraction) or input variables (for validation)
 * NB: sums histograms from all years included in 'v_lumiYears' ! If you want to plot for a single year, activate only that specific year !
 * @param drawInputVars    [true <-> plot input features / selected kinematic variables; else plot templates]
 * @param channel          [If specified, make plots for a specific (lepton) sub-channel]
 * @param template_name    [Specify the name of the templates to plot]
 * @param prefit           [true <-> plot prefit templates; else postfit]
 * @param use_combine_file [true <-> look for Combine FitDiagnostics output rootfile (needed e.g. to make postfit plots); else use my own template file]
 */
void VBFgamma_analysis::Draw_Templates(bool drawInputVars, TString channel, bool plot_onlyMaxNodeEvents, TString template_name, bool prefit, bool use_combine_file)
{
//--- OPTIONS --------------------------------
//--------------------------------------------
    bool draw_errors = true; //true <-> superimpose error bands on plot/ratio plot

	bool draw_logarithm = false; //true <-> plot y-axis in log scale //May be overwriten for special cases below

    bool use_poisson_dataErrors = false; //true <-> use asymmetric poisson errors for data (recommended for low-stat templates), see: https://twiki.cern.ch/twiki/bin/viewauth/CMS/PoissonErrorBars //To be tested

    bool combineFile_fromHarvester = true; //true <-> use CombineHarvester conventions for combine file; else use FitDiagnostics conventions //NB: happened that data does not get drawn because of NaN in combine file (although GetBinContent loos normal...)

//--------------------------------------------
//--------------------------------------------

    if(template_name == "" && classifier_name != "BDT" && classifier_name != "NN") {cout<<BOLD(FRED("Error : classifier_name value ["<<classifier_name<<"] not supported !"))<<endl; return;}
    if(template_name == "") {template_name = classifier_name;}
    if(!drawInputVars && template_name!="BDT") {template_name = "NN_SM";} //Special case: if want to produce SM vs SM classifier plots, make sure we consider NN_SM templates (for BDT, must specify in main) //"NN"?

    cout<<endl<<BYEL("                          ")<<endl<<endl;
	if(drawInputVars) {cout<<FYEL("--- Producing Input Variables Plots / channel : "<<channel<<" ---")<<endl;}
    else {cout<<FYEL("--- Producing "<<template_name<<" Template Plots ---")<<endl;}
    cout<<endl<<BYEL("                          ")<<endl<<endl;

	if(drawInputVars)
	{
		classifier_name = ""; //For naming conventions
		if(drawInputVars && !prefit) {cout<<"Error ! Can not draw postfit input vars yet !"<<endl; return;}
	}


//  ####  ###### ##### #    # #####
// #      #        #   #    # #    #
//  ####  #####    #   #    # #    #
//      # #        #   #    # #####
// #    # #        #   #    # #
//  ####  ######   #    ####  #

	//-- Can use 2 different types of combine files :
	//- the files containing the template histograms, produced with this code (-> only prefit plots)
	//- or, better, the file produced by Combine from the templates : contains the prefit distributions with total errors, and the postfit distribution
	//If want postfit plots, must use the Combine file. If want prefit plots, can use both of them (NB : errors will be different)

    // TString cat_tmp = (region=="") ? "SR" : region+"Cat";
    TString cat_tmp = region;

	if(!prefit) {use_combine_file = true;}
    if(!use_combine_file) {combineFile_fromHarvester = false;} //Necessary convention

    //-- Read input file (may be year-dependent)
    TFile* file_input = NULL;
    TString template_type = template_name;
    TString inputFile_path = "";

	//-- Want to plot ALL selected variables
	vector<TString> total_var_list;
	if(drawInputVars)
	{
		for(int i=0; i<var_list.size(); i++)
		{
			total_var_list.push_back(var_list[i]);
		}
		for(int i=0; i<v_add_var_names.size(); i++)
		{
			total_var_list.push_back(v_add_var_names[i]);
		}
	}
	else //Templates
	{
        //-- Read the relevant NN info file, just to know if we are dealing with multiclass NN templates or not !
        if(template_name.Contains("NN") && !use_combine_file)
        {
            TString NNinfo_input_path = Get_MVAFile_InputPath(template_name, v_lumiYears[0], use_specificMVA_eachYear, true);
            if(!Extract_Values_From_NNInfoFile(NNinfo_input_path, var_list_NN, v_NN_nodeLabels, NN_inputLayerName, NN_outputLayerName, NN_iMaxNode, NN_nNodes, minmax_bounds)) {return;} //Error: missing NN infos
        }

        //-- Can now fill the list of variables (templates names)
        Fill_Variables_List(total_var_list, template_name);
	} //Templates
    if(!use_combine_file)
    {
        inputFile_path = Get_HistoFile_InputPath(!drawInputVars, template_type, cat_tmp, lumiName, use_combine_file, this->filename_suffix, combineFile_fromHarvester, prefit);
        if(inputFile_path == "") {cat_tmp = signal_process; inputFile_path = Get_HistoFile_InputPath(!drawInputVars, template_type, cat_tmp, lumiName, use_combine_file, this->filename_suffix, combineFile_fromHarvester, prefit);} //Retry with 'signal_process' as 'region' argument
    }
    if(inputFile_path == "") {cout<<"Get_HistoFile_InputPath --> file not found ! "<<endl; return;}
    else {file_input = TFile::Open(inputFile_path, "READ");}

    //-- Define ranges of jet/bjets multiplicities -- for 'categ' templates only (modified in 'Get_Template_Range()')
    int nbjets_min = 1, nbjets_max=2, njets_min=2, njets_max=6;
    int nbins_tmp; float xmin_tmp, xmax_tmp;

    vector<float> v_yields_processes(sample_list.size()); //Can store and print yields per process, summed over years (e.g. to compare prefit/postfit)


// #       ####   ####  #####   ####
// #      #    # #    # #    # #
// #      #    # #    # #    #  ####
// #      #    # #    # #####       #
// #      #    # #    # #      #    #
// ######  ####   ####  #       ####

	for(int ivar=0; ivar<total_var_list.size(); ivar++)
	{
		if(drawInputVars) {cout<<endl<<FBLU("* Variable : "<<total_var_list[ivar]<<" ")<<endl<<endl;}

        // draw_logarithm = false; //Force use of linear y-scale

        TH1F* h_tmp = NULL; //Tmp storing histo
        TH1F* hdata_tmp = NULL; //Tmp storing data histo
		TH1F* h_tzq = NULL; //Store tZq shape
		TH1F* h_ttz = NULL; //Store ttZ shape
		TH1F* h_sum_data = NULL; //Will store data histogram
		vector<TH1F*> v_MC_histo; //Will store all MC histograms (1 TH1F* per MC sample)
        TString data_histo_name = "";

		TGraphAsymmErrors* g_data = NULL; //If using Combine file, data are stored in TGAE
		TGraphAsymmErrors* g_tmp = NULL; //Tmp storing graph

		vector<TString> v_MC_samples_legend; //List the MC samples to mention in legend

		//-- Init error vectors
		double x, y, errory_low, errory_high;

		vector<double> v_eyl, v_eyh, v_exl, v_exh, v_x, v_y; //Contain the systematic errors (used to create the TGraphError)
		int nofbins=-1;

        //-- Combine file: histos stored in subdirs -- define dir name
        TString dir_hist_prefix = "";

        //-- Only for FitDiagnostics output (not from CombineHarvester) ?
        if(!combineFile_fromHarvester)
        {
            if(prefit) {dir_hist_prefix = "shapes_prefit/";}
            else {dir_hist_prefix = "shapes_fit_s/";}
        }

        TString var_tmp = ""; //Dummy variable following expected naming convention --> check if exists
        //-- Combine file: if divided full histos into individual bins for fit, will now need to re-combine all the bins into full templates
        //-- First: check contents of the file to see whether templates were split or not
        bool combineIndividualBins=false; //Auto-set below
        int nIndivBins = 1; //Will update this number with the actual nof bins found in histos
        if(use_combine_file)
        {
            var_tmp = total_var_list[ivar];
            if(channel_list[0] != "") {var_tmp+= "_" + channel_list[0];}
            if(region != "" && !drawInputVars) {var_tmp+= "_" + region;}
            // var_tmp+= "_" + v_lumiYears[0]; //Dummy year

            // cout<<dir_hist_prefix<<"bin"+Convert_Number_To_TString(nIndivBins)+"_"<<var_tmp<<endl;
            TString hpath_tmp = dir_hist_prefix; //change naming convention?
            if(combineFile_fromHarvester)
            {
                if(prefit) hpath_tmp+= "_prefit";
                else hpath_tmp+= "_postfit";
            }
            hpath_tmp+= "bin1_" + var_tmp;

            inputFile_path = Get_HistoFile_InputPath(!drawInputVars, "bin1_" + var_tmp, cat_tmp, lumiName, use_combine_file, this->filename_suffix, combineFile_fromHarvester, prefit, false);
            // if(inputFile_path == "") {cout<<"Get_HistoFile_InputPath --> file not found ! "<<endl; return;}
            if(inputFile_path != "") {combineIndividualBins = true;} //Seems like we have fitted individual bins, so in this chode we'll need to combine them all together again //Look for dummy bin (must be present if templates are split per bins)
            else
            {
                inputFile_path = Get_HistoFile_InputPath(!drawInputVars, var_tmp, cat_tmp, lumiName, use_combine_file, this->filename_suffix, combineFile_fromHarvester, prefit, false);
                if(inputFile_path == "") {cout<<"Get_HistoFile_InputPath --> file not found ! "<<endl; continue;}
            }

            file_input = TFile::Open(inputFile_path, "READ");

            if(combineIndividualBins) {Get_Template_Range(nIndivBins, xmin_tmp, xmax_tmp, total_var_list[ivar], plot_onlyMaxNodeEvents, nbjets_min, nbjets_max, njets_min, njets_max, minmax_bounds);}
            // cout<<"nIndivBins "<<nIndivBins<<endl;
            // if(file_input->GetDirectory(hpath_tmp)) {combineIndividualBins = true;} //Seems like we have fitted individual bins, so in this chode we'll need to combine them all together again //Look for dummy bin (must be present if templates are split per bins)
            // if(combineIndividualBins) //Now, need to infer from the file how many individual bins will need to be combined
            // {
            //     while(file_input->GetDirectory(hpath_tmp))
            //     {
            //         nIndivBins++;
            //         hpath_tmp = dir_hist_prefix;
            //         if(combineFile_fromHarvester)
            //         {
            //             if(prefit) hpath_tmp+= "_prefit";
            //             else hpath_tmp+= "_postfit";
            //         }
            //         hpath_tmp+= "bin"+Convert_Number_To_TString(nIndivBins)+"_" + var_tmp;
            //     }
            //     nIndivBins--; //Last bin was not found --> update total nof bins
            //     cout<<"nIndivBins "<<nIndivBins<<endl;
            // }
        }

        float bin_width = -1; //Get bin width of histograms for current variable

        bool data_notEmpty = true; //Default=true <-> data found //NB: if blinded, will set all data bins to 0 manually below

        //-- All histos are for given lumiYears and sub-channels --> Need to sum them all for plots
        for(int iyear=0; iyear<v_lumiYears.size(); iyear++)
        {
        	//Need to rescale signal to fitted signal strength manually, and add its error in quadrature in each bin (verify)
        	double sigStrength = 0;
        	double sigStrength_Error = 0;
        	if(!combineFile_fromHarvester && !prefit)
        	{
        		TTree* t_postfit = (TTree*) file_input->Get("tree_fit_sb");
        		t_postfit->SetBranchAddress("r", &sigStrength);
        		t_postfit->SetBranchAddress("rErr", &sigStrength_Error);
        		t_postfit->GetEntry(0); //Only 1 entry = fit results
        		delete t_postfit; t_postfit = NULL;
        	}

    		for(int ichan=0; ichan<channel_list.size(); ichan++)
    		{
    			//If using my own template file, there is already a "summed channels" version of the histograms
    			if(channel_list[ichan] != channel)
    			{
    				if(use_combine_file) {if(channel != "") {continue;} } //In combine file, to get inclusive plot, must sum all subcategories
    				else {continue;}
    			}

    			//-- Combine file : histos stored in subdirs -- define dir name //Only used if dealing with 'full' templates
    			TString dir_hist = dir_hist_prefix + total_var_list[ivar];
                if(channel_list[ichan] != "") {dir_hist+= "_" + channel_list[ichan];}
                if(region != "" && !drawInputVars) {dir_hist+= "_" + region;}
                dir_hist+= "_" + v_lumiYears[iyear];
                if(use_combine_file && combineFile_fromHarvester)
                {
                    if(prefit) {dir_hist+= "_prefit";}
                    else {dir_hist+= "_postfit";}
                }
                dir_hist+= "/";
                if(use_combine_file && !combineIndividualBins && !file_input->GetDirectory(dir_hist) ) {cout<<ITAL(DIM("ERROR: directory '"<<dir_hist<<"' : not found !"))<<endl; continue;}


// #    #  ####
// ##  ## #    #
// # ## # #
// #    # #
// #    # #    #
// #    #  ####

    			//--- Retrieve all MC samples
    			int nof_skipped_samples = 0; //Get sample index right

    			vector<bool> v_isSkippedSample(sample_list.size()); //Get sample index right (some samples are skipped)

    			for(int isample = 0; isample < sample_list.size(); isample++)
    			{
                    int index_MC_sample = isample - nof_skipped_samples; //Sample index, but not counting data/skipped sample

                    // cout<<"sample_list[isample] "<<sample_list[isample]<<endl;
                    // cout<<"index_MC_sample "<<index_MC_sample<<endl;

    				//-- In Combine, some individual contributions are merged as "Rares"/"EWK", etc.
    				//-- If using Combine file, change the names of the samples we look for, and look only once for histogram of each "group"
    				TString samplename = sample_list[isample];
    				if(use_combine_file)
    				{
    					if(isample > 0 && sample_groups[isample] == sample_groups[isample-1]) {v_isSkippedSample[isample] = true; nof_skipped_samples++; continue;} //if same group as previous sample, skip it
                        else {samplename = sample_groups[isample];}
    				}

    				//-- Protections, special cases
    				if(sample_list[isample] == "DATA") {v_isSkippedSample[isample] = true; nof_skipped_samples++; continue;}
                    else if(sample_list[isample] == "tZq" || sample_list[isample] == "ttZ" || sample_list[isample] == "tWZ") {v_isSkippedSample[isample] = true; nof_skipped_samples++; continue;} //Default: only stack private samples (at SM point), not central samples
                    else if(sample_list[isample] == "NPL_DATA")  {samplename = "NPL";} //Instead of 'NPL_DATA' and 'NPL_MC', we only want to read the merged histo 'NPL'
                    else if(sample_list[isample] == "NPL_MC")  {v_isSkippedSample[isample] = true; nof_skipped_samples++; continue;} //NPL_MC gets substracted from NPL histograms and deleted --> Ignore this vector element //Remove ?

                    //-- Add sample name to list (used for legend) //NB: add even if histo was not found and skipped, because expect that it will be found for some other year/channel/... But if not found at all, legend will be wrong
                    if(iyear==0 && samplename != "DATA")
                    {
                        if(v_MC_histo.size() <=  index_MC_sample) {v_MC_samples_legend.push_back(samplename);}
                        // cout<<"v_MC_histo.size() "<<v_MC_histo.size()<<" / index_MC_sample "<<index_MC_sample<<" / v_MC_samples_legend.size() "<<v_MC_samples_legend.size()<<endl;
                    }
                    if(v_isSkippedSample[isample] == true) {continue;} //Skip this sample

    				// cout<<endl<<UNDL(FBLU("-- Sample : "<<sample_list[isample]<<" : "))<<endl;

    				h_tmp = NULL;
    				TString histo_name = "";
                    if(!drawInputVars && use_combine_file && combineIndividualBins) //Build 'full' template from individual bins
                    {
                        Get_Template_Range(nbins_tmp, xmin_tmp, xmax_tmp, total_var_list[ivar], plot_onlyMaxNodeEvents, nbjets_min, nbjets_max, njets_min, njets_max, minmax_bounds);

                        h_tmp = new TH1F("", "", nIndivBins, xmin_tmp, xmax_tmp);
                        for(int ibin=1; ibin<nIndivBins+1; ibin++)
                        {
                            inputFile_path = Get_HistoFile_InputPath(!drawInputVars, "bin"+Convert_Number_To_TString(ibin)+"_" + var_tmp, cat_tmp, lumiName, use_combine_file, this->filename_suffix, combineFile_fromHarvester, prefit, false);
                            if(inputFile_path == "") {cout<<"Get_HistoFile_InputPath --> file not found ! "<<endl; continue;}
                            file_input = TFile::Open(inputFile_path, "READ");

                            TString dir_hist_tmp = dir_hist_prefix + "bin" + Convert_Number_To_TString(ibin) + "_" + total_var_list[ivar];
                            if(channel_list[ichan] != "") {dir_hist_tmp+= "_" + channel_list[ichan];}
                            if(region != "" && !drawInputVars) {dir_hist_tmp+= "_" + region;}
                            dir_hist_tmp+= "_" + v_lumiYears[iyear];
                            if(combineFile_fromHarvester)
                            {
                                if(prefit) dir_hist_tmp+= "_prefit";
                                else dir_hist_tmp+= "_postfit";
                            }
                            dir_hist_tmp+= "/";

                            // cout<<"dir_hist_tmp/samplename "<<dir_hist_tmp<<samplename<<endl;
                            if(!file_input->GetDirectory(dir_hist_tmp) || !file_input->GetDirectory(dir_hist_tmp)->GetListOfKeys()->Contains(samplename) ) {cout<<FRED("Directory '"<<dir_hist_tmp<<"' or histogram '"<<dir_hist_tmp<<samplename<<"' not found ! Skip...")<<endl; continue;}

                            h_tmp->SetBinContent(ibin, ((TH1F*) file_input->Get(dir_hist_tmp+samplename))->GetBinContent(1)); //Get content/error from individual bin
                            h_tmp->SetBinError(ibin, ((TH1F*) file_input->Get(dir_hist_tmp+samplename))->GetBinError(1));

                            // cout<<"ibin "<<ibin<<endl;
                            // cout<<"h_tmp->GetBinContent("<<ibin<<") "<<h_tmp->GetBinContent(ibin)<<endl;
                            // cout<<"h_tmp->GetBinError(1) "<<h_tmp->GetBinError(1)<<endl;

                            file_input->Close();
                        }
                    }
                    else //Get 'full' templates directly
                    {
                        if(use_combine_file) {histo_name = dir_hist + samplename;}
                        else
                        {
                            histo_name = total_var_list[ivar];
                            if(channel != "") {histo_name+= "_" + channel;}
                            if(region != "" && !drawInputVars) {histo_name+= "_" + region;}
                            histo_name+= "_" + v_lumiYears[iyear];
                            histo_name+= "__" + samplename;
                        }

                        // cout<<"histo_name "<<histo_name<<endl;

                        // Changed -- even if histo not found, still fill dummy vector element (may be just 1 year missing, still need to account for this sample in indices, etc.)
                        if((use_combine_file && !file_input->GetDirectory(dir_hist)->GetListOfKeys()->Contains(samplename)) || (!use_combine_file && !file_input->GetListOfKeys()->Contains(histo_name)) )
                        {
                            if(v_MC_histo.size() <=  index_MC_sample) {v_MC_histo.push_back(NULL);}
                            cout<<ITAL(DIM("Histogram '"<<histo_name<<"' : not found ! Skip..."))<<endl;
                            continue;
                        }
                        //-- Obsolete
                        // if(use_combine_file && !file_input->GetDirectory(dir_hist)->GetListOfKeys()->Contains(samplename) ) {cout<<ITAL(DIM("Histogram '"<<histo_name<<"' : not found ! Skip..."))<<endl; v_isSkippedSample[isample] = true; nof_skipped_samples++; continue;}
                        // else if(!use_combine_file && !file_input->GetListOfKeys()->Contains(histo_name) ) {cout<<ITAL(DIM("Histogram '"<<histo_name<<"' : not found ! Skip..."))<<endl; v_isSkippedSample[isample] = true; nof_skipped_samples++; continue;}

                        h_tmp = (TH1F*) file_input->Get(histo_name);
                        // cout<<"h_tmp->Integral() = "<<h_tmp->Integral()<<endl;
    				}

                    if(use_combine_file && total_var_list[ivar].Contains("countExp")) {h_tmp->Rebin(h_tmp->GetNbinsX());} //Trick: incorrect nof bins in file


                    if(bin_width < 0) {bin_width = h_tmp->GetXaxis()->GetBinWidth(1);}
    				if(draw_errors)
    				{
    					//Initialize error vectors (only once at start)
    					if(nofbins == -1) //if not yet init, get histo parameters
    					{
    						nofbins = h_tmp->GetNbinsX();
    						for(int ibin=0; ibin<nofbins; ibin++)
    						{
    							v_eyl.push_back(0); v_eyh.push_back(0);
                                v_exl.push_back(bin_width / 2); v_exh.push_back(h_tmp->GetXaxis()->GetBinWidth(ibin+1) / 2);
                                // v_exl.push_back(h_tmp->GetXaxis()->GetBinWidth(ibin+1) / 2); v_exh.push_back(h_tmp->GetXaxis()->GetBinWidth(ibin+1) / 2);
    							v_x.push_back( (h_tmp->GetXaxis()->GetBinLowEdge(nofbins+1) - h_tmp->GetXaxis()->GetBinLowEdge(1) ) * ((ibin+1 - 0.5)/nofbins) + h_tmp->GetXaxis()->GetBinLowEdge(1));
    							v_y.push_back(0);
    						}
    					}

    					//-- Add errors in quadrature
    					for(int ibin=0; ibin<nofbins; ibin++) //Start at bin 1
    					{
                            // cout<<"h_tmp->GetBinContent(ibin+1) "<<h_tmp->GetBinContent(ibin+1)<<endl;
                            v_y[ibin]+= h_tmp->GetBinContent(ibin+1); //This vector is used to know where to draw the error zone on plot (= on top of stack)

                            //-- Protection //Prevent drawing of unphysical uncertainties from postfit combine file (should fix the source of the problem... often due to threshold effects near 0)
                            if(abs(h_tmp->GetBinError(ibin+1)) > h_tmp->GetBinContent(ibin+1)*50)
                            {
                                cout<<FRED("Warning: error>content*50, skip this error (bin "<<ibin+1<<" / "<<sample_list[isample]<<" / "<<v_lumiYears[iyear]<<")")<<endl;
                                continue;
                            }

    						//-- If using Combine output file (from MLF), bin error contains total error. Else if using template file directly, just MC stat. error
                            //NB: this is actually incorrect because we can't sum correlated errors simply in quadrature ! Works if summing only MCstat errors, else (e.g. for total error from combine) must do differently
                            v_eyl[ibin]+= pow(h_tmp->GetBinError(ibin+1), 2);
    						v_eyh[ibin]+= pow(h_tmp->GetBinError(ibin+1), 2);

                            //-- Debug printouts for a single bin //NB: ibin starts at 0 for first bin (ibin=6 -> look at histo bin 7)
    						// if(ibin != 0) {continue;} //cout only 1 bin
                            // cout<<"sample_list[isample] "<<sample_list[isample]<<endl;
                            // cout<<"v_lumiYears[iyear] "<<v_lumiYears[iyear]<<endl;
    						// cout<<"x = "<<v_x[ibin]<<endl;    cout<<", y = "<<v_y[ibin]<<endl;    cout<<", sqrt(eyl) = "<<sqrt(v_eyl[ibin])<<endl;    cout<<", sqrt(eyh) = "<<sqrt(v_eyh[ibin])<<endl; //cout<<", exl = "<<v_exl[ibin]<<endl;    cout<<", exh = "<<v_exh[ibin]<<endl;
    					} //loop on bins

    					//-- Draw all errors
    					//--------------------------------------------
    					if(!use_combine_file) //In Combine file, already accounted in binError
    					{
    						for(int itree=0; itree<systTree_list.size(); itree++)
    						{
                                if(systTree_list[itree] != "" && systTree_list[itree] != nominal_tree_name && (sample_list[isample] == "DATA" || sample_list[isample] == "DY" || sample_list[isample].Contains("NPL") || sample_list[isample].Contains("TTbar")) ) {continue;}

    							for(int isyst=0; isyst<syst_list.size(); isyst++)
    							{
    								//-- Protections : not all syst weights apply to all samples, etc.
    								if(syst_list[isyst] != "" && systTree_list[itree] != "" && systTree_list[itree] != nominal_tree_name) {break;} //JES,JER,... -> read first element only
                                    else if(v_lumiYears[iyear] == "2018" && syst_list[isyst].BeginsWith("prefir") ) {continue;} //no prefire in 2018

    								// cout<<"sample "<<sample_list[isample]<<" / channel "<<channel_list[ichan]<<" / syst "<<syst_list[isyst]<<endl;

    								TString histo_name_syst = histo_name + "__";
                                    if(syst_list[isyst] != "" || (systTree_list[itree] != "" && systTree_list[itree] != nominal_tree_name)) {histo_name_syst+= Get_Modified_SystName(syst_list[isyst]+systTree_list[itree], v_lumiYears[iyear], sample_list[isample]);}
                                    else {histo_name_syst+= systTree_list[itree];}

    								if(!file_input->GetListOfKeys()->Contains(histo_name_syst)) {continue;} //No error messages if systematics histos not found

                                    TH1F* histo_syst = NULL; //Read the "systematics histograms"
    								histo_syst = (TH1F*) file_input->Get(histo_name_syst);

    								//Add up here the different errors (quadratically), for each bin separately
    								for(int ibin=0; ibin<nofbins; ibin++)
    								{
    									if(histo_syst->GetBinContent(ibin+1) == 0) {continue;} //Some syst may be null, don't compute diff

    									double tmp = 0;

    									//For each systematic, compute (shifted-nominal), check the sign, and add quadratically to the corresponding bin error
    									tmp = histo_syst->GetBinContent(ibin+1) - h_tmp->GetBinContent(ibin+1);

    									if(tmp>0) {v_eyh[ibin]+= pow(tmp,2);}
    									else if(tmp<0) {v_eyl[ibin]+= pow(tmp,2);}

    									if(ibin > 0) {continue;} //cout only first bin
    								}

    								delete histo_syst;
    							} //end syst loop
    						} //systTree_list loop
    					} //use combine file?
    				} //draw errors?

    				//-- Set histo style (use color vector filled in main) //NB: run for all sub-histos... for simplicity
                    //---------------------------------------------------
    				h_tmp->SetFillStyle(1001);
    				if(samplename == "Fakes") {h_tmp->SetFillStyle(3005);}
    		        else if(samplename == "QFlip" ) {h_tmp->SetFillStyle(3006);}

    				h_tmp->SetFillColor(color_list[isample]);
    				h_tmp->SetLineColor(kBlack);
                    h_tmp->SetLineColor(color_list[isample]);
                    // cout<<"sample_list[isample] "<<sample_list[isample];
                    // cout<<" => color_list[isample] "<<color_list[isample]<<endl;

    				//Check color of previous *used* sample (up to 6, to account for potentially skipped samples)
    				for(int k=1; k<6; k++)
    				{
    					if(isample - k >= 0)
    					{
    						if(v_isSkippedSample[isample-k]) {continue;} //If previous sample was skipped, don't check its color
    						else if(color_list[isample] == color_list[isample-k]) {h_tmp->SetLineColor(color_list[isample]); break;} //If previous sample had same color, don't draw line
    					}
    					else {break;}
    				}
                    if(region == "twz" && samplename == "tWZ") {h_tmp->SetFillColor(kBlue);} //Different color
                    //---------------------------------------------------

                    //-- Fill vector of MC histograms
                    if(v_MC_histo.size() <=  index_MC_sample) {v_MC_histo.push_back((TH1F*) h_tmp->Clone());}
                    else if(!v_MC_histo[index_MC_sample] && h_tmp) {v_MC_histo[index_MC_sample] = (TH1F*) h_tmp->Clone();}
                    else {v_MC_histo[index_MC_sample]->Add((TH1F*) h_tmp->Clone());}
                    if(v_MC_histo[index_MC_sample]) {v_MC_histo[index_MC_sample]->SetDirectory(0);} //Dis-associate histo from TFile //https://root.cern.ch/root/htmldoc/guides/users-guide/ObjectOwnership.html

                    //-- Debug printouts
    				// cout<<"sample : "<<sample_list[isample]<<" / color = "<<color_list[isample]<<" fillstyle = "<<h_tmp->GetFillStyle()<<endl;
    				// cout<<"index_MC_sample "<<index_MC_sample<<endl;
    				// cout<<"v_MC_histo.size() "<<v_MC_histo.size()<<endl;
    				// cout<<"v_MC_samples_legend.size() "<<v_MC_samples_legend.size()<<endl<<endl;

                    v_yields_processes[isample]+= h_tmp->Integral(); //For printouts

    				delete h_tmp; h_tmp = NULL; //No crash ? (else only delete if new)
    			} //end sample loop


// #####    ##   #####   ##
// #    #  #  #    #    #  #
// #    # #    #   #   #    #
// #    # ######   #   ######
// #    # #    #   #   #    #
// #####  #    #   #   #    #

                //-- NB: implemented such that data histos should be read even if blinded; in the latter case, the data is then manually set to 0
                if(use_combine_file)
                {
                    TString dataname = "data"; //FitDiag convention ?
                    if(combineFile_fromHarvester) {dataname = "data_obs";} //CH convention

                    if(combineIndividualBins) //Build 'full' template from individual bins
                    {
                        if(combineFile_fromHarvester) //CH conventions : data = TH1F
                        {
                            hdata_tmp = new TH1F("", "", nIndivBins, xmin_tmp, xmax_tmp);
                            if(use_poisson_dataErrors) {hdata_tmp->SetBinErrorOption(TH1::kPoisson);}

                            for(int ibin=1; ibin<nIndivBins+1; ibin++)
                            {
                                inputFile_path = Get_HistoFile_InputPath(!drawInputVars, "bin"+Convert_Number_To_TString(ibin)+"_" + var_tmp, cat_tmp, lumiName, use_combine_file, this->filename_suffix, combineFile_fromHarvester, prefit, false);
                                if(inputFile_path == "") {cout<<"Get_HistoFile_InputPath --> file not found ! "<<endl; continue;}
                                file_input = TFile::Open(inputFile_path, "READ");

                                // cout<<"ibin "<<ibin<<endl;
                                TString dir_hist_tmp = dir_hist_prefix + "bin" + Convert_Number_To_TString(ibin) + "_" + total_var_list[ivar];
                                if(channel_list[ichan] != "") {dir_hist_tmp+= "_" + channel_list[ichan];}
                                if(region != "" && !drawInputVars) {dir_hist_tmp+= "_" + region;}
                                dir_hist_tmp+= "_" + v_lumiYears[iyear];
                                if(combineFile_fromHarvester)
                                {
                                    if(prefit) dir_hist_tmp+= "_prefit";
                                    else dir_hist_tmp+= "_postfit";
                                }
                                dir_hist_tmp+= "/";

                                // cout<<"dir_hist_tmp+dataname= "<<dir_hist_tmp+dataname<<endl;
                                if(!file_input->GetDirectory(dir_hist_tmp) || !file_input->GetDirectory(dir_hist_tmp)->GetListOfKeys()->Contains(dataname) ) {cout<<FRED("Directory '"<<dir_hist_tmp<<"' or histogram '"<<dir_hist_tmp+dataname<<"' not found ! Skip...")<<endl; continue;}

                                float content = ((TH1F*) file_input->Get(dir_hist_tmp+dataname))->GetBinContent(1), error = ((TH1F*) file_input->Get(dir_hist_tmp+dataname))->GetBinError(1);
                                // cout<<"content "<<content<<endl;
                                if(!isnan(content) && !isinf(content))
                                {
                                    hdata_tmp->SetBinContent(ibin, content); //Get content/error from individual bin
                                    hdata_tmp->SetBinError(ibin, error);
                                }
                                else //NaN data //Seems to happen in some cases for bins with no data
                                {
                                    cout<<FRED("ERROR: bin "<<ibin<<" is NaN/inf ! ("<<dir_hist_tmp+dataname<<")")<<endl;
                                }
                                // cout<<"hdata_tmp->GetBinContent(1) "<<hdata_tmp->GetBinContent(1)<<endl;
                                // cout<<"hdata_tmp->GetBinError(1) "<<hdata_tmp->GetBinError(1)<<endl;

                                file_input->Close();
                            }
                        }
                        else //FitDiag conventions : data = TGraph
                        {
                            double theErrorX_h[nIndivBins];
                            double theErrorY_h[nIndivBins];
                            double theErrorX_l[nIndivBins];
                            double theErrorY_l[nIndivBins];
                            double theY[nIndivBins];
                            double theX[nIndivBins];

                            for(int ibin=1; ibin<nIndivBins+1; ibin++)
                            {
                                // cout<<"ibin "<<ibin<<endl;
                                TString dir_hist_tmp = dir_hist_prefix + "bin" + Convert_Number_To_TString(ibin) + "_" + total_var_list[ivar];
                                if(channel_list[ichan] != "") {dir_hist_tmp+= "_" + channel_list[ichan];}
                                if(region != "" && !drawInputVars) {dir_hist_tmp+= "_" + region;}
                                dir_hist_tmp+= "_" + v_lumiYears[iyear];
                                if(combineFile_fromHarvester) {dir_hist_tmp+= "_prefit";}
                                dir_hist_tmp+= "/";

                                // cout<<"dir_hist_tmp+dataname = "<<dir_hist_tmp+dataname<<endl;
                                if(!file_input->GetDirectory(dir_hist_tmp) || !file_input->GetDirectory(dir_hist_tmp)->GetListOfKeys()->Contains(dataname) ) {cout<<FRED("Directory '"<<dir_hist_tmp<<"' or histogram '"<<dir_hist_tmp+dataname<<"' not found ! Skip...")<<endl; continue;}
                                TGraphAsymmErrors* g_tmp2 = (TGraphAsymmErrors*) file_input->Get(dir_hist_tmp+dataname);
                                Double_t x, y; g_tmp2->GetPoint(0, x, y); //Read y coordinate (x is irrelevant)

                                theX[ibin-1] = ibin - bin_width/2.; theY[ibin-1] = y; //Set x/y coordinates for this point
                                theErrorY_h[ibin-1] = g_tmp2->GetErrorYhigh(0); theErrorY_l[ibin-1] = g_tmp2->GetErrorYlow(0); //Set y-errors
                                theErrorX_h[ibin-1] = 0; theErrorX_l[ibin-1] = 0; //Irrelevant
                            }
                            g_tmp = new TGraphAsymmErrors(nofbins,theX,theY,theErrorX_l,theErrorX_h,theErrorY_l,theErrorY_h); //Build full TGraph
                        }
                    }
                    else //Get 'full' templates directly
                    {
                        if(combineFile_fromHarvester) //CH conventions : data = TH1F
                        {
                            TString dir_hist_tmp = total_var_list[ivar];
                            if(channel_list[ichan] != "") {dir_hist_tmp+= "_" + channel_list[ichan];}
                            if(region != "" && !drawInputVars) {dir_hist_tmp+= "_" + region;}
                            dir_hist_tmp+= "_" + v_lumiYears[iyear];
                            if(prefit) {dir_hist_tmp+= "_prefit";}
                            else {dir_hist_tmp+= "_postfit";}
                            dir_hist_tmp+= "/";

                            // cout<<"dir_hist_tmp+dataname "<<dir_hist_tmp+dataname<<endl;
                            hdata_tmp = (TH1F*) file_input->Get(dir_hist_tmp+dataname)->Clone();
                            if(total_var_list[ivar].Contains("countExp")) {hdata_tmp->Rebin(hdata_tmp->GetNbinsX());} //Trick: incorrect nof bins in file

                            hdata_tmp->SetDirectory(0); //Dis-associate from TFile //https://root.cern.ch/root/htmldoc/guides/users-guide/ObjectOwnership.html
                        }
                        else
                        {
                            data_histo_name = dir_hist + dataname;
                            // cout<<"data_histo_name "<<data_histo_name<<endl;
                            g_tmp = (TGraphAsymmErrors*) file_input->Get(data_histo_name); //stored as TGraph
                        }
                    }

                    if(!combineFile_fromHarvester)
                    {
                        //-- Remove X-axis error bars, not needed for plot
                        for(int ipt=0; ipt<g_tmp->GetN(); ipt++)
                        {
                            g_tmp->SetPointEXhigh(ipt, 0);
                            g_tmp->SetPointEXlow(ipt, 0);
                        }

                        if(!g_data) {g_data = (TGraphAsymmErrors*) g_tmp->Clone();}
                        else //Need to sum TGraphs content by hand
                        {
                            double x_tmp,y_tmp,errory_low_tmp,errory_high_tmp;
                            for(int ipt=0; ipt<g_data->GetN(); ipt++)
                            {
                                g_data->GetPoint(ipt, x, y);
                                errory_low = g_data->GetErrorYlow(ipt);
                                errory_high = g_data->GetErrorYhigh(ipt);

                                g_tmp->GetPoint(ipt, x_tmp, y_tmp);
                                errory_low_tmp = g_tmp->GetErrorYlow(ipt);
                                errory_high_tmp = g_tmp->GetErrorYhigh(ipt);

                                double new_error_low = sqrt(errory_low*errory_low+errory_low_tmp*errory_low_tmp);
                                double new_error_high = sqrt(errory_high_tmp*errory_high_tmp+errory_high_tmp*errory_high_tmp);
                                g_data->SetPoint(ipt, x, y+y_tmp);
                                g_data->SetPointError(ipt,0,0, new_error_low, new_error_high); //ok to add errors in quadrature ?

                                // cout<<"ipt "<<ipt<<" / x1 "<<x<<" / y1 "<<y<<" / error1 "<<errory_low<<", "<<errory_high<<endl;
                                // cout<<"ipt "<<ipt<<" / x2 "<<x_tmp<<" / y2 "<<y_tmp<<" / error2 "<<errory_low_tmp<<", "<<errory_high_tmp<<endl;
                                // cout<<"=> y1+y2 = "<<y+y_tmp<<" / error = "<<new_error_low<<", "<<new_error_high<<endl;
                            }
                        }
                        if(g_tmp) {delete g_tmp; g_tmp = NULL;} //No crash ? (else only delete if new)
                    }
                    else
                    {
                        if(h_sum_data == NULL) {h_sum_data = (TH1F*) hdata_tmp->Clone();}
                        else {h_sum_data->Add((TH1F*) hdata_tmp->Clone());}

                        h_sum_data->SetDirectory(0); //Dis-associate from TFile
                        delete hdata_tmp; hdata_tmp = NULL; //No crash ? (else only delete if new)
                    }
        		}
                else //If using template file made from this code
                // else if(!this->is_blind)
                {
                    data_histo_name = total_var_list[ivar];
                    if(channel != "") {data_histo_name+= "_" + channel;}
                    if(region != "" && !drawInputVars) {data_histo_name+= "_" + region;}
                    data_histo_name+= "_" + v_lumiYears[iyear] + "__data_obs";

        			if(!file_input->GetListOfKeys()->Contains(data_histo_name)) {cout<<data_histo_name<<" : not found"<<endl;}
        			else
        			{
        				h_tmp = (TH1F*) file_input->Get(data_histo_name);
        				if(h_sum_data == NULL) {h_sum_data = (TH1F*) h_tmp->Clone();}
        				else {h_sum_data->Add((TH1F*) h_tmp->Clone());} //not needed anymore (1 channel only)
                        h_sum_data->SetDirectory(0); //Dis-associate from TFile
        			}
        		}

        		if(use_combine_file && !combineFile_fromHarvester && !g_data && !this->is_blind) {cout<<endl<<BOLD(FRED("--- Empty data TGraph !"))<<endl<<endl; data_notEmpty = false;}
        		if((!use_combine_file || combineFile_fromHarvester) && !h_sum_data && !this->is_blind) {cout<<endl<<BOLD(FRED("--- Empty data histogram "<<data_histo_name<<" !"))<<endl<<endl; data_notEmpty = false;}

        		//-- Make sure there are no negative bins
                //-- Also blind the data manually if needed here
        		if(data_notEmpty)
        		{
        			if(use_combine_file && !combineFile_fromHarvester) //TGraph object
        			{
        				for(int ipt=0; ipt<g_data->GetN(); ipt++)
        				{
        					g_data->GetPoint(ipt, x, y);
                            // cout<<"x "<<x<<" / y "<<y<<endl;
                            if(this->is_blind || y<0) {g_data->SetPoint(ipt, x, 0); g_data->SetPointError(ipt,0,0,0,0);}
                        }
        			}
        			else //TH1F object
        			{
        				for(int ibin=1; ibin<h_sum_data->GetNbinsX()+1; ibin++)
        				{
                            if(h_sum_data && (this->is_blind || h_sum_data->GetBinContent(ibin) < 0)) {h_sum_data->SetBinContent(ibin, 0); h_sum_data->SetBinError(ibin, 0);}
                        }
        			}

        		}
                if(use_poisson_dataErrors) {h_sum_data->SetBinErrorOption(TH1::kPoisson);}

            } //channel loop
        } //years loop


// # #    # #####  ###### #    #
// # ##   # #    # #       #  #
// # # #  # #    # #####    ##
// # #  # # #    # #        ##
// # #   ## #    # #       #  #
// # #    # #####  ###### #    #

        //-- Get indices of particular samples, sum the others into 1 single histo (used for ratio subplot)
        TH1F* histo_total_MC = NULL; //Sum of all MC samples

    	//Indices of important samples, for specific treatment
    	int index_tZq_sample = -9;
        int index_ttZ_sample = -9;
        int index_tWZ_sample = -9;
    	int index_NPL_sample = -9;

    	// cout<<"v_MC_histo.size() "<<v_MC_histo.size()<<endl;
    	// cout<<"v_MC_samples_legend.size() "<<v_MC_samples_legend.size()<<endl;

    	//Merge all the MC nominal histograms (contained in v_MC_histo)
    	for(int i=0; i<v_MC_histo.size(); i++)
    	{
    		if(!v_MC_histo[i]) {continue;} //Some templates may be null

    		// cout<<"v_MC_samples_legend[i] "<<v_MC_samples_legend[i]<<endl;

    		if(v_MC_samples_legend[i].Contains("tZq") )
    		{
    			if(index_tZq_sample<0) {index_tZq_sample = i;}
    			if(!h_tzq) {h_tzq = (TH1F*) v_MC_histo[i]->Clone();}
    			else {h_tzq->Add((TH1F*) v_MC_histo[i]->Clone());}
    			// if(doNot_stack_signal) continue; //don't stack
    		}
    		else if(v_MC_samples_legend[i].EndsWith("ttZ") )
    		{
    			if(index_ttZ_sample<0) {index_ttZ_sample = i;}
    			if(!h_ttz) {h_ttz = (TH1F*) v_MC_histo[i]->Clone();}
    			else {h_ttz->Add((TH1F*) v_MC_histo[i]->Clone());}
    			// if(doNot_stack_signal) continue; //don't stack
    		}
            else if(v_MC_samples_legend[i] == "tWZ") {index_tWZ_sample = i;}

    		// cout<<"Adding sample "<<v_MC_samples_legend[i]<<" to histo_total_MC"<<endl;

    		if(!histo_total_MC) {histo_total_MC = (TH1F*) v_MC_histo[i]->Clone();}
    		else {histo_total_MC->Add((TH1F*) v_MC_histo[i]->Clone());}
    	}

        //-- If histo_total_MC is null, variable was not found --> skip it
        if(!histo_total_MC)
        {
            cout<<FRED("Error ! Variable '"<<total_var_list[ivar]<<"' not found ! Skip it...")<<endl;
            continue;
        }


// ##### #    #  ####  #####   ##    ####  #    #
//   #   #    # #        #    #  #  #    # #   #
//   #   ######  ####    #   #    # #      ####
//   #   #    #      #   #   ###### #      #  #
//   #   #    # #    #   #   #    # #    # #   #
//   #   #    #  ####    #   #    #  ####  #    #

		THStack* stack_MC = new THStack;

		//Add legend entries -- iterate backwards, so that last histo stacked is on top of legend
		//Also add MC histograms to the THStack
		for(int i=v_MC_histo.size()-1; i>=0; i--)
		{
			if(!v_MC_histo[i]) {continue;} //Some templates may be null

            if(region=="twz" && v_MC_samples_legend[i] == "tWZ") {continue;} //Put tWZ on top in that region

			stack_MC->Add(v_MC_histo[i]);
			// cout<<"Stacking sample "<<v_MC_samples_legend[i]<<" / integral "<<v_MC_histo[i]->Integral()<<endl;
            // cout<<"stack bin 1 content = "<<((TH1*) stack_MC->GetStack()->Last())->GetBinContent(1)<<endl;
		}
        if(region=="twz") {stack_MC->Add(v_MC_histo[index_tWZ_sample]);} //Put tWZ on top in that region

        //-- Debug printout (THStack bins contents)
        // for(int ibin=1; ibin<last->GetNbinsX()+1; ibin++) {cout<<"stack bin "<<ibin<<" content = "<<((TH1*) stack_MC->GetStack()->Last())->GetBinContent(ibin)<<endl;}


// ####### #
//    #    #       ######  ####  ###### #    # #####
//    #    #       #      #    # #      ##   # #    #
//    #    #       #####  #      #####  # #  # #    #
//    #    #       #      #  ### #      #  # # #    #
//    #    #       #      #    # #      #   ## #    #
//    #    ####### ######  ####  ###### #    # #####

        //NB: for best style, better to adjust legend size exactly to the entries (too much space --> non uniform)
        int n_columns = ceil(nSampleGroups/2.) > 6 ? 6 : ceil(nSampleGroups/2.); //ceil = upper int
        float x_left = 0.94-n_columns*0.12; //Each column allocated same x-space //0.12 needed for most crowded plots
        if(x_left < 0.4) {x_left = 0.4;} //Leave some space for region label
        n_columns = 3; //HARDCODED FOR NOW

        float ylegend = 0.78; //Default
        if(use_paperStyle) {ylegend+= 0.01;}
        TLegend* qw = new TLegend(x_left-0.04,ylegend,0.94,ylegend+0.15); //Default
        qw->SetTextSize(0.035);
        qw->SetNColumns(n_columns);
        qw->SetBorderSize(0);
        qw->SetFillStyle(0); //transparent
        qw->SetTextAlign(12); //align = 10*HorizontalAlign + VerticalAlign //Horiz: 1=left adjusted, 2=centered, 3=right adjusted //Vert: 1=bottom adjusted, 2=centered, 3=top adjusted

        //-- Dummy object, only used to display uncertainty band also in legend
        TH1F* h_uncert = new TH1F("h_uncert", "h_uncert", 1, 0, 1);
        h_uncert->SetFillStyle(3254); //3002 //3004
        h_uncert->SetFillColor(kBlack);
        h_uncert->SetLineWidth(0.);
        // qw->AddEntry(h_uncert, "Uncert.", "F");
        // qw->AddEntry(h_uncert, "Uncertainty", "F");

		//--Data on top of legend
        if(!this->is_blind)
        {
            if(use_combine_file && !combineFile_fromHarvester && g_data != 0) {qw->AddEntry(g_data, "Data" , "ep");}
            else if((!use_combine_file || combineFile_fromHarvester) && h_sum_data != 0) {qw->AddEntry(h_sum_data, "Data" , "ep");}
            else {cout<<__LINE__<<BOLD(FRED(" : null data !"))<<endl;}
        }

		for(int i=0; i<v_MC_histo.size(); i++)
		{
            // cout<<"v_MC_samples_legend["<<i<<"] "<<v_MC_samples_legend[i]<<endl;
			if(!v_MC_histo[i]) {continue;} //Fakes templates can be null

            qw->AddEntry(v_MC_histo[i], v_MC_samples_legend[i], "f");

            // if(v_MC_samples_legend[i].Contains("tZq")) {qw->AddEntry(v_MC_histo[i], "tZq", "f");}
		}

        // qw->SetTextFont(43);
        // gStyle->SetTextFont(72);
        // qw->AddEntry(v_MC_histo[0], ((TString) "\\text{a}"), "L"); //test, to remove


// #####  #####    ##   #    #
// #    # #    #  #  #  #    #
// #    # #    # #    # #    #
// #    # #####  ###### # ## #
// #    # #   #  #    # ##  ##
// #####  #    # #    # #    #

		//-- Canvas definition
		Load_Canvas_Style(); //Default top/bottom/left/right margins: 0.07/0.13/0.16/0.03
		TCanvas* c1 = new TCanvas("c1", "c1", 1000, 800);

        //-- Override Load_Canvas_Style margins
        c1->SetTopMargin(0.06);
        c1->SetBottomMargin(0.30);
        c1->SetRightMargin(0.04);
        c1->SetLeftMargin(0.14);

		if(draw_logarithm) {c1->SetLogy();}

		//Draw stack
		stack_MC->Draw("hist");

		//Draw data
		if(data_notEmpty && !this->is_blind)
		{
			if(use_combine_file && !combineFile_fromHarvester)
			{
				g_data->SetMarkerStyle(20);
				g_data->Draw("epsame"); //e0psame to draw empty data
			}
			else
			{
				h_sum_data->SetMarkerStyle(20);
				h_sum_data->SetMinimum(0.001); //0 fails in logscale
                h_sum_data->Draw("epsame"); //e0psame to draw empty data
			}
		}

        qw->Draw("same"); //Draw legend


// #   # #    #   ##   #    #
//  # #  ##  ##  #  #   #  #
//   #   # ## # #    #   ##
//   #   #    # ######   ##
//   #   #    # #    #  #  #
//   #   #    # #    # #    #

        c1->Update();

        //-- Set Yaxis maximum & minimum
        double ymax = 0;
        if(use_combine_file && !combineFile_fromHarvester)
		{
			Long64_t locmax = TMath::LocMax(g_data->GetN(), g_data->GetY()); //the corresponding x value can be obtained with double xmax = gr->GetX()[locmax];
            if(data_notEmpty) {ymax = g_data->GetY()[locmax];} //Data ymax
            if(ymax < stack_MC->GetMaximum()) {ymax = stack_MC->GetMaximum();} //MC ymax
		}
		else
		{
            if(data_notEmpty) {ymax = h_sum_data->GetMaximum();} //Data ymax
            if(ymax < stack_MC->GetMaximum()) {ymax = stack_MC->GetMaximum();} //MC ymax
		}
        // ymax*= 1.4; //Previous default, for ratio pad 0.25
        ymax*= 1.6; //CHANGED -- larger ratio pad (0.3) -> must make more place for legend
        stack_MC->SetMaximum(ymax);
        // if(ymax > qw->GetY1()) {ymax = qw->GetY1();} //Avoid overlap with TLegend
        // cout<<"qw->GetY1() "<<qw->GetY1()<<endl;
        // cout<<"c1->GetUymax() "<<c1->GetUymax()<<endl;

		stack_MC->SetMinimum(0.0001); //Remove '0' label

		if(draw_logarithm) //Can't use 0
		{
            stack_MC->SetMinimum(1.2); //Default
            if(total_var_list[ivar].Contains("mTW")) {stack_MC->SetMinimum(2.5);}

			stack_MC->SetMaximum(stack_MC->GetMaximum()*20); //Must use higher threshold in log
		}

        c1->Modified();


// ###### #####  #####   ####  #####   ####      ####  #####   ##    ####  #    #
// #      #    # #    # #    # #    # #         #        #    #  #  #    # #   #
// #####  #    # #    # #    # #    #  ####      ####    #   #    # #      ####
// #      #####  #####  #    # #####       #         #   #   ###### #      #  #
// #      #   #  #   #  #    # #   #  #    #    #    #   #   #    # #    # #   #
// ###### #    # #    #  ####  #    #  ####      ####    #   #    #  ####  #    #

		//-- Compute sqrt of quadratic errors
		if(draw_errors)
		{
			for(int ibin=0; ibin<nofbins; ibin++)
			{
				v_eyh[ibin] = pow(v_eyh[ibin], 0.5);
				v_eyl[ibin] = pow(v_eyl[ibin], 0.5);

				// if(ibin > 0) {continue;} //cout only first bin
				// cout<<"x = "<<v_x[ibin]<<endl;    cout<<", y = "<<v_y[ibin]<<endl;    cout<<", eyl = "<<v_eyl[ibin]<<endl;    cout<<", eyh = "<<v_eyh[ibin]<<endl; //cout<<", exl = "<<v_exl[ibin]<<endl;    cout<<", exh = "<<v_exh[ibin]<<endl;
			}
		}

        //-- CHANGED -- Read full error from combine here
        //NB: the only way to get the correct, total uncertainty in a given bin, is to make combine sum all errors by itself  (--total-shapes options) ==> Read corresponding bin error here, and assign it to corresponding histo bin
        if(use_combine_file)
        {
            for(int ibin=1; ibin<nIndivBins+1; ibin++)
            {

                inputFile_path = Get_HistoFile_InputPath(!drawInputVars, "bin"+Convert_Number_To_TString(ibin)+"_" + var_tmp, cat_tmp, lumiName, use_combine_file, this->filename_suffix, combineFile_fromHarvester, prefit, false);
                if(inputFile_path == "") {cout<<"Get_HistoFile_InputPath --> file not found ! "<<endl; continue;}
                file_input = TFile::Open(inputFile_path, "READ");
                TString dir_hist_tmp = prefit? "prefit/":"postfit/"; //Total dir
                // cout<<"dir_hist_tmp/TotalProcs "<<dir_hist_tmp+"TotalProcs"<<" / Total error = "<<((TH1F*) file_input->Get(dir_hist_tmp+"TotalProcs"))->GetBinError(1)<<endl;
                if(!file_input->GetDirectory(dir_hist_tmp) || !file_input->GetDirectory(dir_hist_tmp)->GetListOfKeys()->Contains("TotalProcs") ) {cout<<FRED("Directory '"<<dir_hist_tmp<<"' or histogram '"<<dir_hist_tmp<<"TotalProcs' not found ! Skip...")<<endl; continue;}
                v_eyl[ibin-1] = ((TH1F*) file_input->Get(dir_hist_tmp+"TotalProcs"))->GetBinError(1);
                v_eyh[ibin-1] = ((TH1F*) file_input->Get(dir_hist_tmp+"TotalProcs"))->GetBinError(1);
                file_input->Close();
            }
        }

		//Use pointers to vectors : need to give the adress of first element (all other elements can then be accessed iteratively)
		double* eyl = &v_eyl[0];
		double* eyh = &v_eyh[0];
		double* exl = &v_exl[0];
		double* exh = &v_exh[0];
		double* xx = &v_x[0];
		double* yy = &v_y[0];

		//Create TGraphAsymmErrors with the error vectors / (x,y) coordinates --> Can superimpose it on plot
		TGraphAsymmErrors* gr_error = NULL;

		gr_error = new TGraphAsymmErrors(nofbins,xx,yy,exl,exh,eyl,eyh);
        gr_error->SetFillStyle(3254); //3002 //3004
		gr_error->SetFillColor(kBlack);
		gr_error->Draw("e2 same"); //Superimposes the uncertainties on stack


// #####    ##   ##### #  ####
// #    #  #  #    #   # #    #
// #    # #    #   #   # #    #
// #####  ######   #   # #    #
// #   #  #    #   #   # #    #
// #    # #    #   #   #  ####

// #####  #       ####  #####
// #    # #      #    #   #
// #    # #      #    #   #
// #####  #      #    #   #
// #      #      #    #   #
// #      ######  ####    #

		//-- create subpad to plot ratio
		TPad* pad_ratio = new TPad("pad_ratio", "pad_ratio", 0.0, 0.0, 1.0, 1.0);
		pad_ratio->SetFillColor(0);
		pad_ratio->SetFillStyle(0);
		pad_ratio->SetGridy(1);
		pad_ratio->Draw();
		pad_ratio->cd(0);

        //-- Override Load_Canvas_Style setting
        pad_ratio->SetTopMargin(0.70);
        pad_ratio->SetRightMargin(0.04);
        pad_ratio->SetLeftMargin(0.14);

		if(use_combine_file && !combineFile_fromHarvester && data_notEmpty) //Copy the content of the data graph into a TH1F (NB : symmetric errors... but anyway symmetric for data)
		{
			if(!v_MC_histo[0]) {cout<<__LINE__<<FRED("Error : v_MC_histo[0] is null ! Abort")<<endl; return;}

	        h_sum_data = (TH1F*) v_MC_histo[0]->Clone(); //Clone binning of the MC histos
			h_sum_data->SetFillColor(kBlack);
			h_sum_data->SetLineColor(kBlack);
			// cout<<"h_sum_data->GetNbinsX() "<<h_sum_data->GetNbinsX()<<endl;

	        for(int ipt=0; ipt<g_data->GetN(); ipt++)
	        {
	            g_data->GetPoint(ipt, x, y);
	            double error = g_data->GetErrorY(ipt);

	            h_sum_data->SetBinContent(ipt+1, y);
	            h_sum_data->SetBinError(ipt+1, error);
	        }
		}

		TH1F* histo_ratio_data = NULL;
		if(data_notEmpty)
		{
			histo_ratio_data = (TH1F*) h_sum_data->Clone();
            if(use_poisson_dataErrors) {histo_ratio_data->SetBinErrorOption(TH1::kPoisson);}

			if(!show_pulls_ratio) //Compute ratios (with error bars)
			{
				//To get correct error bars in ratio plot, must only account for errors from data, not MC ! (MC error shown as separate band)
				for(int ibin=1; ibin<histo_total_MC->GetNbinsX()+1; ibin++)
				{
					histo_total_MC->SetBinError(ibin, 0.);
				}

				histo_ratio_data->Divide(histo_total_MC);

                //Debug printouts
                // int ibin = 7;
    			// cout<<"h_sum_data->GetBinContent("<<ibin<<") "<<h_sum_data->GetBinContent(ibin)<<endl;
    			// cout<<"h_sum_data->GetBinError("<<ibin<<") "<<h_sum_data->GetBinError(ibin)<<endl;
    			// cout<<"histo_total_MC->GetBinContent("<<ibin<<") "<<histo_total_MC->GetBinContent(ibin)<<endl;
    			// cout<<"histo_total_MC->GetBinError("<<ibin<<") "<<histo_total_MC->GetBinError(ibin)<<endl;
                // cout<<"histo_ratio_data->GetBinError("<<ibin<<") "<<histo_ratio_data->GetBinError(ibin)<<endl;
                // cout<<"histo_ratio_data->GetBinError("<<ibin<<") "<<histo_ratio_data->GetBinError(ibin)<<endl;
			}
		 	else //-- Compute pulls (no error bars)
			{
				for(int ibin=1; ibin<histo_ratio_data->GetNbinsX()+1; ibin++)
				{
					//Add error on signal strength (since we rescale signal manually)
					// double bin_error_mu = v_MC_histo.at(index_tZq_sample)->GetBinError(ibin) * sig_strength_err;
					// cout<<"bin_error_mu = "<<bin_error_mu<<endl;

					double bin_error_mu = 0; //No sig strength uncert. for prefit ! //-- postfit -> ?

					//Quadratic sum of systs, stat error, and sig strength error
					double bin_error = pow(pow(histo_total_MC->GetBinError(ibin), 2) + pow(histo_ratio_data->GetBinError(ibin), 2) + pow(bin_error_mu, 2), 0.5);

					// if(ibin==1) {cout<<"Data = "<<histo_ratio_data->GetBinContent(1)<<" / Total MC = "<<histo_total_MC->GetBinContent(1)<<" / error = "<<bin_error<<endl;}

					if(!histo_total_MC->GetBinError(ibin)) {histo_ratio_data->SetBinContent(ibin,-99);} //Don't draw null markers
					else{histo_ratio_data->SetBinContent(ibin, (histo_ratio_data->GetBinContent(ibin) - histo_total_MC->GetBinContent(ibin)) / bin_error );}
				}
			}

			//-- Debug printouts
			// cout<<"histo_ratio_data->GetBinContent(5) "<<histo_ratio_data->GetBinContent(5)<<endl;
			// cout<<"histo_ratio_data->GetBinError(5) "<<histo_ratio_data->GetBinError(5)<<endl;

			//Don't draw null data
			for(int ibin=1; ibin<histo_ratio_data->GetNbinsX()+1; ibin++)
			{
				// cout<<"histo_ratio_data["<<ibin<<"] = "<<histo_ratio_data->GetBinContent(ibin)<<endl;

                if(std::isnan(histo_ratio_data->GetBinContent(ibin)) || std::isinf(histo_ratio_data->GetBinContent(ibin)) || histo_ratio_data->GetBinContent(ibin) == 0) {histo_ratio_data->SetBinContent(ibin, -99);}
			}
		}
		else {histo_ratio_data = (TH1F*) histo_total_MC->Clone();}

		if(show_pulls_ratio) {histo_ratio_data->GetYaxis()->SetTitle("Pulls");}
		else
        {
            histo_ratio_data->GetYaxis()->SetTitle("Data/MC");
            // histo_ratio_data->GetYaxis()->SetTitle("Data/Prediction");
        }
		histo_ratio_data->GetYaxis()->SetTickLength(0.);
        histo_ratio_data->GetYaxis()->SetTitleOffset(1.15);
        // histo_ratio_data->GetYaxis()->SetTitleOffset(1.15);
        histo_ratio_data->GetXaxis()->SetTitleOffset(1.05);
        histo_ratio_data->GetYaxis()->SetLabelSize(0.04);
        histo_ratio_data->GetXaxis()->SetLabelSize(0.045);
		histo_ratio_data->GetXaxis()->SetLabelFont(42);
		histo_ratio_data->GetYaxis()->SetLabelFont(42);
		histo_ratio_data->GetXaxis()->SetTitleFont(42);
		histo_ratio_data->GetYaxis()->SetTitleFont(42);
        histo_ratio_data->GetYaxis()->SetNdivisions(503); //grid draw on primary tick marks only
		histo_ratio_data->GetXaxis()->SetNdivisions(505);
        histo_ratio_data->GetYaxis()->SetTitleSize(0.06);
		histo_ratio_data->GetXaxis()->SetTickLength(0.04);
		histo_ratio_data->SetMarkerStyle(20);
		histo_ratio_data->SetMarkerSize(1.2); //changed from 1.4
        histo_ratio_data->GetXaxis()->SetTitleSize(0.05);

        //-- If a point is outside the y-range of the ratio pad defined by SetMaximum/SetMinimum(), it disappears with its error
        //-- Trick: fill 2 histos with points either above/below y-range, to plot some markers indicating missing points (cleaner)
        //NB: only for ratio plot, not pulls
        float ratiopadmin = 0.4, ratiopadmax = 1.6; //Define ymin/ymax for ratio plot

        //-- Zoom in ratio plot for CR plots (better agreement)
        if(total_var_list[ivar].Contains("CR") || total_var_list[ivar].Contains("SRttZ4l")) {ratiopadmin = 0.8; ratiopadmax = 1.2;}

        TH1F* h_pointsAboveY = (TH1F*) histo_ratio_data->Clone();
        h_pointsAboveY->SetMarkerStyle(26); //Open triangle pointing up
        h_pointsAboveY->SetMarkerSize(1.5);
        TH1F* h_pointsBelowY = (TH1F*) histo_ratio_data->Clone();
        h_pointsBelowY->SetMarkerStyle(32); //Open triangle pointing down
        h_pointsBelowY->SetMarkerSize(1.5);
        if(show_pulls_ratio)
		{
			histo_ratio_data->SetMinimum(-2.99);
			histo_ratio_data->SetMaximum(2.99);
		}
		else
		{
            //-- Default
            histo_ratio_data->SetMinimum(ratiopadmin); //NB: removes error bars if data point is below ymin...?
            histo_ratio_data->SetMaximum(ratiopadmax);

            //-- Fill histos with points outside yrange
            for(int ibin=1; ibin<histo_ratio_data->GetNbinsX()+1; ibin++)
            {
                //-- Debug printouts
                // cout<<"histo_ratio_data->GetBinContent("<<ibin<<") "<<histo_ratio_data->GetBinContent(ibin)<<endl;
                // cout<<"histo_ratio_data->GetBinError("<<ibin<<") "<<histo_ratio_data->GetBinError(ibin)<<endl;

                //-- Default: make point invisible
                h_pointsAboveY->SetBinContent(ibin, -999);
                h_pointsBelowY->SetBinContent(ibin, -999);

                if(histo_ratio_data->GetBinContent(ibin) > ratiopadmax && h_sum_data->GetBinContent(ibin) >= 1)
                {
                    //Adjust error
                    float initial_y = histo_ratio_data->GetBinContent(ibin);
                    float initial_err = histo_ratio_data->GetBinError(ibin);
                    float new_err = initial_err - (initial_y-ratiopadmax);
                    if(new_err<0) {new_err=0.;}

                    //-- Debug printouts
                    // cout<<"ABOVE, bin "<<ibin<<endl;
                    // cout<<"initial_y "<<initial_y<<endl;
                    // cout<<"initial_err "<<initial_err<<endl;
                    // cout<<"new_err "<<new_err<<endl;

                    h_pointsAboveY->SetBinContent(ibin, ratiopadmax-0.05); //Add some padding
                    h_pointsAboveY->SetBinError(ibin, new_err);
                }
                else if(histo_ratio_data->GetBinContent(ibin) < ratiopadmin && h_sum_data->GetBinContent(ibin) >= 1)
                {
                    //Adjust error
                    float initial_y = histo_ratio_data->GetBinContent(ibin);
                    float initial_err = histo_ratio_data->GetBinError(ibin);
                    float new_err = initial_err - (ratiopadmin-initial_y);
                    if(new_err<0) {new_err=0.;}

                    //-- Debug printouts
                    // cout<<"BELOW, bin "<<ibin<<endl;
                    // cout<<"histo_total_MC->GetBinContent(ibin) "<<histo_total_MC->GetBinContent(ibin)<<endl;
                    // cout<<"initial_y "<<initial_y<<endl;
                    // cout<<"initial_err "<<initial_err<<endl;
                    // cout<<"new_err "<<new_err<<endl;

                    h_pointsBelowY->SetBinContent(ibin, ratiopadmin+(ratiopadmin/10.)); //Add some padding
                    h_pointsBelowY->SetBinError(ibin, new_err);
                }
            }
		}

        //-- SET X_AXIS TITLES
		if(drawInputVars) {histo_ratio_data->GetXaxis()->SetTitle(Get_Variable_Name(total_var_list[ivar]));}
		else
		{
            //-- Hardcoded x-titles
            TString varname = total_var_list[ivar];
            TString xtitle = Get_Template_XaxisTitle(varname, this->use_paperStyle);
            histo_ratio_data->GetXaxis()->SetTitle(xtitle);

			if(total_var_list[ivar].Contains("categ")) //Vertical text X labels (categories names)
			{
                histo_ratio_data->GetXaxis()->SetTitle("");
                histo_ratio_data->GetXaxis()->SetLabelSize(0.06); //Increase x-label size
                histo_ratio_data->GetXaxis()->SetLabelOffset(0.02); //Add some x-label offset
                histo_ratio_data->LabelsOption("v", "X"); //Make X labels vertical

                //Hard-coded
                {
                    const char *labels[10]  = {"1bj,2j","1bj,3j","1bj,4j","1bj,5j","1bj,6j","2bj,2j","2bj,3j","2bj,4j","2bj,5j","2bj,6j"};
                    for(int i=1;i<=10;i++) {histo_ratio_data->GetXaxis()->SetBinLabel(i,labels[i-1]);}
                }
			}
            else if(total_var_list[ivar].Contains("countExp")) {histo_ratio_data->GetXaxis()->SetLabelSize(0.);} //Counting exp. <-> single bin <-> x labels useless
            else if(use_combine_file) //Need to hard-code x-axis labels
            {
                int nbins=1; float xmin=0, xmax=0;
                int nbjets_min = 1, nbjets_max=2, njets_min=2, njets_max=6;
                vector<float> dummy;
                Get_Template_Range(nbins, xmin, xmax, total_var_list[ivar], plot_onlyMaxNodeEvents, nbjets_min, nbjets_max, njets_min, njets_max, dummy);

                //Arbitrary bins
                if(total_var_list[ivar].Contains("NN") || total_var_list[ivar].Contains("BDT"))
                {
                    // histo_ratio_data->GetYaxis()->SetMoreLogLabels();
                    histo_ratio_data->GetXaxis()->SetLabelSize(0.05); //Increase x-label size
                    histo_ratio_data->GetXaxis()->SetLabelOffset(0.02); //Add some x-label offset

                    for(int i=1;i<=nbins;i++)
                    {
                        // TString label = Convert_Number_To_TString(xmin + (i-1)*((xmax-xmin)/nbins));
                        TString label = "Bin " + Convert_Number_To_TString(i);
                        histo_ratio_data->GetXaxis()->SetBinLabel(i, label);
                        // histo_ratio_data->LabelsOption("v", "X"); //X labels vertical

                        // custom_axis = new TGaxis(0., 0.,  1., histo_ratio_data->GetMaximum(), xmin, xmax, 503, "-S");
                        // custom_axis->Draw("same");
                    }
                }
                else if(total_var_list[ivar].Contains("mTW"))
                {
                    histo_ratio_data->GetXaxis()->SetNdivisions(15); //Must set Ndivisions=nbins

                    vector<TString> v_labels { "0"," "," "," "," ","50"," "," "," "," ","100"," "," "," "," ","150"}; //NB: empty entry <-> " " (space)
                    // vector<TString> v_labels { "0","","20","","40","","60","","80","","100","","120","0","140", "" };

                    for(int ibin=0;ibin<nbins+1;ibin++)
                    {
                        // cout<<"ibin "<<ibin<<" / v_labels[i] "<<v_labels[ibin]<<endl;
                        // TString label = Convert_Number_To_TString(xmin + (i-1)*((xmax-xmin)/nbins));
                        // ChangeLabel (Int_t labNum=0, Double_t labAngle=-1., Double_t labSize=-1., Int_t labAlign=-1, Int_t labColor=-1, Int_t labFont=-1, TString labText="")
                        histo_ratio_data->GetXaxis()->ChangeLabel(ibin+1, -1, -1, -1, -1, -1, v_labels[ibin]);
                    }
                }
            }
		}

		pad_ratio->cd(0);
		if(show_pulls_ratio) {histo_ratio_data->Draw("HIST P");} //Draw ratio points
        else
        {
            histo_ratio_data->Draw("E1 X0 P"); //Draw ratio points ; E1 : perpendicular lines at end ; X0 : suppress x errors

            h_pointsAboveY->Draw("E1 X0 P same");
            h_pointsBelowY->Draw("E1 X0 P same");
        }

        //-- Compute a chi2-style quantity
        // float total_chi2 = 0;
        // for(int ibin=0; ibin<h_sum_data->GetNbinsX(); ibin++)
        // {
        //     float chi2 = abs(h_sum_data->GetBinContent(ibin+1) - histo_total_MC->GetBinContent(ibin+1)) / (h_sum_data->GetBinError(ibin+1)+histo_total_MC->GetBinError(ibin+1));
        //     total_chi2+= chi2;
        //     cout<<"-- ibin "<<ibin<<"--> chi2 "<<chi2<<endl;
        // }
        // cout<<"--> TOTAL chi2 "<<total_chi2/h_sum_data->GetNbinsX()<<endl;



// ###### #####  #####   ####  #####   ####     #####    ##   ##### #  ####
// #      #    # #    # #    # #    # #         #    #  #  #    #   # #    #
// #####  #    # #    # #    # #    #  ####     #    # #    #   #   # #    #
// #      #####  #####  #    # #####       #    #####  ######   #   # #    #
// #      #   #  #   #  #    # #   #  #    #    #   #  #    #   #   # #    #
// ###### #    # #    #  ####  #    #  ####     #    # #    #   #   #  ####

		TGraphAsymmErrors* gr_ratio_error = NULL;
		if(draw_errors)
		{
			//Copy previous TGraphAsymmErrors, then modify it -> error TGraph for ratio plot
			TGraphAsymmErrors *thegraph_tmp = NULL;
			double *theErrorX_h;
			double *theErrorY_h;
			double *theErrorX_l;
			double *theErrorY_l;
			double *theY;
			double *theX;

			thegraph_tmp = (TGraphAsymmErrors*) gr_error->Clone();
			theErrorX_h = thegraph_tmp->GetEXhigh();
			theErrorY_h = thegraph_tmp->GetEYhigh();
			theErrorX_l = thegraph_tmp->GetEXlow();
			theErrorY_l = thegraph_tmp->GetEYlow();
			theY        = thegraph_tmp->GetY() ;
			theX        = thegraph_tmp->GetX() ;

			//Divide error --> ratio
			for(int i=0; i<thegraph_tmp->GetN(); i++)
			{
				theErrorY_l[i] = theErrorY_l[i]/theY[i];
				theErrorY_h[i] = theErrorY_h[i]/theY[i];
				theY[i]=1; //To center the filled area around "1"
			}

			gr_ratio_error = new TGraphAsymmErrors(thegraph_tmp->GetN(), theX , theY ,  theErrorX_l, theErrorX_h, theErrorY_l, theErrorY_h);
            gr_ratio_error->SetFillStyle(3254); //3002 //3004
            gr_ratio_error->SetFillColor(kBlack); //kBlue+2 //kCyan

			pad_ratio->cd(0);
			if(!show_pulls_ratio) {gr_ratio_error->Draw("e2 same");} //Draw error bands in ratio plot

            //-- Add sub-legend here ? (stat or stat+syst)
		} //draw errors


//  ####   ####   ####  #    # ###### ##### #  ####   ####
// #    # #    # #      ##  ## #        #   # #    # #
// #      #    #  ####  # ## # #####    #   # #       ####
// #      #    #      # #    # #        #   # #           #
// #    # #    # #    # #    # #        #   # #    # #    #
//  ####   ####   ####  #    # ######   #   #  ####   ####

		//-- Draw ratio y-lines manually
		TH1F *h_line1 = NULL;
		TH1F *h_line2 = NULL;
		if(data_notEmpty)
		{
			h_line1 = new TH1F("","",this->nbins, h_sum_data->GetXaxis()->GetXmin(), h_sum_data->GetXaxis()->GetXmax());
			h_line2 = new TH1F("","",this->nbins, h_sum_data->GetXaxis()->GetXmin(), h_sum_data->GetXaxis()->GetXmax());
			// TH1F *h_line3 = new TH1F("","",this->nbins, h_sum_data->GetXaxis()->GetXmin(), h_sum_data->GetXaxis()->GetXmax());
			for(int ibin=1; ibin<this->nbins +1; ibin++)
			{
				if(show_pulls_ratio)
				{
					h_line1->SetBinContent(ibin, -1);
					h_line2->SetBinContent(ibin, 1);
				}
				else
				{
                    h_line1->SetBinContent(ibin, 0.75);
                    h_line2->SetBinContent(ibin, 1.25);
                    // h_line1->SetBinContent(ibin, 0.5);
					// h_line2->SetBinContent(ibin, 1.5);
				}
			}
			h_line1->SetLineStyle(6);
			h_line2->SetLineStyle(6);
			h_line1->Draw("hist same");
			h_line2->Draw("hist same");
		}

        // TString Y_label = ""; //cf. CMS guidelines
		double xmax_stack = stack_MC->GetXaxis()->GetXmax();
		double xmin_stack = stack_MC->GetXaxis()->GetXmin();
        TString Y_label = "Events / bin"; //Default
		if(!use_combine_file) //Compute bin width
		{
			double xmax = histo_total_MC->GetXaxis()->GetXmax();
			double xmin = histo_total_MC->GetXaxis()->GetXmin();
			Y_label = "Events / " + Convert_Number_To_TString( (xmax - xmin) / histo_total_MC->GetNbinsX(), 3); //Automatically get the Y label depending on binning
            Y_label+= Get_Unit_Variable(total_var_list[ivar]);
        }
        else //combine ruins x-axis info, so can't rely on binning of the TH1F; call same functions as used to produce the histos to get the same binning
        {
            int nbins = 0; float xmin = 0, xmax = 0;
            if(drawInputVars) {Get_Variable_Range(total_var_list[ivar], nbins, xmin, xmax);}
            else {int nbjets_min=0,nbjets_max=0,njets_min=0,njets_max=0; vector<float> minmax_bounds; Get_Template_Range(nbins, xmin, xmax, total_var_list[ivar], plot_onlyMaxNodeEvents, nbjets_min, nbjets_max, njets_min, njets_max, minmax_bounds);} //NB: for NNs the binning is anyway arbitrary (would need to read NN_settings file, etc.)

            if(xmax != 0)
            {
                Y_label = "Events / " + Convert_Number_To_TString( (xmax - xmin) / nbins, 3); //Automatically get the Y label depending on binning
                Y_label+= Get_Unit_Variable(total_var_list[ivar]);
            }
        }
        if(total_var_list[ivar].Contains("countExp")) {Y_label = "Events";} //No unit for simple counting experiment

		if(stack_MC!= 0)
		{
			stack_MC->GetXaxis()->SetLabelFont(42);
			stack_MC->GetYaxis()->SetLabelFont(42);
			stack_MC->GetYaxis()->SetTitleFont(42);
			stack_MC->GetYaxis()->SetTitleSize(0.06);
            stack_MC->GetYaxis()->SetTickLength(0.04);
			stack_MC->GetXaxis()->SetLabelSize(0.0);
			stack_MC->GetYaxis()->SetLabelSize(0.048);
			stack_MC->GetXaxis()->SetNdivisions(505);
			stack_MC->GetYaxis()->SetNdivisions(506);
            stack_MC->GetYaxis()->SetTitleOffset(1.15);
            // stack_MC->GetYaxis()->SetTitleOffset(1.28);
			stack_MC->GetYaxis()->SetTitle(Y_label);

            stack_MC->GetXaxis()->SetTickLength(0.);
		}

    	//----------------
    	// CAPTIONS //
    	//----------------
    	// -- using https://twiki.cern.ch/twiki/pub/CMS/Internal/FigGuidelines
        // -- About fonts: https://root.cern.ch/doc/master/classTAttText.html#T5

		float l = c1->GetLeftMargin();
		float t = c1->GetTopMargin();
        l+= 0.08;

		TString cmsText = "CMS";
		TLatex latex;
		latex.SetNDC();
		latex.SetTextColor(kBlack);
        // latex.SetTextFont(61);
        latex.SetTextFont(62); //Changed
		latex.SetTextAlign(11);
		latex.SetTextSize(0.06);
        if(use_paperStyle) {latex.DrawLatex(l + 0.04, 0.87, cmsText);} //CMS guideline: within frame
		else {latex.DrawLatex(l + 0.01, 0.95, cmsText);} //Default: outside frame

		TString extraText = "Preliminary";
		latex.SetTextFont(52);
		latex.SetTextSize(0.05);
		if(!use_paperStyle) {latex.DrawLatex(l + 0.12, 0.91+0.04, extraText);}

		float lumi = lumiValue;
		TString lumi_ts = Convert_Number_To_TString(lumi);
		lumi_ts += " fb^{-1} (13 TeV)";
		latex.SetTextFont(42);
		latex.SetTextAlign(31);
		latex.SetTextSize(0.04);
        if(use_paperStyle) {latex.DrawLatex(0.96, 0.95,lumi_ts);}
        else {latex.DrawLatex(0.96, 0.95,lumi_ts);}

		//------------------
		//-- channel info
		TLatex text2 ;
		text2.SetNDC();
		text2.SetTextAlign(13);
		text2.SetTextSize(0.045);
		text2.SetTextFont(42);

        TString info_data = Get_Region_Label(region, total_var_list[ivar]);
        if(info_data != "")
        {
            if(use_paperStyle) {text2.DrawLatex(l + 0.04,0.83,info_data);}
            else {text2.DrawLatex(0.20,0.90,info_data);} //Default: outside frame
        }


// #    # #####  # ##### ######     ####  #    # ##### #####  #    # #####
// #    # #    # #   #   #         #    # #    #   #   #    # #    #   #
// #    # #    # #   #   #####     #    # #    #   #   #    # #    #   #
// # ## # #####  #   #   #         #    # #    #   #   #####  #    #   #
// ##  ## #   #  #   #   #         #    # #    #   #   #      #    #   #
// #    # #    # #   #   ######     ####   ####    #   #       ####    #

        TString outdir = "";
		if(drawInputVars)
		{
            outdir = "plots/input_vars/";
            mkdir(outdir.Data(), 0777);
            outdir+= cat_tmp;
            mkdir(outdir.Data(), 0777);
            outdir+= "/" + lumiName;
            mkdir(outdir.Data(), 0777);
		}
		else
		{
            outdir = "plots/templates/";
			mkdir(outdir.Data(), 0777);
            if(cat_tmp != "")
            {
                outdir+= cat_tmp + "/";
                mkdir(outdir.Data(), 0777);
            }
            outdir+= lumiName;
            mkdir(outdir.Data(), 0777);
            if(prefit) {outdir+= "/prefit";}
            else {outdir+= "/postfit";}
            mkdir(outdir.Data(), 0777);
		}
        outdir+= "/";

		//Output
		TString output_plot_name;
		if(drawInputVars)
		{
			output_plot_name = outdir + total_var_list[ivar];
		}
		else
		{
            output_plot_name = outdir + total_var_list[ivar] + "_template";
		}
		if(channel != "") {output_plot_name+= "_" + channel;}
        if(!drawInputVars && plot_onlyMaxNodeEvents) {output_plot_name+= "_maxNode";} //Cases for which we want to cut on a multiclass MVA-SM
		output_plot_name+= this->filename_suffix;
		if(draw_logarithm) {output_plot_name+= "_log";}
        if(this->is_blind) {output_plot_name+= "_blind";}
        if(use_paperStyle) {output_plot_name+= ".eps";} //Use .eps to have correct \ell labels
		else {output_plot_name+= this->plot_extension;}

		c1->SaveAs(output_plot_name);

		if(data_notEmpty)
		{
			delete h_line1; h_line1 = NULL;
			delete h_line2; h_line2 = NULL;
		}
		if(draw_errors) {delete gr_error; delete gr_ratio_error; gr_error = NULL; gr_ratio_error = NULL;}
		delete pad_ratio; pad_ratio = NULL;

		delete c1; c1 = NULL;
		delete qw; qw = NULL;
		delete stack_MC; stack_MC = NULL;
		if(use_combine_file && !combineFile_fromHarvester) {delete g_data; g_data = NULL;}
        if(h_uncert) {delete h_uncert; h_uncert = NULL;}
	} //Var loop

	file_input->Close();

    //-- Activate this function to printout yields in each subregion //NB: only works with my own template files (not those from combine)
    vector<TString> vregions;
    vregions.push_back("xxx");
    // Print_Yields_fromHistograms(inputFile_path, template_name, v_lumiYears, vregions, sample_list);

    //-- Debug printouts
    // for(int isample=0; isample<sample_list.size(); isample++)
    // {
    //     cout<<"sample "<<sample_list[isample]<<" --> "<<v_yields_processes[isample]<<endl;
    // }

	return;
}































//--------------------------------------------
//  ######  ######## ########     ######  ##    ##  ######  ########
// ##    ## ##          ##       ##    ##  ##  ##  ##    ##    ##
// ##       ##          ##       ##         ####   ##          ##
//  ######  ######      ##        ######     ##     ######     ##
//       ## ##          ##             ##    ##          ##    ##
// ##    ## ##          ##       ##    ##    ##    ##    ##    ##    ###
//  ######  ########    ##        ######     ##     ######     ##    ###

//    ###    ########  ########  ########  ########  ######   ######  ########  ######
//   ## ##   ##     ## ##     ## ##     ## ##       ##    ## ##    ## ##       ##    ##
//  ##   ##  ##     ## ##     ## ##     ## ##       ##       ##       ##       ##
// ##     ## ##     ## ##     ## ########  ######    ######   ######  ######    ######
// ######### ##     ## ##     ## ##   ##   ##             ##       ## ##             ##
// ##     ## ##     ## ##     ## ##    ##  ##       ##    ## ##    ## ##       ##    ##
// ##     ## ########  ########  ##     ## ########  ######   ######  ########  ######
//--------------------------------------------

//Problem : in Potato code, systematics variations weights are encoded into arrays. Hence, can not directly set branch addresses via SetBranchAddress('name', &var)
//==> My solution : for each of these arrays, hard-code 1 class member array with corresponding size. This member array 'reads' the array stored in the ntuple.
//==> To make things simpler, I then use a vector<double*> so that each element will effectively read the value of a given systematic variation. From there, can proceed as usual...
//Use this function to hard-code which vector element corresponds to which systematics, and set the address
//NB : some inconsistent indices : down variation may be element 0 or 1... hence, correspondances must be hard-coded !
void VBFgamma_analysis::SetBranchAddress_SystVariationArray(TTree* t, TString systname, vector<Double_t*> &v_doubles, int isyst)
{
    TString array_name = ""; //Name of the systematic array as stored in the ntuple
    double* address_memberArray = NULL; //Address to the class member which will hold the array values (hardcoded)
    int index=-1; //index of the specific array member which holds the value of the syst of interest (hardcoded)

    if(systname == "") {return;}
    else if(systname.BeginsWith("PU"))
    {
        address_memberArray = array_PU;
        array_name = "varWeightPU";
        if(systname.EndsWith("Down")) {index = 0;}
        else if(systname.EndsWith("Up")) {index = 1;}
    }
    else if(systname.BeginsWith("prefir"))
    {
        address_memberArray = array_prefiringWeight;
        array_name = "varWeightPrefire";
        if(systname.EndsWith("Down")) {index = 0;}
        else if(systname.EndsWith("Up")) {index = 1;}
    }
    else if(systname.BeginsWith("Btag"))
    {
        address_memberArray = array_Btag;
        array_name = "btagEventWeightVar";

        if(systname.EndsWith("HFUp")) {index = 0;}
        else if(systname.EndsWith("HFDown")) {index = 1;}
        else if(systname.EndsWith("LFUp")) {index = 2;}
        else if(systname.EndsWith("LFDown")) {index = 3;}
        else if(systname.EndsWith("HFstats1Up")) {index = 4;}
        else if(systname.EndsWith("HFstats1Down")) {index = 5;}
        else if(systname.EndsWith("HFstats2Up")) {index = 6;}
        else if(systname.EndsWith("HFstats2Down")) {index = 7;}
        else if(systname.EndsWith("LFstats1Up")) {index = 8;}
        else if(systname.EndsWith("LFstats1Down")) {index = 9;}
        else if(systname.EndsWith("LFstats2Up")) {index = 10;}
        else if(systname.EndsWith("LFstats2Down")) {index = 11;}
        else if(systname.EndsWith("CFerr1Down")) {index = 12;}
        else if(systname.EndsWith("CFerr1Up")) {index = 13;}
        else if(systname.EndsWith("CFerr2Down")) {index = 14;}
        else if(systname.EndsWith("CFerr2Up")) {index = 15;}
    }
    else if(systname.BeginsWith("jetPUID"))
    {
        address_memberArray = array_jetPileupID;
        array_name = "varWeightJetPileupID";
        if(systname.EndsWith("EffDown")) {index = 1;}
        else if(systname.EndsWith("EffUp")) {index = 0;}
        else if(systname.EndsWith("MTDown")) {index = 3;}
        else if(systname.EndsWith("MTUp")) {index = 2;}
    }
    else if(systname.BeginsWith("FR"))
    {
        address_memberArray = array_fakeFactor;
        array_name = "varWeightFakeFactor";
        if(systname.Contains("_pt"))
        {
            if(systname.EndsWith("Down")) {index = 2;}
            else if(systname.EndsWith("Up")) {index = 3;}
        }
        else if(systname.Contains("_be"))
        {
            if(systname.EndsWith("Down")) {index = 4;}
            else if(systname.EndsWith("Up")) {index = 5;}
        }
        else //FR norm. variations //For David's FR, this is the only set of variations (first 2 array elements)
        {
            if(systname.EndsWith("Down")) {index = 0;}
            else if(systname.EndsWith("Up")) {index = 1;}
        }

        if(systname.Contains("m_")) {index+= 6;} //Convention: first 6 elements correspond to FR ele, last 6 to FR mu
    }
    else if(systname.BeginsWith("PDF"))
    {
        address_memberArray = array_PDFtotal;
        array_name = "varWeightPdfTotal";
        if(systname.EndsWith("Down")) {index = 1;}
        else if(systname.EndsWith("Up")) {index = 0;}
    }
    else if(systname.BeginsWith("ME"))
    {
        //-- NB: there are 6 physical variations stored; should plot them, and use only that with the largest effect (enveloppe)
        // var_weight_me[0] = Event.weightMEScaleUp;
        // var_weight_me[1] = Event.weightMEScaleDown;
        // var_weight_me[2] = Event.weightMEFacScaleUp;
        // var_weight_me[3] = Event.weightMEFacScaleDown;
        // var_weight_me[4] = Event.weightMERenScaleUp;
        // var_weight_me[5] = Event.weightMERenScaleDown;

        address_memberArray = array_ME;
        array_name = "varWeightMe";
        if(systname.EndsWith("Down")) {index = 1;}
        else if(systname.EndsWith("Up")) {index = 0;}
    }
    else if(systname.BeginsWith("alphas"))
    {
        address_memberArray = array_alphaS;
        array_name = "varWeightAlphas";
        if(systname.EndsWith("Down")) {index = 1;}
        else if(systname.EndsWith("Up")) {index = 0;}
    }
    else if(systname.BeginsWith("ISR"))
    {
        address_memberArray = array_partonShower;
        array_name = "varWeightPs";
        if(systname.EndsWith("Down")) {index = 1;}
        else if(systname.EndsWith("Up")) {index = 0;}
    }
    else if(systname.BeginsWith("FSR"))
    {
        address_memberArray = array_partonShower;
        array_name = "varWeightPs";
        if(systname.EndsWith("Down")) {index = 3;}
        else if(systname.EndsWith("Up")) {index = 2;}
    }
    else if(systname.BeginsWith("LepEff_mu"))
    {
        if(systname.Contains("Loose")) {address_memberArray = array_LepEffLoose_mu; array_name = "varWeightMuonLoose";}
        else {address_memberArray = array_LepEffTight_mu; array_name = "varWeightMuonTight";}
        if(systname.EndsWith("Down")) {index = 0;}
        else if(systname.EndsWith("Up")) {index = 1;}
    }
    else if(systname.BeginsWith("LepEff_el"))
    {
        if(systname.Contains("Loose")) {address_memberArray = array_LepEffLoose_el; array_name = "varWeightElectronLoose";}
        else {address_memberArray = array_LepEffTight_el; array_name = "varWeightElectronTight";}
        if(systname.EndsWith("Down")) {index = 0;}
        else if(systname.EndsWith("Up")) {index = 1;}
    }
    else {cout<<FRED("ERROR ! Systematic '"<<systname<<"' not included in function SetBranchAddress_SystVariation() from Helper.cxx ! Can *not* compute it !")<<endl; return;}

    t->SetBranchStatus(array_name, 1);
    t->SetBranchAddress(array_name, address_memberArray);
    v_doubles[isyst] = &address_memberArray[index];

    return;
}


















//--------------------------------------------
// ##     ## ######## ########   ######   ########
// ###   ### ##       ##     ## ##    ##  ##
// #### #### ##       ##     ## ##        ##
// ## ### ## ######   ########  ##   #### ######
// ##     ## ##       ##   ##   ##    ##  ##
// ##     ## ##       ##    ##  ##    ##  ##
// ##     ## ######## ##     ##  ######   ########

// ######## ######## ##     ## ########  ##          ###    ######## ########  ######
//    ##    ##       ###   ### ##     ## ##         ## ##      ##    ##       ##    ##
//    ##    ##       #### #### ##     ## ##        ##   ##     ##    ##       ##
//    ##    ######   ## ### ## ########  ##       ##     ##    ##    ######    ######
//    ##    ##       ##     ## ##        ##       #########    ##    ##             ##
//    ##    ##       ##     ## ##        ##       ##     ##    ##    ##       ##    ##
//    ##    ######## ##     ## ##        ######## ##     ##    ##    ########  ######
//--------------------------------------------

/**
 * The main code is producing/plotting template histograms for each sample separately
 * But we may want to group some processes together for Combine fit (e.g. "Rares", "EWK", ...)
 * ===> In addition to storing individual histos, also merge+store histos for the relevant process groups
 * NB: here the order of loops is important because we sum histograms recursively, and the 'sample_list' loop must be the most nested one !
 */
void VBFgamma_analysis::MergeSplit_Templates(bool makeHisto_inputVars, TString filename, vector<TString> total_var_list, TString template_name, TString category, bool force_normTemplate_positive)
{
    cout<<endl<<FYEL("==> Merging/splitting histograms in TFile : ")<<filename<<endl;

	if(!Check_File_Existence(filename) ) {cout<<endl<<FRED("File "<<filename<<" not found! Abort merging procedure !")<<endl; return;}
	TFile* f = TFile::Open(filename, "UPDATE");

    // int n_total_histos = Count_nofHistos_inTFile(f); //Too slow for huge files
    int counter = 0; //Count nof read histos

	//NB :here the order of loops is important because we sum histograms recursively ! The 'sample_list' loop *must be the most nested one* !
    for(int ivar=0; ivar<total_var_list.size(); ivar++) //There may be more than 1 template (e.g. several NN output nodes)
    {
        for(int iyear=0; iyear<v_lumiYears.size(); iyear++)
        {
            // cout<<"v_lumiYears[iyear] "<<v_lumiYears[iyear]<<endl;

            for(int ichan=0; ichan<channel_list.size(); ichan++)
        	{
                // cout<<"channel_list[ichan] "<<channel_list[ichan]<<endl;

        		for(int itree=0; itree<systTree_list.size(); itree++)
        		{
        			if(systTree_list[itree] != "" && systTree_list[itree] != nominal_tree_name && channel_list.size() > 1 && channel_list[ichan] == "") {continue;}

        			for(int isyst=0; isyst<syst_list.size(); isyst++)
        			{
                        // cout<<"syst_list[isyst] "<<syst_list[isyst]<<endl;

        				// if(((channel_list.size() > 1 && channel_list[ichan] == "") || (systTree_list[itree] != "" && systTree_list[itree] != nominal_tree_name)) && syst_list[isyst] != "") {continue;}
        				if(systTree_list[itree] != "" && systTree_list[itree] != nominal_tree_name && syst_list[isyst] != "") {break;}
                        else if(v_lumiYears[iyear] == "2018" && syst_list[isyst].BeginsWith("prefir") ) {continue;} //no prefire in 2018

        				TH1F* h_merging = NULL; //Tmp merged histogram

        				for(int isample=0; isample<sample_list.size(); isample++)
        				{
                        	//-- Protections : not all syst weights apply to all samples, etc.
                            if(sample_list[isample] == "DATA" && ((systTree_list[itree] != "" && systTree_list[itree] != nominal_tree_name) || syst_list[isyst] != "")) {continue;} //nominal data only
                            else if(makeHisto_inputVars && sample_groups[isample] != "NPL") {continue;} //For control plots, only need to substract prompt NPL from data-driven NPL
                            else if((sample_list[isample].Contains("NPL") && syst_list[isyst] != "" && !syst_list[isyst].BeginsWith("FR")) || (!sample_list[isample].Contains("NPL") && syst_list[isyst].BeginsWith("FR"))) {continue;} //NPL <-> only fakes sytematics; all others <-> no fakes systematics
                            else if(systTree_list[itree] != "" && systTree_list[itree] != nominal_tree_name && (sample_list[isample] == "DY" || sample_list[isample].Contains("TTbar") || sample_list[isample].Contains("NPL")) ) {continue;}

        					//-- Check if this sample needs to be merged, i.e. if the samples before/after belong to the same "group of samples"
        					bool merge_this_sample = false;
        					if(!isample && sample_groups.size() > 1 && sample_groups[isample+1] == sample_groups[isample]) {merge_this_sample = true;}
        					else if(isample > 0 && isample == sample_list.size()-1 && sample_groups[isample-1] == sample_groups[isample]) {merge_this_sample = true;}
        					else if(isample > 0 && isample < sample_list.size()-1 && (sample_groups[isample+1] == sample_groups[isample] || sample_groups[isample-1] == sample_groups[isample])) {merge_this_sample = true;}

                            // cout<<endl<<"Year "<<v_lumiYears[iyear]<< " / tree "<<systTree_list[itree]<<" / chan "<<channel_list[ichan]<<" / syst "<<syst_list[isyst]<<" / sample "<<sample_list[isample]<<endl;
        					// cout<<"merge_this_sample "<<merge_this_sample<<endl;
        					if(!merge_this_sample) {continue;} //Only care about samples to merge : others are already stored in file

                            TString histoname = ""; //Default: merge full histograms
                            histoname+= total_var_list[ivar];
        					if(channel_list[ichan] != "") {histoname+= "_" + channel_list[ichan];}
                            if(region != "" && !makeHisto_inputVars) {histoname+= "_" + region;}
                            histoname+= "_" + v_lumiYears[iyear] + "__" + (sample_list[isample] == "DATA"? "data_obs":sample_list[isample]);
        					if(syst_list[isyst] != "" || (systTree_list[itree] != "" && systTree_list[itree] != nominal_tree_name)) {histoname+= "__" + Get_Modified_SystName(syst_list[isyst]+systTree_list[itree], v_lumiYears[iyear], sample_list[isample]);}
        					else if(systTree_list[itree] != "" && systTree_list[itree] != nominal_tree_name) {histoname+= "__" + systTree_list[itree];}

                            //-- Protection: replace '-' (hyphen) with 'm' character (hyphen in histo name causes errors at reading)
                            histoname.ReplaceAll('-', 'm');

                            // cout<<"histoname = "<<histoname<<endl;

                            if(!f->GetListOfKeys()->Contains(histoname) )
        					{
        						if(systTree_list[itree] == "" && syst_list[isyst] == "") {cout<<endl<<DIM("Histo "<<histoname<<" not found in file "<<filename<<" !")<<endl;}
                                continue;
                            }

                            //NB -- very slow for large files !
        					TH1F* h_tmp = (TH1F*) f->Get(histoname); //Get individual histograms

                            counter++; //Increment counter of read histos
                            if(counter % 1000 == 0) {cout<<DIM("Read "<<counter<<" histograms...")<<endl;}
                            // if(counter % 1000 == 0) {cout<<DIM("Read "<<counter<<"/"<<n_total_histos<<" histograms...")<<endl;}

                            // cout<<"h_merging "<<h_merging<<endl;
        					if(h_tmp)
        					{
        						if(!h_merging) {h_merging = (TH1F*) h_tmp->Clone();}
        						else {h_merging->Add(h_tmp);}
                                // cout<<"h_tmp->Integral() = "<<h_tmp->Integral()<<endl;
        					}
        					// else {cout<<DIM("h_tmp is null ! ("<<histoname<<")")<<endl;}
        					// cout<<"h_merging->Integral() = "<<h_merging->Integral()<<endl;

        					delete h_tmp; h_tmp = NULL;
        					if(!h_merging) {cout<<"Syst "<<syst_list[isyst]<<systTree_list[itree]<<" / chan "<<channel_list[ichan]<<" / sample "<<sample_list[isample]<<endl; cout<<"h_merging is null ! Latest histo read "<<histoname<<"... You should check this !"<<endl; continue;}

        					//Check if next sample will be merged with this one, or else if must write the histogram
        					if(isample < sample_list.size()-1 && sample_groups[isample+1] == sample_groups[isample]) {continue;}
        					else
        					{
                                TString histoname_new = "";
                                histoname_new+= total_var_list[ivar];
                                // if(category != "") {histoname_new+= "_" + category;}
                                if(channel_list[ichan] != "") {histoname_new+="_"  + channel_list[ichan];}
                                if(region != "" && !makeHisto_inputVars) {histoname_new+= "_" + region;}
                                histoname_new+= "_" + v_lumiYears[iyear] + "__" + sample_groups[isample];
                                if(syst_list[isyst] != "" || (systTree_list[itree] != "" && systTree_list[itree] != nominal_tree_name)) {histoname_new+= "__" + Get_Modified_SystName(syst_list[isyst]+systTree_list[itree], v_lumiYears[iyear], sample_groups[isample]);}
                                else if(systTree_list[itree] != "" && systTree_list[itree] != nominal_tree_name) {histoname_new+= "__" + systTree_list[itree];}

                                //-- Protection: replace '-' (hyphen) with 'm' character (hyphen in histo name causes errors at reading)
                                histoname_new.ReplaceAll('-', 'm');

        						if(force_normTemplate_positive && sample_list[isample] != "DATA")
        						{
                                    Avoid_Histogram_EmptyOrNegativeBins(h_merging);
        						}
        						// cout<<"h_merging->Integral() = "<<h_merging->Integral()<<endl;

        						f->cd();
        						h_merging->Write(histoname_new, TObject::kOverwrite);
        						// cout<<"-- Writing merged histo "<<histoname_new<<" with integral "<<h_merging->Integral()<<endl;

        						delete h_merging; h_merging = NULL;
                            } //write histo
                        } //sample loop

        			} //syst loop
        		} //tree loop
        	} //channel loop
        } //years loop
    } //vars loop

    // cout<<DIM("(Total: "<<n_total_histos<<" histograms)")<<endl;

	f->Close();

    // cout<<endl<<FYEL("Updated file: " )<<filename<<endl<<endl<<endl;
    cout<<FYEL("... Done")<<" (file: "<<filename<<")"<<endl<<endl<<endl;

	return;
}
