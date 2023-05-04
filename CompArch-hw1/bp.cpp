/* 046267 Computer Architecture - Winter 20/21 - HW #1                  */
/* This file should hold your implementation of the predictor simulator */

#include "bp_api.h"
#include <cmath>
#include <vector>
#include <iostream>
using namespace std;
#define SNT 0
#define WNT 1
#define WT 2
#define ST 3
#define using_share_lsb 1
#define using_share_mid 2
#define not_using_share 0

unsigned tag_mask;
unsigned hist_mask;

class fsm { 
   private:
      unsigned fsmState;
   public:  
      // constructors
      fsm(unsigned fsm_init_state){
         this->fsmState = fsm_init_state;
      }
    
      // destructor
      ~fsm(){}

      // getters:
   bool get_pred(){ // get prediction according to current fsm state
      if(this->fsmState == SNT){ // Strongly not taken - return false
         return false;
      }
      else if (this->fsmState == WNT) { // Weakly not taken - return false 
         return false;
      }
      else if (this->fsmState == WT) { // Weakly taken - return true 
         return true;
      }
      else { // Strongly taken - return true 
         return true;
      }
   }
   
    // setters
    void next_state(bool taken){
      if(this->fsmState == SNT){ // Strongly not taken - return false
         if(taken == 1){ // taken
            this->fsmState = WNT;
         }
         return;
      }
      else if (this->fsmState == WNT) { // Weakly not taken - return false 
         if(taken == 1){ // taken - move to WT
            this->fsmState = WT;
         }
         else { // not taken - move to SNT
            this->fsmState = SNT;
         }
         return;
      }
      else if (this->fsmState == WT) { // Weakly taken - return true 
         if(taken == 1){ // taken - move to ST
            this->fsmState = ST;
         }
         else { // not taken - move to WNT
            this->fsmState = WNT;
         }
         return;
      }
      else { // Strongly taken - return true 
         if(taken == 0){ // not taken - move to WT
            this->fsmState = WT;
         }
         return;
      }
      
    }

};


class btb_line { 
public:  
   unsigned tag;
   unsigned history;
   fsm *fsm_table;
   uint32_t pred_dst; // predicted target address


   // constructors
   btb_line(unsigned historySize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, uint32_t tag_pc, uint32_t targetPc){ // updates only tag & pred_dst
         // initializes fsm_table and history to zero !!!!
         /* init local history if isGlobalHist is false: */
         this->tag = tag_pc;
         this->pred_dst = targetPc;
         this->history = 0;
         if (!isGlobalTable){ /* this table holds the fsm state for each history*/
            fsm_table = new fsm (pow(2, historySize));
            for (int i = 0; i < (pow(2, historySize)); i++) {
               // need to check **************************************
               fsm_table[i] = fsm(fsmState);
            }
         }
   }

    // destructor
    ~btb_line(){
      //delete[] fsm_table;
    }


};

// classes btb_line and btb:
class btb { 
public:
   vector<btb_line> *btb_vector;
   unsigned global_history;
   fsm *global_fsm_table;
   unsigned btbSize;
   bool isGlobalHist;
   bool isGlobalTable;
   int Shared;
   unsigned historySize;
   unsigned fsmState;
   unsigned flush_num;           // Machine flushes
	unsigned br_num;      	      // Number of branch instructions
	unsigned size;	
   // constructors
btb(){}
    
    // destructor
    ~btb(){
      //delete[] global_fsm_table;
    }
   
    //method functions
      void update_entry(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst, uint32_t tag_pc){
         this->br_num++; // count updates
         unsigned *history_used;
         fsm *fsm_table_used;
         unsigned fsm_entry;
         unsigned tag_entry = pc % btbSize;
         bool same_tag = true;
         // not in table - create line:
         if ((btb_vector[tag_entry]).empty()){ // entry is null
            // init line
btb_vector->insert(btb_vector->begin() + tag_entry, btb_line( historySize, fsmState, isGlobalHist, isGlobalTable, tag_pc, targetPc));
            same_tag = false;
         }
         else if (((*btb_vector)[tag_entry]).tag != tag_pc){ // need to delete line (different tag)
            // destroy line:
            btb_vector->erase(btb_vector->begin() + tag_entry);
            // init new line:
btb_vector->insert(btb_vector->begin() + tag_entry, btb_line( historySize, fsmState, isGlobalHist, isGlobalTable, tag_pc, targetPc));
            same_tag = false;
            //if (taken) {
            //   this->flush_num++;
            //}
         }

         /* first step - choose local or global history */
         if (isGlobalHist){ // choose global history
            history_used = &this->global_history;
         }
         else { // local history
            history_used = &(*btb_vector)[tag_entry].history;
         }
         /* second step - find fsm_table */
         if (isGlobalTable){ // choose global fsm_table
            fsm_table_used = this->global_fsm_table;
         }
         else { // local fsm_table
            fsm_table_used = (*btb_vector)[tag_entry].fsm_table;
         }
         /* third step - check if shared or not */
         if (Shared == using_share_lsb){
            fsm_entry = *history_used ^ (pc&hist_mask);
         }
         else if (Shared == using_share_mid){
            fsm_entry = *history_used ^ ((pc>>16)&hist_mask);
         }
         else { // not_using_share
            fsm_entry = *history_used;
         }

         // update history and fsm:
         if (same_tag){ // check target prediction
            if (targetPc != pred_dst) { // wrong prediction
               (*btb_vector)[tag_entry].pred_dst = targetPc;
            }
         }

         // update history:
         unsigned hist_val = *history_used;
         hist_val = (hist_val << 1);
         if (taken){
            hist_val += 1;
         }
         *history_used = hist_val;
         
         // update fsm:
         fsm_table_used[fsm_entry].next_state(taken);
         return;
      }


    bool find_entry_return_pred(uint32_t pc, uint32_t tag_pc, uint32_t *dst){
      unsigned history_used;
      fsm *fsm_table_used;
      unsigned fsm_entry;
      unsigned tag_entry = pc % btbSize;
      // not in table - return pc + 4 and false
      if (btb_vector[tag_entry].empty()){ // entry is null
         *dst = (pc<<2)+4;
         return false; // not found
      }
      // in table - check if taken:
      /* first step - choose local or global history */
      if (isGlobalHist){ // choose global history
         history_used = this->global_history;
      }
      else { // local history
         history_used = (*btb_vector)[tag_entry].history;
      }
      /* second step - find fsm_table */
      if (isGlobalTable){ // choose global fsm_table
         fsm_table_used = this->global_fsm_table;
      }
      else { // local fsm_table
         fsm_table_used = (*btb_vector)[tag_entry].fsm_table;
      }
      /* third step - check if shared or not */
      if (Shared == using_share_lsb){
         fsm_entry = history_used ^ (pc&hist_mask);
      }
      else if (Shared == using_share_mid){
         fsm_entry = history_used ^ ((pc>>16)&hist_mask);
      }
      else { // not_using_share
         fsm_entry = history_used;
      }
      /* last step - get prediction */
      if(fsm_table_used[fsm_entry].get_pred()){ // taken
         *dst = (*btb_vector)[tag_entry].pred_dst;
         return true; // found and taken
      }
         *dst = (pc<<2)+4;
         return false; // not taken
      }
   };


/* -------------------------------------------------------------------- */

btb *btb_table = new btb[1];

int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared){
            /* init btb table */
            btb_table[0] = btb();
		         /* init btb table - vector of btb lines: */
         btb_table->btb_vector = new vector<btb_line> [btbSize];
         btb_table->btbSize = btbSize;
         btb_table->isGlobalHist = isGlobalHist;
         btb_table->isGlobalTable = isGlobalTable;
         btb_table->Shared = Shared;
         btb_table->historySize = historySize;
         btb_table->fsmState = fsmState;
         btb_table->flush_num = 0;
         btb_table->br_num = 0;

         /* init global history if isGlobalHist is true: */
         if (isGlobalHist){
            btb_table->global_history = 0;
         }
         if (isGlobalTable){ /* this table holds the fsm state for each history*/
            btb_table->global_fsm_table = new fsm (pow(2, historySize));
            for (int i = 0; i < pow(2, historySize); i++) {
               // need to check **************************************
               btb_table->global_fsm_table[i] = fsm(fsmState);
            }
         }

            /* create mask with ones in the first lsbits <unsigned>(0)<size>(1):
            example - if size is 3 than: mask = 000000...000111 */
            tag_mask = pow(2, tagSize) - 1; 
            hist_mask = pow(2, historySize) - 1;

	return 0;
}

bool BP_predict(uint32_t pc, uint32_t *dst){ 
   /* if pc tag is in btb -
   if taken - return true, dst->pred_dst
   if not taken - return false, dst-> pc+4  */
   /* if pc tag is not in btb return false, dst-> pc+4 */
   uint32_t tag_pc = ((pc>>2) / btb_table->btbSize) & tag_mask;
	return btb_table->find_entry_return_pred( pc>>2 ,tag_pc , dst);
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst){
   uint32_t tag_pc = ((pc>>2) / btb_table->btbSize) & tag_mask;

   // check if prediction was correct:
   if((!taken && ((pc + 4) != pred_dst)) || (taken && (targetPc != pred_dst))){ 
		btb_table->flush_num++;
	}

   // update entry:
	btb_table->update_entry(pc, targetPc, taken, pred_dst, tag_pc);
   return;
}

void BP_GetStats(SIM_stats *curStats){
   curStats->flush_num = btb_table->flush_num;
   curStats->br_num = btb_table->br_num;
   //curStats->size = callfunction-------------------------------------------------;
	return;
}

