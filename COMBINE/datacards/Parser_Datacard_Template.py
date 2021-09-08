#Usage : 'python Parser_Datacard_Template.py [channel] [variable] [root file] ...'

import fileinput
import os
import sys, getopt
import string
import re
import os

# //--------------------------------------------

total = len(sys.argv)
#cmdargs = str(sys.argv)

theVar = str(sys.argv[1])
channel = str(sys.argv[2])
year = str(sys.argv[3])
theFiletoRead = sys.argv[4]
systChoice = str(sys.argv[5])
statChoice = str(sys.argv[6])
datacard_dir = str(sys.argv[7])

# region = str(sys.argv[8]) #'SR', 'ttZ' (CR), etc.

print('\n * Creating datacard for year : '+year+' / channel : '+channel+' / variable : '+theVar)

fileToSearch = "Template_Datacard.txt" #TEMPLATE card to parse
file = open(fileToSearch).read()
# print('fileToSearch', fileToSearch)
#if any(x in theVar for x in ['_CR']): fileToSearch = "Template_Datacard_noSig.txt" #Special case: don't consider any signal in CRs (approximation)

# //--------------------------------------------

# if(channel!="" and channel!="all" and channel!="uuu" and channel!="uue" and channel!="eeu" and channel!="eee" and channel!="ee" and channel!="uu" and channel!="ue"):
#     print("wrong channel")
#     print("channel should be '', 'all', 'uuu', 'uue', 'eeu' or 'uuu' or 'ee' or 'uu' or 'ue'")
#     exit()

#-- Channels
if channel=="all": #Use the channel='all' keyword because parser needs to read some arg ! But don't want to appear in datacards. Also remove the "_" in front
    channel=""
else:
    channel = "_" + channel #Keep the "_" in front
    # varchan="varchan" #if no subcategorization, want to remove the "_" between 'var' and 'chan' !

# //--------------------------------------------

#-- Shape systs
if systChoice=="withShape":
    shape = ""
elif systChoice == "noShape":
    shape = "#"
else:
    print("Wrong systChoice value ! should be 'withShape' or 'noShape'")
    exit()

#-- MC stat uncertainty
if statChoice=="withStat":
    stat = ""
elif statChoice == "noStat":
    stat = "#"
else:
    print("Wrong statChoice value ! should be 'withStat' or 'noStat'")
    exit()

#-- SR/CR differences (when using RooParametricHist)
SR = '' #Default: consider region as SR
CR = '#'
if 'SR' in theVar:
    SR = ''
elif 'CR' in theVar:
    CR = ''
    SR = '#'

#-- Year-to-year correlations
is2016="#"
is2017="#"
is2018="#"
is201617="#"
is201718="#"
if year=="2016":
    is2016=""
    is201617=""
elif year=="2017":
    is2017=""
    is201617=""
    is201718=""
elif year=="2018":
    is2018=""
    is201718=""

#-- Hard-coded special cases: e.g. if a lN syst. is correlated between years with different values, use a marker replaced with year-specific values by parsing code
Lumi1718 = "-"
Lumi161718 = "-" #LumiXY
if year=="2016":
    Lumi161718 = "1.006"
elif year=="2017":
    Lumi1718 = "1.006"
    Lumi161718 = "1.009"
elif year=="2018":
    Lumi1718 = "1.002"
    Lumi161718 = "1.02"

#--------------------------------------------

# ele_sys = "" #Ele systematics only in ele channels
# if channel=="uuu" or "mmm" in channel:
#     ele_sys = "#"

#-- Can add a rateParam line to control normalization of processes from datacard
# ratePar = "#" #empty to activate, or '#' to disactivate
# sigPar = "tZq" #e.g. "tZq"
# rateVal = "1" #or '2' to double the rate of the process ? (verify)

#--------------------------------------------

#-- REPLACE KEYWORDS #Order may be important (if subkeywords are included in larger keywords, etc.)
file = file.replace("[YEAR]", year)
file = file.replace("[2016]", is2016)
file = file.replace("[2017]", is2017)
file = file.replace("[2018]", is2018)
file = file.replace("[201617]", is201617)
file = file.replace("[201718]", is201718)
file = file.replace("[Lumi161718]", Lumi161718) #Was: LumiXY
file = file.replace("[Lumi1718]", Lumi1718)
file = file.replace("[SHAPE]", shape)
file = file.replace("[STAT]", stat)
file = file.replace("filetoread", theFiletoRead)
file = file.replace("[VAR]",theVar)
file = file.replace("_[CHAN]", channel)
file = file.replace("[SR]", SR)
file = file.replace("[CR]", CR)

#--------------------------------------------

#-- Create output datacard
outname = datacard_dir+"/datacard_"+theVar;
if channel != "": outname+= channel;
outname+= ".txt"

print('==> Datacard ', outname, ' created...')

f = open(outname, 'w')
f.write(file)
f.close()
