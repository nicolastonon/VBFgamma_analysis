import ROOT
import copy
from VJJEvent import VJJEvent, _defaultVjjCfg

#FIXME
class ReadComputeObservables:
    '''
    xxx
    '''

    def __init__(self, isData=False, mva_names=[]):

        self.isData = isData
        self.pfix = 'vjj_'
        self.outvars=[]
        self.photonExtra=[]
        # self.vjjEvent = VJJEvent()
        self.mva_names = mva_names
        self.selCfg = copy.deepcopy(_defaultVjjCfg)

        return


    def CreateAllBranches(self, out, isDefaultTree=True):

        if not isDefaultTree: self.CreateVJJBranches(out)
        self.CreateCategBranches(out)
        self.CreateMVABranches(out)
        if not self.isData:
            self.CreateGenBranches(out)
            self.CreateWeightBranches(out)

        return


    def CreateVJJEventBranches(self, out):

        self.pfix = 'vjj_'

        #floating point variables
        for v in ['qglqWgt', 'qglgWgt',
                  'v_pt', 'v_eta', 'v_phi', 'v_m', 'v_ystar',
                  'lead_pt', 'lead_eta', 'lead_phi', 'lead_m', 'lead_qgl', 'lead_dr2v','lead_dphiv','lead_detav',
                  'sublead_pt','sublead_eta','sublead_phi', 'sublead_m', 'sublead_qgl', 'sublead_dr2v','sublead_dphiv','sublead_detav',
                  'j_maxAbsEta','j_minAbsEta',
                  'jj_pt','jj_eta','jj_phi','jj_m','jj_dr2v','jj_scalarht','jj_deta','jj_dphi','jj_sumabseta',
                  'vjj_pt', 'vjj_eta', 'vjj_phi', 'vjj_m', 'vjj_dphi' ,'vjj_deta',
                  'vjj_scalarht', 'vjj_isotropy', 'vjj_circularity', 'vjj_sphericity', 'vjj_aplanarity',
                  'vjj_C', 'vjj_D',
                  'centj_pt', 'centj_eta', 'centj_phi', 'centj_m', 'centj_ystar', 'centj_dr2v',
                  'htsoft','centhtsoft']:
            outv=self.pfix+v
            self.outvars.append(outv)
            out.branch(outv,'F' , limitedPrecision=False)

        # integer variables
        for v in ['nwgt', 'fs', 'trig', 'nextraj','ncentj', 'lead_flav', 'sublead_flav']:
            outv=self.pfix+v
            self.outvars.append(outv)
            out.branch(outv,'I')

        # arrays
        out.branch(self.pfix+"wgt", "F", lenVar="nwgt" ,  limitedPrecision=False)

        return


    def CreateVJJBranches(self, out):
        '''
        Prepare the branches
        '''

        #FIXME
        # self.vjjEvent.makeBranches(out, False)
        # self.vjjEvent.resetOutVars()
        self.CreateVJJEventBranches(out)

        #From VJJselector
        out.branch('vjj_passSkim','O')
        out.branch('vjj_isGood','O')
        out.branch('genvjj_isGood','O')
        out.branch('genvjj_hasPromptPhoton','O')

        return


    def CreateCategBranches(self, out):
        '''
        Prepare the branches
        '''

        #Final event categories
        out.branch('vjj_isHighVPt','O')
        out.branch('vjj_isLowVPt','O')
        out.branch('vjj_isHighVPtmm','O')
        out.branch('vjj_isLowVPtmm','O')
        out.branch('vjj_isHighVPtee','O')
        out.branch('vjj_isLowVPtee','O')

        return


    def CreateMVABranches(self, out):
        '''
        Prepare the branches
        '''

        for mva_name in self.mva_names:
            out.branch('vjj_mva_{0}'.format(mva_name), 'F')

        return


    def CreateGenBranches(self, out):
        '''
        Prepare the branches
        '''

        out.branch('vjj_photonIsMatched' , 'B' )
        out.branch('vjj_maxGenPhotonPt' , 'F' )

        return


    def CreateWeightBranches(self, out):
        '''
        Prepare the branches
        '''

        out.branch('vjj_xsection' , 'F' )
        out.branch('vjj_lumiWeights' , 'F' , lenVar='vjj_nlumiWeights' )
        out.branch('vjj_weight' , 'F' )
        out.branch('vjj_sfweight_up' , 'F')
        out.branch('vjj_sfweight_down' , 'F')

        return


    def SetBranchesValues_fromInputTree():
        '''
        #FIXME
        '''


        return




















# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------

    #FIXME
    def FillVariables(self, out, v, jets):

        #select jets
        cleanJets=[j for j in jets if j.DeltaR(v)>self.selCfg['min_jetdr2v'] and abs(j.eta)<self.selCfg['max_jetEta'] and j.pt>self.selCfg['min_jetPt'] ]
        cleanJets.sort(key = lambda x : x.pt, reverse=True)

        tagJets=[j for j in cleanJets if j.pt>self.selCfg['min_tagJetPt'] ]
        if len(tagJets)<2 : return False
        if tagJets[0].pt<self.selCfg['min_leadTagJetPt'] : return False

        out.fillBranch(self.pfix+'lead_pt',      tagJets[0].pt)
        out.fillBranch(self.pfix+'lead_eta',     tagJets[0].eta)
        out.fillBranch(self.pfix+'lead_phi',     tagJets[0].phi)
        out.fillBranch(self.pfix+'lead_m',       tagJets[0].mass)
        out.fillBranch(self.pfix+'lead_dr2v',    tagJets[0].DeltaR(v))
        out.fillBranch(self.pfix+'lead_dphiv',   tagJets[0].DeltaPhi(v))
        out.fillBranch(self.pfix+'lead_detav',   abs(tagJets[0].eta-v.Eta()))
        if hasattr(tagJets[0],'partonFlavour'):
            out.fillBranch(self.pfix+'lead_flav',    tagJets[0].partonFlavour)
        out.fillBranch(self.pfix+'sublead_pt',   tagJets[1].pt)
        out.fillBranch(self.pfix+'sublead_eta',  tagJets[1].eta)
        out.fillBranch(self.pfix+'sublead_phi',  tagJets[1].phi)
        out.fillBranch(self.pfix+'sublead_m',    tagJets[1].mass)
        out.fillBranch(self.pfix+'sublead_dr2v', tagJets[1].DeltaR(v))
        out.fillBranch(self.pfix+'sublead_dphiv',   tagJets[1].DeltaPhi(v))
        out.fillBranch(self.pfix+'sublead_detav',   abs(tagJets[1].eta-v.Eta()))
        if hasattr(tagJets[1],'partonFlavour'):
            out.fillBranch(self.pfix+'sublead_flav', tagJets[1].partonFlavour)

        try:
            out.fillBranch(self.pfix+'lead_qgl',      tagJets[0].qgl)
            out.fillBranch(self.pfix+'sublead_qgl',   tagJets[1].qgl)
        except:
            pass

        #dijet system
        jj=tagJets[0].p4()+tagJets[1].p4()
        jj_m=jj.M()
        if(jj_m<self.selCfg['min_mjj']) : return False
        out.fillBranch(self.pfix+'jj_m',    jj_m)
        out.fillBranch(self.pfix+'jj_pt',   jj.Pt())
        out.fillBranch(self.pfix+'jj_eta',  jj.Eta())
        out.fillBranch(self.pfix+'jj_phi',  jj.Phi())
        out.fillBranch(self.pfix+'jj_dr2v', jj.DeltaR(v))
        jj_scalarht=tagJets[0].pt+tagJets[1].pt
        out.fillBranch(self.pfix+'jj_scalarht',jj_scalarht)
        etaList=[tagJets[0].eta,tagJets[1].eta]
        maxEta=max(etaList)
        minEta=min(etaList)
        etaAbsList=[abs(x) for x in etaList]
        out.fillBranch(self.pfix+'j_maxAbsEta',max(etaAbsList))
        out.fillBranch(self.pfix+'j_minAbsEta',min(etaAbsList))
        jj_sumabseta=sum(etaAbsList)
        out.fillBranch(self.pfix+'jj_deta',abs(maxEta-minEta))
        out.fillBranch(self.pfix+'jj_sumabseta',jj_sumabseta)
        out.fillBranch(self.pfix+'jj_dphi',tagJets[0].p4().DeltaPhi( tagJets[1].p4() ))
        out.fillBranch(self.pfix+'v_ystar',veta-0.5*jj.Eta() )

        #hard process object candidates
        vjj=v+jj;
        out.fillBranch(self.pfix+'vjj_pt', vjj.Pt())
        out.fillBranch(self.pfix+'vjj_eta',vjj.Eta())
        out.fillBranch(self.pfix+'vjj_phi',vjj.Phi())
        out.fillBranch(self.pfix+'vjj_m',  vjj.M())
        out.fillBranch(self.pfix+'vjj_scalarht',v.Pt()+jj_scalarht)
        out.fillBranch(self.pfix+'vjj_dphi',v.DeltaPhi(tagJets[0].p4()+tagJets[1].p4() ))
        out.fillBranch(self.pfix+'vjj_deta',abs(v.Eta()-jj.Eta()))

        #FIXME
        # eventShape = ROOT.EventShapeVariables()
        # eventShape.addObject( v )
        # eventShape.addObject( tagJets[0].p4() )
        # eventShape.addObject( tagJets[1].p4() )
        # out.fillBranch(self.pfix+"vjj_isotropy",    eventShape.isotropy() )
        # out.fillBranch(self.pfix+"vjj_circularity", eventShape.circularity() )
        # out.fillBranch(self.pfix+"vjj_sphericity",  eventShape.sphericity() )
        # out.fillBranch(self.pfix+"vjj_aplanarity",  eventShape.aplanarity() )
        # out.fillBranch(self.pfix+"vjj_C",           eventShape.C() )
        # out.fillBranch(self.pfix+"vjj_D",           eventShape.D() )
        # del eventShape

        #extra radiation activity
        extraJets=[j for j in cleanJets if not j in tagJets]
        nextraj,ncentj=len(extraJets),0
        htsoft,centhtsoft=0.,0.
        minEtaStar=minEta+0.2
        maxEtaStar=maxEta-0.2
        for j in extraJets:
            htsoft+=j.pt
            if j.eta<minEtaStar : continue
            if j.eta>maxEtaStar : continue
            ncentj+=1
            centhtsoft+=j.pt
            if ncentj>1 : continue
            out.fillBranch(self.pfix+'centj_pt',j.pt)
            out.fillBranch(self.pfix+'centj_eta',j.eta)
            out.fillBranch(self.pfix+'centj_phi',j.phi)
            out.fillBranch(self.pfix+'centj_m',  j.mass)
            out.fillBranch(self.pfix+'centj_ystar',j.eta-0.5*jj.Eta() )
            out.fillBranch(self.pfix+'centj_dr2v',j.DeltaR(v))

        out.fillBranch(self.pfix+'ncentj',ncentj)
        out.fillBranch(self.pfix+'centhtsoft',centhtsoft)
        out.fillBranch(self.pfix+'nextraj',nextraj)
        out.fillBranch(self.pfix+'htsoft',htsoft)

        #ready to fill entry
        return True

