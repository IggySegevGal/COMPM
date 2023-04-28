/* 046267 Computer Architecture - Winter 20/21 - HW #1                  */
/* This file should hold your implementation of the predictor simulator */

#include "bp_api.h"
#define SNT 0
#define WNT 1
#define WT 2
#define ST 3

unsigned tag_mask;
unsigned hist_mask;


int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared){
            /* create mask with ones in the first lsbits <unsigned>(0)<size>(1):
            example - if size is 3 than: mask = 000000...000111 */
            tag_mask = (2 ^ tagSize) - 1; 
            hist_mask = (2 ^ historySize) - 1;
	return -1;
}

bool BP_predict(uint32_t pc, uint32_t *dst){ /////////////////// uint32_t to binary!!!!!!!!! how??~!???!?!?!?!
	return false;
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst){
	return;
}

void BP_GetStats(SIM_stats *curStats){
	return;
}

// classes btb_line and btb:
class btb { 
private:
   vector<btb_line> *btb_vector;
   //ifdef isGlobalHist {}
   unsigned global_history;
   unsigned *global_fsm_table;
public:  
   // constructors
   btb(){}
   btb(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared){
         /* init btb table - vector of btb lines: */
         btb_vector = new vector<btb_line> [btbSize];
         /* init global history if isGlobalHist is true: */
         if (isGlobalHist){
            global_history = 0;
         }
         if (isGlobalTable){ /* this table holds the fsm state for each history*/
            global_fsm_table = new fsm (2 ^ historySize);
            for (int i = 0; i < 2 ^ historySize; i++) {
               // need to check **************************************
               global_fsm_table[i] = fsm(fsmState);
            }
         }
   }
    
    // destructor
    ~btb(){}

    // getters:
   int get_();
   int get_();
   
    //method functions
    void remove_();

   };

class btb_line { 
private:
   unsigned tag;
   bool *history;
   // add tag - maybe bool array

public:  
   // constructors
   btb_line(){}
   btb_line(unsigned historySize){
      // and if isGlobalHist initialize
      //history = new bool[historySize];

   }

    // destructor
    ~btb_line(){
      delete[] history;
    }

    // getters:
    int get(); 

    // setters
    void set_(string );

}

class fsm { 
   private:
      unsigned fsmState;
   public:  
      // constructors
      fsm();
      fsm(unsigned fsm_init_state){
         this->fsmState = fsm_init_state;
      }
    
      // destructor
      ~fsm();

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
    void next_state(bool fsm_pred){
      if(this->fsmState == SNT){ // Strongly not taken - return false
         if(fsm_pred == 1){ // taken
            this->fsmState = WNT;
         }
         return;
      }
      else if (this->fsmState == WNT) { // Weakly not taken - return false 
         if(fsm_pred == 1){ // taken - move to WT
            this->fsmState = WT;
         }
         else { // not taken - move to SNT
            this->fsmState = SNT;
         }
         return;
      }
      else if (this->fsmState == WT) { // Weakly taken - return true 
         if(fsm_pred == 1){ // taken - move to ST
            this->fsmState = ST;
         }
         else { // not taken - move to WNT
            this->fsmState = WNT;
         }
         return;
      }
      else { // Strongly taken - return true 
         if(fsm_pred == 0){ // not taken - move to WT
            this->fsmState = WT;
         }
         return;
      }
      
    }

}