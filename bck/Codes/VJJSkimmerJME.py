'''
Adapted from VJJSkimmer.py code by Nicolas TONON (DESY)

Main functionalities:
- Compute high-level observables and store them in output GetTree
- Skim events based on final categories (-> produce lightweight ntuples)
- Handle JEC/JER/MET variations separately (must be chained to the nanoAOD jetmetHelperRun2 module -- see: https://github.com/cms-nanoAOD/nanoAOD-tools/blob/master/python/postprocessing/modules/jme/jetmetHelperRun2.py)
'''

DEBUG = True #True <-> debug printouts #FIXME

# //--------------------------------------------
# //--------------------------------------------

import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True
import os
from PhysicsTools.NanoAODTools.postprocessing.framework.datamodel import Collection
from PhysicsTools.NanoAODTools.postprocessing.framework.eventloop import Module
from UserCode.VJJSkimmer.samples.Sample import Sample
from UserCode.VJJSkimmer.samples.campaigns.Manager import Manager as CampaignManager

#FIXME
from PhysicsTools.NanoAODTools.postprocessing.framework.output import * #OutputTree
from VJJEvent import VJJEvent,_defaultVjjCfg,_defaultGenVjjCfg,_defaultVjjSkimCfg
from ReadComputeObservables import ReadComputeObservables
from UserCode.VJJSkimmer.postprocessing.modules.JetSelector import *

# //--------------------------------------------
# //--------------------------------------------

class VJJSkimmerJME(Module):

    def __init__(self, sample, campaign):

        self.sample = Sample(sample)
        self.campaign = campaign

        self.isData           = self.sample.isData()
        self.era              = self.sample.year()

        self.allWeights       = self.campaign.get_allWeightIndices(sample)
        self.nWeights         = len(self.allWeights)
        self.lumiWeights      = self.campaign.get_lumi_weight(sample)
        self.xSection         = self.campaign.get_xsection(sample)

        #-- Try to load BDTReader module (reimplemented in VJJPlotter code) via python dictionaries
        try:
            ROOT.gSystem.Load("libUserCodeVJJPlotter")
            dummy = ROOT.BDTReader()
            #Load it via ROOT ACLIC. NB: this creates the object file in the CMSSW directory,
            #causing problems if many jobs are working from the same CMSSW directory
        except Exception as e:
            print "Could not load module via python, trying via ROOT", e
            if "/BDTReader_cc.so" not in ROOT.gSystem.GetLibraries():
                print "Load C++ Worker"
                if 'CMSSW_BASE' in os.environ:
                    ROOT.gROOT.ProcessLine(".L %s/src/UserCode/VJJPlotter/src/BDTReader.cc++" % os.environ['CMSSW_BASE'])
                else:
                    ROOT.gROOT.ProcessLine(".L ../../VJJPlotter/src/BDTReader.cc++")
            dummy = ROOT.BDTReader()


    def beginJob(self):
        pass


    def endJob(self):
        pass


# //--------------------------------------------
# //--------------------------------------------
########  ########  ######   #### ##    ## ######## #### ##       ########
##     ## ##       ##    ##   ##  ###   ## ##        ##  ##       ##
##     ## ##       ##         ##  ####  ## ##        ##  ##       ##
########  ######   ##   ####  ##  ## ## ## ######    ##  ##       ######
##     ## ##       ##    ##   ##  ##  #### ##        ##  ##       ##
##     ## ##       ##    ##   ##  ##   ### ##        ##  ##       ##
########  ########  ######   #### ##    ## ##       #### ######## ########
# //--------------------------------------------
# //--------------------------------------------

    def beginFile(self, inputFile, outputFile, inputTree, wrappedOutputTree):
        '''
        Init commands executed when opening file
        '''

        #-- Add custom histo
        outputFile.cd()
        self.hTotals = ROOT.TH1D("TotalNumbers" , "" , self.nWeights , 0 , self.nWeights )
        nTotals          = self.campaign.get_allNTotals(self.sample.ds)
        for wid in range(self.nWeights):
            self.hTotals.SetBinContent( wid + 1 , nTotals[wid] )
            self.hTotals.GetXaxis().SetBinLabel( wid + 1 , self.allWeights[wid][0] )

        #-- Init BDTReader
        self.BDTReader = ROOT.BDTReader(True,True,True)
        self.BDTReader.Init(inputTree._ttreereader.GetTree())


 #####  #####    ##   #    #  ####  #    # ######  ####
 #    # #    #  #  #  ##   # #    # #    # #      #
 #####  #    # #    # # #  # #      ###### #####   ####
 #    # #####  ###### #  # # #      #    # #           #
 #    # #   #  #    # #   ## #    # #    # #      #    #
 #####  #    # #    # #    #  ####  #    # ######  ####

        #-- Define output tree
        self.out = wrappedOutputTree #Default nanoAOD output tree

        #FIXME
        self.obsMaker = ReadComputeObservables(self.isData, self.BDTReader.outputNames)

        self.obsMaker.CreateAllBranches(self.out, isDefaultTree=True)

        #NEW TREE
        self.myTTree = ROOT.TTree("myTree", "myTree")
        self.myTree = OutputTree(outputFile, self.myTTree, inputTree) #OK
        self.obsMaker.CreateAllBranches(self.myTree, isDefaultTree=False)

        '''
        #Final event catgegories
        self.out.branch('vjj_isHighVPt','O')
        self.out.branch('vjj_isLowVPt','O')
        self.out.branch('vjj_isHighVPtmm','O')
        self.out.branch('vjj_isLowVPtmm','O')
        self.out.branch('vjj_isHighVPtee','O')
        self.out.branch('vjj_isLowVPtee','O')

        #BDT variables
        for mva_name in self.BDTReader.outputNames:
            self.out.branch('vjj_mva_{0}'.format( mva_name ) , 'F' )

        #Gen-level info and weights
        if not self.isData:
            self.out.branch('vjj_photonIsMatched' , 'B' )
            self.out.branch('vjj_maxGenPhotonPt' , 'F' )

            self.out.branch('vjj_xsection' , 'F' )
            self.out.branch('vjj_lumiWeights' , 'F' , lenVar='vjj_nlumiWeights' )
            self.out.branch('vjj_weight' , 'F' )

            self.out.branch('vjj_sfweight_up' , 'F')
            self.out.branch('vjj_sfweight_down' , 'F')
        '''


        '''
        self.myTree.Branch('vjj_isHighVPt','O')
        self.myTree.Branch('vjj_isLowVPt','O')
        self.myTree.Branch('vjj_isHighVPtmm','O')
        self.myTree.Branch('vjj_isLowVPtmm','O')
        self.myTree.Branch('vjj_isHighVPtee','O')
        self.myTree.Branch('vjj_isLowVPtee','O')
        '''

        #-- Define keep/drop filepath
        # print('//////')
        # keep_drop = '{0}/python/UserCode/VJJSkimmer/postprocessing/etc/keep_and_drop_skimmer_new.txt'.format(os.getenv('CMSSW_BASE' , '.'))
        # self.myOutput = FullOutput(inputFile, inputTree, outputFile, outputbranchSelection=keep_drop, fullClone=False, maxEntries=None)
        # self.myTree = self.myOutput._tree
        # print('//////')

        # self.vjjEvent = VJJEvent(cfg=_defaultVjjCfg)
        # self.vjjEvent.makeBranches(self.myTree)

        # self.obsComputer.CreateVJJBranches(self.myTree)

        # self.myTree._tree.Print()

        # self.myTree.branch('vjj_v_eta','F')
        # list_br = inputTree.GetListOfBranches()
        # print(list_br)

        # for br in list_br:
        #     print(br)
        #     self.myTree.branch('vjj_isHighVPt','O')

        return


# //--------------------------------------------
# //--------------------------------------------
######## ##    ## ########  ######## #### ##       ########
##       ###   ## ##     ## ##        ##  ##       ##
##       ####  ## ##     ## ##        ##  ##       ##
######   ## ## ## ##     ## ######    ##  ##       ######
##       ##  #### ##     ## ##        ##  ##       ##
##       ##   ### ##     ## ##        ##  ##       ##
######## ##    ## ########  ##       #### ######## ########
# //--------------------------------------------
# //--------------------------------------------

    def endFile(self, inputFile, outputFile, inputTree, wrappedOutputTree):
        outputFile.cd()
        self.hTotals.SetDirectory(outputFile)
        self.hTotals.Sumw2()
        self.hTotals.Write()

        self.myTree.write() #FIXME

        return


# //--------------------------------------------
# //--------------------------------------------
   ###    ##    ##    ###    ##       ##    ## ######## ########
  ## ##   ###   ##   ## ##   ##        ##  ##       ##  ##
 ##   ##  ####  ##  ##   ##  ##         ####       ##   ##
##     ## ## ## ## ##     ## ##          ##       ##    ######
######### ##  #### ######### ##          ##      ##     ##
##     ## ##   ### ##     ## ##          ##     ##      ##
##     ## ##    ## ##     ## ########    ##    ######## ########
# //--------------------------------------------
# //--------------------------------------------

    def analyze(self, event):

        """
        Process each event --> return True (go to next module) or False (fail, go to next event)
        """

        if DEBUG:
            print('-- ENTRY: ' + str(event._entry))
            print('-- EVENT: ' + str(event.event))

        #access JEC SFs
        #print('JEC', event.Jet_corr_JEC)

        #jet selection #FIXME only nom
        all_jets = Collection(event, "Jet")
        jetsIdx = event.vjj_jets
        jets = [all_jets[i] for i in jetsIdx]

        #FIXME -- may be simpler to use a FullOutput object... to have same in nom/jec
        v = ROOT.TLorentzVector(); v.SetPtEtaPhiM(event.vjj_v_pt, event.vjj_v_eta, event.vjj_v_phi, event.vjj_v_m)
        #self.obsMaker.FillVariables(v, self.myTree, jets)

        #-- By default, set all category flags to False
        for b in ['LowVPt' , 'HighVPt' , 'HighVPtmm','LowVPtmm' ,'HighVPtee' ,'LowVPtee']:
            self.out.fillBranch('vjj_is{0}'.format( b ) , False)
            #self.myTree.fillBranch('vjj_is{0}'.format( b ) , False) #FIXME

        # self.myTree.fillBranch('vjj_isHighVPt', True) #FIXME
        # self.myTree.fillBranch('vjj_v_eta', event.vjj_v_eta) #FIXME

        #-- Check if event enters the different categories
        category = ""
        isLow = False
        isHigh = False
        high_pt_lowerCut = 200 # 175 if self.era == 2016 else 200
        min_vjj_jj_m = 200; min_v_pt = 75; vjj_lead_ptCut = 50; vjj_sublead_ptCut = 50
        vjj_v_etaCut = 1.442; vjj_jj_detaCut = 3.0

        if event.vjj_isGood and event.vjj_jj_m > min_vjj_jj_m and event.vjj_lead_pt > vjj_lead_ptCut and event.vjj_sublead_pt > vjj_sublead_ptCut and event.vjj_fs in [22,121,169]:

            #LowVPt
            if event.vjj_v_pt > min_v_pt and abs(event.vjj_v_eta) < vjj_v_etaCut and abs(event.vjj_jj_deta) > vjj_jj_detaCut and event.vjj_jj_m > 200:
                if event.vjj_fs == 22:
                    if event.vjj_trig != 2: isLow = True
                elif event.vjj_trig == 3: isLow = True

            #HighVPt
            if not isLow and event.vjj_v_pt>high_pt_lowerCut:
                if event.vjj_fs == 22:
                    if event.vjj_trig == 2: isHigh = True
                elif event.vjj_trig == 3: isHigh = True

            #Define category name
            if isHigh: category = "HighVPt"
            elif isLow: category = "LowVPt"

            #Update category name
            if category != '':
                if event.vjj_fs == 22: pass
                elif event.vjj_fs == 121: category += 'ee'
                elif event.vjj_fs == 169: category += 'mm'

        #-- Reject events not entering any category
        if category == "":
            return False

        self.out.fillBranch('vjj_is{0}'.format(category), True) #Update category flag
        self.out.fillBranch('vjj_is{0}'.format(category) , True)

        #-- Only run the BDTReader if the event got selected
        self.BDTReader.Process(event._entry) #Process entry
        for i in range(self.BDTReader.outputNames.size()): #Write all selected MVAs to output
            mva_name = self.BDTReader.outputNames[i]
            self.out.fillBranch('vjj_mva_{0}'.format( mva_name ) , self.BDTReader.mvaValues[i] )

        #-- Write MC weight infos to output
        if not self.isData:

            #Xsec
            self.out.fillBranch('vjj_xsection',self.xSection)

            #Theory weights
            lumiweights = []
            for windex in range(self.nWeights):
                wid = self.allWeights[windex][1]
                lumiweights.append(event.genvjj_wgt[wid]*self.lumiWeights[category][windex])
            self.out.fillBranch('vjj_lumiWeights' , lumiweights)

            wsf = event.vjj_photon_effWgt
            wsf_up = event.vjj_photon_effWgtUp
            wsf_down = event.vjj_photon_effWgtDn
            if 'mm' in category:
                wsf = event.vjj_mu_effWgt
                wsf_up = event.vjj_mu_effWgtUp
                wsf_down = event.vjj_mu_effWgtDn
            elif 'ee' in category:
                wsf = event.vjj_ele_effWgt
                wsf_up = event.vjj_ele_effWgtUp
                wsf_down = event.vjj_ele_effWgtDn

            #Prefire
            prefirew = event.PrefireWeight if self.era != 2018 else 1

            #Object SFs
            self.out.fillBranch('vjj_weight' , wsf*event.puWeight*prefirew )
            self.out.fillBranch('vjj_sfweight_down' , wsf_down/wsf )
            self.out.fillBranch('vjj_sfweight_up' , wsf_up/wsf )

            #Gen-level info
            genParts = Collection(event, "GenPart")
            vjjPhotons = Collection(event, "vjj_photons" , 'vjj_nphotons')
            if event.vjj_nphotons > 0:
                photons = Collection(event, "Photon" )
                selectedPhotonIndex = ord(event.vjj_photons[0])
                photon_genPartIdx = photons[selectedPhotonIndex].genPartIdx
                if photon_genPartIdx > -1: self.out.fillBranch( 'vjj_photonIsMatched' , genParts[photon_genPartIdx].statusFlags & 1 )
                else: self.out.fillBranch( 'vjj_photonIsMatched' , -10 )
            else: self.out.fillBranch( 'vjj_photonIsMatched' , -20 )

            maxGenPhotonPt = -1
            for genPart in genParts:
                if genPart.pdgId == 22:
                    if genPart.statusFlags & 1 :
                        if genPart.pt > maxGenPhotonPt :
                            maxGenPhotonPt = genPart.pt
            self.out.fillBranch( 'vjj_maxGenPhotonPt' , maxGenPhotonPt )

        self.myTree.fill() #FIXME

        return True #Select event, pass to next module / write entry to output

