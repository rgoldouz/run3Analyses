import subprocess
import sys
import os
import glob

# Fix missing $HOME to avoid ROOT crash
if not os.environ.get("HOME"):
    tmp_home = "/tmp/" + os.environ.get("USER", "default_user")
    os.environ["HOME"] = tmp_home
    if not os.path.exists(tmp_home):
        os.makedirs(tmp_home)

os.environ["ROOTIGNOREMIMES"] = "1"

infiles = sys.argv[12:]
print(infiles)
val = sys.argv[1:12]
print(val)

# --- Build core analysis execution strings ---
text = ''
text += '    TChain* ch    = new TChain("Events") ;\n'
for fn in infiles:
    a, b = fn.split(':')
    text += '    ch ->Add("' +  b + '");\n'
text += '    MyAnalysis * t1 = new MyAnalysis(ch);\n'
text += '    t1->Analyze("' + val[0]+'", "' + val[1] + '" , "'+ val[2] + '" , "'+ val[3] + '" , "'+ val[4] + '" , ' + val[5] + ' , '+ val[6] + ' , '+ val[7] + ' , '+ val[8] + ' , '+ val[9] + ' , '+ val[10] +', t1);\n'
text += '    delete t1;\n'

# --- Write a true C++ main application to src/main.C ---
if not os.path.exists('src'):
    os.makedirs('src')

SHNAME1 = 'src/main.C'
SHFILE1 = (
    '#include "MyAnalysis.h"\n'
    '#include "TChain.h"\n\n'
    'int main() {\n' + \
    text + \
    '    return 0;\n'
    '}'
)

open(SHNAME1, 'wt').write(SHFILE1)


# Strip out problematic Bash functions from current environment
clean_env = {k: v for k, v in os.environ.items() if not k.startswith("BASH_FUNC_")}

# --- FIXED: Chain both CMSSW (cmsenv) and RestFrames environment generation together ---
# This ensures both setups build on top of each other dynamically inside a single subshell stream
cmssw_src_dir = "/groups/mhildret/rgoldouz/CMSSW_15_0_10/src/"
restframes_script = "/groups/mhildret/rgoldouz/run3Analyses/ZprimeToTt/RestFrames/setup_RestFrames.sh"

source_cmd = (
    f"cd {cmssw_src_dir} && eval `scramv1 runtime -sh` && cd - && "
    f"source {restframes_script} && env"
)

print("Initializing complete execution environment (CMSSW + RestFrames)...")
dump = subprocess.check_output(["bash", "-c", source_cmd], env=clean_env).decode("utf-8")

for line in dump.splitlines():
    if "=" in line:
        try:
            key, value = line.split("=", 1)
            if not key.startswith("BASH_FUNC_"):
                os.environ[key] = value
        except ValueError:
            continue

# --- CRITICAL RUNTIME ENVIRONMENT PATHWAYS ---
current_dir = os.getcwd()
os.environ["LD_LIBRARY_PATH"] = (
    current_dir + "/lib:" + 
    current_dir + "/RestFrames/lib:" + 
    current_dir + ":" + 
    os.environ.get("LD_LIBRARY_PATH", "")
)
os.environ["CPATH"] = current_dir + ":" + "/groups/mhildret/rgoldouz/run3Analyses/ZprimeToTt/RestFrames/include:/groups/mhildret/rgoldouz/run3Analyses/ZprimeToTt/include"
os.environ["ROOT_INCLUDE_PATH"] = current_dir + ":" + os.environ.get("ROOT_INCLUDE_PATH", "")

env = os.environ.copy()
env["OMP_NUM_THREADS"] = "1"
env["MKL_NUM_THREADS"] = "1"
env["OPENBLAS_NUM_THREADS"] = "1"
env["ROOT_DISABLE_IMT"] = "1"

# --- RUNNING MAKE VIA TRANSFERRED MAKEFILE ---
print("Compiling src/main.C on the node...")
exit_code = subprocess.call(["make", "bin/RunAll"], env=env)

if exit_code != 0:
    print("Compilation via Makefile failed! Exiting job execution context.")
    sys.exit(1)

# --- RUN THE COMPILED NATIVE EXECUTABLE ---
print("Running compiled binary application...")
try:
    # check_call will immediately raise an exception if ./bin/RunAll fails/crashes
    subprocess.check_call(["./bin/RunAll"], env=env)
except subprocess.CalledProcessError as e:
    print(f"CRITICAL: C++ Executable ./bin/RunAll crashed or failed with exit code {e.returncode}!")
    sys.exit(e.returncode)

# Get matching files safely
files = glob.glob("ANoutput*.root")

if not files:
    print("No output ROOT files found. Skipping hadd.")
    sys.exit(1)

if os.path.exists("ANoutput.root"):
    os.remove("ANoutput.root")

os.system('hadd -f ANoutput.root ANoutput*.root')
