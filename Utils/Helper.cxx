#include "Helper.h"

using namespace std;

//-- HARDCODE THE BASE DIR. CONTAINING PRIVATE NTUPLES (used by all codes)
TString NTUPLEDIR = "./input_ntuples/"; //FIXLOCAL
//TString NTUPLEDIR = "/nfs/dust/cms/user/ntonon/TopEFT/CMSSW_10_2_24/src/potato-nicolas/nicolas/output/Analyzer3l-V12-AllSamples-d20210304-t102904/merged_ntuples/"; //FIXCMSSW


//--------------------------------------------
// ##        #######  ##      ##         ##       ######## ##     ## ######## ##
// ##       ##     ## ##  ##  ##         ##       ##       ##     ## ##       ##
// ##       ##     ## ##  ##  ##         ##       ##       ##     ## ##       ##
// ##       ##     ## ##  ##  ## ####### ##       ######   ##     ## ######   ##
// ##       ##     ## ##  ##  ##         ##       ##        ##   ##  ##       ##
// ##       ##     ## ##  ##  ##         ##       ##         ## ##   ##       ##
// ########  #######   ###  ###          ######## ########    ###    ######## ########

// ##     ## ######## ##       ########  ######## ########
// ##     ## ##       ##       ##     ## ##       ##     ##
// ##     ## ##       ##       ##     ## ##       ##     ##
// ######### ######   ##       ########  ######   ########
// ##     ## ##       ##       ##        ##       ##   ##
// ##     ## ##       ##       ##        ##       ##    ##
// ##     ## ######## ######## ##        ######## ##     ##

// ######## ##     ## ##    ##  ######   ######
// ##       ##     ## ###   ## ##    ## ##    ##
// ##       ##     ## ####  ## ##       ##
// ######   ##     ## ## ## ## ##        ######
// ##       ##     ## ##  #### ##             ##
// ##       ##     ## ##   ### ##    ## ##    ##
// ##        #######  ##    ##  ######   ######
//--------------------------------------------

//Use stat function (from library sys/stat) to check if a file exists
bool Check_File_Existence(const TString& name, bool printError_missingFile)
{
    struct stat buffer;
    bool found = (stat (name.Data(), &buffer) == 0); //true if file exists
    if(!found && printError_missingFile) {cout<<endl<<BRED(BOLD(FWHT("ERROR : file "<<name<<" not found !")))<<endl<<endl;}
    return found;
}

//Move file with bash command 'mv'
int MoveFile(TString origin_path, TString dest_path)
{
	TString command = "mv "+origin_path + " " + dest_path;

	return system(command.Data() );
}

//Copy file with bash command 'cp'
int CopyFile(TString origin_path, TString dest_path)
{
	TString command = "cp "+origin_path + " " + dest_path;

	return system(command.Data() );;
}

//Convert a double into a TString
//@ precision --> can choose if TString how many digits the TString should display
//NB: alternatively, can simply use TString::Form(format,var). Ex: Form("%d", (int) a), Form("%f", (float) f), ...
TString Convert_Number_To_TString(double number, int precision/*=3*/)
{
    //-- NB: setprecision rounding may not be correct, cf. https://stackoverflow.com/questions/10922366/does-setprecision-in-c-round-if-so-why-am-i-seeing-this
    //==> Manual rounding
    float nearest = roundf(number * pow(10,precision)) / pow(10,precision);

	stringstream ss;
	ss << std::setprecision(precision) << nearest;
	TString ts = ss.str();
	return ts;
}

//Convert a TString into a float
float Convert_TString_To_Number(TString ts)
{
	float number = 0;
	string s = ts.Data();
	stringstream ss(s);

    //Bad characters may produce errors. Read 'out' these chars
    while(ss >> number || !ss.eof())
    {
        if(ss.fail()) //Found bad char. Remove it, read the remaining string
        {
            ss.clear();
            char dummy;
            ss >> dummy;
            continue;
        }
    }

	return number;
}

//Find a number into a TString, and returns it as a float
float Find_Number_In_TString(TString ts)
{
	TString tmp = ""; int number = 0;
	string s = ts.Data(); int ts_size = s.size(); //Only to get TString size

	for (int i=0; i < ts_size; i++)
	{
		if( isdigit(ts[i]) )
		{
			do
			{
				tmp += ts[i];
				i++;
			} while(isdigit(ts[i]) || ts[i] == '.'); //NB : Pay attention to quotes : "a" creates a 2-char array (letter+terminator) -> string. 'a' identifies a single character !

		  break;
		}
	}

	return Convert_TString_To_Number(tmp);
}

//Translates signs into words
TString Convert_Sign_To_Word(TString ts_in)
{
	TString sign = "";

	if(ts_in.Contains("<=")) {sign = "MaxEq";}
	else if(ts_in.Contains(">=")) {sign = "MinEq";}
	else if(ts_in.Contains(">")) {sign = "Min";}
	else if(ts_in.Contains("<")) {sign = "Max";}
	else if(ts_in.Contains("==")) {sign = "Eq";}
	else if(ts_in.Contains("!=")) {sign = "Diff";}

	return sign;
}

//Used when a "cut" is composed of 2 conditions -> breaks it into a pair of TStrings <cut1,cut2>
pair<TString,TString> Break_Cuts_In_Two(TString multiple_cut)
{
	TString cut1 = "", cut2 = "";

	string s = multiple_cut.Data(); int size = s.size(); //To get TString size

	for (int i=0; i < size; i++) //Extract condition 1
	{
		if(multiple_cut[i] == '&' || multiple_cut[i] == '|') {break;} //stop when we find '&&' or '||'
		if(multiple_cut[i] != ' ') {cut1 += multiple_cut[i];} //else, store characters in TString
	}

	TString tmp = "";
	for (int i=0; i < size; i++) //Extract condition 2
	{
		tmp+= multiple_cut[i];
		if( (!tmp.Contains("&&") && !tmp.Contains("||")) || multiple_cut[i] == '&' || multiple_cut[i] == '|') {continue;}
		if(multiple_cut[i] != ' ') {cut2 += multiple_cut[i];}
	}

	pair<TString,TString> the_cuts; the_cuts.first = cut1; the_cuts.second = cut2;

	return the_cuts;
}

//Check if a given value passes a given cut definition
bool Is_Event_Passing_Cut(TString cut_def, double value)
{
	double cut_tmp;

	if(!cut_def.Contains("&&") && !cut_def.Contains("||")) //If cut contains only 1 condition
	{
		cut_tmp = Find_Number_In_TString(cut_def);
		if(cut_def.Contains(">=") && value < cut_tmp)		 {return false;}
		else if(cut_def.Contains("<=") && value > cut_tmp)	 {return false;}
		else if(cut_def.Contains(">") && !cut_def.Contains(">=") && value <= cut_tmp)	 {return false;}
		else if(cut_def.Contains("<") && !cut_def.Contains("<=") && value >= cut_tmp) 	 {return false;}
		else if(cut_def.Contains("==") && value != cut_tmp)  {return false;}
		else if(cut_def.Contains("!=") && value == cut_tmp)  {return false;}
	}
	else if(cut_def.Contains("&&") && cut_def.Contains("||"))
	{
		cout<<BOLD(FRED("ERROR ! Cut definition too complex ; break it !"))<<endl;
	}
	else if(cut_def.Contains("&&") )//If '&&' in the cut def, break it in 2
	{
		TString cut1 = Break_Cuts_In_Two(cut_def).first;
		TString cut2 = Break_Cuts_In_Two(cut_def).second;
		//CUT 1
		cut_tmp = Find_Number_In_TString(cut1);
		if(cut1.Contains(">=") && value < cut_tmp)			 {return false;}
		else if(cut1.Contains("<=") && value > cut_tmp)		 {return false;}
		else if(cut1.Contains(">") && value <= cut_tmp) 	{return false;}
		else if(cut1.Contains("<") && value >= cut_tmp) 	{return false;}
		else if(cut1.Contains("==") && value != cut_tmp) 	 {return false;}
		else if(cut1.Contains("!=") && value == cut_tmp) 	 {return false;}
		//CUT 2
		cut_tmp = Find_Number_In_TString(cut2);
		if(cut2.Contains(">=") && value < cut_tmp)			 {return false;}
		else if(cut2.Contains("<=") && value > cut_tmp)		 {return false;}
		else if(cut2.Contains(">") && value <= cut_tmp)	{return false;}
		else if(cut2.Contains("<") && value >= cut_tmp)  {return false;}
		else if(cut2.Contains("==") && value != cut_tmp) 	 {return false;}
		else if(cut2.Contains("!=") && value == cut_tmp) 	 {return false;}
	}
	else if(cut_def.Contains("||") )//If '||' in the cut def, break it in 2
	{
		TString cut1 = Break_Cuts_In_Two(cut_def).first;
		TString cut2 = Break_Cuts_In_Two(cut_def).second;

		bool pass_cut1 = true; bool pass_cut2 = true; //Need to pass at least 1 cut

		//CUT 1
		cut_tmp = Find_Number_In_TString(cut1);
		if(cut1.Contains(">=") && value < cut_tmp)			 {pass_cut1 = false;}
		else if(cut1.Contains("<=") && value > cut_tmp)		 {pass_cut1 = false;}
		else if(cut1.Contains(">") && value <= cut_tmp)	{pass_cut1 = false;}
		else if(cut1.Contains("<") && value >= cut_tmp) 	{pass_cut1 = false;}
		else if(cut1.Contains("==") && value != cut_tmp) 	 {pass_cut1 = false;}
		else if(cut1.Contains("!=") && value == cut_tmp) 	 {pass_cut1 = false;}
		//CUT 2
		cut_tmp = Find_Number_In_TString(cut2);
		if(cut2.Contains(">=") && value < cut_tmp)			 {pass_cut2 = false;}
		else if(cut2.Contains("<=") && value > cut_tmp)		 {pass_cut2 = false;}
		else if(cut2.Contains(">") && value <= cut_tmp)		 	{pass_cut2 = false;}
		else if(cut2.Contains("<") && value >= cut_tmp) 	{pass_cut2 = false;}
		else if(cut2.Contains("==") && value != cut_tmp) 	 {pass_cut2 = false;}
		else if(cut2.Contains("!=") && value == cut_tmp) 	 {pass_cut2 = false;}

		if(!pass_cut1 && !pass_cut2) {return false;}
	}

	//If no condition was matched, it means the values passes the cut
	return true;
}

//Rescales the input value so that it retains its relative position in the new scale
double Compute_RangeScaled_Value(double value, double min_old, double max_old, double min_new, double max_new)
{
	return ((value - min_old) / (max_old - min_old)) * (max_new - min_new) + min_new;
}

//Returns the nof std deviations w.r.t. central value of the input value
double Compute_StdDevScaled_Value(double value, double mean, double dev)
{
	return (value - mean) / dev;
}

//Pass the names of the dir contents as references
bool Get_Dir_Content(string dir, vector<TString> &files)
{
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL)
    {
        cout << "Error(" << errno << ") opening " << dir << endl;
        return false;
    }

    while ((dirp = readdir(dp)) != NULL)
    {
        files.push_back(string(dirp->d_name));
    }

    closedir(dp);

    return true;
}

//Split a long TString in smaller TStrings which are all separated by a common delimiter
//Only to remember the logic, no useful return for now (could return the array, but then loose control of the dynamic pointer...)
TString Split_TString_Into_Keys(TString ts, TString delim)
{
    TObjArray *tx = ts.Tokenize("_");
    // tx->Print();

    for(Int_t i = 0; i < tx->GetEntries(); i++)
    {
        TString tmp = ((TObjString *)(tx->At(i)))->String();
        std::cout << tmp << std::endl;
    }

    delete tx;

    return ts;
}

/**
 * Count the number of histograms found in a TFile
 * NB: very slow for huge files !
 */
int Count_nofHistos_inTFile(TFile* f)
{
    TKey* key = NULL;
    Int_t total = 0;

    TIter next((TList*) f->GetListOfKeys());
    while(key = (TKey*)next())
    {
        TClass* cl = gROOT->GetClass(key->GetClassName());
        if(cl->InheritsFrom("TH1"))
        {
            TH1* h = (TH1*) key->ReadObj();
            total++;
            // cout << "Histo found: " << h->GetName() << " - " << h->GetTitle() << endl;
        }
    }

    // cout << "Found " << total << " histograms" << endl;
    return total;
}


/**
 * Overloaded function
 */
int Count_nofHistos_inTFile(TString fname)
{
    TFile *f = TFile::Open(fname, "READ"); //Open
    if(!f || f->IsZombie()) {cout << "Unable to open " << fname << " for reading..." <<endl; return -1;}

    int total = Count_nofHistos_inTFile(f); //Count

    f->Close(); //Close

    return total; //Return count
}

/**
 * Round float to bottom/upper value, with precision determined by 'roundLevel' (e.g. 0.1)
 */
float Round_Float_toDecimal(float x, float roundLevel, bool roundDown)
{
    if(roundDown) {return int(x/roundLevel)*roundLevel;}
    return int(x/roundLevel + 1)*roundLevel;; //Round up
}

vector<TString> Break_TString(TString input, TString delimiter)
{
    vector<TString> output;
    TObjArray *tx = input.Tokenize(delimiter);
    // tx->Print();
    for (Int_t i = 0; i < tx->GetEntries(); i++) {output.push_back( ((TObjString *)(tx->At(i)))->String() );}
    return output;
}




//--------------------------------------------
// ########     ###     ######  ####  ######
// ##     ##   ## ##   ##    ##  ##  ##    ##
// ##     ##  ##   ##  ##        ##  ##
// ########  ##     ##  ######   ##  ##
// ##     ## #########       ##  ##  ##
// ##     ## ##     ## ##    ##  ##  ##    ##
// ########  ##     ##  ######  ####  ######

// ##     ## ######## ##       ########  ######## ########
// ##     ## ##       ##       ##     ## ##       ##     ##
// ##     ## ##       ##       ##     ## ##       ##     ##
// ######### ######   ##       ########  ######   ########
// ##     ## ##       ##       ##        ##       ##   ##
// ##     ## ##       ##       ##        ##       ##    ##
// ##     ## ######## ######## ##        ######## ##     ##

// ######## ##     ## ##    ##  ######
// ##       ##     ## ###   ## ##    ##
// ##       ##     ## ####  ## ##
// ######   ##     ## ## ## ## ##
// ##       ##     ## ##  #### ##
// ##       ##     ## ##   ### ##    ##
// ##        #######  ##    ##  ######
//--------------------------------------------

//Increment weight of last bin by 'weight'
void Fill_Last_Bin_TH1F(TH1F* h, double weight)
{
	h->SetBinContent(h->GetNbinsX(), h->GetBinContent(h->GetNbinsX()) + weight);

	return;
}

//Increment weight of first bin by 'weight'
void Fill_First_Bin_TH1F(TH1F* h, double weight)
{
	h->SetBinContent(1, h->GetBinContent(1) + weight);

	return;
}

//Enforce desired canvas style
void Load_Canvas_Style()
{
	// For the canvas:
	gStyle->SetCanvasBorderMode(0);
	gStyle->SetCanvasColor(0); // must be kWhite but I dunno how to do that in PyROOT
	gStyle->SetCanvasDefH(600); //Height of canvas
	gStyle->SetCanvasDefW(600); //Width of canvas
	gStyle->SetCanvasDefX(0);   //POsition on screen
	gStyle->SetCanvasDefY(0);
	gStyle->SetPadBorderMode(0);
	gStyle->SetPadColor(0); // kWhite
	gStyle->SetPadGridX(0); //false
	gStyle->SetPadGridY(0); //false
	gStyle->SetGridColor(0);
	gStyle->SetGridStyle(3);
	gStyle->SetGridWidth(1);
	gStyle->SetFrameBorderMode(0);
	gStyle->SetFrameBorderSize(1);
	gStyle->SetFrameFillColor(0);
	gStyle->SetFrameFillStyle(0);
	gStyle->SetFrameLineColor(1);
	gStyle->SetFrameLineStyle(1);
	gStyle->SetFrameLineWidth(1);
	gStyle->SetHistLineColor(1);
	gStyle->SetHistLineStyle(0);
	gStyle->SetHistLineWidth(1);
	gStyle->SetEndErrorSize(2);
	gStyle->SetOptFit(1011);
	gStyle->SetFitFormat("5.4g");
	gStyle->SetFuncColor(2);
	gStyle->SetFuncStyle(1);
	gStyle->SetFuncWidth(1);
	gStyle->SetOptDate(0);
	gStyle->SetOptFile(0);
	gStyle->SetOptStat(0); // To display the mean and RMS:   SetOptStat("mr");
	gStyle->SetStatColor(0); // kWhite
	gStyle->SetStatFont(42);
	gStyle->SetStatFontSize(0.04);
	gStyle->SetStatTextColor(1);
	gStyle->SetStatFormat("6.4g");
	gStyle->SetStatBorderSize(1);
	gStyle->SetStatH(0.1);
	gStyle->SetStatW(0.15);
	gStyle->SetPadTopMargin(0.07);
	gStyle->SetPadBottomMargin(0.13);
	gStyle->SetPadLeftMargin(0.16);
	gStyle->SetPadRightMargin(0.03);
	gStyle->SetOptTitle(0);
	gStyle->SetTitleFont(42);
	gStyle->SetTitleColor(1);
	gStyle->SetTitleTextColor(1);
	gStyle->SetTitleFillColor(10);
	gStyle->SetTitleFontSize(0.05);
	gStyle->SetTitleColor(1, "XYZ");
	gStyle->SetTitleFont(42, "XYZ");
	gStyle->SetTitleSize(0.06, "XYZ");
	gStyle->SetTitleXOffset(0.9);
	gStyle->SetTitleYOffset(1.25);
	gStyle->SetLabelColor(1, "XYZ");
	gStyle->SetLabelFont(42, "XYZ");
	gStyle->SetLabelOffset(0.007, "XYZ");
	gStyle->SetLabelSize(0.05, "XYZ");
	gStyle->SetAxisColor(1, "XYZ");
	gStyle->SetStripDecimals(1); // kTRUE
	gStyle->SetTickLength(0.03, "XYZ");
	gStyle->SetNdivisions(510, "XYZ");
	gStyle->SetPadTickX(1);  // To get tick marks on the opposite side of the frame
	gStyle->SetPadTickY(1);
	gStyle->SetOptLogx(0);
	gStyle->SetOptLogy(0);
	gStyle->SetOptLogz(0);
	gStyle->SetPaperSize(20.,20.);
}

//Enforce the Combine name conventions
TString Combine_Naming_Convention(TString name)
{
	if(name.Contains("Up") || name.Contains("Down") ) {return name;} //If systematics is already following Combine convention

	else if(name.Contains("__plus"))
	{
		int i = name.Index("__plus"); //Find index of substring
		name.Remove(i); //Remove substring
		name+= "Up"; //Add Combine syst. suffix
	}

	else if(name.Contains("__minus"))
	{
		int i = name.Index("__minus"); //Find index of substring
		name.Remove(i); //Remove substring
		name+= "Down"; //Add Combine syst. suffix
	}

	return name;
}

//Modifies the TMVA standard output to extract only the useful ranking information (hard-coded way)
void Extract_Ranking_Info(TString TMVA_output_file, TString channel)
{
	ifstream file_in(TMVA_output_file.Data() );

	ofstream file_out("ranking_tmp.txt");

	file_out<<"--- "<<channel<<" : Variable Ranking ---"<<endl<<endl;

	string string_start = "Ranking input variables (method specific)...";
	string string_stop = "----------------------------------------------";

	string line;

	while(!file_in.eof( ) )
	{
		getline(file_in, line);

		if(line.find(string_start) != std::string::npos) {break;} //When 'string_start' is found, start copying lines into output file
	}

	int k=0;
	while(!file_in.eof( ) )
	{
		getline(file_in, line);

		if(k>3) //We know that the first 3 lines are useless
		{
			if(line.find(string_stop) != std::string::npos) {break;} //Don't stop until all variables are read

			TString string_to_write = line;
			int index = string_to_write.First(":"); //Find first occurence of ':' in the lines, and remove the text before it (useless)
		    string_to_write.Remove(0,index);

		    string s = string_to_write.Data(); int size = s.size(); //Compute size of the TString this way (ugly)
		    for(int i=0; i<size; i++)
		    {
		        if(isdigit(string_to_write[i]) ) //Only copy what is after the ranking index, remove the rest
		        {
		            string_to_write.Remove(0,i);
		            break;
		        }
		    }

			file_out<<string_to_write.Data()<<endl;
		}

		k++;
	}

	TString mv_command = "mv ranking_tmp.txt " + TMVA_output_file;
	int tmp = system(mv_command.Data() ); //Replace TMVA output textfile by modified textfile !

	return;
}

//Read modified TMVA ranking file in order to fill 2 vectors (passed by reference) ordered by decreasing rank (var names & importances)
//NB : Hard-coded !
void Get_Ranking_Vectors(TString channel, vector<TString> &v_BDTvar_name, vector<double> &v_BDTvar_importance)
{
	TString file_rank_path = "./outputs/Rankings/RANKING_"+channel+".txt";
	if(!Check_File_Existence(file_rank_path.Data()) ) {std::cout<<file_rank_path<<" not found -- Abort"<<endl; return;}
	ifstream file_in(file_rank_path);

	string line;
	getline(file_in, line); getline(file_in, line);

	while(!file_in.eof())
	{
		getline(file_in, line);

		TString ts = line; //Convert to TString for easy modification

		int index = ts.First(":"); //Find first occurence of ':' in the lines, and remove the text before it (useless)
		ts.Remove(0,index+1); //Remove first useless characters

		TString var_name = "";
		string ts_tmp = ts.Data(); int size = ts_tmp.size();
		for(int ichar=0; ichar<size; ichar++)
		{
			if(var_name != "" && ts[ichar] == ' ') {break;} //end of var name
			else if(ts[ichar] == ' ') {continue;} //Begin at var name

			var_name+=ts[ichar];
		}

		v_BDTvar_name.push_back(var_name);


		index = ts.First(":"); //Find other occurence of ':' (placed before variable importance number)
		ts.Remove(0,index+1); //After that, only the importance number remains in the TString

 		TString var_importance_string = "";
 		ts_tmp = ts.Data(); size = ts_tmp.size();
		for(int ichar=0; ichar<size; ichar++)
		{
			if(ts[ichar] == ' ') {continue;}
			var_importance_string+=ts[ichar];
		}

		//Convert tstring to number (importance of the var)
		double var_importance = 0;
		if(var_importance_string.Contains("e-01") )
		{
			index = var_importance_string.Index('e');
			var_importance_string.Remove(index);
			var_importance = Convert_TString_To_Number(var_importance_string);
			var_importance*= 0.1;
		}
		else if(var_importance_string.Contains("e-02") )
		{
			index = var_importance_string.Index('e');
			var_importance_string.Remove(index);
			var_importance = Convert_TString_To_Number(var_importance_string);
			var_importance*= 0.01;
		}

		v_BDTvar_importance.push_back(var_importance);
	}


	//Because while loop on 'eof' --> one line too many at the end --> erase last entry
	v_BDTvar_name.erase(v_BDTvar_name.begin() + v_BDTvar_name.size()-1);
	v_BDTvar_importance.erase(v_BDTvar_importance.begin() + v_BDTvar_importance.size()-1);

	return;
}

//Loof for differences between 2 different histos stored in 2 different TFiles
void Compare_Histograms(TString filepath1, TString filepath2, TString histname1, TString histname2)
{
    TFile* f1 = TFile::Open(filepath1, "READ");
    TFile* f2 = TFile::Open(filepath2, "READ");

    if(!f1->GetListOfKeys()->Contains(histname1) ) {cout<<FRED("Error : "<<histname1<<" not found in file "<<filepath1<<"")<<endl; return;}
    if(!f2->GetListOfKeys()->Contains(histname2) ) {cout<<FRED("Error : "<<histname2<<" not found in file "<<filepath2<<"")<<endl; return;}

	TH1F* h1 = (TH1F*) f1->Get(histname1);
	TH1F* h2 = (TH1F*) f2->Get(histname2);

	for(int ibin=0; ibin<h1->GetNbinsX(); ibin++)
	{
		double diff = fabs((h1->GetBinContent(ibin+1) - h2->GetBinContent(ibin+1))) / h1->GetBinContent(ibin+1);

		if(diff > 1./100)
		{
			cout<<"WARNING : bin "<<ibin+1<<" different ! ("<<h1->GetBinContent(ibin+1)<<" / "<<h2->GetBinContent(ibin+1)<<")"<<endl;
		}
	}

	delete h1;
	delete h2;

	f1->Close();
	f2->Close();

	return;
}

//Rescale input value, depending on the mean and variance of this variable
float Rescale_Input_Variable(float value, float val1, float val2)
{
    return (value * val2) + val1; //sklearn.preprocessing.MinMaxScaler (-1;+1)

    // return (value - val1) / sqrt(val2); //sklearn.preprocessing.StandardScaler
}

//Get mirror histo (rescaled to nominal if necessary)
//As done by Benjamin/ttH : https://github.com/stiegerb/cmgtools-lite/blob/80X_M17_tHqJan30_bbcombination/TTHAnalysis/python/plotter/makeShapeCardsTHQ.py#L609-L633
//Directly modify the histos passed in args
void Get_Mirror_Histogram(TH1F*& hcentral, TH1F*& hvariation, TH1F*& hmirror, bool is_ShapeOnly)
{
	hmirror = 0; //empty at beginning
	hmirror = (TH1F*) hcentral->Clone(); //Get binning, etc.

	//Rescale variation to nominal (shape-only)
    if(is_ShapeOnly) {hvariation->Scale(hcentral->Integral() / hvariation->Integral());}

	for(int ibin=1; ibin<=hcentral->GetNbinsX(); ibin++)
	{
		double yA = hvariation->GetBinContent(ibin); //'up = alternate'
		double y0 = hcentral->GetBinContent(ibin); //'central'
		double yM = y0; //'down = mirror'

		if(y0 > 0 && yA > 0) {yM = y0 * y0/yA;}
		else if(yA == 0) {yM = 2*y0;}

		hmirror->SetBinContent(ibin, yM);
	}

    if(is_ShapeOnly) {hmirror->Scale(hcentral->Integral() / hmirror->Integral());} //Rescale mirror to nominal (shape-only)
	else //Mirror normalization, cf github
	{
    	double mirror_norm = pow(hcentral->Integral(), 2) / hvariation->Integral();
		hmirror->Scale(mirror_norm / hvariation->Integral());
	}

	return;
}

//Get mirror histo, as done by ttH2017 :  https://github.com/CERN-PH-CMG/cmgtools-lite/blob/759bdc4213c50db48cb695ae498f7a97794a1410/TTHAnalysis/python/plotter/uncertaintyFile.py#L106
//Directly modify the hdown histo
void Get_TemplateSymm_Histogram(TH1F*& hcentral, TH1F*& hvariation, TH1F*& hmirror, bool is_ShapeOnly)
{
	hmirror = 0; //empty at beginning
	hmirror = (TH1F*) hcentral->Clone();

    if(hcentral->Integral() <= 0 || hvariation->Integral() <= 0) {return;}

	//Protection -- check if I artificially set the bin to a float ~null distribution
	//If that is the case, just do mirror = central
	if(hcentral->Integral() == hcentral->GetNbinsX() * hcentral->GetBinContent(1) || hvariation->Integral() == hvariation->GetNbinsX() * hvariation->GetBinContent(1))
	{
		return;
	}

	hmirror->Multiply(hmirror);
	hmirror->Divide(hvariation);

	// cout<<"hmirror->Integral() "<<hmirror->Integral()<<endl;

	return;
}

/**
 * In FCNC analysis, we want to reuse the closure shape systematics from ttH, even though they were derived in different regions
 * Thus we want to inflate these systematics by 20%. Can be done in datacard parser for norm syst. For shape syst, check the different (syst - nominal) in each bin, and inflate by 20%
 */
void Inflate_Syst_inShapeTemplate(TH1F*& h_toInflate, TH1F* h_nominal, float inflation_factor)
{
	if(!h_toInflate || !h_nominal || !h_toInflate->GetEntries() || !h_nominal->GetEntries()) {cout<<"Error in Inflate_Syst_inShapeTemplate() !"<<endl; return;}

	for(int ibin=1; ibin<h_toInflate->GetNbinsX()+1; ibin++)
	{
		//syst - nominal
		float diff_bin = h_toInflate->GetBinContent(ibin) - h_nominal->GetBinContent(ibin);

		h_toInflate->SetBinContent(ibin, h_toInflate->GetBinContent(ibin) + inflation_factor * diff_bin); //+- X%, depending on direction of variation
	}

	return;
}


//--------------------------------------------
//    ###    ##    ##    ###    ##       ##    ##  ######  ####  ######
//   ## ##   ###   ##   ## ##   ##        ##  ##  ##    ##  ##  ##    ##
//  ##   ##  ####  ##  ##   ##  ##         ####   ##        ##  ##
// ##     ## ## ## ## ##     ## ##          ##     ######   ##   ######
// ######### ##  #### ######### ##          ##          ##  ##        ##
// ##     ## ##   ### ##     ## ##          ##    ##    ##  ##  ##    ##
// ##     ## ##    ## ##     ## ########    ##     ######  ####  ######

//  ######  ########  ########  ######  #### ######## ####  ######
// ##    ## ##     ## ##       ##    ##  ##  ##        ##  ##    ##
// ##       ##     ## ##       ##        ##  ##        ##  ##
//  ######  ########  ######   ##        ##  ######    ##  ##
//       ## ##        ##       ##        ##  ##        ##  ##
// ##    ## ##        ##       ##    ##  ##  ##        ##  ##    ##
//  ######  ##        ########  ######  #### ##       ####  ######

// ######## ##     ## ##    ##  ######   ######
// ##       ##     ## ###   ## ##    ## ##    ##
// ##       ##     ## ####  ## ##       ##
// ######   ##     ## ## ## ## ##        ######
// ##       ##     ## ##  #### ##             ##
// ##       ##     ## ##   ### ##    ## ##    ##
// ##        #######  ##    ##  ######   ######
//--------------------------------------------

//Modifies arguments passed by reference according to command args
bool Apply_CommandArgs_Choices(int argc, char **argv, vector<TString>& v_lumiYear, TString& region_choice, TString& template_name)
{
	if(argc >= 2)
	{
        TString arg1 = (TString) argv[1]; arg1.ToLower();

        if(arg1 == "2016") {v_lumiYear.resize(0); v_lumiYear.push_back("2016");}
        else if(arg1 == "2017") {v_lumiYear.resize(0); v_lumiYear.push_back("2017");}
        else if(arg1 == "2018") {v_lumiYear.resize(0); v_lumiYear.push_back("2018");}
        else if(arg1 == "201617") {v_lumiYear.resize(0); v_lumiYear.push_back("2016"); v_lumiYear.push_back("2017");}
        else if(arg1 == "201618") {v_lumiYear.resize(0); v_lumiYear.push_back("2016"); v_lumiYear.push_back("2018");}
        else if(arg1 == "201718") {v_lumiYear.resize(0); v_lumiYear.push_back("2017"); v_lumiYear.push_back("2018");}
        else if(arg1 == "run2") {v_lumiYear.resize(0); v_lumiYear.push_back("2016"); v_lumiYear.push_back("2017"); v_lumiYear.push_back("2018");}

        else if(arg1 == "sr_highvpt") {region_choice = "SR_HighVPt";}
        else if(arg1 == "sr_lowvpt") {region_choice = "SR_LowVPt";}
        else if(arg1 == "cree_highvpt") {region_choice = "CRee_HighVPt";}
        else if(arg1 == "cree_lowvpt") {region_choice = "CRee_LowVPt";}
        else if(arg1 == "crmm_highvpt") {region_choice = "CRmm_HighVPt";}
        else if(arg1 == "crmm_lowvpt") {region_choice = "CRmm_LowVPt";}

        else if(arg1.Contains("nn") || arg1=="Zpt") {template_name = (TString) argv[1];}

        else
        {
            cout<<BOLD(FRED("Error : wrong arguments at execution !"))<<endl;
            cout<<"argc = "<<argc<<endl;
            cout<<"argv[1] = '"<<argv[1]<<"'"<<endl;
            if(argc >= 3) {cout<<"argv[2] = '"<<argv[2]<<"'"<<endl;}
            cout<<UNDL("--> Syntax : ./analysis_main.exe [lumi] [region]")<<endl;
            cout<<UNDL("(Please check conventions for 'lumi' arg. in code)")<<endl;

            return 0;
        }

		if(argc >= 3)
		{
            TString arg2 = (TString) argv[2]; arg2.ToLower();

            if(arg2 == "2016") {v_lumiYear.resize(0); v_lumiYear.push_back("2016");}
            else if(arg2 == "2017") {v_lumiYear.resize(0); v_lumiYear.push_back("2017");}
            else if(arg2 == "2018") {v_lumiYear.resize(0); v_lumiYear.push_back("2018");}
            else if(arg2 == "201617") {v_lumiYear.resize(0); v_lumiYear.push_back("2016"); v_lumiYear.push_back("2017");}
            else if(arg2 == "201618") {v_lumiYear.resize(0); v_lumiYear.push_back("2016"); v_lumiYear.push_back("2018");}
            else if(arg2 == "201718") {v_lumiYear.resize(0); v_lumiYear.push_back("2017"); v_lumiYear.push_back("2018");}
            else if(arg2 == "run2") {v_lumiYear.resize(0); v_lumiYear.push_back("2016"); v_lumiYear.push_back("2017"); v_lumiYear.push_back("2018");}

            else if(arg2 == "sr_highvpt") {region_choice = "SR_HighVPt";}
            else if(arg2 == "sr_lowvpt") {region_choice = "SR_LowVPt";}
            else if(arg2 == "cree_highvpt") {region_choice = "CRee_HighVPt";}
            else if(arg2 == "cree_lowvpt") {region_choice = "CRee_LowVPt";}
            else if(arg2 == "crmm_highvpt") {region_choice = "CRmm_HighVPt";}
            else if(arg2 == "crmm_lowvpt") {region_choice = "CRmm_LowVPt";}

            else if(arg2.Contains("nn") || arg2=="Zpt") {template_name = (TString) argv[2];}

			else
			{
				cout<<BOLD(FRED("Error : wrong arguments at execution !"))<<endl;
				cout<<"argc = "<<argc<<endl;
				cout<<"argv[1] = '"<<argv[1]<<"'"<<endl;
				cout<<"argv[2] = '"<<argv[2]<<"'"<<endl;
                cout<<UNDL("--> Syntax : ./analysis_main.exe [lumi] [region]")<<endl;
                cout<<UNDL("(Please check conventions for 'lumi' arg. in code)")<<endl;

				return 0;
			}
		}
	}

	return 1;
}

//Select a color for each activated sample
//NB : also account for the DATA sample in color vector, but keep its element empty !
void Get_Samples_Colors(vector<int>& v_colors, std::vector<TColor*>& v_custom_colors, vector<TString> v_samples, vector<TString> v_groups, int color_scheme)
{
    //Define some TColors, which we can then use to define the default color vector. NB : must only delete them after all plots have been created.
    v_custom_colors.resize(25, 0);

    //-- Colors definition : New color index + RGB values (from 0 to 1)
    v_custom_colors[0] = new TColor(9000, 31./255., 120./255., 180./255.); //Dark blue
    v_custom_colors[1] = new TColor(9001, 166./255., 206./255., 227./255.); //Light blue
    v_custom_colors[2] = new TColor(9002, 51./255., 160./255., 44./255.); //Dark green 2
    v_custom_colors[3] = new TColor(9003, 178./255., 223./255., 138./255.); //Light green
    v_custom_colors[4] = new TColor(9004, 255./255., 129./255., 123./255.); //Salmon
    v_custom_colors[5] = new TColor(9005, 53./255., 184./255., 99./255.); //Mild green
    v_custom_colors[6] = new TColor(9006, 227./255., 65./255., 61./255.); //Dark red
    v_custom_colors[7] = new TColor(9007, 255./255., 210./255., 47./255.); //Mild yellow
    v_custom_colors[8] = new TColor(9008, 152./255., 78./255., 163./255.); //Dark purple
    v_custom_colors[9] = new TColor(9009, 226./255., 125./255., 96./255.); //Soft red / terracotta
    v_custom_colors[10] = new TColor(9010, 232./255., 168./255., 124./255.); //Soft orange / salmon / tacao
    v_custom_colors[11] = new TColor(9011, 195./255., 141./255., 158./255.); //Desaturated pink / oriental pink
    v_custom_colors[12] = new TColor(9012, 5./255., 56./255., 107./255.); //Very dark blue / catalina blue
    v_custom_colors[13] = new TColor(9013, 85./255., 122./255., 149./255.); //Desaturated dark blue / smalt blue
    v_custom_colors[14] = new TColor(9014, 252./255., 205./255., 4./255.); //Vivid yellow / supernova
    v_custom_colors[15] = new TColor(9015, 134./255., 179./255., 279./255.); //Desaturated blue / Half baked
    v_custom_colors[16] = new TColor(9016, 252./255., 225./255., 129./255.); //Soft yellow / sweet corn
    v_custom_colors[17] = new TColor(9017, 133./255., 220./255., 186./255.); //Lime green / algae green
    v_custom_colors[18] = new TColor(9018, 204./255., 205./255., 198./255.); //Soft gray

    //-- Also nice: kOrange-2, ...

//--------------------------------------------
    if(color_scheme == 0) //Custom color scheme
    {
        for(int isample=0; isample<v_groups.size(); isample++)
		{
            if(v_groups[isample] == "VBFgamma") {v_colors[isample] = v_custom_colors[6]->GetNumber();}

            else if(v_groups[isample].BeginsWith("GJets")) {v_colors[isample] = v_custom_colors[11]->GetNumber();}

            else if(v_groups[isample] == "DiPhoton") {v_colors[isample] = v_custom_colors[3]->GetNumber();}

            else if(v_groups[isample] == "WJetsToLNu") {v_colors[isample] = v_custom_colors[0]->GetNumber();}
            else if(v_groups[isample] == "WJetsToQQ") {v_colors[isample] = v_custom_colors[0]->GetNumber();}
            else if(v_groups[isample] == "ttbar") {v_colors[isample] = v_custom_colors[0]->GetNumber();}
            else if(v_groups[isample] == "ttGJets") {v_colors[isample] = v_custom_colors[0]->GetNumber();}
            else if(v_groups[isample] == "ZGTo2LG") {v_colors[isample] = v_custom_colors[0]->GetNumber();}
            else if(v_groups[isample] == "WGToLNuG") {v_colors[isample] = v_custom_colors[0]->GetNumber();}

            else if(v_groups[isample] == "QCD") {v_colors[isample] = v_custom_colors[18]->GetNumber();}

            else if(v_groups[isample].Contains("DY")) {v_colors[isample] = v_custom_colors[16]->GetNumber();}

            else if(v_groups[isample] == "LLJJ") {v_colors[isample] = v_custom_colors[17]->GetNumber();}

            else {v_colors[isample] = isample;}
        }
    }

    else if(color_scheme == 2) //TESTING
    {
        for(int isample=0; isample<v_groups.size(); isample++)
        {
            v_colors[isample] = isample;
        }
    }

	return;
}


/*
//Use custom color palette
//-- Idea : take good-looking/efficient color palettes from web and apply it manually
void Set_Custom_ColorPalette(vector<TColor*> &v_custom_colors, vector<int> &v, vector<TString> v_sampleGroups)
{ TColor* col = new TColor(1700, 141./255., 211./255., 199./255.);
    // col.SetRGB(141./255., 211./255., 199./255.);
    // cout<<col->GetNumber()<<endl;

    // Int_t dark   = TColor::GetColorDark(color_index);
    // Int_t bright = TColor::GetColorBright(color_index);

    // Int_t ci = TColor::GetFreeColorIndex(); //returns a free color index which can be used to define a user custom color.
    // TColor *color = new TColor(ci, 0.1, 0.2, 0.3);

	v.resize(10);
	v_custom_colors.resize(10, 0);

	//-- Colors definition : Number + RGB value, from 0 to 1
    v_custom_colors[0] = 0;
    v_custom_colors[1] = new TColor(9002, 31./255., 120./255., 180./255.);
    v_custom_colors[2] = new TColor(9001, 166./255., 206./255., 227./255.);
    v_custom_colors[3] = new TColor(9004, 51./255., 160./255., 44./255.);
    v_custom_colors[4] = new TColor(9003, 178./255., 223./255., 138./255.);
    // v_custom_colors[5] = new TColor(9005, 251./255., 251./255., 153./255.);
	// v_custom_colors[6] = new TColor(9006, 227./255., 26./255., 28./255.);

    // int index_customColor = 0;
    // for(int isample=0; isample<v_sampleGroups.size(); isample++)
    // {
    //     if(isample >=1 && v_sampleGroups[isample] != v_sampleGroups[isample-1]) {index_customColor++;}
    //     if(v_custom_colors[index_customColor] != 0) {v[isample] = v_custom_colors[index_customColor]->GetNumber();}
    // }

    return;
}
*/

//Store here the binnings and ranges of all the variables to be plotted via ControlHistograms files
bool Get_Variable_Range(TString var, int& nbins, float& xmin, float& xmax)
{
    // {nbins = 20; xmin = -1; xmax = 1; return true;}

    //Categories are either 0 or 1 (NB : encoded in Char_t!)
    if(var.BeginsWith("is_") || var.BeginsWith("passed") ) {nbins = 2; xmin = 0; xmax = 2;}

    if(var == "vjj_v_pt") {nbins = 20; xmin = 0; xmax = 600;}
    else if(var == "vjj_v_eta") {nbins = 20; xmin = -2.4; xmax = 2.4;}
    else if(var == "vjj_v_ystar") {nbins = 20; xmin = -4; xmax = 4;}

    else if(var == "vjj_lead_pt") {nbins = 20; xmin = 0; xmax = 800;}
    else if(var == "vjj_lead_eta") {nbins = 20; xmin = -4; xmax = 4;}
    else if(var == "vjj_lead_m") {nbins = 20; xmin = 0; xmax = 80;}

    else if(var == "vjj_sublead_pt") {nbins = 20; xmin = 0; xmax = 400;}
    else if(var == "vjj_sublead_eta") {nbins = 20; xmin = -4; xmax = 4;}
    else if(var == "vjj_sublead_m") {nbins = 20; xmin = 0; xmax = 50;}

    else if(var == "vjj_jj_pt") {nbins = 20; xmin = 0; xmax = 600;}
    else if(var == "vjj_jj_eta") {nbins = 20; xmin = -4; xmax = 4;}
    else if(var == "vjj_jj_m") {nbins = 20; xmin = 200; xmax = 1600;}
    else if(var == "vjj_jj_scalarht") {nbins = 20; xmin = 100; xmax = 1100;}
    else if(var == "vjj_jj_deta") {nbins = 20; xmin = 0; xmax = 5;}
    else if(var == "vjj_jj_sumabseta") {nbins = 20; xmin = 0; xmax = 6;}

    else if(var == "vjj_vjj_pt") {nbins = 20; xmin = 0; xmax = 200;}
    else if(var == "vjj_vjj_eta") {nbins = 20; xmin = -7; xmax = 7;}
    else if(var == "vjj_vjj_m") {nbins = 20; xmin = 500; xmax = 2500;}
    else if(var == "vjj_vjj_scalarht") {nbins = 20; xmin = 300; xmax = 1500;}
    else if(var == "vjj_vjj_sphericity") {nbins = 20; xmin = 0; xmax = 600;}
    else if(var == "vjj_vjj_aplanarity") {nbins = 20; xmin = 0; xmax = 50;}
    else if(var == "vjj_vjj_isotropy") {nbins = 20; xmin = 0.2; xmax = 1.;}
    else if(var == "vjj_vjj_C") {nbins = 20; xmin = 0; xmax = 2000000;}
    else if(var == "vjj_vjj_D") {nbins = 20; xmin = 0; xmax = 20000000;}
    else if(var == "vjj_jj_dr2v") {nbins = 20; xmin = 2.; xmax = 5.;}

    else if(var == "vjj_newvaraplanarity") {nbins = 20; xmin = 0; xmax = 0.1;}
    else if(var == "vjj_newvarC") {nbins = 20; xmin = 0; xmax = 1;}
    else if(var == "vjj_newvarD") {nbins = 20; xmin = 0; xmax = 0.2;}
    else if(var == "vjj_newvarjj_vdphi") {nbins = 20; xmin = 2.5; xmax = 4;}
    else if(var == "vjj_newvarsublead_dphivj") {nbins = 20; xmin = 0; xmax = 6;}
    else if(var == "vjj_newvarlead_dphivj") {nbins = 20; xmin = 2.; xmax = 5;}
    else if(var == "vjj_newvarlead_detavj") {nbins = 20; xmin = 0; xmax = 4;}
    else if(var == "vjj_newvarsublead_detavj") {nbins = 20; xmin = 0; xmax = 4;}
    else if(var == "vjj_newvarvjj_deta") {nbins = 20; xmin = 0; xmax = 6;}
    else if(var == "vjj_newvarcircularity") {nbins = 20; xmin = 0; xmax = 1.;}
    else if(var == "vjj_newvarisotropy") {nbins = 20; xmin = 0.2; xmax = 1.;}
    else if(var == "vjj_newvarsphericity") {nbins = 20; xmin = 0; xmax = 0.8;}

    else if(var == "vjj_mva_NeginHigh2016BDT") {nbins = 20; xmin = -0.5; xmax = 0.5;}
    else if(var == "vjj_mva_NeginHigh2017BDT") {nbins = 20; xmin = -0.5; xmax = 0.5;}
    else if(var == "vjj_mva_NeginHigh2018BDT") {nbins = 20; xmin = -0.5; xmax = 0.5;}

    else if(var == "vjj_njets") {nbins = 8; xmin = 2; xmax = 10;}
    else if(var == "vjj_nextraj") {nbins = 7; xmin = 0; xmax = 7;}
    else if(var == "vjj_j_maxAbsEta") {nbins = 20; xmin = 0; xmax = 4.5;}
    else if(var == "vjj_j_minAbsEta") {nbins = 20; xmin = 0; xmax = 2.5;}
    else if(var == "vjj_centhtsoft" || var == "vjj_htsoft") {nbins = 20; xmin = 0; xmax = 100;}

    else if(var.BeginsWith("cos", TString::kIgnoreCase)) {nbins = 20; xmin = -1.; xmax = 1.;}
    else if(var.Contains("CSV", TString::kIgnoreCase)) {nbins = 20; xmin = 0.; xmax = 1.1;}
    else if(var.Contains("dR") || var.Contains("DelR", TString::kIgnoreCase) ) {nbins = 20; xmin = 0; xmax = 6.;}
    else if(var.Contains("deta", TString::kIgnoreCase) ) {nbins = 15; xmin = 0; xmax = 6.;}
    else if(var.Contains("eta", TString::kIgnoreCase) ) {nbins = 20; xmin = -3.; xmax = 3.;}
    else if(var.Contains("_phi", TString::kIgnoreCase) ) {nbins = 20; xmin = -3.; xmax = 3.;}
    else if(var.Contains("_dphi", TString::kIgnoreCase) ) {nbins = 20; xmin = -3.; xmax = 3.;}
    else if(var.Contains("_qgl", TString::kIgnoreCase) ) {nbins = 20; xmin = 0.; xmax = 1.;}
    else if(var.Contains("_dr2v", TString::kIgnoreCase) ) {nbins = 20; xmin = 1.; xmax = 6.;}
    else if(var.Contains("_isotropy", TString::kIgnoreCase) ) {nbins = 20; xmin = 0.; xmax = 1.;}
    else if(var.Contains("_circularity", TString::kIgnoreCase) ) {nbins = 20; xmin = 0.; xmax = 1.;}
    else if(var.Contains("vjj_mva_", TString::kIgnoreCase) ) {nbins = 20; xmin = -1.; xmax = 1.;}

    else if(var.Contains("_is") ) {nbins = 2; xmin = 0.; xmax = 2.;}

    else if(var == "njets") {nbins = 8; xmin = 0.; xmax = 8;}
    else if(var == "nbjets") {nbins = 6; xmin = 0.; xmax = 6;}

    // else if(var == "Lep3Pt") {nbins = 20; xmin = 0; xmax = 80;}

    else {nbins = 20; xmin = -5.; xmax = 300.;}
    // else {return false;}

	return true;
}

//Return the binning of the variable passed in arg
//NB: use Contains() function because I am passing full variable names as arguments (e.g. 'NN_SRtZq')
void Get_Template_Range(int& nbins, float& xmin, float& xmax, TString template_name, bool plot_onlyMaxNodeEvents, int& nbjets_min, int& nbjets_max, int& njets_min, int& njets_max, vector<float> minmax_bounds)
{
    nbins = 15; //Default

    xmin = -1; xmax = 1; //BDT: [-1,1]

    if(template_name.Contains("NN") || template_name.Contains("BDT")) {nbins = 15;} //MVA default

    if(template_name.Contains("NN")) //NN: [0,1]
    {
        xmin = 0;
        if(plot_onlyMaxNodeEvents) //Special case: only plot events in their max. node, then by construction there can be no events with x<1/3 --> Adapt axis
        {
            xmin = 0.3; nbins = 10; //Default
        }
    }
    else if(template_name.Contains("categ")) {nbins = (nbjets_max-nbjets_min+1)*(njets_max-njets_min+1); xmin = 0; xmax = nbins;} //1 bin per sub-category
    else if(template_name.Contains("ZptCos")) {nbins = 12; xmin = 0; xmax = 12;} //2D Zpt-cosThetaStarPolZ (as in TOP-18-009) //4bins in Zpt, 3 in cosTheta
    else if(template_name.Contains("Zpt")) //1D Zpt
    {
        nbins = 5;
        // nbins = 8;
        xmin = 0; xmax = 400;
    }

    if(template_name.Contains("mTW"))
    {
        nbins=15; xmin=0; xmax=150;
    }
    if(template_name.Contains("countExp")) {nbins=1; xmin=0; xmax=1;}
    if(template_name.Contains("channel")) {nbins=2; xmax=2;}

    //-- Debug printouts
    // cout<<"template_name "<<template_name<<endl;
    // cout<<"nbins "<<nbins<<endl;
    // cout<<"xmin "<<xmin<<endl;
    // cout<<"xmax "<<xmax<<endl;

    return;
}

//Return beautified variable name
TString Get_Variable_Name(TString var)
{
    // l -> italic l, different font
    // add [GeV] units for some vars ?
    // 'l' for LateX curvy l ; but belongs to TMathText, and not compatible with TLatex '#' chars?
    //'#leftrightarrow' : <->
    //Big delimiters : #leftX, #rightX, with X=|,(,{,[ (and opposite for right)

    if(var == "maxDijetDelR") {return "max. #DeltaR(j,j)";}
    if(var == "dEtaFwdJetBJet") {return "#Delta#eta#left(j^{fwd},b#right)";}
    if(var == "dEtaFwdJetClosestLep") {return "#Delta#eta#left(j^{fwd},l^{closest}#right)";}
    if(var == "dR_jprimeClosestLep") {return "#DeltaR#left(j',l^{closest}#right)";}
    if(var == "dR_tClosestLep") {return "#DeltaR#left(t,l^{closest}#right)";}
    if(var == "dEta_bjprime") {return "#Delta#eta#left(j',b#right)";}
    if(var == "dEta_lWjprime") {return "#Delta#eta#left(j',l_{W}#right)";}
    if(var == "dEta_tjprime") {return "#Delta#eta#left(j',t#right)";}
    if(var == "dEta_Zjprime") {return "#Delta#eta#left(j',Z#right)";}
    if(var == "dR_bjprime") {return "#DeltaR#left(j',b#right)";}
    if(var == "dR_lWjprime") {return "#DeltaR#left(j',l_{W}#right)";}
    if(var == "dR_Zjprime") {return "#DeltaR#left(j',Z#right)";}
    if(var == "dR_blW") {return "#DeltaR#left(b,l_{W}#right)";}
    if(var == "dR_tZ") {return "#DeltaR#left(t,Z#right)";}
    if(var == "dR_ZlW") {return "#DeltaR#left(Z,l_{W}#right)";}
    if(var == "mHT") {return "m_{HT}";}
    if(var == "mTW") {return "m_{T}^{W}";}
    if(var == "Mass_3l") {return "m_{3l}";}
    if(var == "forwardJetAbsEta") {return "#left|#eta#left(j^{fwd}#right)#right|";}
    if(var == "jPrimeAbsEta") {return "#left|#eta(j')#right|";}
    if(var == "jprime_Pt") {return "p_{T}(j')";}
    if(var == "maxDeepCSV") {return "max. DeepCSV";}
    if(var == "deepCSV_2nd") {return "2nd max. DeepCSV";}
    if(var == "maxDeepJet") {return "max. DeepJet score";}
    if(var == "deepJet_2nd") {return "2nd max. DeepJet";}
    if(var == "delRljPrime") {return "#DeltaR(j',l)";}
    if(var == "lAsymmetry") {return "q_{l} #upoint #left|#eta(l)#right|";}
    if(var == "maxDijetMass" || var == "maxDiJet_M") {return "max. m_{j,j}";}
    if(var == "mbjMax") {return "max. m_{b,j}";}
    if(var == "maxDelPhiLL") {return "max. #Delta#varphi(l,l)";}
    if(var == "recoZ_dPhill") {return "#Delta#varphi(l,l)_{Z}";}
    if(var == "metEt") {return "p_{T}^{miss}";} //"E_{T}^{miss}"
    if(var == "recoZ_Mass") {return "m_{Z}";}
    if(var == "Mass_tZ") {return "m_{tZ}";}
    if(var == "maxDijetPt" || var == "maxDiJet_Pt") {return "max. p_{T}(j,j)";}
    if(var == "maxDijetDelPhi") {return "max #Delta#varphi(j,j)";}
    if(var == "maxEtaJet") {return "max. #left|#eta(j)#right|";}
    if(var == "minDelRbL") {return "min. #DeltaR(b,l)";}
    if(var == "Top_delRbl") {return "#DeltaR#left(l^{t},b^{t}#right)";}
    if(var == "Top_delRbW") {return "#DeltaR#left(W^{t},b^{t}#right)";}
    if(var == "cosThetaStarPolTop") {return "cos#left(#theta^{*}_{top}#right)";}
    if(var == "cosThetaStarPolZ") {return "cos#left(#theta^{*}_{Z}#right)";}
    if(var == "njets") {return "jet multiplicity";}
    if(var == "nbjets") {return "b jet multiplicity";}
    if(var == "recoZ_Pt") {return "p_{T}(Z)";}
    if(var == "recoZ_Eta") {return "#eta(Z)";}
    if(var == "recoLepTop_Pt") {return "p_{T}(t)";}
    if(var == "recoLepTop_Eta") {return "#eta(t)";}
    if(var == "recoLepTopLep_Pt") {return "p_{T}(l_{W})";}
    if(var == "recoLepTopLep_Eta") {return "#eta(l_{W})";}
    if(var == "channel") {return "Leptonic channel";}

    if(var == "jet1_pt") {return "p_{T}(leading jet)";}
    if(var == "jet2_pt") {return "p_{T}(second-leading jet)";}
    if(var == "jet3_pt") {return "p_{T}(third-leading jet)";}
    if(var == "lep1_pt") {return "p_{T}(leading lepton)";}
    if(var == "lep2_pt") {return "p_{T}(second-leading lepton)";}
    if(var == "lep3_pt") {return "p_{T}(third-leading lepton)";}

    //GenPlotter variables
    if(var == "Z_pt" || var == "recoZ_Pt") {return "p_{T}(Z)";}
    if(var == "Z_eta" || var == "recoZ_Eta") {return "#eta(Z)";}
    if(var == "Z_m" || var == "recoZ_Eta") {return "m(Z)";}
    if(var == "Zreco_m" || var == "recoZ_Eta") {return "m(Z_{reco})";}
    if(var == "Top_pt") {return "p_{T}(t)";}
    if(var == "Top_eta") {return "#eta(t)";}
    if(var == "Top_m") {return "m(t)";}
    if(var == "TopZsystem_M" || var == "TopZsystem_m") {return "m(tZ)";}
    if(var == "LeadingTop_pt") {return "p_{T}(t^{lead})";}
    if(var == "LeadingTop_eta") {return "#eta(t^{lead})";}
    if(var == "Zreco_dPhill") {return "#Delta#varphi_{ll}";}
    if(var == "cosThetaStarPol_Top") {return "cos#left(#theta^{*}_{top}#right)";}
    if(var == "cosThetaStarPol_Z") {return "cos#left(#theta^{*}_{Z}#right)";}

    return var;
}

//Get var name corresponding to category
//isFake <-> use different flags for prompt/fake events
//NB: always call this function to automate cuts on flags, since they differ for NPL sample, etc.
TString Get_Category_Boolean_Name(TString region, bool isFake)
{
    TString name = "";

    region.ToLower();

    // if(region=="tZq") {return "is_tZq_3l_SR";}
    // else if(region=="ttZ") {return "is_ttZ_3l_SR";}
    // else if(region=="tWZ") {return "is_tWZ_3l_SR";}

    if(region=="signal") {name = "is_signal_SR";}
    else if(region=="tzq") {name = "is_tzq_SR";}
    else if(region=="ttz") {name = "is_ttz_SR";}
    else if(region=="ttz4l") {name = "is_ttz4l_SR";}

    else if(region=="xg") {name = "is_xg_CR";}
    else if(region=="zz") {name = "is_zz_CR";}
    else if(region=="tx") {name = "is_tX_CR";}
    else if(region=="tt") {name = "is_tt_CR";}
    else if(region=="wz") {name = "is_wz_CR";}
    else if(region=="dy") {name = "is_dy_CR";}

    else {cout<<"Warning: empty category name..."<<endl;}

    if(isFake && name != "") {name+= "Fake";}

    return name;
}

//Computes total nof entries which will be processed by the Produce_Templates() function, so that the Timebar is correct
//NB1 : don't account for nof syst weights, since they are computed all at once when reading an event
//Obsolete -- NB2 : this returns the nof entries which will get read, not processed (else, should take cuts into account, etc.)
float Count_Total_Nof_Entries(TString dir_ntuples, TString t_name_nominal, vector<TString> v_samples, vector<TString> v_systTrees, vector<TString> v_cut_name, vector<TString> v_cut_def, vector<TString> v_lumiYears, bool makeHisto_inputVars, bool noSysts_inputVars)
{
    float total_nentries = 0;

    if(makeHisto_inputVars && noSysts_inputVars) //syst weights not computed
    {
        v_systTrees.resize(1);
    }

    //--- Define the cut chain to apply to events
    TString cuts_chain = "";
    /*
    for(int ivar=0; ivar<v_cut_name.size(); ivar++)
    {
        if(v_cut_def[ivar] != "")
        {
            if(cuts_chain != "") {cuts_chain+= " && ";}

            if(!v_cut_def[ivar].Contains("&&") && !v_cut_def[ivar].Contains("||")) {cuts_chain+= v_cut_name[ivar] + v_cut_def[ivar];} //If cut contains only 1 condition
            else if(v_cut_def[ivar].Contains("&&") && v_cut_def[ivar].Contains("||")) {cout<<BOLD(FRED("ERROR ! Wrong cut definition !"))<<endl;}
            else if(v_cut_def[ivar].Contains("&&") )//If '&&' in the cut, break it in 2
            {
                cuts_chain+= v_cut_name[ivar] + Break_Cuts_In_Two(v_cut_def[ivar]).first;
                cuts_chain+= " && ";
                cuts_chain+= v_cut_name[ivar] + Break_Cuts_In_Two(v_cut_def[ivar]).second;
            }
            else if(v_cut_def[ivar].Contains("||") )//If '||' in the cut, break it in 2
            {
                cuts_chain+= v_cut_name[ivar] + Break_Cuts_In_Two(v_cut_def[ivar]).first;
                cuts_chain+= " || ";
                cuts_chain+= v_cut_name[ivar] + Break_Cuts_In_Two(v_cut_def[ivar]).second;
            }
        }
    }*/

    for(int iyear=0; iyear<v_lumiYears.size(); iyear++)
    {
        TString dir_tmp = dir_ntuples + v_lumiYears[iyear] + "/";

        for(int isample=0; isample<v_samples.size(); isample++)
        {
            // cout<<"v_samples[isample] "<<v_samples[isample]<<endl;

            //Open input TFile
            TString inputfile = dir_tmp + v_samples[isample] + ".root";
            // cout<<"inputfile "<<inputfile<<endl;
            if(!Check_File_Existence(inputfile))
            {
                // cout<<endl<<"File "<<inputfile<<FRED(" not found!")<<endl;
                continue;
            }
            TFile* file_input = TFile::Open(inputfile, "READ");

            for(int itree=0; itree<v_systTrees.size(); itree++)
    		{
                TTree* tree = NULL;
                TString tmp = v_systTrees[itree];
    			if(tmp == "") {tmp = t_name_nominal;}
                tree = (TTree*) file_input->Get(tmp);
    			if(!tree)
    			{
    				// cout<<BOLD(FRED("ERROR : tree '"<<tmp<<"' not found in file : "<<inputfile<<" ! Skip !"))<<endl;
    				continue; //Skip sample
    			}

                // float nentries_tmp = tree->GetEntries("passedBJets");
                float nentries_tmp = tree->GetEntries(cuts_chain);

                total_nentries+= nentries_tmp; //Multiply nof entries by nof loops

                // cout<<"total_nentries = "<<total_nentries<<endl;
            } //trees
        } //samples
    } //years

    // cout<<"total_nentries "<<total_nentries<<endl;

    return total_nentries;
}

//-- For systs uncorrelated between years, need to add the year in the systName (to make it unique)
//-- NB: for first arg. can pass the concatenation of 'syst_list[isyst]+systTree_list[itree]' because only 1 can be non-empty at a time !
//-- NB: could also use the 'nuisance edit rename process channel oldname newname' directive directly in Combine datacards
TString Get_Modified_SystName(TString systname, TString lumiYear, TString samplename)
{
    TString samplename_tmp = samplename; //Don't distinguish central/private signal samples
    if(samplename_tmp.Contains("tZq")) {samplename_tmp = "tZq";}
    else if(samplename_tmp.Contains("ttZ")) {samplename_tmp = "ttZ";}
    else if(samplename_tmp.Contains("tWZ")) {samplename_tmp = "tWZ";}

    //-- Only list the systematics which are *not* to be correlated in-between years
    //--> Need to add year as suffix (<-> unique)
    if(systname.BeginsWith("BtagHFstats")
    || systname.BeginsWith("BtagLFstats")

    || systname.BeginsWith("JER")
    || systname.BeginsWith("MET")
    || systname.BeginsWith("JES")
    || systname.BeginsWith("AbsoluteStat")
    || systname.BeginsWith("TimePtEta")
    || systname.BeginsWith("RelativeJEREC1")
    || systname.BeginsWith("RelativeJEREC2")
    || systname.BeginsWith("RelativePtEC1")
    || systname.BeginsWith("RelativePtEC2")
    || systname.BeginsWith("RelativeSample")
    || systname.BeginsWith("RelativeStatEC")
    || systname.BeginsWith("RelativeStatHF")
    || systname.BeginsWith("RelativeStatFSR")
    )
    {
        if(systname.EndsWith("Up"))
        {
            int i = systname.Index("Up"); //Find position of suffix
            systname.Remove(i); //Remove suffix
            systname+= lumiYear+"Up"; //Add year + suffix
        }
        else if(systname.EndsWith("Down"))
        {
            int i = systname.Index("Down"); //Find position of suffix
            systname.Remove(i); //Remove suffix
            systname+= lumiYear+"Down"; //Add year + suffix
        }
    }
    //-- Only list the systematics which are *not* to be correlated in-between samples
    //Add sample name as suffix <-> unique
    else if(systname.BeginsWith("ISR")
    || systname.BeginsWith("ME"))
    {
        if(systname.EndsWith("Up"))
        {
            int i = systname.Index("Up"); //Find position of suffix
            systname.Remove(i); //Remove suffix
            systname+= samplename_tmp+"Up"; //Add year + suffix
        }
        else if(systname.EndsWith("Down"))
        {
            int i = systname.Index("Down"); //Find position of suffix
            systname.Remove(i); //Remove suffix
            systname+= samplename_tmp+"Down"; //Add year + suffix
        }
    }

    return systname;
}

//Get the address of the relevant GEN-level histogram, so that it can be superimposed on data/MC comparison plots
void Get_Pointer_GENHisto(TH1F*& h, TString variable)
{
    TString input_file_name = "../Pheno/Plotter/GENhisto.root";
    if(!Check_File_Existence(input_file_name)) {cout<<BOLD(FRED("Input histo file "<<input_file_name<<" not found ! Can't superimpose GEN-level EFT histo..."))<<endl; return;}

    TFile* f = TFile::Open(input_file_name);

    TString hname = "Z_pt_tzq_sm"; //-- hardcoded for testing
    if(!f->GetListOfKeys()->Contains(hname) ) {cout<<ITAL("Histogram '"<<hname<<"' : not found...")<<endl; return;}
    h = (TH1F*) f->Get(hname);

    return;
}

//Get the arbitrary x-axis value corresponding to the jet and bjet multiplicities of the event
//Logic: check if njets<=min, >=max, or ==x for any other multiplicity in-between to determine event bin
float Get_x_jetCategory(float njets, float nbjets, int nbjets_min, int nbjets_max, int njets_min, int njets_max)
{
    // int nbjets_min = 1, nbjets_max = 2;
    // int njets_min = 2, njets_max = 6;

    int bin = 0; //will return bin x-axis value, assuming fix width of 1

    bool stop_loop = false;

    {
        for(int ibjet=nbjets_min; ibjet<=nbjets_max; ibjet++) //Loop on possible nbjets values
        {
            if(stop_loop) {break;} //Bin found

            for(int ijet=njets_min; ijet<=njets_max; ijet++) //Loop on possible njets values
            {
                if(nbjets==ibjet || (nbjets<=nbjets_min && ibjet==nbjets_min) || (nbjets>=nbjets_max && ibjet==nbjets_max)) //nbjets categ found
                {
                    if(njets==ijet || (njets<=njets_min && ijet==njets_min) || (njets>=njets_max && ijet==njets_max)) //njets categ found
                    {
                        stop_loop = true; break; //Bin found
                    }
                }

                bin++; //Bin not found, increment
            }
        }
    }
    if(!stop_loop) {cout<<BOLD(FRED("Warning: jet category not found ! Return -1"))<<endl; return -1.;} //Category not found

    // cout<<"njets "<<njets<<" / nbjets "<<nbjets<<" --> categ "<<bin + 0.5<<endl;

    return bin + 0.5; //Assume bin width of 1. --> add event in center of bin
}

//Get x center position of bin corresponding to the Zpt and cosThetaStarPolZ values given in arg (following TOP-18-009 conventions)
float Get_x_ZptCosCategory(float Zpt, float cosThetaStarPolZ)
{
    if(Zpt < 100)
    {
        if(cosThetaStarPolZ < -0.6) {return 1-0.5;} //Corresponds to SR1 --> Fill at 0.5 (first bin)
        else if(cosThetaStarPolZ < 0.6) {return 2-0.5;}
        else {return 3-0.5;}
    }
    else if(Zpt < 200)
    {
        if(cosThetaStarPolZ < -0.6) {return 4-0.5;} //Corresponds to SR1 --> Fill at 0.5 (first bin)
        else if(cosThetaStarPolZ < 0.6) {return 5-0.5;}
        else {return 6-0.5;}
    }
    else if(Zpt < 400)
    {
        if(cosThetaStarPolZ < -0.6) {return 7-0.5;} //Corresponds to SR1 --> Fill at 0.5 (first bin)
        else if(cosThetaStarPolZ < 0.6) {return 8-0.5;}
        else {return 9-0.5;}
    }
    else
    {
        if(cosThetaStarPolZ < -0.6) {return 10-0.5;} //Corresponds to SR1 --> Fill at 0.5 (first bin)
        else if(cosThetaStarPolZ < 0.6) {return 11-0.5;}
        else {return 12-0.5;}
    }

    cout<<FRED("ERROR: relevant bin not found in Get_x_ZptCosCategory() ! Zpt = "<<Zpt<<" / cosThetaStarPolZ = "<<cosThetaStarPolZ<<"")<<endl;

    return -1;
}

//Get the path of the relevant MVA input file (.xml for BDT, .pb for NN), depending on specific analysis options. Intended for use in Produce_Templates() function
TString Get_MVAFile_InputPath(TString MVA_type, TString year, bool use_specificMVA_eachYear, bool load_NN_info)
{
    TString fullpath = ""; //Full input path
    TString basepath = "./weightsMVA/" + MVA_type + "/SM/"; //Base path
    TString path_suffix = ""; //Suffix to append to base path; in-between the 2 we also need the year information, which we may want to modify if the nominal file is not found (see below)

    if(load_NN_info) {path_suffix+= "NN_info.txt";} //Read NN info file (input features, etc.)
    else {path_suffix+= "model.pb";} //Load NN model
    if(use_specificMVA_eachYear)

    {
        fullpath = basepath + year + "/" + path_suffix;
        cout<<DIM("Trying to open MVA file "<<fullpath<<" ... ");

        if(!Check_File_Existence(fullpath)) //If did not find year-specific file, also try to look for full Run 2 file
        {
            fullpath = basepath + "Run2/" + path_suffix; //Try to read Run2 file (i.e. not year-specific) file instead
            if(year == "Run2" || !Check_File_Existence(fullpath)) {cout<<BOLD(FRED("ERROR: no MVA input file found !"))<<endl; return "";}
            else {cout<<DIM("NOT FOUND !")<<endl; cout<<FBLU("--> Using file ")<<fullpath<<FBLU(" instead !")<<endl; usleep(3000000);}
        }
        else {cout<<DIM("FOUND !")<<endl;}
    }
    else
    {
        fullpath = basepath + "Run2/" + path_suffix;
        cout<<DIM("Trying to open MVA file "<<fullpath<<" ... ");
        if(Check_File_Existence(fullpath)) {cout<<DIM("FOUND !")<<endl;}
        else {cout<<BOLD(FRED("ERROR: no MVA input file found !"))<<endl; return "";}
    }

    // cout<<"fullpath "<<fullpath<<endl;
    return fullpath;
}

//Get the path of the file containing the relevant histograms (either templates or input variables), depending on specific analysis options. Intended for use in Draw_Templates() function
TString Get_HistoFile_InputPath(bool is_templateFile, TString template_type, TString region, TString year, bool use_CombineFile, TString filename_suffix, int categorization_strategy, bool combineFile_fromHarvester, bool prefit, bool printout)
{
    // bool printout = true; //true <-> display printouts

    TString fullpath = ""; //Full input path
    if(region != "") {region = "_" + region;}

	if(use_CombineFile) //Reading a template file produced by Combine (specific naming conventions)
	{
        if(combineFile_fromHarvester) //File produced by CombineHarvester, different naming
        {
            fullpath = "./outputs/dir_shapes_tmp/shapes_";
            // fullpath = "./outputs/shapes_";
            // fullpath+= prefit? "prefit_":"postfit_";
            fullpath+= "postfit_"; //always use postfit files ?
            fullpath+= "datacard_";
            fullpath+= template_type + ".root";

            if(!is_templateFile) //Control plots, hardcoded name
            {
                fullpath = "./outputs/dir_shapes_tmp/shapes_";
                // fullpath = "./outputs/shapes_";
                fullpath+= prefit? "prefit_":"postfit_";
                fullpath+= "control.root";
            }

            if(printout) {cout<<DIM("Trying to open input file "<<fullpath<<" ... ");}
            if(Check_File_Existence(fullpath))
            {
                if(printout) {cout<<DIM("FOUND !")<<endl;}
                return fullpath;
            }
            else
            {
                fullpath = "./outputs/shapes.root"; //Try a generic name

                if(printout) {cout<<endl<<DIM("Trying to open input file "<<fullpath<<" ... ");}
                if(Check_File_Existence(fullpath))
                {
                    if(printout) {cout<<DIM("FOUND !")<<endl;}
                    return fullpath;
                }
                else {cout<<BOLD(FRED("ERROR: shapes.root file from CombineHarvester not found !"))<<endl; return "";}
            }
        }
        else //File produced by FitDiagnostics
        {
            fullpath = "./outputs/fitDiagnostics_";
            fullpath+= template_type + region + filename_suffix + ".root";

            if(printout) {cout<<DIM("Trying to open input file "<<fullpath<<" ... ");}
            if(Check_File_Existence(fullpath))
            {
                if(printout) {cout<<DIM("FOUND !")<<endl;}
                return fullpath;
            }
            else
            {
                fullpath = "./outputs/fitDiagnostics.root"; //Try a generic name

                if(printout) {cout<<endl<<DIM("Trying to open input file "<<fullpath<<" ... ");}
                if(Check_File_Existence(fullpath))
                {
                    if(printout) {cout<<DIM("FOUND !")<<endl;}
                    return fullpath;
                }
                else {cout<<BOLD(FRED("ERROR: fitDiagnostics file from Combine not found !"))<<endl; return "";}
            }
        }
    }
	else //Reading my own file
	{
        TString region_tmp = "";

		if(!is_templateFile) {fullpath = "outputs/ControlHistograms" + region + "_" + year + filename_suffix + ".root";} //Input variables
		else {fullpath = "outputs/Templates_" + template_type + region_tmp + "_" + year + filename_suffix + ".root";} //Templates

        cout<<DIM("Trying to open input file "<<fullpath<<" ... ");
        if(Check_File_Existence(fullpath)) {cout<<DIM("FOUND !")<<endl; return fullpath;}
        else
        {
            cout<<BOLD(FRED("ERROR: file "<<fullpath<<" not found !"))<<endl;
            if(year != "Run2") //Retry with Run2 file
            {
                if(!is_templateFile) {fullpath = "outputs/ControlHistograms" + region + "_Run2" + filename_suffix + ".root";} //Input variables
                else {fullpath = "outputs/Templates_" + template_type + region_tmp + "_Run2" + filename_suffix + ".root";} //Templates
                cout<<DIM("Trying to open input file "<<fullpath<<" ... ");
                if(Check_File_Existence(fullpath)) {cout<<DIM("FOUND !")<<endl; return fullpath;}
                else
                {
                    cout<<BOLD(FRED("ERROR: file "<<fullpath<<" not found ! Can not plot any histogram !"))<<endl;
                    return "";
                }
            }
            else {return "";}
        }
    }

    // cout<<"fullpath "<<fullpath<<endl;
    return fullpath;
}

//Read a NN info file and fill values via argument
bool Extract_Values_From_NNInfoFile(TString NNinfo_input_path, vector<TString>& var_list_NN, vector<TString>& v_NN_nodeLabels, TString& NN_inputLayerName, TString& NN_outputLayerName, int& NN_iMaxNode, int& NN_nNodes, vector<float>& minmax_bounds)
{
    if(!Check_File_Existence(NNinfo_input_path) ) {cout<<endl<<FRED("File "<<NNinfo_input_path<<" not found!")<<endl; return false;}

    cout<<DIM("Retrieving MVA information from : "<<NNinfo_input_path<<"")<<endl;
    ifstream file_in(NNinfo_input_path);
    string line;
    while(!file_in.eof())
    {
        getline(file_in, line);
        // TString ts_line(line);
        stringstream ss(line);
        TString varname; float tmp1, tmp2; //Values tmp1 and tmp2 could be the mean and variance, or min and max, etc. depending on the rescaling
        ss >> varname >> tmp1 >> tmp2;
        // cout<<" varname "<<varname<<" tmp1 "<<tmp1<<" tmp2 "<<tmp2<<endl;
        if(varname != "") //Last line may be empty
        {
            if(tmp1 == -1 && tmp2 == -1) {NN_inputLayerName = varname;} //Name of input layer
            else if(tmp1 == -2 && tmp2 == -2) {NN_outputLayerName = varname;} //Name of output layer
            else if(tmp1 == -3 && tmp2 == -3) {NN_nNodes = Convert_TString_To_Number(varname);} //Number of output nodes
            else if(tmp1 == -4 && tmp2 == -4) {v_NN_nodeLabels.push_back(varname);} //Label(s) of the node(s), e.g. 'tZq'/'ttZ'/'Backgrounds'
            else if(varname=="bounds")
            {
                minmax_bounds.push_back(tmp1); minmax_bounds.push_back(tmp2);
            }
            // else if(varname=="bounds") {minmax_bounds = std::make_pair(tmp1, tmp2);}
            else if(tmp1 != -5 && tmp2 != -5) //Input variables
            {
                var_list_NN.push_back(varname);
                // std::pair <float,float> pair_tmp = std::make_pair(tmp1, tmp2);
            }
        }
    }
    // cout<<DIM("-->  "<<NN_inputLayerName<<"")<<endl;
    // cout<<DIM("-->  "<<NN_outputLayerName<<"")<<endl;
    // cout<<DIM("-->  "<<NN_nNodes<<" nodes")<<endl;

    if(NN_inputLayerName == "" || NN_outputLayerName == "" || NN_nNodes == -1 || !minmax_bounds.size()) {cout<<endl<<FRED("ERROR : NN input/output info not found !")<<endl; return false;} //Need this info for NN

    return true;
}

TString Get_Region_Label(TString region, TString variable)
{
    TString label = "";

    //-- Region-dependent label //Order matters
    // region.ToLower();
    // if(region=="signal" || region=="sr") {label = "\\text{SR-3}\\ell";} //Main SR

    if(region.Contains("SR")) {label = "#splitline{SR}";} //NB: if using a TMathText (like \ell), must use TMathText for all math symbols
    else if(region.Contains("CR"))
    {
        label = "#splitline{DY";
        if(region.Contains("ee")) {label+= " #rightarrow ee";}
        else if(region.Contains("mm")) {label+= " #rightarrow #mu#mu";}
        label+= " CR}";
    }

    if(region.Contains("HighVPt")) {label+= "{(High-p_{T})}";}
    else if(region.Contains("LowVPt")) {label+= "{(Low-p_{T})}";}

    return label;
}

//Set the list of variables based on arguments
void Fill_Variables_List(vector<TString>& variable_list, TString template_name)
{
    // TString template_basename = template_name; if(template_name.Contains("NN_")) {template_basename = template_name;}
    variable_list.push_back(template_name);

    return;
}

//-- Hardcode template naming conventions for plot's X-axis
//NB: 'paperStyle' <-> display NN bin number instead of score itself --> adapt X title //Default = false
TString Get_Template_XaxisTitle(TString variable, bool paperStyle)
{
    TString title = variable;

    //-- Previous conventions
    if(variable == "NN0") {title = "NN output (tZq node)";}
    else if(variable == "NN1") {title = "NN output (t#bar{t}Z node)";}
    else if(variable == "NN2") {title = "NN output (Others node)";}
    // else if(variable == "NN2") {title = "NN output (Backgrounds node)";}
    else if(variable.Contains("NN")) {title = "NN output";}

    //Other region templates conventions
    if(variable.Contains("mTW")) {title = "m_{T}^{W}";}
    else if(variable.Contains("countExp")) {title = "Counting experiment";}

    return title;
}

//Get the unit to display on the x-axis title (e.g.: 'Events / 5 GeV')
//cf. https://twiki.cern.ch/twiki/bin/view/CMS/Internal/PubGuidelines#Figures_and_tables
TString Get_Unit_Variable(TString var)
{
    TString unit = " units"; //Default //Must include space

    //GeV
    if(var.Contains("pt", TString::kIgnoreCase)
    || var.Contains("metEt", TString::kIgnoreCase)
    || var.Contains("mTW", TString::kIgnoreCase)
    || var.Contains("m_", TString::kIgnoreCase)
    || var.Contains("mass", TString::kIgnoreCase)
    ) {unit = " GeV";} //Must include space

    else if(var.Contains("dphi", TString::kIgnoreCase))  {unit = " radians";}

    //-- Dimensionless x-axis
    return unit;
}

/**
 * Read template histograms and extract/printout the yields per fit region
 */
void Print_Yields_fromHistograms(TString filepath, TString template_name, vector<TString> v_years, vector<TString> v_regions, vector<TString> v_processes)
{
//--------------------------------------------
    TString bin = ""; //"" <-> get yields for full histo; "bin1" <-> only for bin1, etc.
    // TString bin = "bin6"; //"" <-> get yields for full histo; "bin1" <-> only for bin1, etc.
//--------------------------------------------

//-- SETUP

    if(!Check_File_Existence(filepath))
    {
        cout<<endl<<"File "<<filepath<<FRED(" not found!")<<endl;
        return;
    }
    TFile* f = TFile::Open(filepath);

    v_years.push_back("Run2");
    vector<vector<vector<float>>> v3_years_regions_proc(v_years.size()); //1 element per year + Run2

//-- FILL VECTORS

    for(int iyear=0; iyear<v_years.size()-1 ; iyear++)
    {
        v3_years_regions_proc[iyear].resize(v_regions.size());
        v3_years_regions_proc[v_years.size()-1].resize(v_regions.size()); //Run2
        for(int iregion=0; iregion<v_regions.size(); iregion++)
        {
            v3_years_regions_proc[iyear][iregion].resize(v_processes.size());
            v3_years_regions_proc[v_years.size()-1][iregion].resize(v_processes.size()); //Run2
            for(int iproc=0; iproc<v_processes.size(); iproc++)
            {
                TString histoname;
                if(bin != "") {histoname+= bin + "_";}
                histoname+= template_name;
                if(v_regions[iregion] == "SRother" || v_regions[iregion] == "WZCR") {histoname = "mTW";}
                else if(v_regions[iregion] == "SRttZ4l" || v_regions[iregion] == "ZZCR") {histoname = "countExp";}
                histoname+=  "_" + v_regions[iregion] + "_" + v_years[iyear] + "__";
                if(v_processes[iproc] == "DATA") {histoname+= "data_obs";}
                else {histoname+= v_processes[iproc];}
                // cout<<"histoname "<<histoname<<endl;

                if(!f->GetListOfKeys()->Contains(histoname)) {cout<<"ERROR ! Histogram "<<histoname<<" not found !"<<endl; continue;}

                TH1F* th1f = (TH1F*) f->Get(histoname);
                v3_years_regions_proc[iyear][iregion][iproc] = th1f->Integral();
                v3_years_regions_proc[v_years.size()-1][iregion][iproc]+= th1f->Integral(); //Run2
                th1f = NULL;
            } //processes
        } //regions
    } //years

//-- PRINTOUTS

    for(int iyear=0; iyear<v_years.size(); iyear++)
    {
        cout<<endl<<BOLD(FMAG("=== YEAR "<<v_years[iyear]<<""))<<endl<<endl;
        for(int iregion=0; iregion<v_regions.size(); iregion++)
        {
            cout<<endl<<BOLD(FYEL("== REGION "<<v_regions[iregion]<<""))<<endl<<endl;
            float sum_mc = 0;
            for(int iproc=0; iproc<v_processes.size(); iproc++)
            {
                if(v_processes[iproc] == "tZq" || v_processes[iproc] == "ttZ" || v_processes[iproc] == "tWZ") {continue;}
                // if(v_processes[iproc] == "PrivMC_tZq" || v_processes[iproc] == "PrivMC_ttZ" || v_processes[iproc] == "PrivMC_tWZ") {continue;}

                cout<<"- PROCESS "<<v_processes[iproc]<<endl;
                cout<<"--> "<<v3_years_regions_proc[iyear][iregion][iproc]<<endl;
                if(v_processes[iproc] != "DATA") {sum_mc+= v3_years_regions_proc[iyear][iregion][iproc];}
            }
            cout<<"- TOTAL MC => "<<sum_mc<<endl;
        }
    }

    return;
}
