import ROOT
from ROOT import TCanvas, TGraph, gStyle
import logging
import os
import sys
import numpy as np
import itertools
import subprocess as sp
import getopt # command line parser
#import argparse
#import configparser
import numpy
import matplotlib.pyplot as plt

def test():

    ROOT.gROOT.SetBatch(True)

    WC_values = [-4., -2., -1., 0., 1., 2., 4.] #Must be doubles
    operator = "ctw"

    list_filepaths = []
    for val in WC_values:
        list_filepaths.append('./higgsCombine_'+operator+'_'+str(int(val))+'.MultiDimFit.mH120.root')

	#list_filepaths.append('./higgsCombine_ctz_-4.MultiDimFit.mH120.root')
    #list_filepaths.append('./higgsCombine_ctz_-2.MultiDimFit.mH120.root')
    #list_filepaths.append('./higgsCombine_ctz_-1.MultiDimFit.mH120.root')
    #list_filepaths.append('./higgsCombine_ctz_0.MultiDimFit.mH120.root')
    #list_filepaths.append('./higgsCombine_ctz_1.MultiDimFit.mH120.root')
    #list_filepaths.append('./higgsCombine_ctz_2.MultiDimFit.mH120.root')
    #list_filepaths.append('./higgsCombine_ctz_4.MultiDimFit.mH120.root')

    c = ROOT.TCanvas('','',1000,800)
    c.SetGrid(1)
    c.SetTopMargin(0.1)
    l = c.GetLeftMargin()

    #-- Get x-y coordinates for TGraph representing NLL function
    ientry = 1 #Which entry to read ? #FIXME
    NLL_values = []
    for filepath in list_filepaths:
        print('filepath', filepath)

        #-- Get scan TTree
        rootFile = ROOT.TFile.Open(filepath)

        limitTree = rootFile.Get('limit')
        limitTree.GetEntry(ientry) #This is where we read the relevant entry

        print('deltaNLL', limitTree.GetLeaf('deltaNLL').GetValue(0)) #For non-arrays, always read leaf element 0
        print('nll0', limitTree.GetLeaf('nll0').GetValue(0))
        print('nll', limitTree.GetLeaf('nll').GetValue(0))

        NLL_tmp = limitTree.GetLeaf('deltaNLL').GetValue(0) + limitTree.GetLeaf('nll0').GetValue(0) + limitTree.GetLeaf('nll').GetValue(0)
        NLL_values.append(2 * NLL_tmp)

    graph = ROOT.TGraph(len(WC_values), numpy.asarray(WC_values), numpy.asarray(NLL_values))

    for ipt in range(graph.GetN()):
        x, y = ROOT.Double(0), ROOT.Double(0) #Necessary to pass by reference in GetPoint()
        graph.GetPoint(ipt, x, y)
        print(ipt, x, y)

    f = ROOT.TF1("f", "[2] * x * x + [1] * x + [0]")
    # f = ROOT.TF1("f", "[4] * x * x * x * x + [3] * x * x * x + [2] * x * x + [1] * x + [0]")
    graph.Fit(f)

    # min = f.GetMinimum()
    # NLL_values = [val-min for val in NLL_values]
    # graph = ROOT.TGraph(len(WC_values), numpy.asarray(WC_values), numpy.asarray(NLL_values))
    # graph.Fit(f)

    # for ipt in range(graph.GetN()):
    #     x, y = ROOT.Double(0), ROOT.Double(0) #Necessary to pass by reference in GetPoint()
    #     graph.GetPoint(ipt, x, y)
    #     print(ipt, x, y)

    graph.SetMarkerStyle(20)
    graph.SetMarkerSize(1.3)
    graph.SetMarkerColor(ROOT.kBlue)
    graph.Draw("AP") #A:axes, P: markers, L:line

    c.Print('plot_test.png')

    return

if __name__ == "__main__":

    test()

