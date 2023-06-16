/* 046267 Computer Architecture - HW #3 */
/* Implementation (skeleton)  for the dataflow statistics calculator */

#include "dflow_calc.h"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits.h>
#include <cmath>
#include <vector>
using std::FILE;
using std::string;
using std::cout;
using std::endl;
using std::cerr;
using std::ifstream;
using std::stringstream;
using namespace std;

class inst { 
public:
InstInfo inst_info;
int depth;
int inst_idx;
int src1dep_idx;
int src2dep_idx;

   // constructors
   inst(){
   }
    
    // destructor
    ~inst(){
    }
   
    //method functions
	void init_inst(InstInfo inst_info, int inst_idx ){
		this->inst_info = inst_info;
		//this->depth = opsLatency[inst_info.opcode]; //in case we have no dependencies, the depth of the operation is 0.
		this->depth = 0;
		this->inst_idx = inst_idx;
		this->src1dep_idx = -1; //initial to dependent on ENTRY
		this->src2dep_idx = -1; //initial to dependent on ENTRY
   	}

   };
   
class ProgCtx_class { 
public:
vector<inst>  inst_vector; 
int opsLatency[MAX_OPS];


   // constructors
   ProgCtx_class(){
   }
    
    // destructor
    ~ProgCtx_class(){
    }
   
    //method functions
	void init_prog_ctx(const unsigned int opsLatency[]){
		//this->inst_vector.resize(numOfInsts);
 		for(int i=0; i<MAX_OPS;i++){
			this->opsLatency[i]=opsLatency[i];		
		}
		
		
   	}
   	
	void push_back_inst(inst inst_tmp){
		// add inst to ProgCtx_class
		this->inst_vector.push_back(inst_tmp);
	}
	
	void update_inst_depth(inst *inst_tmp){
		// we go over the old instructions, and check if there is RAW, if yes - we update the depth
		// we do it before we add curr_inst to the vector
		int src1Idx_depth = 0;
		int src2Idx_depth = 0;
		std::vector<inst>::iterator it;
		for (it = inst_vector.begin() ; it != inst_vector.end(); ++it){
            if (it->inst_info.dstIdx == int(inst_tmp->inst_info.src1Idx)){
				src1Idx_depth = this->opsLatency[it->inst_info.opcode] + (it->depth); //sum the depth of the child, and the depth of the inst opcode itself
            }
			if (it->inst_info.dstIdx == int(inst_tmp->inst_info.src2Idx)){
				src2Idx_depth = this->opsLatency[it->inst_info.opcode] + (it->depth); //sum the depth of the child, and the depth of the inst opcode itself
            }
    	}
		inst_tmp->depth = src1Idx_depth > src2Idx_depth ? src1Idx_depth : src2Idx_depth;
		//cout << "src1Idx_depth=" << src1Idx_depth << endl;
		//cout << "src2Idx_depth=" << src2Idx_depth << endl;
	}
	
	void update_inst_dependecies_idx(inst *inst_tmp){
		// we go over the old instructions, and check if there is RAW, if yes - we add the update depth
		// we do it before we add curr_inst to the vector
		int src1dep_idx = -1;
		int src2dep_idx = -1;
		std::vector<inst>::iterator it;
		for (it = inst_vector.begin() ; it != inst_vector.end(); ++it){
            if (it->inst_info.dstIdx == int(inst_tmp->inst_info.src1Idx)){
				src1dep_idx = it->inst_idx;
            }
			if (it->inst_info.dstIdx == int(inst_tmp->inst_info.src2Idx)){
				src2dep_idx = it->inst_idx;
            }
    	}
		inst_tmp->src1dep_idx = src1dep_idx;
		inst_tmp->src2dep_idx = src2dep_idx;
	}
	
	int get_depth(unsigned int theInst){
		// we go over the list of instructions, and return the depth of the desired inst
		std::vector<inst>::iterator it;
		for (it = inst_vector.begin() ; it != inst_vector.end(); ++it){
            if (it->inst_idx == int(theInst)){
				return it->depth;
            }
    	}
		return -1;
	}
	
	int get_dependecies(unsigned int theInst, int *src1DepInst, int *src2DepInst){
		// we go over the list of instructions, and return the dependecies
		std::vector<inst>::iterator it;
		for (it = inst_vector.begin() ; it != inst_vector.end(); ++it){
            if (it->inst_idx == int(theInst)){
				*src1DepInst = it->src1dep_idx;
				*src2DepInst = it->src2dep_idx;
				return 0; //success
            }
    	}
		return -1;
	}
	
	int get_total_depth(){
		// we go over the list of instructions, and return the depth 
		int max_tmp_depth = 0;
		int tmp_depth;
		std::vector<inst>::iterator it;
		for (it = inst_vector.begin() ; it != inst_vector.end(); ++it){
            tmp_depth = (it->depth) + (this->opsLatency[it->inst_info.opcode]); //sum the depth of the most "high" child, and the depth of the inst opcode imself
//cout << "(it->dept=" << it->depth << endl;
			if (tmp_depth > max_tmp_depth){
				max_tmp_depth = tmp_depth;
            }
    	}
		return max_tmp_depth;
	}
	
   };
   
ProgCtx analyzeProg(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numOfInsts) {
	ProgCtx_class *prog_ctx = new ProgCtx_class; // i need to declere it with pointer? or its ok?
	if (prog_ctx == PROG_CTX_NULL) {
		return PROG_CTX_NULL; // TODO - we need to do somthing with it?
	}
	inst curr_inst;
	prog_ctx->init_prog_ctx(opsLatency);
	for (int i=0; i < int(numOfInsts); i++){
		curr_inst.init_inst(progTrace[i], i);
		prog_ctx->update_inst_depth(&curr_inst);
		prog_ctx->update_inst_dependecies_idx(&curr_inst);
		prog_ctx->push_back_inst(curr_inst);
	}
    return prog_ctx;
}

void freeProgCtx(ProgCtx ctx) {
	ProgCtx_class* casting_ctx = (ProgCtx_class*)ctx; 
	delete casting_ctx;//TODO - check
}

int getInstDepth(ProgCtx ctx, unsigned int theInst) {
	ProgCtx_class* casting_ctx = (ProgCtx_class*)ctx; 
	int return_value;
	return_value = casting_ctx->get_depth(theInst);
    return return_value;
}

int getInstDeps(ProgCtx ctx, unsigned int theInst, int *src1DepInst, int *src2DepInst) {
	ProgCtx_class* casting_ctx = (ProgCtx_class*)ctx; 
	int return_value;
	return_value = casting_ctx->get_dependecies(theInst,src1DepInst,src2DepInst);
    return return_value;
}

int getProgDepth(ProgCtx ctx) {
	ProgCtx_class* casting_ctx = (ProgCtx_class*)ctx; 
	int return_value;
	return_value = casting_ctx->get_total_depth();
    return return_value;
}


