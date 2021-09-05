#include "VBFgamma_analysis.h"

using namespace std;

int main(int argc, char **argv)
{
//---------------------------------------------------------------------------
//  #######  ########  ######## ####  #######  ##    ##  ######
// ##     ## ##     ##    ##     ##  ##     ## ###   ## ##    ##
// ##     ## ##     ##    ##      ##     ## ####  ## ##
// ##     ## ########     ##     ##  ##     ## ## ## ##  ######
// ##     ## ##           ##     ##  ##     ## ##  ####       ##
// ##     ## ##           ##     ##  ##     ## ##   ### ##    ##
//  #######  ##           ##    ####  #######  ##    ##  ######
//---------------------------------------------------------------------------

    //-- M A I N    A N A L  S I S    O P T I O N S --
    TString signal_process = "VBFgamma";
    TString region = ""; //Select a specific event category : '' (all preselected events) / 'tZq' / 'ttZ' / 'signal'
    bool use_systematics = false; //true <-> will compute/store systematics selected below
    bool split_JEC = false; //true <-> store all split JEC variations (in addition to total JEC -- much slower!)
    bool is_blind = false; //true <-> don't read/store data events

    //-- N T U P L E S --
    bool process_samples_byGroup = false; //true <-> read grouped samples (if already hadded together), else read individual samples and combine them when creating histograms if needed (default)

    //-- M V A --
    TString classifier_name = "NN"; //'BDT' or 'NN'
    bool use_specificMVA_eachYear = false; //true <-> look for year-specific MVA weight files

    //-- T E M P L A T E S --
    bool split_analysis_by_channel = false; //true <-> will *also* produce templates/histos/plots for each subchannel (defined below)
    TString template_name = ""; //Ex: 'BDT', 'NN', 'categ' (nbjet/njet bins), 'Zpt', 'ZptCos', 'NN_ctz', 'NN_all', 'NN_SM', ...

    //-- P L O T T I N G --
    bool show_pulls_ratio = false; //true <-> bottom pad shows pull; else shows data/mc ratio (w/ errors)
    TString plot_extension = ".png"; //Default file extension for plots //For 'paper plots', .pdf extension is enforced
    bool plot_onlyMaxNodeEvents = true; //For multiclass NN-SM template plots only (EFT strategy 2): true <-> only include events if they have their max output value in the corresponding node
    TString nominal_tree_name = "Events"; //Name of the nominal tree to read in rootfiles
    bool use_paperStyle = false; //true <-> Add 'preliminary' label on plots


//-----------------------------------------------------------------------------------------
// ##       ##     ## ##     ## #### ##    ##  #######   ######  #### ######## ##    ##
// ##       ##     ## ###   ###  ##  ###   ## ##     ## ##    ##  ##     ##     ##  ##
// ##       ##     ## #### ####  ##  ####  ## ##     ## ##        ##     ##      ####
// ##       ##     ## ## ### ##  ##  ## ## ## ##     ##  ######   ##     ##       ##
// ##       ##     ## ##     ##  ##  ##  #### ##     ##       ##  ##     ##       ##
// ##       ##     ## ##     ##  ##  ##   ### ##     ## ##    ##  ##     ##       ##
// ########  #######  ##     ## #### ##    ##  #######   ######  ####    ##       ##
//-----------------------------------------------------------------------------------------
//-- Choose here what data you want to consider (separate ntuples, templates, ... per year)
//Naming convention enforced : 2016+2017 <-> "201617" ; etc.; 2016+2017+2018 <-> "Run2"
//NB : years must be placed in the right order !

	vector<TString> set_lumi_years; //FIXME
    set_lumi_years.push_back("2016");
    // set_lumi_years.push_back("2017");
    // set_lumi_years.push_back("2018");


//-----------------------------------------------------------------------------------------
//   ######  ##     ## ########  ######
//  ##    ## ##     ##    ##    ##    ##
//  ##       ##     ##    ##    ##
//  ##       ##     ##    ##     ######
//  ##       ##     ##    ##          ##
//  ##    ## ##     ##    ##    ##    ##
//   ######   #######     ##     ######
//-----------------------------------------------------------------------------------------
//ex: set_v_cut_name.push_back("NBJets"); set_v_cut_def.push_back(">0 && <4"); set_v_cut_IsUsedForBDT.push_back(false);
//NB : if variable is to be used in BDT, can't ask it to take unique value (==) !

	vector<TString> set_v_cut_name;
	vector<TString> set_v_cut_def;
	vector<bool> set_v_cut_IsUsedForBDT;

    // set_v_cut_name.push_back("nJets");  set_v_cut_def.push_back("==3 || ==4"); set_v_cut_IsUsedForBDT.push_back(false);
    // set_v_cut_name.push_back("passedBJets");  set_v_cut_def.push_back("==1"); set_v_cut_IsUsedForBDT.push_back(false); //enforce final tZq 3l selection
    // set_v_cut_name.push_back("is_tzq_SR");  set_v_cut_def.push_back("==1"); set_v_cut_IsUsedForBDT.push_back(false);


//---------------------------------------------------------------------------
//  ######  ##     ##    ###    ##    ## ##    ## ######## ##        ######
// ##    ## ##     ##   ## ##   ###   ## ###   ## ##       ##       ##    ##
// ##       ##     ##  ##   ##  ####  ## ####  ## ##       ##       ##
// ##       ######### ##     ## ## ## ## ## ## ## ######   ##        ######
// ##       ##     ## ######### ##  #### ##  #### ##       ##             ##
// ##    ## ##     ## ##     ## ##   ### ##   ### ##       ##       ##    ##
//  ######  ##     ## ##     ## ##    ## ##    ## ######## ########  ######
//---------------------------------------------------------------------------
    std::vector<TString > thechannellist;

    thechannellist.push_back(""); //KEEP ! (<-> no subcategorization, used for plots, etc.)

    if(split_analysis_by_channel)
    {
        thechannellist.push_back("uuu");
        thechannellist.push_back("uue");
        thechannellist.push_back("eeu");
        thechannellist.push_back("eee");
    }


//---------------------------------------------------------------------------
//  ######     ###    ##     ## ########  ##       ########  ######
// ##    ##   ## ##   ###   ### ##     ## ##       ##       ##    ##
// ##        ##   ##  #### #### ##     ## ##       ##       ##
//  ######  ##     ## ## ### ## ########  ##       ######    ######
//       ## ######### ##     ## ##        ##       ##             ##
// ##    ## ##     ## ##     ## ##        ##       ##       ##    ##
//  ######  ##     ## ##     ## ##        ######## ########  ######
//---------------------------------------------------------------------------

//-- List of sample names (as found in ./input_ntuples) //thesamplegroups <-> can merge multiple ntuples into same group (plotting)
    vector<TString> thesamplelist, thesamplegroups;

    //DATA (single sample, in first position)
    thesamplelist.push_back("DATA"); thesamplegroups.push_back("DATA");

    //Signal
    thesamplelist.push_back("VBFgamma"); thesamplegroups.push_back("VBFgamma");
    // thesamplelist.push_back("GJets"); thesamplegroups.push_back("GJets");

    //TTbar
    thesamplelist.push_back("ttbar2l"); thesamplegroups.push_back("ttbar2l");

    //TTG
    thesamplelist.push_back("ttGJets"); thesamplegroups.push_back("ttGJets");

    //DY
    thesamplelist.push_back("DY"); thesamplegroups.push_back("DY");

    //LLJJ
    thesamplelist.push_back("LLJJ"); thesamplegroups.push_back("LLJJ");

    //DiPhoton
    thesamplelist.push_back("DiPhoton"); thesamplegroups.push_back("DiPhoton");

    //WJets
    thesamplelist.push_back("WJetsToLNu"); thesamplegroups.push_back("WJets");
    thesamplelist.push_back("WJetsToQQ"); thesamplegroups.push_back("WJets");

    //ZGTo2LG
    thesamplelist.push_back("ZGTo2LG"); thesamplegroups.push_back("ZGTo2LG");

    //WGToLNuG
    thesamplelist.push_back("WGToLNuG"); thesamplegroups.push_back("WGToLNuG");

    //QCD
    thesamplelist.push_back("QCD"); thesamplegroups.push_back("QCD");


//---------------------------------------------------------------------------
// ########  ########  ########       ##     ##    ###    ########   ######
// ##     ## ##     ##    ##          ##     ##   ## ##   ##     ## ##    ##
// ##     ## ##     ##    ##          ##     ##  ##   ##  ##     ## ##
// ########  ##     ##    ##          ##     ## ##     ## ########   ######
// ##     ## ##     ##    ##           ##   ##  ######### ##   ##         ##
// ##     ## ##     ##    ##            ## ##   ##     ## ##    ##  ##    ##
// ########  ########     ##             ###    ##     ## ##     ##  ######
//---------------------------------------------------------------------------
//Variables used in BDT training (and evaluation)
//For NN, will read necessary input files in logfile, and include them automatically

    std::vector<TString > thevarlist;
    // thevarlist.push_back("recoZ_Pt");


//---------------------------------------------------------------------------
//  #######  ######## ##     ## ######## ########       ##     ##    ###    ########   ######
// ##     ##    ##    ##     ## ##       ##     ##      ##     ##   ## ##   ##     ## ##    ##
// ##     ##    ##    ##     ## ##       ##     ##      ##     ##  ##   ##  ##     ## ##
// ##     ##    ##    ######### ######   ########       ##     ## ##     ## ########   ######
// ##     ##    ##    ##     ## ##       ##   ##         ##   ##  ######### ##   ##         ##
// ##     ##    ##    ##     ## ##       ##    ##         ## ##   ##     ## ##    ##  ##    ##
//  #######     ##    ##     ## ######## ##     ##         ###    ##     ## ##     ##  ######
//---------------------------------------------------------------------------
//Can add additionnal vars which are NOT used in TMVA NOR for cuts, only for CR plots !
//NOTE : Branch can be linked to only *one* variable via SetBranchAddress ; if additional variable is already present in other variable vector, it is removed from this vector !

    vector<TString> set_v_add_var_names;

    set_v_add_var_names.push_back("vjj_v_pt");
    set_v_add_var_names.push_back("vjj_v_eta");
    set_v_add_var_names.push_back("vjj_v_phi");
    set_v_add_var_names.push_back("vjj_v_m");
    set_v_add_var_names.push_back("vjj_v_ystar");
    set_v_add_var_names.push_back("vjj_jj_pt");
    set_v_add_var_names.push_back("vjj_jj_eta");
    set_v_add_var_names.push_back("vjj_jj_phi");
    set_v_add_var_names.push_back("vjj_jj_m");
    set_v_add_var_names.push_back("vjj_jj_dr2v");
    set_v_add_var_names.push_back("vjj_jj_scalarht");
    set_v_add_var_names.push_back("vjj_jj_deta");
    set_v_add_var_names.push_back("vjj_jj_dphi");
    set_v_add_var_names.push_back("vjj_vjj_isotropy");
    set_v_add_var_names.push_back("vjj_vjj_circularity");
    set_v_add_var_names.push_back("vjj_vjj_sphericity");
    set_v_add_var_names.push_back("vjj_vjj_aplanarity");
    set_v_add_var_names.push_back("vjj_vjj_C");
    set_v_add_var_names.push_back("vjj_vjj_D");
    set_v_add_var_names.push_back("vjj_htsoft");
    set_v_add_var_names.push_back("vjj_centhtsoft");
    set_v_add_var_names.push_back("vjj_nextraj");
    set_v_add_var_names.push_back("vjj_ncentj");


//---------------------------------------------------------------------------
//  ######  ##    ##  ######  ######## ######## ##     ##    ###    ######## ####  ######   ######
// ##    ##  ##  ##  ##    ##    ##    ##       ###   ###   ## ##      ##     ##  ##    ## ##    ##
// ##         ####   ##          ##    ##       #### ####  ##   ##     ##     ##  ##       ##
//  ######     ##     ######     ##    ######   ## ### ## ##     ##    ##     ##  ##        ######
//       ##    ##          ##    ##    ##       ##     ## #########    ##     ##  ##             ##
// ##    ##    ##    ##    ##    ##    ##       ##     ## ##     ##    ##     ##  ##    ## ##    ##
//  ######     ##     ######     ##    ######## ##     ## ##     ##    ##    ####  ######   ######
//---------------------------------------------------------------------------

    vector<TString> theSystWeights; //List of systematics implemented as event weights
    theSystWeights.push_back(""); //KEEP ! (<-> nominal event weight)

    vector<TString> theSystTree; //List of systematics implemented as separate TTrees
    theSystTree.push_back(""); //KEEP ! (<-> nominal TTree)

    if(use_systematics) //Define here the list of syst to run //Missing: JERC, leptonID, ME, PDFs, ...
    {
        //-- Implemented as separate TTrees

        //* Total JEV
        theSystTree.push_back("JESDown"); theSystTree.push_back("JESUp");
        theSystTree.push_back("JERDown"); theSystTree.push_back("JERUp");
        theSystTree.push_back("METDown"); theSystTree.push_back("METUp");

        //* Split JEC sources
        if(split_JEC)
        {
            theSystTree.push_back("AbsoluteStatDown"); theSystTree.push_back("AbsoluteStatUp");
            theSystTree.push_back("AbsoluteScaleDown"); theSystTree.push_back("AbsoluteScaleUp");
            theSystTree.push_back("AbsoluteMPFBiasDown"); theSystTree.push_back("AbsoluteMPFBiasUp");
            theSystTree.push_back("FragmentationDown"); theSystTree.push_back("FragmentationUp");
            theSystTree.push_back("SinglePionECALDown"); theSystTree.push_back("SinglePionECALUp");
            theSystTree.push_back("SinglePionHCALDown"); theSystTree.push_back("SinglePionHCALUp");
            theSystTree.push_back("FlavorQCDDown"); theSystTree.push_back("FlavorQCDUp");
            theSystTree.push_back("TimePtEtaDown"); theSystTree.push_back("TimePtEtaUp");
            theSystTree.push_back("RelativeJEREC1Down"); theSystTree.push_back("RelativeJEREC1Up");
            theSystTree.push_back("RelativeJEREC2Down"); theSystTree.push_back("RelativeJEREC2Up");
            theSystTree.push_back("RelativeJERHFDown"); theSystTree.push_back("RelativeJERHFUp");
            theSystTree.push_back("RelativePtBBDown"); theSystTree.push_back("RelativePtBBUp");
            theSystTree.push_back("RelativePtEC1Down"); theSystTree.push_back("RelativePtEC1Up");
            theSystTree.push_back("RelativePtEC2Down"); theSystTree.push_back("RelativePtEC2Up");
            theSystTree.push_back("RelativePtHFDown"); theSystTree.push_back("RelativePtHFUp");
            theSystTree.push_back("RelativeBalDown"); theSystTree.push_back("RelativeBalUp");
            theSystTree.push_back("RelativeSampleDown"); theSystTree.push_back("RelativeSampleUp");
            theSystTree.push_back("RelativeFSRDown"); theSystTree.push_back("RelativeFSRUp");
            theSystTree.push_back("RelativeStatFSRDown"); theSystTree.push_back("RelativeStatFSRUp");
            theSystTree.push_back("RelativeStatECDown"); theSystTree.push_back("RelativeStatECUp");
            theSystTree.push_back("RelativeStatHFDown"); theSystTree.push_back("RelativeStatHFUp");
            theSystTree.push_back("PileUpDataMCDown"); theSystTree.push_back("PileUpDataMCUp");
            theSystTree.push_back("PileUpPtRefDown"); theSystTree.push_back("PileUpPtRefUp");
            theSystTree.push_back("PileUpPtBBDown"); theSystTree.push_back("PileUpPtBBUp");
            theSystTree.push_back("PileUpPtEC1Down"); theSystTree.push_back("PileUpPtEC1Up");
            theSystTree.push_back("PileUpPtEC2Down"); theSystTree.push_back("PileUpPtEC2Up");
            theSystTree.push_back("PileUpPtHFDown"); theSystTree.push_back("PileUpPtHFUp");
        }

        //-- Implementend as event weights

        //* EXP
        // theSystWeights.push_back("PUDown"); theSystWeights.push_back("PUUp");

        //* THEORY
        // theSystWeights.push_back("PDFDown"); theSystWeights.push_back("PDFUp");
    }


//---------------------------------------------------------------------------
// ######## ##     ## ##    ##  ######  ######## ####  #######  ##    ##        ######     ###    ##       ##        ######
// ##       ##     ## ###   ## ##    ##    ##     ##  ##     ## ###   ##       ##    ##   ## ##   ##       ##       ##    ##
// ##       ##     ## ####  ## ##          ##     ##  ##     ## ####  ##       ##        ##   ##  ##       ##       ##
// ######   ##     ## ## ## ## ##          ##     ##  ##     ## ## ## ##       ##       ##     ## ##       ##        ######
// ##       ##     ## ##  #### ##          ##     ##  ##     ## ##  ####       ##       ######### ##       ##             ##
// ##       ##     ## ##   ### ##    ##    ##     ##  ##     ## ##   ###       ##    ## ##     ## ##       ##       ##    ##
// ##        #######  ##    ##  ######     ##    ####  #######  ##    ##        ######  ##     ## ######## ########  ######
//---------------------------------------------------------------------------

//*** CHOOSE HERE FROM BOOLEANS WHAT YOU WANT TO DO !

//-----------------    TRAINING
    bool train_BDT = false; //Train selected BDT in selected region (with events in training category)

//-----------------    TEMPLATES CREATION
    bool create_templates = false; //Create MVA templates

//-----------------    CONTROL HISTOGRAMS
    bool create_inputVar_histograms = true; //Create histograms of input variables, for plotting

//-----------------    PLOTS
    TString plotChannel = ""; //Can choose to plot particular subchannel //uu, ue, ee, ...

    bool draw_templates = false; //Plot templates of selected BDT, in selected region
        bool prefit = true; //true <-> plot prefit templates ; else postfit (requires combine output file)
        bool use_combine_file = false; //true <-> use MLF output file from Combine (can get postfit plots, total error, etc.)

    bool draw_input_vars = true; //Plot input variables
        bool draw_input_allChannels = false; //true <-> also draw for eachs split channel

    bool compare_template_shapes = false;

//-----------------    OTHER

//-----------------













//--------------------------------------------
//--------------------------------------------
//--- Automated from here -- no need to modify
//--------------------------------------------
//--------------------------------------------

//--------------------------------------------
//    ###    ##     ## ########  #######  ##     ##    ###    ######## ####  ######
//   ## ##   ##     ##    ##    ##     ## ###   ###   ## ##      ##     ##  ##    ##
//  ##   ##  ##     ##    ##    ##     ## #### ####  ##   ##     ##     ##  ##
// ##     ## ##     ##    ##    ##     ## ## ### ## ##     ##    ##     ##  ##
// ######### ##     ##    ##    ##     ## ##     ## #########    ##     ##  ##
// ##     ## ##     ##    ##    ##     ## ##     ## ##     ##    ##     ##  ##    ##
// ##     ##  #######     ##     #######  ##     ## ##     ##    ##    ####  ######
//--------------------------------------------

//-- Apply choices given via command line, if any
	Apply_CommandArgs_Choices(argc, argv, set_lumi_years, region, template_name);

    // int nthreads = 4; ROOT::EnableImplicitMT(nthreads); //Enable multi-threading (I have 8 available threads)

    //#############################################
    //  CREATE INSTANCE OF CLASS & INITIALIZE
    //#############################################

    VBFgamma_analysis* theAnalysis = new VBFgamma_analysis(thesamplelist, thesamplegroups, theSystWeights, theSystTree, thechannellist, thevarlist, set_v_cut_name, set_v_cut_def, set_v_cut_IsUsedForBDT, set_v_add_var_names, plot_extension, set_lumi_years, show_pulls_ratio, region, signal_process, classifier_name, is_blind, use_specificMVA_eachYear, nominal_tree_name, process_samples_byGroup, use_paperStyle);
    if(theAnalysis->stop_program) {cout<<"=== 'stop_program=true' ---> Exiting... "<<endl; return 1;}

    //#############################################
    // TRAINING
    //#############################################

    if(train_BDT) {theAnalysis->Train_BDT("");}

    //#############################################
    //  TEMPLATES CREATION
    //#############################################

    if(create_templates) {theAnalysis->Produce_Templates(template_name, false, plot_onlyMaxNodeEvents);}

    //#############################################
    //  CONTROL HISTOGRAMS
    //#############################################

    if(create_inputVar_histograms) {theAnalysis->Produce_Templates(template_name, true, plot_onlyMaxNodeEvents);}

    //#############################################
    //  DRAW PLOTS
    //#############################################

    //All channels
    if(draw_templates)
    {
        theAnalysis->Draw_Templates(false, plotChannel, plot_onlyMaxNodeEvents, template_name, prefit, use_combine_file); //chosen channel

        if(plotChannel == "") //By default, also want to plot templates in subchannels
        {
            for(int ichan=1; ichan<thechannellist.size(); ichan++)
            {
                theAnalysis->Draw_Templates(false, thechannellist[ichan], plot_onlyMaxNodeEvents, template_name, prefit, use_combine_file);
            }
        }
    }

    if(draw_input_vars)
    {
        theAnalysis->Draw_Templates(true, plotChannel, plot_onlyMaxNodeEvents, "", prefit, use_combine_file);
        if(draw_input_allChannels)
        {
            for(int ichan=1; ichan<thechannellist.size(); ichan++)
            {
                theAnalysis->Draw_Templates(true, thechannellist[ichan], plot_onlyMaxNodeEvents, "", prefit, use_combine_file);
            }
        }
    }

    //#############################################
    //  OTHER FUNCTIONS
    //#############################################

    //#############################################
    //  FINALIZE
    //#############################################

    delete theAnalysis; theAnalysis = NULL;
}
