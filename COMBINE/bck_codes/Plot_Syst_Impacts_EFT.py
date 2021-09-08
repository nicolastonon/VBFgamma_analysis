#-- Make impact plots

import ROOT
import logging
import os
import sys
import numpy
import itertools
import subprocess as sp
from Utils.ColoredPrintout import colors
import getopt # command line parser
import argparse
import numpy as np
import subprocess
from settings import opts #Custom dictionnary of settings
import shutil


#    # ###### #      #####  ###### #####
#    # #      #      #    # #      #    #
###### #####  #      #    # #####  #    #
#    # #      #      #####  #      #####
#    # #      #      #      #      #   #
#    # ###### ###### #      ###### #    #

# Run a shell subprocess
def run_command(inputs):
    try:
        stdout = subprocess.check_output(inputs,stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
        # Log the error and re-raise the exception
        logging.exception(e)
        logging.error(e.cmd)
        stdout = e.output
        for l in stdout.split('\n'):
            logging.info(l)
        raise e
    for l in stdout.split('\n'):
        logging.info(l)


def init():

    log_file = 'fitter.log'

    logger = logging.getLogger(__name__)
    FORMAT1 = '%(message)s'
    FORMAT2 = '[%(levelname)s] %(message)s'
    FORMAT3 = '[%(levelname)s][%(name)s] %(message)s'

    frmt1 = logging.Formatter(FORMAT1)
    frmt2 = logging.Formatter(FORMAT2)
    frmt3 = logging.Formatter(FORMAT3)

    logging.basicConfig(
        level=logging.DEBUG,
        format=FORMAT2,
        filename=log_file,
        filemode='w'
    )

    # Configure logging to also output to stdout
    console = logging.StreamHandler()
    console.setLevel(logging.INFO)
    console.setFormatter(frmt2)
    logging.getLogger('').addHandler(console)

def log_subprocess_output(pipe,level):
        ### Pipes Popen streams to logging class ###
        for line in iter(pipe.readline, ''):
            if level=='info': logging.info(line.rstrip('\n'))
            if level=='err': logging.error(line.rstrip('\n'))


 #    #  ####  #####  #    #  ####  #####    ##    ####  ######
 #    # #    # #    # #   #  #      #    #  #  #  #    # #
 #    # #    # #    # ####    ####  #    # #    # #      #####
 # ## # #    # #####  #  #        # #####  ###### #      #
 ##  ## #    # #   #  #   #  #    # #      #    # #    # #
 #    #  ####  #    # #    #  ####  #      #    #  ####  ######

def makeWorkspace(SM=False, datacard='datacard.txt', verbosity=0):
	### Generates a workspace from a datacard and fit parameterization file ###
	logging.info(colors.fg.lightblue + "Creating workspace" + colors.reset)
	if not os.path.isfile(datacard):
	    logging.error("Datacard does not exist!")
	    sys.exit()
	CMSSW_BASE = os.getenv('CMSSW_BASE')

	if SM:
	    args = ['text2workspace.py',datacard,'-P','HiggsAnalysis.CombinedLimit.PhysicsModel:multiSignalModel', '-o','SMWorkspace.root']
	    # Map signal strengths to processes in all bins
	    for iproc,proc in enumerate(opts["processes"]):
		args.extend(['--PO', 'map=.*/'+proc+':'+opts["SM_mus"][iproc]+'[1,'+str(opts["SMmu_ranges"][opts["SM_mus"][iproc]][0])+','+str(opts["SMmu_ranges"][opts["SM_mus"][iproc]][1])+']'])
	else: args = ['text2workspace.py',datacard,'-P','EFTModel:eftmodel','--PO','fits=./Parameterization_EFT.npy','-o','EFTWorkspace.root']

	if verbosity>0: args.extend(['-v', str(verbosity)])
	args.extend(['--channel-masks']) #Creates additional parameters allowing to later mask specific channels

	# Remove pre-existing WS
	if SM: sp.call(['rm','SMWorkspace.root'])
	else: sp.call(['rm','EFTWorkspace.root'])

	logging.info(colors.fg.purple + ' '.join(args) + colors.reset)
	process = sp.Popen(args, stdout=sp.PIPE, stderr=sp.PIPE)
	with process.stdout,process.stderr:
	    log_subprocess_output(process.stdout,'info')
	    log_subprocess_output(process.stderr,'err')
	process.wait()

	return


#=====================================

def Make_Impact_Plots(POIs, workspace, freeze=True, verbosity=0, other='', exp=False, only_collect_outputs=False, name='', ntoys=0, SM=False, batch=''):
    '''
    Generate impact plots. Can choose to run the per-nuisance fits on the grid.

    #See: https://github.com/cms-analysis/CombineHarvester/blob/master/CombineTools/scripts/plotImpacts.py
    #Available options:
    #parser.add_argument('--input', '-i', help='input json file')
    #parser.add_argument('--output', '-o', help='name of the output file to create')
    #parser.add_argument('--translate', '-t', help='JSON file for remapping of parameter names')
    #parser.add_argument('--units', default=None, help='Add units to the best-fit parameter value')
    #parser.add_argument('--per-page', type=int, default=30, help='Number of parameters to show per page')
    #parser.add_argument('--max-pages', type=int, default=None, help='Maximum number of pages to write')
    #parser.add_argument('--height', type=int, default=600, help='Canvas height, in pixels')
    #parser.add_argument('--left-margin', type=float, default=0.4, help='Left margin, expressed as a fraction')
    #parser.add_argument('--label-size', type=float, default=0.021, help='Parameter name label size')
    #parser.add_argument('--cms-label', default='Internal', help='Label next to the CMS logo')
    #parser.add_argument('--transparent', action='store_true', help='Draw areas as hatched lines instead of solid')
    #parser.add_argument('--checkboxes', action='store_true', help='Draw an extra panel with filled checkboxes')
    #parser.add_argument('--blind', action='store_true', help='Do not print best fit signal strength')
    #parser.add_argument('--color-groups', default=None, help='Comma separated list of GROUP=COLOR') #Ex.: --color-groups xxx=2 (red)
    #parser.add_argument('--pullDef',  default=None, help="Choose the definition of the pull, see HiggsAnalysis/CombinedLimit/python/#calculate_pulls.py for options")
    #parser.add_argument('--POI', default=None, help='Specify a POI to draw')
    '''

    print(colors.fg.lightblue + "Enter function Make_Impact_Plots()\n" + colors.reset)

    if len(POIs) == 0:
        print(colors.fg.red + "ERROR: empty list POIs\n" + colors.reset)
        exit(1)

    if name == '':
        if SM: name = 'SM'
        else: name = 'EFT'
    # elif name[0] != '.': name = '.{}'.format(name) #Convention: add a '.' before custom name

    if only_collect_outputs == False:

        #-- Do the initial fit
        args = ['combineTool.py','-M','Impacts','-d',workspace,'--doInitialFit','--robustFit','1','-m','125','--cminPoiOnlyFit'] #NB: -m 125 required

        #-- Options
        args.extend(['--redefineSignalPOIs',','.join('{}'.format(poi) for poi in POIs)])
        if SM: args.extend(['--setParameters',','.join([','.join('{}=1'.format(mu) for mu in opts['SM_mus'])])]) #Set default values to 1
        else: args.extend(['--setParameters',','.join([','.join('{}=0'.format(poi) for poi in opts["wcs"])])]) #Set default values to 0 #Mask channels if needed
        args.extend(['-n','{}'.format(name)])
        if freeze:
            frozen_pois = []
            if SM: frozen_pois = [par for par in opts['SM_mus'] if par not in POIs] #Define which WCs are frozen
            else: frozen_pois = [wc for wc in opts['wcs'] if wc not in POIs]
            if len(frozen_pois)>0: args.extend(['--freezeParameters',','.join('{}'.format(poi) for poi in frozen_pois)]) #Freeze other parameters
        else: args.extend(['--floatOtherPOIs','1']) #Float other parameters defined in the physics model
        if exp:
            if ntoys == 0: args.extend(['-t','-1']) #Assume MC expected
            else: args.extend(['-t',str(ntoys),'-s','12345']) #Generate MC toys #NB: provide this random seed as arg, propagated through calls (-- needed modification in CHarv/Impacts.py, otherwise can't deal with automatic seed suffix --)
        if verbosity>0: args.extend(['-v', str(verbosity)])
        if other: args.extend(other)
        # args.extend(['--setParameterRanges', ':'.join('{}={},{}'.format(poi,opts["wc_ranges"][poi][0],opts["wc_ranges"][poi][1]) for poi in opts['wcs'])]) #for poi in POIs
        if SM: args.extend(['--setParameterRanges', ':'.join('{}={},{}'.format(mu, opts["SMmu_ranges"][mu][0], opts["SMmu_ranges"][mu][1]) for mu in opts["SM_mus"])])
        else: args.extend(['--setParameterRanges', ':'.join('{}={},{}'.format(wc, opts["wc_ranges"][wc][0], opts["wc_ranges"][wc][1]) for wc in opts["wcs"])])
        args.extend(['--autoBoundsPOIs=%s' % (','.join(POIs))])
        args.extend(['--autoMaxPOIs=%s' % (','.join(POIs))])
        # args.extend(['--trackParameters',','.join(poi for poi in opts['wcs'] if poi not in POIs)])

        #print(colors.fg.purple + ' '.join(args) + colors.reset)
        #run_command(args)

        logging.info(colors.fg.purple + " ".join(args) + colors.reset)
        process = sp.Popen(args, stdout=sp.PIPE, stderr=sp.PIPE)
        with process.stdout,process.stderr:
            log_subprocess_output(process.stdout,'info')
            log_subprocess_output(process.stderr,'err')
        process.wait()

        #-- Do a fit for each nuisance parameter in the datacard
        args = ['combineTool.py','-M','Impacts','--doFits','--robustFit','1','-d',workspace,'-m','125','--cminPoiOnlyFit']
        args.extend(['--redefineSignalPOIs',','.join('{}'.format(poi) for poi in POIs)])
        if SM: args.extend(['--setParameters',','.join([','.join('{}=1'.format(mu) for mu in opts['SM_mus'])])]) #Set default values to 1
        else: args.extend(['--setParameters',','.join([','.join('{}=0'.format(poi) for poi in opts["wcs"])])]) #Set default values to 0 #Mask channels if needed

        #-- NB: '--algo impact' hardcodes options '--floatOtherPOIs=1 --saveInactivePOI=1', hence must not repeat these
        if freeze:
            frozen_pois = []
            if SM: frozen_pois = [par for par in opts['SM_mus'] if par not in POIs] #Define which WCs are frozen
            else: frozen_pois = [wc for wc in opts['wcs'] if wc not in POIs]
            if len(frozen_pois)>0: args.extend(['--freezeParameters',','.join('{}'.format(poi) for poi in frozen_pois)]) #Freeze other parameters
        if exp:
            if ntoys == 0: args.extend(['-t','-1']) #Assume MC expected
            else: args.extend(['-t',str(ntoys),'-s','12345']) #Generate MC toys
        if verbosity>0: args.extend(['-v', str(verbosity)])
        if other:               args.extend(other)
        if SM: args.extend(['--setParameterRanges', ':'.join('{}={},{}'.format(mu, opts["SMmu_ranges"][mu][0], opts["SMmu_ranges"][mu][1]) for mu in opts["SM_mus"])])
        else: args.extend(['--setParameterRanges', ':'.join('{}={},{}'.format(wc, opts["wc_ranges"][wc][0], opts["wc_ranges"][wc][1]) for wc in opts["wcs"])])
        args.extend(['--autoBoundsPOIs=%s' % (','.join(POIs))])
        args.extend(['--autoMaxPOIs=%s' % (','.join(POIs))])
        if name != '': args.extend(['-n','{}'.format(name)])
        # args.extend(['--trackParameters',','.join(poi for poi in opts['wcs'] if poi not in POIs)])

        if batch=='crab': args.extend(['--job-mode','crab3','--task-name',name.replace('.',''),'--custom-crab','Utils/custom_crab.py'])
        if batch=='condor': args.extend(['--job-mode','condor','--task-name',name.replace('.',''),'--dry-run'])

        logging.info(colors.fg.purple + " ".join(args) + colors.reset)
        process = sp.Popen(args, stdout=sp.PIPE, stderr=sp.PIPE)
        with process.stdout,process.stderr:
            log_subprocess_output(process.stdout,'info')
            log_subprocess_output(process.stderr,'err')
        process.wait()

        # Condor needs execution permissions on the .sh file, so we used --dry-run
        # Add the permission and complete the submission #Also this stores all out/err files in a dedicated directory
        if batch=='condor':
            if os.path.exists('condor{}'.format(name)):
                logging.error("Directory condor{} already exists!".format(name))
                logging.error("OVERWRITING !")
                shutil.rmtree('condor{}'.format(name), ignore_errors=True)
                #return

            sp.call(['mkdir','condor{}'.format(name)])
            sp.call(['chmod','a+x','condor_{}.sh'.format(name.replace('.',''))])
            logging.info('Now submitting condor jobs.')
            condorsub = sp.Popen(['condor_submit','-append','initialdir=condor{}'.format(name),'condor_{}.sub'.format(name.replace('.',''))], stdout=sp.PIPE, stderr=sp.PIPE)
            with condorsub.stdout,condorsub.stderr:
                log_subprocess_output(condorsub.stdout,'info')
                log_subprocess_output(condorsub.stderr,'err')
            condorsub.wait()

        if batch: logging.info(colors.fg.lightblue + "Done with gridScan batch submission." + colors.reset)
        else: logging.info(colors.fg.lightblue + "Done with gridScan." + colors.reset)

        if batch != '': return #If we just submitted jobs, need to wait that they finish before we collect outputs

    #-- Create a json file using as input the files generated in the previous two steps
    json_filename = 'impacts_'+name+'.json'
    args = ['combineTool.py','-M','Impacts','-o',json_filename,'-d',workspace,'-m','125']
    args.extend(['--redefineSignalPOIs',','.join('{}'.format(poi) for poi in POIs)])
    if exp:
        if ntoys == 0: args.extend(['-t','-1']) #Assume MC expected
        else: args.extend(['-t',str(ntoys),'-s','12345']) #Generate MC toys
    if verbosity>0: args.extend(['-v', str(verbosity)])
    if other: args.extend(other)
    if name != '': args.extend(['-n','{}'.format(name)])

    logging.info(colors.fg.purple + " ".join(args) + colors.reset)
    process = sp.Popen(args, stdout=sp.PIPE, stderr=sp.PIPE)
    with process.stdout,process.stderr:
        log_subprocess_output(process.stdout,'info')
        log_subprocess_output(process.stderr,'err')
    process.wait()

    #-- Create the impact plot pdf file
    outfilename_tmp = 'impacts_'+name
    for poi in POIs:
        outfilename = outfilename_tmp
        if outfilename=='impacts' or len(POIs)>1: outfilename+= '_'+str(poi)
        args = ['plotImpacts.py','-i',json_filename,'--POI','%s' % (poi),'-o',outfilename,'--per-page','20','--translate','../Plotting/rename.json','--cms-label','Internal']

        logging.info(colors.fg.purple + " ".join(args) + colors.reset)
        process = sp.Popen(args, stdout=sp.PIPE, stderr=sp.PIPE)
        with process.stdout,process.stderr:
            log_subprocess_output(process.stdout,'info')
            log_subprocess_output(process.stderr,'err')
        process.wait()

    #-- Create the impact plot png file (only first page -- for easier inclusion in powerpoints, etc.)
    outfilename_tmp = 'impacts_'+name
    for poi in POIs:
        outfilename = outfilename_tmp
        if outfilename=='impacts' or len(POIs)>1: outfilename+= '_'+str(poi)
        outfilename+= '.png'
        args = ['plotImpacts.py','-i',json_filename,'--POI','%s' % (poi),'-o',outfilename,'--per-page','10','--max-pages','1','--translate','../Plotting/rename.json','--cms-label','Internal']

        logging.info(colors.fg.purple + " ".join(args) + colors.reset)
        process = sp.Popen(args, stdout=sp.PIPE, stderr=sp.PIPE)
        with process.stdout,process.stderr:
            log_subprocess_output(process.stdout,'info')
            log_subprocess_output(process.stderr,'err')
        process.wait()

    return


def Make_NLL_Scan_NuisancePar(workspace, nuisance, POIs, npoints=100, range=[-4,4], freeze=True, verbosity=0, exp=False, other='', ntoys=0):
    '''
    Perform a NLL grid scan for a given nuisance parameter.
    '''

    print(colors.fg.lightblue + "Enter function Make_NLL_Scan_NuisancePar()\n" + colors.reset)

    #-- Perform grid scan for single nuisance
    args = ['combineTool.py','-d',workspace,'-M','MultiDimFit','--algo','grid','--points',str(npoints),'-n',('_paramFit_'+nuisance),'-P',nuisance,'--saveInactivePOI','1','--robustFit','1']
    args.extend(['--setParameters',','.join('{}=0'.format(poi) for poi in [nuisance]+opts['wcs'])]) #Set default values to 0 for all WCs and nuisance
    #-- NB: '--algo impact' hardcodes options '--floatOtherPOIs=1 --saveInactivePOI=1', hence must not repeat these
    if freeze: args.extend(['--freezeParameters',','.join('{}'.format(poi) for poi in opts['wcs'] if poi not in POIs)]) #Freeze others
    if exp:
        if ntoys == 0: args.extend(['-t','-1']) #Assume MC expected
        else: args.extend(['-t',str(ntoys),'-s','12345']) #Generate MC toys
    if verbosity>0: args.extend(['-v', str(verbosity)])
    if other: args.extend(other)
    args.extend(['--setParameterRanges',nuisance+'='+str(range[0])+','+str(range[1])])
    args.extend(['--autoBoundsPOIs=%s' % (','.join(POIs))])
    args.extend(['--autoMaxPOIs=%s' % (','.join(POIs))])

    logging.info(colors.fg.purple + " ".join(args) + colors.reset)
    process = sp.Popen(args, stdout=sp.PIPE, stderr=sp.PIPE)
    with process.stdout,process.stderr:
        log_subprocess_output(process.stdout,'info')
        log_subprocess_output(process.stderr,'err')
    process.wait()

    #-- Plot NLL scan
    inputfile = 'higgsCombine_paramFit_'+nuisance+'.MultiDimFit.mH125'+('' if ntoys==0 else '.12345')+'.root'
    args = ['plot1DScan.py',inputfile,'--POI', nuisance] #Minimal args
    args.extend(['--y-cut','7','--y-max','8','--main-color','1','--logo','CMS','--logo-sub','Internal']) #Optional args
    if exp: args.extend(['--main-label','Expected'])
    else: args.extend(['--main-label','Observed'])
    args.extend(['--output','scan_'+nuisance]) #Output filename
    args.extend(['--translate','../Plotting/rename.json']) #Translate names using JSON file
    #args.extend(['--breakdown','1']) #'do quadratic error subtraction using --others'

    logging.info(colors.fg.purple + " ".join(args) + colors.reset)
    process = sp.Popen(args, stdout=sp.PIPE, stderr=sp.PIPE)
    with process.stdout,process.stderr:
        log_subprocess_output(process.stdout,'info')
        log_subprocess_output(process.stderr,'err')
    process.wait()

    return


def Make_NLL_Scan_AllNuisancePars(workspace, POIs, npoints=100, range=[-4,4], freeze=True, verbosity=0, exp=False, other='', ntoys=0):
    '''
    Run NLL grid scans for all nuisance parameters found in the input RooWorkspace.

    NB: how to run this on the grid ?
    '''

    print(colors.fg.lightblue + "Enter function Make_NLL_Scan_AllNuisancePars()\n" + colors.reset)

    nuisances = []

    logging.info("Retrieving complete list of nuisances from workspace {}".format(workspace))

    rootFile = ROOT.TFile.Open(workspace);
    w = rootFile.Get("w") #Retrieve WS object
    #w.Print()
    mc = w.genobj("ModelConfig") #Get ModelConfig object
    #mc.GetParametersOfInterest().Print("V") #Print POIs
    #mc.GetNuisanceParameters().Print("V") #Print nuisances
    set_nuisances = mc.GetNuisanceParameters() #Get list of nuisances

    iter = set_nuisances.createIterator()
    var = iter.Next()
    while var :
        #print var.GetName()
        nuisances.append(var.GetName())
        var = iter.Next()
    #print(nuisances)
    print(colors.fg.lightblue + '-- Found' + len(nuisances) + ' nuisances parameters...\n' +  colors.reset)

    if len(nuisances)==0:
        logging.info(colors.fg.red + "ERROR: empty list of nuisances from file " + workspace + colors.reset)
        exit(1)

    #-- Do grid scan and plot for each individual nuisance
    for nuisance in nuisances:
        print(colors.fg.lightblue + "-- Nuisance " + nuisance + "\n" + colors.reset)
        Make_NLL_Scan_NuisancePar(workspace, nuisance, POIs, npoints, range, freeze, verbosity, exp, other, ntoys=ntoys)

    return


# //--------------------------------------------
# //--------------------------------------------

##     ##    ###    #### ##    ##
###   ###   ## ##    ##  ###   ##
#### ####  ##   ##   ##  ####  ##
## ### ## ##     ##  ##  ## ## ##
##     ## #########  ##  ##  ####
##     ## ##     ##  ##  ##   ###
##     ## ##     ## #### ##    ##

# //--------------------------------------------
# //--------------------------------------------

if __name__ == "__main__":

    init()
    print('Example usage: [python Plot_Syst_Impacts_EFT.py -m impacts -d ./EFTWorkspace.root --POI ctz --freeze --exp]')

# User options -- Default values
# //--------------------------------------------
    datacard_path = './datacard.txt'
    exp = False #True <-> Asimov a-priori expected; False <-> observed
    verb=0
    name = '' #Suffix added to output filenames
    fixedPointNLL = False #True <-> perform the NLL scan at a fixed point (different Combine options)
    freeze=False
    POIs=[]
    mode = 'impacts' #'impacts' (make impact plot) / 'scan_nuisance' (scan 1 specific nuisance) / 'scan_all' (scan all nuisances)
    nuisance = '' #Name of single nuisance parameter to scan
    npoints = 50 #Number of points to scan nuisance(s)
    only_collect_outputs = False #True <-> for impact plot, only collect output files previously produced e.g. on the grid
    batch = '' #'' (interactive) or 'condor'
    ntoys = 0 # >0 <-> will generate MC toys rather than Asimov dataset
    SM = False #True <-> consider SM scenario (rather than SMEFT)

# Set up the command line arguments
# //--------------------------------------------
    parser = argparse.ArgumentParser(description='Make impact plots for specific/all nuisance parameters')
    parser.add_argument('--exp', help='Use MC predictions only (no data)', nargs='?', const=1) #nargs='?' <-> 0 or 1 arg (default value is const=1)
    parser.add_argument("--collect", metavar="collect", help="Impact plot: only collect output files previously produced e.g. on the grid", nargs='?', const=1)
    parser.add_argument("--freeze", metavar="freeze", help="Freeze other POIs", nargs='?', const=1)
    parser.add_argument("-d", metavar="workspace/datacard path", help="Path to the datacard")
    parser.add_argument("-v", metavar="Combine verbosity level", help="Set combine output verbosity")
    parser.add_argument('-P','--POI', metavar="POI", nargs='+', help='Define POI(s)', required=False) #Takes >=0 args
    parser.add_argument("-m", metavar="m", help="[impacts] or [scan_nuisance] or [scan_all]")
    parser.add_argument("-n", metavar="name", help="add suffix to output filename")
    parser.add_argument("--nuisance", metavar="nuisance", help="Name of single nuisance to scan")
    parser.add_argument("--npoints", metavar="npoints", help="Number of points to scan nuisance(s)")
    parser.add_argument("--batch", metavar="batchmode", help="(crab or condor -- only condor for now)", nargs='?', const=1)
    parser.add_argument("--toys", metavar="ntoys", help="Number of MC toys to generate")
    parser.add_argument("--sm", metavar="SM", help="Consider SM scenario (rather than SMEFT)", nargs='?', const=1)

    args = parser.parse_args()
    if args.d: datacard_path = args.d
    if args.v: verb = int(args.v)
    if args.exp: exp = True
    if args.n: name = args.n
    if args.freeze: freeze = True
    if args.POI: POIs = args.POI
    else:
        print('ERROR: missing arg --POI !')
        exit(1)
    if args.m: mode = args.m
    if args.nuisance: nuisance = args.nuisance
    if args.npoints: npoints = args.npoints
    if args.collect: only_collect_outputs = True
    if args.batch: batch = 'condor' #Only use condor
    if args.toys: ntoys = args.toys
    if args.sm: SM = True

    if mode not in ['impact','impacts','scan_nuisance','scan_all']:
        logging.info(colors.fg.lightblue + "ERROR: wrong option mode" + colors.reset)
        exit(1)
    #if batch=='crab':
    #    print('ERROR: crab mode is not supported yet ! Use condor to submit jobs !')
    #    exit(1)
    if ntoys>0 and exp == False:
        print('ERROR: you set both [exp=False] and [ntoys>0] which are incompatible... !')
        exit(1)


# //--------------------------------------------

    if 'txt' in datacard_path:
        if only_collect_outputs == False: makeWorkspace(SM=SM, datacard=datacard_path, verbosity=verb) #Create RooWorkspace from txt datacard
        if SM == True: datacard_path = 'SMWorkspace.root'
	else: datacard_path = 'EFTWorkspace.root'

    if mode in ['impact','impacts']: Make_Impact_Plots(POIs, workspace=datacard_path, freeze=freeze, verbosity=verb, other='', exp=exp, only_collect_outputs=only_collect_outputs, name=name, ntoys=ntoys, SM=SM, batch=batch)

    elif mode == 'scan_nuisance': Make_NLL_Scan_NuisancePar(datacard_path, nuisance, POIs, npoints=npoints, range=[-4,4], freeze=freeze, verbosity=verb, exp=exp, other='', ntoys=ntoys)

    elif mode == 'scan_all': Make_NLL_Scan_AllNuisancePars(datacard_path, POIs, npoints=npoints, range=[-4,4], freeze=freeze, verbosity=verb, exp=exp, other='', ntoys=ntoys)
