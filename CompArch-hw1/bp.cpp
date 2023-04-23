/* 046267 Computer Architecture - Winter 20/21 - HW #1                  */
/* This file should hold your implementation of the predictor simulator */

#include "bp_api.h"
#define SNT 0
#define WNT 1
#define WT 2
#define ST 3

int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared){
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
   bool *global_history;
public:  
   // constructors
   btb(){}
   btb(unsigned btbSize){
      btb_vector = new vector<btb_line> [ btbSize];

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
      history = new bool[historySize];

   }

    // destructor
    ~btb_line(){
      delete[] history;
    }

    // getters:
    int get(); 

    // setters
    void set_(string );

};

class fsm { 
   private:
      unsigned fsmState;
   public:  
      // constructors
      fsm();
      fsm(unsigned fsm_init_state){
         this->fsm_state = fsm_init_state;
      }
    
      // destructor
      ~fsm();

      // getters:
   bool get_state(unsigned fsm_state){
      if(this->fsm_state == SNT){ // Strongly not taken - return false
         return false;
      }
      else if (this->fsm_state == WNT) { // Weakly not taken - return false 
         return false;
      }
      else if (this->fsm_state == WT) { // Weakly taken - return true 
         return true;
      }
      else { // Strongly taken - return true 
         return true;
      }
   }
   
    // setters
    void set_state(bool fsm_pred){
      if(this->fsm_state == SNT){ // Strongly not taken - return false
         if(fsm_pred == 1){ // taken
            this->fsm_state = WNT;
         }
         return;
      }
      else if (this->fsm_state == WNT) { // Weakly not taken - return false 
         if(fsm_pred == 1){ // taken - move to WT
            this->fsm_state = WT;
         }
         else { // not taken - move to SNT
            this->fsm_state = SNT;
         }
         return;
      }
      else if (this->fsm_state == WT) { // Weakly taken - return true 
         if(fsm_pred == 1){ // taken - move to ST
            this->fsm_state = ST;
         }
         else { // not taken - move to WNT
            this->fsm_state = WNT;
         }
         return;
      }
      else { // Strongly taken - return true 
         if(fsm_pred == 0){ // not taken - move to WT
            this->fsm_state = WT;
         }
         return;
      }
      
    }

   };