<!--
```
CODE EXAMPLE
```

=== Emoji list (see https://gist.github.com/rxaviers/7360908)
:arrow_right:
:heavy_exclamation_mark:
:heavy_check_mark:
:link:
:white_check_mark:
:heavy_multiplication_x:
:x:
:negative_squared_cross_mark:
:bangbang:
:white_check_mark:
:copyright:
:clock430:
:no_entry:
:ok:
:arrow_right_hook:
:paperclip:
:open_file_folder:
:chart_with_upwards_trend:
:lock:
:hourglass:
:warning:
:construction:
:fr:
:one: :two: :hash:
:underage:
:put_litter_in_its_place:
:new:


#HOW TO HIDE CONTENTS (which can be viewed by cliking icon) :
<details>
<summary>[NameOfHiddenContent]:</summary>
[theHiddenContent]
</details>
-------------------------------------------->
:construction: **README UNDER CONSTRUCTION**


> Codes and instructions to :
<!-- > * train BDTs (TMVA) and DNNs (Keras) -->
> * create MVA templates
> * create control plots
> * perform analysis-specific studies
> * generate and run COMBINE datacards


#### Table Of Contents

- [Installation and setup](#installation-and-setup)
    - [Interfacing with CMSSW](#interfacing-with-cmssw)
    - [Input ntuples](#input-ntuples)
- [Analysis](#analysis)
    - [Configuration](#configuration)
    - [Input ntuples](#input-ntuples)
    - [Running the code](#running-the-code)
- [Event yields](#event-yields)
<!-- - [Combine](#combine) -->


<!-- ![Mining gold](https://images.deepai.org/converted-papers/1805.00013/x1.png) -->
<!-- *[Credit : 10.1103/PhysRevLett.121.111801]* -->

# Installation and setup

This standalone Ntuple-analysis code does not require CMSSW, and can be run locally offline.

<!-- However, running `Combine` requires a CMSSW environment. -->

<!--
:heavy_exclamation_mark: `Potato` and `Combine` are currently supported under CMSSW_10_2_X; however, Tensorflow2 (needed for NN training/evaluation) is only supported under CMSSW_11_1_X; so you may need to use different versions for each !
-->

Instructions for setting up `Combine` properly are given in the dedicated [directory](https://github.com/nicolastonon/VBFgamma_analysis/COMBINE).

## Interfacing with CMSSW

Please follow the following instructions to install and interface the code with CMSSW. This has been tested on LXPLUS servers under SL7.

<!-- [need >=11_1_0 for Tensorflow2 compatibility ! Else crashes at NN evaluation] -->
```
cd PATH

export SCRAM_ARCH=slc7_amd64_gcc700
export CMSSW_VERSION=CMSSW_10_2_27
cmsrel $CMSSW_VERSION
cd $CMSSW_VERSION/src; cmsenv

#nanoAOD tools
git clone https://github.com/cms-nanoAOD/nanoAOD-tools.git PhysicsTools/NanoAODTools
cd PhysicsTools/NanoAODTools
cd -

#This code
git clone https://github.com/nicolastonon/VBFgamma_analysis.git
[Add 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:DIRPATH' into your .bashrc/profile settings, needed for custom lib]
scram b -j 5

cd VBFgamma_analysis
make
```

## Input ntuples

The directory [input_ntuples](https://github.com/nicolastonon/VBFgamma_analysis/input_ntuples) is intended to contain the ntuples to be analyzed, produced via the [VjjSkimmer](https://gitlab.cern.ch/nshafiei/vjjskimmer) framework.
Alternatively, you may hardcode the relevant path to the ntuple directory in 'Utils/Helper.cxx'.
Ntuples must be stored into sub-directories corresponding to their years of data-taking ('2016'/'2017'/'2018').
If working on remote servers, it may be practical to create symlinks pointing to the actual ntuples directories, as such:
```
ln -s /nfs/dust/cms/user/abs_path_to_ntupledir/2016 ./input_ntuples/2016
```

<!-- You may run the code `input_ntuples/Split_FullSamples.cxx` in order to further split the Ntuples by sub-categories (for faster access), to compute and store the per-event EFT parameterizations in private SMEFT samples, to produce the data-driven Fakes sample, etc. -->

# Analysis

The basic workflow is the following :

* The [Makefile](https://github.com/nicolastonon/VBFgamma_analysis/tree/master/Makefile) compiles all the codes. Use [Makefile.cmssw](https://github.com/nicolastonon/VBFgamma_analysis/tree/master/Makefile.cmssw) instead if you're working under CMSSW.

* The [analysis_main.cxx](https://github.com/nicolastonon/VBFgamma_analysis/tree/master/analysis_main.cxx) code contains the *main()* which runs the function calls. There are also defined all the customisable user-options.

* The [VBFgamma_analysis](https://github.com/nicolastonon/VBFgamma_analysis/tree/master/VBFgamma_analysis.cxx) class contains all the main analysis functions (make templates, plots, studies, etc.)

* The [Helper](https://github.com/nicolastonon/VBFgamma_analysis/tree/master/Helper.cxx) code contains additional, helper functions.

**The non-expert user should only modify the [analysis_main.cxx](https://github.com/nicolastonon/VBFgamma_analysis/tree/master/analysis_main.cxx) code, compile, and run it.**

## Configuration

Modify the [analysis_main.cxx](https://github.com/nicolastonon/VBFgamma_analysis/tree/master/analysis_main.cxx) code to configure the analysis.

The configuration interface relies heavily on the use of vectors, and is intended to be self-explanatory.

In particular, you can easily configure the :
* general options (signal process, etc.)
* list of data-taking years to consider
* skimming cuts to apply to the ntuples
* list of samples
* list of BDT input variables (if used)
* list of additional variables, not used in the MVA -- but only e.g. for control plots
* list of shape systematics to compute and store

Select the functions to call by setting the corresponding booleans accordingly.

## Running the code

### Locally

Copy either 'Makefile.local' or 'Makefile.cmssw' to 'Makefile', depending on your case.

```
make
./analysis_main.exe

#Optional -- specify a year and region, e.g. :
./analysis_main.exe 2016 signal
```

:arrow_right: Output root files (containing templates, control histograms, TMVA control file, etc.) are stored in the [outputs](https://github.com/nicolastonon/VBFgamma_analysis/tree/master/outputs) directory.

:arrow_right: Output plots are stored in the [plots](https://github.com/nicolastonon/VBFgamma_analysis/tree/master/plots) directory.

<!-- ### On HTCondor

*NB1: only works under CMSSW.*
*NB2: make sure the code is compiled first.*
```
make
./submit analyze #Will run ./analysis_main.exe on condor, and store outputs in relevant sub-dirs
``` -->


# Event yields

The code [Utils/Yield_Table.cxx](https://github.com/nicolastonon/VBFgamma_analysis/tree/master/Utils/Yield_Table.cxx) opens the input ntuples, and automatically computes the event yields for all selected processes.
Modify the *main()* options to select the data-taking year, processes, etc.

```
make
./Yield_Table.exe
```

<!-- # ROCS

Move to the [ROCS](https://github.com/nicolastonon/VBFgamma_analysis/tree/master/ROCS) directory.

The code [Compare_ROC_curves.cxx](https://github.com/nicolastonon/VBFgamma_analysis/tree/master/ROCS/Compare_ROC_curves.cxx) will read histogram files to plot the corresponding ROC curves.
It makes it easy to superimpose and compare several ROCS.
*NB1: plotting functions are implemented in [ROC_Plotter.cxx](https://github.com/nicolastonon/VBFgamma_analysis/tree/master/ROCS/ROC_Plotter.cxx)*
*NB2: specific naming conventions must be enforced for this code to work.*

Modify the *main()* options to define the paths of the input rootfiles, etc.

```
./make_plot.sh
``` -->

<!-- # Combine

See the dedicated [README](https://github.com/nicolastonon/VBFgamma_analysis/COMBINE). -->
