import datetime
import os
from os import path
import sys
from lobster import cmssw
from lobster.core import AdvancedOptions, Category, Config, MultiProductionDataset, StorageConfiguration, Workflow, Dataset,ParentDataset
sys.path.append(os.path.abspath("."))
import Files_ULall_nano
import math


SAMPLES = {}
SAMPLES.update(Files_ULall_nano.UL17)
timestamp_tag = datetime.datetime.now().strftime('%Y%m%d_%H%M')
production_tag = "AnalysisZprimeTT"            # For 'full_production' setup

# Only run over lhe steps from specific processes/coeffs/runs
process_whitelist = []
coeff_whitelist   = []
runs_whitelist    = []  # (i.e. MG starting points)

master_label = '%s_%s' % (production_tag,timestamp_tag)

input_path   = "/store/user/"
output_path  = "/store/user/$USER/FullProduction/%s" % (production_tag)
workdir_path = "/tmpscratch/users/$USER/FullProduction/%s" % (production_tag)
plotdir_path = "~/www/lobster/FullProduction/%s" % (production_tag)

storage = StorageConfiguration(
    input=[
        "file:///cms/cephfs/data/store/user/",
        "root://cmsxrootd.crc.nd.edu//store/user/",        
        "root://cmsxcache.crc.nd.edu//store/user/",
    ],
    output=[
        # Until a separate bug is fixed file://cms/cephfs needs to be the first output so the initial lobster validation passes.
        "file:///cms/cephfs/data"+output_path,
        "root://cmsxrootd.crc.nd.edu/"+output_path,
        "root://cmsxcache.crc.nd.edu/"+output_path,
    ],
    disable_input_streaming=True,
)

#################################################################
# Worker Res.:
#   Cores:  12    | 4
#   Memory: 16000 | 8000
#   Disk:   13000 | 6500
#################################################################

gs_resources = Category(
    name='gs',
    cores=1,
    memory=3500,
    disk=6000,
    mode='max'
)

#################################################################
wf = []
totalJob=0
for key, value in SAMPLES.items():
#    if 'UL17' not in key:
#       continue
    mypath='/cms/cephfs/data/store/user/rgoldouz/FullProduction/AnalysisZprimeTT/Analysis_' + key
    orgPath='/cms/cephfs/data/store/user/'+value[0][0]
    FPT=15
    cat = gs_resources
    FPT = math.ceil(2.5 / float(value[11]))
    if path.exists(mypath) and len(os.listdir(mypath))>0:
        continue
    nfiles = sum(os.path.isfile(os.path.join(orgPath, f))    for f in os.listdir(orgPath))
    totalJob+=math.ceil(nfiles/float(FPT))
    print (key,f" is submitted --> number of files {nfiles}, with {FPT} per task --> {math.ceil(nfiles/float(FPT))} task")
    Analysis = Workflow(
        label='Analysis_%s' % (key),
        sandbox=cmssw.Sandbox(release='/groups/mhildret/rgoldouz/run3Analyses/CMSSW_15_0_10'),
        globaltag=False,
        command='python3 Lobster_check.py ' + key + ' ' + value[1] +' ' + value[2] +' ' +value[3] +' ' +value[4] +' ' +value[5] +' ' +value[6] +' ' +value[7] +' ' +value[8] +' ' +value[9] +' ' +value[10]  +' ' +value[12] +' @inputfiles',
        extra_inputs=[
            '../Makefile',
            'Lobster_check.py',
            '../lib',
            '../include',
            '../bin',
            '../src',
        ],
        outputs=['ANoutput.root'],
        dataset=Dataset(
           files=value[0],
           patterns=["*.root"],
           files_per_task =FPT
        ),
#        merge_command='hadd @outputfiles @inputfiles',
#        merge_size='2G',
#        category=gs_resources
        category=cat
    )
    wf.append(Analysis)
print(f"total jobs {totalJob}")
config = Config(
    label=master_label,
    workdir=workdir_path,
    plotdir=plotdir_path,
    storage=storage,
    workflows=wf,
    advanced=AdvancedOptions(
        bad_exit_codes=[127, 160,137],
        osg_version='3.6',
        abort_threshold=100,
        abort_multiplier=100,
        full_monitoring=True, 
        log_level=1,
        dump_core=True
    )
)

