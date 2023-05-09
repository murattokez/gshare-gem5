#ifndef __CPU_PRED_GSHARE_PRED_HH__
#define __CPU_PRED_GSHARE_PRED_HH__

#include <vector>

#include "base/types.hh"
#include "cpu/pred/bpred_unit.hh"
#include "base/sat_counter.hh"
#include "params/GshareBP.hh"



#include "params/BranchPredictor.hh"

namespace gem5{



namespace branch_prediction{

   

class GshareBP : public BPredUnit {

    public:
        //default bp constructor
        GshareBP(const GshareBPParams &params);

        bool lookup(ThreadID tid, Addr branch_addr, void * &bp_history);
        /*pcden gelen adrese bakarak taken(1) veya non-taken(0) döndürür
        ayrıca squash veya update'de ihtiyac duyulan herhangi bir state'i
        store etmek icin bir BPHistory nesnesi olusturur.
        @param branch_addr The address of the branch to look up.
        @param bp_history Pointer that will be set to the BPHistory object.
        @return Whether or not the branch is taken.
        */

        void uncondBranch(ThreadID tid, Addr pc, void * &bp_history);
        /* eger bir uncond branch gelirse global history taken olarak guncellenir ve
        global history'i store etmek icin bir BPhistory objecti yaratılır
        */

        void btbUpdate(ThreadID tid, Addr branch_addr, void * &bp_history);
        /* eger BTB'de invalid bir entry veya entry bulunamazsa branch non-taken olarak
        tahmin edilir. 
        @param branch_addr The address of the branch to look up.
        @param bp_history Pointer to any bp history state.
        @return Whether or not the branch is taken.
        */

        void update(ThreadID tid, Addr branch_addr, bool taken, void *bp_history,
                bool squashed);
        /*taken/non taken bilglieriyle BP guncellenir.
        @param branch_addr Branch'in guncelenecek Pc'si yani adresi
        @param taken branch'in taken/non taken durumu
        @param bp_history branch tahmin edildiginde olusturulan BPHistory
        nesnesinin pointerı
        */

        void squash(ThreadID tid, void *bp_history);
        /* global branch history'i bir squash'a geri store eder.
        Icınde onceki global branch history'e sahip olan BPHistory nesnesinin pointerı
        */
    
        unsigned getGHR(ThreadID tid, void *bp_history) const; //global history register

    private:

    inline unsigned calcLocHistIdx(Addr &branch_addr);
    /*This function calculates the index into the local predictor table for a given branch
    instruction. It does this by using the branch address to compute a hash value.
    */

    inline void updateGlobalHistTaken(ThreadID tid);
    /** Updates global history as taken. */


    inline void updateGlobalHistNotTaken(ThreadID tid);
    /** Updates global history as not taken. */


    struct BPHistory { //bir branchin tahmin eidlmesi uzerine olsturulan branch history information
#ifdef DEBUG
        BPHistory()
        { newCount++; }
        ~BPHistory()
        { newCount--; }

        static int newCount;
#endif
        unsigned globalHistory;
    };
    



/*The BPHistory struct is a simple data structure that is used to store information about
the history of a branch prediction. It contains a single member variable, globalHistory,
which is an unsigned integer that holds the global history of branch predictions for a particular hardware thread.

The #ifdef DEBUG block at the beginning of the struct definition is used to enable or disable
debugging information. When DEBUG is defined (e.g., through a compiler flag or #define DEBUG directive),
the BPHistory struct will include additional member functions for tracking the number of BPHistory objects 
that have been created and destroyed. These functions are used to help debug memory leaks and 
other issues related to the allocation and deallocation of BPHistory objects.

The globalHistory member variable is used to store the global history of branch predictions for 
a particular hardware thread. This information can be used by the branch predictor to make more accurate 
predictions. For example, if a hardware thread has recently encountered a series of branches that were taken, 
the global history register might be set to a value like 1011 (binary), indicating that the last four branches were taken. 
The branch predictor could use this information to bias its prediction toward taken branches in the future.
*/

/** Flag for invalid predictor index */
    static const int invalidPredictorIndex = -1;

    /** Global history register. Contains as much history as specified by
     *  globalHistoryBits. Actual number of bits used is determined by
     *  globalHistoryMask and choiceHistoryMask. */
    std::vector<unsigned> globalHistory;

    /** Number of bits for the global history. Determines maximum number of
        entries in global and choice predictor tables. */
    unsigned globalHistoryBits;

    /** Mask to apply to globalHistory to access choice history table.
     *  Based on choicePredictorSize.*/
    unsigned choiceHistoryMask;

    /** Mask to control how much history is stored. All of it might not be
     *  used. */
    unsigned historyRegisterMask;

    

    /** Number of entries in the choice predictor. */
    unsigned choicePredictorSize;

    /** Number of bits in the choice predictor's counters. */
    unsigned choiceCtrBits;

    /** Array of counters that make up the choice predictor. */
    std::vector<SatCounter8> choiceCtrs;

    /** Thresholds for the counter value; above the threshold is taken,
     *  equal to or below the threshold is not taken.
     */
    unsigned choiceThreshold;
};


} // namespace branch_prediction
} // namespace gem5

#endif // __CPU_PRED_GSHARE_PRED_HH__