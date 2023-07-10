#include "cpu/pred/gshare.hh"

#include "base/bitfield.hh"
#include "base/intmath.hh"
#include "cpu/nativetrace.hh"
#include "arch/generic/vec_pred_reg.hh"

namespace gem5{




namespace branch_prediction{


GshareBP::GshareBP(const GshareBPParams &params)
    : BPredUnit(params),
      globalHistory(params.numThreads, 0), // initialize global history register for each thread to 0
      globalHistoryBits(params.globalPredictorSize), // number of bits in the global history register
      choicePredictorSize(params.PHTPredictorSize), // size of the choice predictor (in entries)
      choiceCtrBits(params.PHTCtrBits), // number of bits in the counters of the choice predictor
      choiceCtrs(choicePredictorSize, SatCounter8(choiceCtrBits))

{
    if(!isPowerOf2(choicePredictorSize)){
        fatal("Invalid choice predictor size!\n");
    }// check that choice predictor size is a power of 2

    // Set up choiceHistoryMask
    // this is equivalent to mask(log2(choicePredictorSize)
    choiceHistoryMask = choicePredictorSize - 1;

    //Setup the array of counters for the choice predictor.
    //choicCtrs vektorunun boyutunu yeniden ayarlar
   // choiceCtrs.resize(choicePredictorSize);

    
    // initialize the counters in the choice predictor to have the specified number of bits
   /* for (int i = 0; i < choicePredictorSize; ++i)
        choiceCtrs[i].setBits(choiceCtrBits);*/

     //Set up historyRegisterMask
    historyRegisterMask = mask(globalHistoryBits);


    // Set thresholds for the predictors' counters
    // This is equivalent to (2^(Ctr))/2 - 1
    choiceThreshold = (1ULL << (choiceCtrBits - 1)) - 1;

}

// helper function that calculates the index into the local history table
inline
unsigned
GshareBP::calcLocHistIdx(Addr &branch_addr)
{
    // Get low order bits after removing instruction offset.
    return (branch_addr >> instShiftAmt) & historyRegisterMask;
}


// helper function that updates the global history register when a branch is taken
inline
void
GshareBP::updateGlobalHistTaken(ThreadID tid)
{
    // shift left by 1 and set the least significant bit to 1
    globalHistory[tid] = (globalHistory[tid] << 1) | 1;
    // apply historyRegisterMask to ensure only globalHistoryBits are used
    globalHistory[tid] = globalHistory[tid] & historyRegisterMask;
}

// Update the global history register for a not taken branch
inline
void
GshareBP::updateGlobalHistNotTaken(ThreadID tid)
{
    // Shift global history left by 1 bit and set the least significant bit to 0
    globalHistory[tid] = (globalHistory[tid] << 1);
    // Mask the global history with historyRegisterMask to keep it within the specified size
    globalHistory[tid] = globalHistory[tid] & historyRegisterMask;
}


void
GshareBP::btbUpdate(ThreadID tid, Addr branch_addr, void * &bp_history)
{
    // Clear the least significant bit of the global history
    globalHistory[tid] &= (historyRegisterMask & ~1ULL);
}

bool GshareBP::lookup(ThreadID tid, Addr branch_addr, void *&bp_history){
    //local history_idx is used to index into the local history table
    unsigned local_history_idx;

    //choise_prediction is used to store the prediction made by the choice predictor
    bool choice_prediction;

    //Calculate the index into the local history table
    local_history_idx = calcLocHistIdx(branch_addr);

    //Calculate the index into the choice predictor
    unsigned choice_prediction_idx = (globalHistory[tid] & historyRegisterMask) ^ local_history_idx;

    // Lookup in the choice predictor to see which one to use
    choice_prediction = choiceThreshold < choiceCtrs[choice_prediction_idx & choiceHistoryMask];
    /* eger thresholddan buyukse taken kucukse non taken demistik. burda indexdeki deger choiceCtrs vektorunde o indexindeki store edilen
    counter objesinin read() metodu kullanılarak elde edilir.
    */


    // Create BPHistory and pass it back to be recorded.
    BPHistory *history = new BPHistory;
    history->globalHistory = globalHistory[tid] & historyRegisterMask;
    bp_history = (void *)history;

    // Return the prediction made by the choice predictor
    if (choice_prediction) 
    {
         updateGlobalHistTaken(tid);
         return true;
     } 
     else 
     {
         updateGlobalHistNotTaken(tid);
         return false;
     }


}

void
GshareBP::uncondBranch(ThreadID tid, Addr pc, void * &bp_history)
{

    // Create BPHistory and pass it back to be recorded.
    BPHistory *history = new BPHistory;
    history->globalHistory = globalHistory[tid] & historyRegisterMask;
    bp_history = static_cast<void *>(history);

    updateGlobalHistTaken(tid);
}

void
GshareBP::update(ThreadID tid, Addr branch_addr, bool taken,
                     void *bp_history, bool squashed)
{
    assert(bp_history);

    BPHistory *history = static_cast<BPHistory *>(bp_history);

    unsigned local_history_idx = calcLocHistIdx(branch_addr);

    if (squashed) {
        // Global history restore and update
        globalHistory[tid] = (history->globalHistory << 1) | taken;
        globalHistory[tid] &= historyRegisterMask;

        return;
    }

    // there was a prediction.
    unsigned choice_predictor_idx = (local_history_idx ^ history->globalHistory) & choiceHistoryMask;
    if(taken)
    {
        choiceCtrs[choice_predictor_idx]++; //increment metodu vardı degistirdim
    }
    else
    {
        choiceCtrs[choice_predictor_idx]--; //decrement metodu vardı degistirdim
    }

    // We're done with this history, now delete it.
    delete history;
}

void
GshareBP::squash(ThreadID tid, void *bp_history)
{

    BPHistory *history = static_cast<BPHistory *>(bp_history);

    // Restore global history to state prior to this branch.
    globalHistory[tid] = history->globalHistory;

    delete history;
}

/*GshareBP*
GshareBPParams::create()
{
    return new GshareBP(this);
}*/

unsigned
GshareBP::getGHR(ThreadID tid, void *bp_history) const
{
    return static_cast<BPHistory *>(bp_history)->globalHistory;
}




#ifdef DEBUG
int
GshareBP::BPHistory::newCount = 0;
#endif









}//namespace bp

}//name space gem5
