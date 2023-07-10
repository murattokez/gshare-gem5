
# gshare-gem5

gshare branch prediction implemantation on gem5



## gem5 GshareBP implementation

   
### Step 1: gshare.cc and gshare.hh
First, two files named gshare.cc and gshare.hh should be moved inside the /gem5/src/cpu/pred folder. 

### Step 2: BranchPredictor.py

Before adding a SimObject to gem5, you need to create a Python class for the SimObject you will add. This Python class includes the parameters of your SimObject that can be controlled from your Python configuration file. We will add a new class to our existing gem5/src/cpu/pred/BranchPredictor.py class file that inherits from the "BranchPredictor" class for Branch Predictions. 

```
class GshareBP(BranchPredictor):
 type = 'GshareBP'
 cxx_class = 'gem5::branch_prediction::GshareBP'
 cxx_header = "cpu/pred/gshare.hh"
 PHTPredictorSize = Param.Unsigned(16384, "Size of local predictor")
 PHTCtrBits = Param.Unsigned(2, "Bits per counter")
 globalPredictorSize = Param.Unsigned('16', "Size of global Predictor")

```

### Step 3: SConscript
To be able to compile the C++ file we need to instruct the build system about these files. gem5 uses SCons as its build system. Therefore, an SConscript file should be written for the added SimObject, specifying what needs to be built in the system. An SConscript file is a simple Python file, and more information can be found in online resources. Enter the existing gem5/src/cpu/pred/SConscript file.

```
SimObject('BranchPredictor.py', sim_objects=[
    'IndirectPredictor', 'SimpleIndirectPredictor', 'BranchPredictor',
    'LocalBP', 'GshareBP', 'TournamentBP', 'BiModeBP', 'TAGEBase', 'TAGE', 'LoopPredictor',
    'TAGE_SC_L_TAGE', 'TAGE_SC_L_TAGE_64KB', 'TAGE_SC_L_TAGE_8KB',
    'LTAGE', 'TAGE_SC_L_LoopPredictor', 'StatisticalCorrector', 'TAGE_SC_L',
    'TAGE_SC_L_64KB_StatisticalCorrector',
    'TAGE_SC_L_8KB_StatisticalCorrector',
    'TAGE_SC_L_64KB', 'TAGE_SC_L_8KB', 'MultiperspectivePerceptron',
    'MultiperspectivePerceptron8KB', 'MultiperspectivePerceptron64KB',
    'MPP_TAGE', 'MPP_LoopPredictor', 'MPP_StatisticalCorrector',
    'MultiperspectivePerceptronTAGE', 'MPP_StatisticalCorrector_64KB',
    'MultiperspectivePerceptronTAGE64KB', 'MPP_TAGE_8KB',
    'MPP_LoopPredictor_8KB', 'MPP_StatisticalCorrector_8KB',
    'MultiperspectivePerceptronTAGE8KB'])

```

Also, the following line should be added to the end of the file.

```
Source('gshare.cc')
```

### Step 4: Build gem5
After completing all the steps, we need to rebuild the system from scratch in order to add the gshare SimObject to gem5. You can choose the desired ISA for the build. We are working with RISCV. Open the terminal and navigate to the gem5 directory, then enter the following command:

```
scons build/RISCV/gem5.opt
```

Now gem5 has gshare Branch Prediction. To verify this, while in the gem5 directory, you can enter the following command in the terminal:

```
build/RISCV/gem5.opt configs/example/se.py --help
```

If you see the gshareBP parameter in the --bp_type section, it means that the algorithm has been successfully added.
