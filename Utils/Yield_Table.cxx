#include "Helper.h"

using namespace std;

//Hardcode nicer latex-compatible category names //Can use '\\mathbf{}' for bold, but does not work with greek letters ?
//NB: makes use of local definitions from AN/paper
TString Get_Category_LatexName(TString cat)
{
    TString name = cat;

    // if(cat.Contains("signal", TString::kIgnoreCase)) {name = "\\sr";}

    return name;
}


//--------------------------------------------
// ##    ## #### ######## ##       ########     ########    ###    ########  ##       ########
//  ##  ##   ##  ##       ##       ##     ##       ##      ## ##   ##     ## ##       ##
//   ####    ##  ##       ##       ##     ##       ##     ##   ##  ##     ## ##       ##
//    ##     ##  ######   ##       ##     ##       ##    ##     ## ########  ##       ######
//    ##     ##  ##       ##       ##     ##       ##    ######### ##     ## ##       ##
//    ##     ##  ##       ##       ##     ##       ##    ##     ## ##     ## ##       ##
//    ##    #### ######## ######## ########        ##    ##     ## ########  ######## ########
//--------------------------------------------

void Compute_Write_Yields(vector<TString> v_samples, vector<TString> v_label, TString category, TString signal, TString lumi, bool group_samples_together, bool remove_totalSF, TString channel, TString treename)
{
    bool create_latex_table = true; //true <-> also output latex-format yield tables
        bool blind = false; //true <-> don't include DATA in latex tables (but still include in printouts)
        int precision = 1; //Nof decimals displayed after floating point

//--------------------------------------------

    cout<<endl<<BYEL("                          ")<<endl<<endl;
	cout<<FYEL("--- Will count the yields for each sample ---")<<endl;
	cout<<"(category : "<<category<<" / lumi : "<<lumi<<" / channel : "<<channel<<")"<<endl;
    cout<<endl<<BYEL("                          ")<<endl<<endl;

    mkdir("./outputs/", 0777);
    mkdir("./outputs/yields", 0777);
    TString outname = "./outputs/yields/Yields_"+category+"_"+lumi;
    if(channel != "") {outname+= "_" + channel;}
    outname+= ".txt";

    TString outname_latex = "./outputs/yields/latex/Yields_"+category+"_"+lumi;
    if(create_latex_table)
    {
        mkdir("./outputs/yields/latex", 0777);
        if(channel != "") {outname_latex+= "_" + channel;}
        outname_latex+= ".txt";
    }
    ofstream file_latex; //Declaration

    vector<TString> v_years; //'Run2' -> Sum all 3 years
    if(lumi == "Run2") {v_years.push_back("2016"); v_years.push_back("2017"); v_years.push_back("2018");}
    else {v_years.push_back(lumi);}

    if(group_samples_together == false)
    {
        for(int isample=0; isample<v_label.size(); isample++)
        {
            // cout<<"v_label[isample] "<<v_label[isample];
            v_label[isample] = v_samples[isample];
            // cout<<" ==> "<<v_label[isample]<<endl;
        }
    }

//--------------------------------------------

    if(create_latex_table)
    {
        file_latex.open(outname_latex.Data());

    	//NB : '\' character must be escaped as '\\' in C++!
        // file_latex<<"\\begin{table}[]"<<endl; //Horizontal
        file_latex<<"\\begin{sidewaystable}[]"<<endl; //Vertical //Requires package rotating
        file_latex<<"\\centering"<<endl;
        file_latex<<"\\small"<<endl; //normal, small, tiny
        file_latex<<"\\begin{tabular}{|c|"; //Init with 1 column (leftmost column, e.g. listing different event categories)
    	for(int isample=0; isample<v_label.size(); isample++)
    	{
            // cout<<"v_label[isample] "<<v_label[isample]<<endl; //Debug

            if(v_label[isample].Contains("TTbar") || v_label[isample].Contains("DY")) {continue;} //Consider DD NPL, not MC
            else if(v_samples[isample] == "DATA") {continue;} //Data appended manually

            // cout<<"Pass "<<(isample == v_label.size()-1 || v_label[isample] != v_label[isample+1])<<endl; //Debug

    		if(isample == v_label.size()-1 || v_label[isample] != v_label[isample+1])
    		{
    			file_latex<<"c|"; //1 column per process (-1 already printed above)
    		}
    	}
        file_latex<<"|c|"; //Total SM
        if(!blind) {file_latex<<"|c|";} //Data
    	file_latex<<"}"<<endl;
    	file_latex<<"\\hline"<<endl;
    	file_latex<<" & "; //Leave leftmost case empty
    	for(int isample=0; isample<v_label.size(); isample++) //Declare all process names
    	{
            if(v_label[isample].Contains("TTbar") || v_label[isample].Contains("DY")) {continue;}
            else if(v_samples[isample] == "DATA") {continue;}

            if(isample == v_label.size()-1 || v_label[isample] != v_label[isample+1])
    		{
                // if(v_label[isample] == "ttZ") {file_latex<<"$t\\bar{t}Z$ & ";}
                if(v_label[isample].Contains("tZq")) {file_latex<<"$\\tZq$ & ";}
                else if(v_label[isample].Contains("ttZ")) {file_latex<<"$\\ttZ$ & ";}
                else if(v_label[isample].Contains("tWZ")) {file_latex<<"$\\tWZ$ & ";}
                else if(v_label[isample] == "ttW") {file_latex<<"$t\\ttW$ & ";}
                else if(v_label[isample] == "ttH") {file_latex<<"$t\\ttH$ & ";}
                else if(v_label[isample] == "tX") {file_latex<<"$t(\\tX$ & ";}
                else if(v_label[isample] == "WZ") {file_latex<<"$\\WZ$ & ";}
                else if(v_label[isample] == "VVV") {file_latex<<"$\\VVV$ & ";}
                else if(v_label[isample] == "XG") {file_latex<<"$\\Xg$ & ";}
    			else {file_latex<<""<<v_label[isample]<<" & ";}
    		}
    	}
        file_latex<<"Total SM";
        if(!blind) {file_latex<<" & Data";}
    	file_latex<<"\\\\ \\hline"<<endl;

        // file_latex<<" & "; //Leftmost column case empty
        TString cat_latex_name = Get_Category_LatexName(category); //Leftmost column case = category name
        file_latex<<cat_latex_name + " & ";
    } //Latex

//--------------------------------------------

//--------------------------------------------
	ofstream file_out(outname.Data());
	file_out<<"## Yields  in "<<category<<" category, "<<channel<<" channel ("<<lumi<<") ##"<<endl;
	file_out<<"____________________________________________"<<endl;
	file_out<<"____________________________________________"<<endl;

	//NB : don't declare inside sample loop, cause might want to sum samples
    vector<double> v_yields_proc_allYears(v_samples.size(), 0), v_statErr_proc_allYears(v_samples.size(), 0); //Sum yields per process for all years
    double yield_tmp = 0;
	double yield_signals = 0;
	double yield_bkg = 0;
	double yield_DATA = 0;

	double statErr_tmp = 0;
	double statErr_signals = 0;

 // #   # ######   ##   #####     #       ####   ####  #####
 //  # #  #       #  #  #    #    #      #    # #    # #    #
 //   #   #####  #    # #    #    #      #    # #    # #    #
 //   #   #      ###### #####     #      #    # #    # #####
 //   #   #      #    # #   #     #      #    # #    # #
 //   #   ###### #    # #    #    ######  ####   ####  #

    for(int iyear=0; iyear<v_years.size(); iyear++)
    {
        TString dir_ntuples = NTUPLEDIR + v_years[iyear] + "/"; //NTUPLEDIR is defined in Utils/Helper.h
        // TString dir_ntuples = NTUPLEDIR + v_years[iyear] + "/";

    	//FIRST LOOP ON SAMPLES : check here if files are missing ; else, may interfer with summing of several processes (TTZ, Rares, ...)
    	for(int isample=0; isample<v_samples.size(); isample++)
    	{
    		TString filepath = dir_ntuples + v_samples[isample]+".root";
    		// cout<<"-- File "<<filepath<<endl;
    		if(!Check_File_Existence(filepath) )
    		{
    			//ERASE MISSING SAMPLES FROM VECTORS
    			v_samples.erase(v_samples.begin() + isample);
    			v_label.erase(v_label.begin() + isample);

    			cout<<FRED("File "<<filepath<<" not found ! Erased index '"<<isample<<"' from vectors")<<endl;
    		}
    	}


    //  ####    ##   #    # #####  #      ######    #       ####   ####  #####
    // #       #  #  ##  ## #    # #      #         #      #    # #    # #    #
    //  ####  #    # # ## # #    # #      #####     #      #    # #    # #    #
    //      # ###### #    # #####  #      #         #      #    # #    # #####
    // #    # #    # #    # #      #      #         #      #    # #    # #
    //  ####  #    # #    # #      ###### ######    ######  ####   ####  #

    	for(int isample=0; isample<v_samples.size(); isample++)
    	{
    		TString filepath = dir_ntuples + v_samples[isample]+".root";
    		cout<<"-- File "<<filepath<<endl;

    		if(!Check_File_Existence(filepath) )
    		{
    			cout<<FRED("File "<<filepath<<" not found !")<<endl;
    			continue;
    		}

            bool isData = false;
            if(v_samples[isample] == "DATA") {isData = true;}

    		// cout<<FBLU("Sample : "<<v_samples[isample]<<"")<<endl;

    		TFile* f = new TFile(filepath);
    		TTree* t = NULL;
            t = (TTree*) f->Get(treename);
            if(!t) {cout<<FRED("Tree '"<<treename<<"' not found ! Skip !")<<endl; continue;}
            t->SetBranchStatus("*", 0); //disable all branches, speed up reading

            //FIXME
    		// Double_t weight = 1., weight_avg = 0.; //Event weight (gen-level weight, smeared by systematics)
            // Float_t eventMCFactor, weightMENominal; //Sample-dependent factor computed at Potato-level (lumi*xsec/SWE)
            // t->SetBranchStatus("eventWeight", 1);
    		// t->SetBranchAddress("eventWeight", &weight);
            // t->SetBranchStatus("eventMCFactor", 1);
    		// t->SetBranchAddress("eventMCFactor", &eventMCFactor);
            // t->SetBranchStatus("weightMENominal", 1);
    		// t->SetBranchAddress("weightMENominal", &weightMENominal);

            Float_t weight;
            Int_t vjj_nlumiWeights = 113;
            Float_t vjj_photon_effWgt, vjj_weight, vjj_mu_effWgt, vjj_lumiWeights[vjj_nlumiWeights];
            if(!isData)
            {
                t->SetBranchStatus("vjj_photon_effWgt", 1); t->SetBranchAddress("vjj_photon_effWgt", &vjj_photon_effWgt);
                t->SetBranchStatus("vjj_weight", 1); t->SetBranchAddress("vjj_weight", &vjj_weight);
                t->SetBranchStatus("vjj_lumiWeights", 1); t->SetBranchAddress("vjj_lumiWeights", vjj_lumiWeights);
                t->SetBranchStatus("vjj_mu_effWgt", 1); t->SetBranchAddress("vjj_mu_effWgt", &vjj_mu_effWgt);
            }

            Bool_t vjj_isGood;
            Int_t vjj_fs, vjj_trig;
            Float_t vjj_v_eta, vjj_jj_deta, vjj_jj_m, vjj_v_pt, vjj_lead_pt, vjj_sublead_pt;
            t->SetBranchStatus("vjj_v_eta", 1); t->SetBranchAddress("vjj_v_eta", &vjj_v_eta);
            t->SetBranchStatus("vjj_jj_deta", 1); t->SetBranchAddress("vjj_jj_deta", &vjj_jj_deta);
            t->SetBranchStatus("vjj_jj_m", 1); t->SetBranchAddress("vjj_jj_m", &vjj_jj_m);
            t->SetBranchStatus("vjj_v_pt", 1); t->SetBranchAddress("vjj_v_pt", &vjj_v_pt);
            t->SetBranchStatus("vjj_isGood", 1); t->SetBranchAddress("vjj_isGood", &vjj_isGood);
            t->SetBranchStatus("vjj_fs", 1); t->SetBranchAddress("vjj_fs", &vjj_fs);
            t->SetBranchStatus("vjj_jj_m", 1); t->SetBranchAddress("vjj_jj_m", &vjj_jj_m);
            t->SetBranchStatus("vjj_lead_pt", 1); t->SetBranchAddress("vjj_lead_pt", &vjj_lead_pt);
            t->SetBranchStatus("vjj_sublead_pt", 1); t->SetBranchAddress("vjj_sublead_pt", &vjj_sublead_pt);
            t->SetBranchStatus("vjj_trig", 1); t->SetBranchAddress("vjj_trig", &vjj_trig);

            Bool_t vjj_photonIsMatched;
            if(v_samples[isample] == "QCD" || v_samples[isample] == "ttbar")
            {
                t->SetBranchStatus("vjj_photonIsMatched", 1); t->SetBranchAddress("vjj_photonIsMatched", &vjj_photonIsMatched);
            }

            //--- Cut on relevant event selection (e.g. 3l SR, ttZ CR, etc.) -- stored as Char_t
            // Char_t is_goodCategory; //Categ. of event


     // ###### #    # ###### #    # #####    #       ####   ####  #####
     // #      #    # #      ##   #   #      #      #    # #    # #    #
     // #####  #    # #####  # #  #   #      #      #    # #    # #    #
     // #      #    # #      #  # #   #      #      #    # #    # #####
     // #       #  #  #      #   ##   #      #      #    # #    # #
     // ######   ##   ###### #    #   #      ######  ####   ####  #

    		int nentries = t->GetEntries();
    		for(int ientry=0; ientry<nentries; ientry++)
    		{
    			weight=1.;

    			t->GetEntry(ientry);

                // if(channel == "uuu" && chan != 0) {continue;}
                // if(channel == "uue" && chan != 1) {continue;}
                // if(channel == "eeu" && chan != 2) {continue;}
                // if(channel == "eee" && chan != 3) {continue;}

                //--- Cut on category value
                // if(category != "" && !is_goodCategory) {continue;}

                //FIXME
                float ptCut = 200, fs = 22, mjj=200;
                bool lowPtCut= (abs(vjj_v_eta)<1.442 && abs(vjj_jj_deta) > 3.0 && vjj_jj_m > 500 && vjj_v_pt > 75);
                bool generalCuts = ((vjj_isGood) && (vjj_fs==fs) && (vjj_jj_m>mjj) && (vjj_lead_pt>50) && (vjj_sublead_pt>50));
                bool pass = (vjj_trig == 2 || (vjj_trig==3 && !lowPtCut)) && generalCuts && vjj_v_pt > ptCut;
                if((v_samples[isample] == "QCD" || v_samples[isample] == "ttbar") && vjj_photonIsMatched == 1) {pass = false;}
                if(!pass) {continue;}

                // if(isnan(weight*eventMCFactor) || isinf(weight*eventMCFactor))
                // {
                //     cout<<BOLD(FRED("* Found event with weight*eventMCFactor = "<<weight<<"*"<<eventMCFactor<<" ; remove it..."))<<endl; break; //continue;
                // }

                if(!isData) {weight = vjj_photon_effWgt * vjj_weight * vjj_lumiWeights[0] / vjj_mu_effWgt;} //FIXME

                v_yields_proc_allYears[isample]+= weight;
                v_statErr_proc_allYears[isample]+= weight*weight;

                if(v_samples[isample] == "DATA") //DATA
    			{
    				yield_DATA+= weight;
    				// statErr_DATA+= weight;
    			}
                else //MC
                {
                    yield_tmp+= weight; statErr_tmp+= weight*weight;

                    if(v_label[isample] == signal || v_label[isample] == "signal") //Signals, group together
                    {
                        yield_signals+= weight;
                        statErr_signals+= weight*weight;
                        // cout<<"yield_signals "<<yield_signals<<endl;
                    }
                    else if(v_samples[isample] != "DATA" && !v_samples[isample].Contains("tZq") && !v_samples[isample].Contains("ttZ") && !v_samples[isample].Contains("tWZ") && !v_samples[isample].Contains("TTbar") && !v_samples[isample].Contains("DY")) //Backgrounds //Don't consider: signals / private samples / MC fakes / ...
                    {
                        yield_bkg+= weight;
                    }
                }
            } //event loop

    		// cout<<"yield_bkg = "<<yield_bkg<<endl;
    		// cout<<"yield_tmp "<<yield_tmp<<endl;

    		//Check if restart counter, or merge processes
            if(lumi != "Run2")
            {
                if(v_samples[isample] != "DATA" && (isample == v_label.size()-1 || v_label[isample] != v_label[isample+1]))
                {
                    file_out<<"--------------------------------------------"<<endl;
                    file_out<<left<<setw(25)<<v_label[isample]<<setprecision(4)<<yield_tmp;
                    // file_out<<v_label[isample]<<"\\t"<<yield_tmp;

                    file_out<<" (+/- "<<sqrt(statErr_tmp)<<" stat.)"<<endl;
                    // cout<<left<<setw(25)<<v_label[isample]<<yield_tmp<<endl;

                    if(create_latex_table && !v_label[isample].Contains("TTbar") && !v_label[isample].Contains("DY") && !v_label[isample].Contains("DATA") && v_label[isample]!="tZq" && v_label[isample]!="ttZ" && v_label[isample]!="tWZ")
                    // if(create_latex_table && !v_label[isample].Contains("TTbar") && !v_label[isample].Contains("DY") && !v_label[isample].Contains("DATA")  && !v_label[isample].Contains("PrivMC"))
                    {
                        file_latex<<fixed<<setprecision(precision)<<abs(yield_tmp)<<" ($\\pm$"<<fixed<<setprecision(precision)<<sqrt(statErr_tmp)<<") & "; //Single process
                    }

                    yield_tmp = 0; //Reset after writing to file
                    statErr_tmp = 0;
                } //write result
            }
        } //sample loop

    } //year loop

    if(lumi == "Run2") //Run 2: need to sum quadratically per-year errors
    {
        float yield_currentGroup = 0, statErr_currentGroup = 0;

        for(int isample=0; isample<v_samples.size(); isample++)
        {
            if(v_samples[isample] == "DATA") {continue;} //printed separately

            //Combine errors from all years quadratically
            yield_currentGroup+= v_yields_proc_allYears[isample], statErr_currentGroup+= v_statErr_proc_allYears[isample]; //NB: no need to square uncert. here, it is already ! (cf. above)

            if(group_samples_together && v_samples[isample] != "DATA" && isample < v_label.size()-1 && v_label[isample] == v_label[isample+1]) {continue;} //Sum processes from same group

            //Printout
            file_out<<"--------------------------------------------"<<endl;
            file_out<<left<<setw(25)<<v_label[isample]<<setprecision(4)<<yield_currentGroup;
            // file_out<<v_label[isample]<<"\\t"<<yield_tmp;

            file_out<<" (+/- "<<sqrt(statErr_currentGroup)<<" stat.)"<<endl;
            // cout<<left<<setw(25)<<v_label[isample]<<yield_tmp<<endl;

            if(create_latex_table && !v_label[isample].Contains("TTbar") && !v_label[isample].Contains("DY") && !v_label[isample].Contains("DATA") && v_label[isample]!="tZq" && v_label[isample]!="ttZ" && v_label[isample]!="tWZ")
            // if(create_latex_table && !v_label[isample].Contains("TTbar") && !v_label[isample].Contains("DY") && !v_label[isample].Contains("DATA")  && !v_label[isample].Contains("PrivMC"))
            {
                file_latex<<fixed<<setprecision(precision)<<abs(yield_currentGroup)<<" ($\\pm$"<<fixed<<setprecision(precision)<<sqrt(statErr_currentGroup)<<") & "; //Single process
            }

            yield_currentGroup = 0; statErr_currentGroup = 0; //Reset
        }
    }

	file_out<<endl<<"____________________________________________"<<endl;
	file_out<<"____________________________________________"<<endl;
	file_out<<left<<setw(25)<<"Signal"<<setprecision(5)<<yield_signals;
	file_out<<endl;

    file_out<<endl<<"____________________________________________"<<endl;
	file_out<<"____________________________________________"<<endl;
	file_out<<left<<setw(25)<<"Total background"<<setprecision(5)<<yield_bkg;
	file_out<<endl;

    file_out<<endl<<"____________________________________________"<<endl;
	file_out<<"____________________________________________"<<endl;
	file_out<<left<<setw(25)<<"Total MC"<<setprecision(5)<<yield_signals+yield_bkg;
	file_out<<endl;

	file_out<<endl<<"____________________________________________"<<endl;
	file_out<<"____________________________________________"<<endl;
	file_out<<left<<setw(25)<<"DATA"<<setprecision(5)<<yield_DATA<<endl;
	file_out<<"____________________________________________"<<endl;
	file_out<<"____________________________________________"<<endl;

    if(create_latex_table)
    {
        file_latex<<fixed<<setprecision(precision)<<yield_signals+yield_bkg; //Total SM
        if(!blind) {file_latex<<fixed<<setprecision(precision)<<" & "<<setprecision(precision)<<yield_DATA;} //Data
        file_latex<<" \\\\ \\hline"<<endl;
        file_latex<<"\\end{tabular}"<<endl;
        file_latex<<"\\caption{Event yields for the "<<lumi<<" data-taking period.}"<<endl;
        file_latex<<"\\label{tab:yields}"<<endl;
        // file_latex<<"\\end{table}"<<endl;
        file_latex<<"\\end{sidewaystable}"<<endl;
    }

	cout<<endl<<FYEL("-- Wrote file : "<<outname<<"")<<endl;
	if(create_latex_table) {cout<<FYEL("-- Wrote file : "<<outname_latex<<"")<<endl;}

	return;
}








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
//--------------------------------------------
//--------------------------------------------

int main(int argc, char **argv)
{
    cout<<FYEL("USAGE : ./Yield_Table.exe [region] [2016,2017,2018,Run2]")<<endl<<endl;

//== OPTIONS ==
//--------------------------------------------
    //-- Default args (can be over-riden via command line args)
    TString signal = "";
    TString treename = "Events";

    //-- Category: '' <-> all events ; 'xxx' <-> only include events satisfying condition xxx //E.g.: 'is_signal_SR'
    TString category = "SR";

    TString lumi = "all"; //'2016','2017','2018','Run2,'all''
    TString channel = ""; //'',uuu,uue,eeu,eee
    bool remove_totalSF = false; //SFs are applied to default weights ; can divide weight by total SF again to get nominal weight
    bool group_samples_together = false; //true <-> group similar samples together
    bool process_samples_byGroup = false; //true <-> read grouped samples (if already hadded together), else read individual samples and combine them when creating histograms if needed (default)

//--------------------------------------------
//--------------------------------------------

    TString region = ""; vector<TString> v_lumis(1); TString dummy;
    Apply_CommandArgs_Choices(argc, argv, v_lumis, region, dummy); //Get lumi/region via command line
    if(region != "") {category = Get_Category_Boolean_Name(region);}
    if(v_lumis.size() == 3) {lumi = "Run2";}
    else if(v_lumis[0] != "") {lumi = v_lumis[0];}

//--------------------------------------------
//--------------------------------------------

	//Sample names and labels //NB : labels must be latex-compatible
	vector<TString> v_samples; vector<TString> v_label;

    //-- Read ntuples merged by sample groups
    if(process_samples_byGroup)
    {
        v_samples.push_back("DATA"); v_label.push_back("DATA");
    }
    //-- Read individual ntuples
    else
    {
        v_samples.push_back("DATA"); v_label.push_back("DATA");
        v_samples.push_back("VBFgamma"); v_label.push_back("VBFgamma");
        v_samples.push_back("GJets"); v_label.push_back("GJets");
        v_samples.push_back("ttbar"); v_label.push_back("ttbar");
        v_samples.push_back("ttGJets"); v_label.push_back("ttGJets");
        v_samples.push_back("DiPhoton"); v_label.push_back("DiPhoton");
        v_samples.push_back("WJetsToLNu"); v_label.push_back("WJetsToLNu");
        v_samples.push_back("WJetsToQQ"); v_label.push_back("WJetsToQQ");
        v_samples.push_back("ZGTo2LG"); v_label.push_back("ZGTo2LG");
        v_samples.push_back("WGToLNuG"); v_label.push_back("WGToLNuG");
        v_samples.push_back("QCD"); v_label.push_back("QCD");
        // v_samples.push_back("DYJetsNLO"); v_label.push_back("DYJetsNLO");
        // v_samples.push_back("LLJJ"); v_label.push_back("LLJJ");
    }

//--------------------------------------------
//--------------------------------------------

    if(lumi == "all")
    {
        Compute_Write_Yields(v_samples, v_label, category, signal, "2016", group_samples_together, remove_totalSF, channel, treename);
        Compute_Write_Yields(v_samples, v_label, category, signal, "2017", group_samples_together, remove_totalSF, channel, treename);
        Compute_Write_Yields(v_samples, v_label, category, signal, "2018", group_samples_together, remove_totalSF, channel, treename);
        Compute_Write_Yields(v_samples, v_label, category, signal, "Run2", group_samples_together, remove_totalSF, channel, treename); //should sum all years
    }
    else {Compute_Write_Yields(v_samples, v_label, category, signal, lumi, group_samples_together, remove_totalSF, channel, treename);}

	return 0;
}
