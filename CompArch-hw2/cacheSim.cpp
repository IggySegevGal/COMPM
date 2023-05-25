/* 046267 Computer Architecture - Winter 20/21 - HW #2 */

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
//using std::vector;
//using std::cmath; // need to checkkkkkkkkkkkkkkkkk

/* statistics struct */

/* A structure to return information about the currect simulator state */
typedef struct {
	double L1Miss;
	double L1Hit;
	double L2Miss;
	double L2Hit;
	double AccTime;
	double AccNum;
} SIM_stats;
/* ----- global stats: */
SIM_stats stats;

/* ---------------------------------------------------- cache_line class ------------------------------------------------ */
class cache_line { 
public:
unsigned long int address;
unsigned long int tag;
bool dirty;
unsigned set;


   // constructors
   cache_line(){
	this->address = ULONG_MAX; // init to maximum value of unsigned long int, assuming addersses are aligned to 4 
	this->tag = ULONG_MAX; // init to maximum value of unsigned long int, assuming block size is never zero bits
	this->dirty = 0;
   }
    
    // destructor
    ~cache_line(){
    }
   
    //method functions
	void init_line(unsigned long int address, char operation,unsigned sets,unsigned BSize){
		// calculate setBits from sets:
		unsigned setBits = log2(sets);
		this->set = (address >> BSize) % sets;
		this->dirty = 0;
		if (operation == 'w'){
			this->dirty = 1;
		}
		this->address = address;
		this->tag = address >> (setBits+BSize); // shift right to remove bits of block size, set size
		
   	}

   };

/* ---------------------------------------------- cache_class ---------------------------------------- */
class cache_class { 
public:
unsigned MemCyc;
unsigned BSize;
unsigned LSize;
unsigned LAssoc;
unsigned LCy;
unsigned WrAlloc;
unsigned ways;
unsigned sets;
std::vector<std::vector<cache_line> > cache_table; // sets X ways [cache_line] 
// vector<unsigned> lru_vec; // for each set

   // constructors
	cache_class(){
	}
    
    // destructor
    ~cache_class(){
    }
   
    //method functions
   	void init_class(unsigned MemCyc, unsigned BSize , unsigned LSize, unsigned LAssoc, unsigned LCyc , unsigned WrAlloc){
			this->MemCyc = MemCyc;
			this->BSize = BSize;
			this->LSize = LSize;
			this->LAssoc = LAssoc;
			this->LCy = LCyc;
			this->WrAlloc = WrAlloc;
			this->ways = pow(2,LAssoc);
			this->sets = (pow(2,LSize) / pow(2,BSize)) / ways;
			// init a 1D cache: sets -> number of rows, ways -> number of columns init empty;
			this->cache_table.resize(sets);//, vector<cache_line>(ways)); // only init sets, not ways
			//this->lru_vec = lru_vec.resize(sets);
	}

	//method functions
	cache_line create_cache_line(unsigned long int address, char operation){
		// create cache_line object and initialize it:
		cache_line tmp_cache_line;
 		tmp_cache_line.init_line( address, operation, this->sets, this->BSize);
		return tmp_cache_line;
	}

   	void push_back_line(cache_line line){
		// push back to cache:
		this->cache_table[line.set].push_back(line);
	
	}

	bool erase_cache_line(cache_line line){ 
		// erase line:
		bool res = false;
		std::vector<cache_line>::iterator it;
        for (it = cache_table[line.set].begin() ; it != cache_table[line.set].end(); ++it){
            if (it->tag == line.tag){
				res = it->dirty;
                cache_table[line.set].erase(it);
				return res; // is dirty
            }
    	}
		return false;
	}

	void update_LRU(cache_line line){ // erase and push back
		// erase line:
		if(erase_cache_line(line)){ // line was dirty
			line.dirty = 1;
		}
		// push back to cache:
		push_back_line( line);
		return;
	}

	int is_exist(cache_line line){ //returns index of line in cache if exists, else -1
		// check line:
		vector<cache_line>::iterator it;
		int i = 0;
        for (it = cache_table[line.set].begin() ; it != cache_table[line.set].end(); ++it){
		//cout << "hey" << endl;
            if (it->tag == line.tag){
                cache_table[line.set].erase(it);
				return i;
            }
			i++;
    	}
		//cout << "bye" << endl;
		return -1;
	}


   };
/* ---------------------------------------------------- globals: -------------------------------------------------- */
cache_class L1;
cache_class L2;
/* --------------------------------------------------- general function ------------------------------------------- */
void handle_command(unsigned long int address, char operation){
// prepare new cache lines:
	cache_line L1_newline = L1.create_cache_line(address, operation);
	cache_line L2_newline = L2.create_cache_line(address, operation);
	
	// update LRU == (erase and pushback, because most recently used element is last)
	
	if (L1.is_exist(L1_newline) != -1){ // hit L1 (write & read)
		stats.L1Hit++;
		stats.AccTime += L1.LCy;
		L1.update_LRU( L1_newline);// update LRU, update Dirty if write
	} 
	else if ((L1.is_exist(L1_newline) == -1) && (L2.is_exist(L2_newline) != -1)) { // miss L1 hit L2
		stats.L1Miss++;
		stats.L2Hit++;
		stats.AccTime += L1.LCy + L2.LCy;
		if ((L1.WrAlloc && operation=='w') || (operation=='r')){ // L1 WA: (WA && operation=='W') || (operation=='R')
		
			L2.update_LRU(L2_newline); // update LRU L2 with new
			if (L1.cache_table[L1_newline.set].size() == L1.ways){ // if L1 set is full:
				cache_line tmp_line = *L1.cache_table[L1_newline.set].begin();
				cache_line tmp_lineL2 = L2.create_cache_line(tmp_line.address, operation);
				if(tmp_line.dirty != 1){//not dirty:
					// L1.cache_table[tmp_line.set].erase(tmp_line); // remove first element in set from L1
					L1.erase_cache_line(tmp_line); // remove first element in set from L1
				}
				else{ //dirty: 
					// write first element in set from L1 to L2 and then remove from L1 update LRU L2 with removed
					
					//L1.cache_table[tmp_line.set].erase(tmp_line); // remove first element in set from L1
					L1.erase_cache_line(tmp_line); // remove first element in set from L1
					L2.update_LRU( tmp_lineL2);// update LRU, update Dirty if write
					//if(L2.is_exist(tmp_lineL2)){ // insert old L1 element to L2
					//	L2.update_LRU( tmp_lineL2);// update LRU, update Dirty if write
					//}
					//else{
					//	if(L2.cache_table[L2_newline.set].size() == L2.ways){ // L2 is full
					//		cache_line tmp_L1 = L1.create_cache_line(L2.cache_table[L2_newline.set].begin()->address, operation);
					//			if(L1.is_exist(tmp_L1)){ // if in L1 - remove from L1
					//				L1.erase_cache_line(tmp_L1);
					//			}
					//		L2.cache_table[L2_newline.set].erase(L2.cache_table[L2_newline.set].begin()); // remove first element from L2 LRU 
					//	}
					//	L2.push_back_line(tmp_lineL2); // push back to L2
					//}
				}
			}

			L1.push_back_line(L1_newline); // insert line to L1, update LRU L1
			
		}
		else{ // L1 NWA: else(NWA)
		
			// L1 nothing
			// update Dirty L2 if write
			// update LRU L2 
			L2.update_LRU( L2_newline);
		}
	}
	else if((L1.is_exist(L1_newline) == -1) && (L2.is_exist(L2_newline) == -1)){ // miss L1 miss L2
		stats.L1Miss++;
		stats.L2Miss++;
		stats.AccTime += L1.LCy + L2.LCy + L2.MemCyc;
			if ((operation =='r') || (L1.WrAlloc && L2.WrAlloc) || (L1.WrAlloc && !L2.WrAlloc)){ // L1 WA L2 WA  or // L1 WA L2 NWA or (operation='R')
				if(L2.cache_table[L2_newline.set].size() == L2.ways){ // if L2 set is full:
					//not dirty: remove first element in set from L2
					 cache_line tmp_L1 = L1.create_cache_line(L2.cache_table[L2_newline.set].begin()->address, operation);
							if(L1.is_exist(tmp_L1)){ // if in L1 - remove from L1
								L1.erase_cache_line(tmp_L1);
							}
					L2.cache_table[L2_newline.set].erase(L2.cache_table[L2_newline.set].begin());
					
				}
				L2.push_back_line(L2_newline); //insert line to L2 and update LRU L2
				if (L1.cache_table[L1_newline.set].size() == L1.ways){ // if L1 set is full:
					cache_line tmp_line = *L1.cache_table[L1_newline.set].begin();
					cache_line tmp_lineL2 = L2.create_cache_line(tmp_line.address, operation);
					if(tmp_line.dirty != 1){//not dirty:
						L1.erase_cache_line(tmp_line); // remove first element in set from L1						
						//L1.cache_table[L1_newline.set].erase(tmp_line); // remove first element in set from L1
					}
					else{ //dirty: 
						// write first element in set from L1 to L2 and then remove from L1 update LRU L2 with removed
						
						L1.erase_cache_line(tmp_line); // remove first element in set from L1
						//L1.cache_table[L1_newline.set].erase(tmp_line); // remove first element in set from L1
						//if(L2.cache_table[L2_newline.set].size() == L2.ways){ // L2 is full
						//	cache_line tmp_L1 = L1.create_cache_line(L2.cache_table[L2_newline.set].begin()->address, operation);
						//	if(L1.is_exist(tmp_L1)){ // if in L1 - remove from L1
						//		L1.erase_cache_line(tmp_L1);
						//	}
						//	L2.cache_table[L2_newline.set].erase(L2.cache_table[L2_newline.set].begin()); // remove first element from L2 LRU 
						//}
						L2.update_LRU( tmp_lineL2);// update LRU, update Dirty if write
						//if(L2.is_exist(tmp_lineL2)){ // insert old L1 element to L2
						//	L2.update_LRU( tmp_lineL2);// update LRU, update Dirty if write
						//}
						//else{
						//	L2.push_back_line(tmp_lineL2); // push back to L2
						//}
					}
				}

				L1.push_back_line(L1_newline); // insert line to L1, update LRU L1
			} 
			// else if((!L1.WrAlloc && L2.WrAlloc)){ // L1 NWA L2 WA
			// 	if(L2.cache_table[L1_newline.set].size() == L2.ways){ // if L2 set is full:
			// 		//not dirty: remove first element in set from L2
					 
			// 		if (L2.cache_table[L1_newline.set].begin()->dirty == 1){ //dirty: write first element in set from L2 to mem and then remove from L2
			// 			// mem access
			// 		}
			// 		L2.cache_table[L1_newline.set].erase(L2.cache_table[L1_newline.set].begin());
					
			// 	}
			// 	L2.push_back_line(L1_newline); //insert line to L2 and update LRU L2			
			// }
			else if((!L1.WrAlloc && !L2.WrAlloc)){ // L1 NWA L2 NWA
				//write in mem
			}

			
				

	}

	

}


/* ----------------------------------------------------- main ------------------------------------------------------- */
int main(int argc, char **argv) {

	if (argc < 19) {
		cerr << "Not enough arguments" << endl;
		return 0;
	}

	// Get input arguments

	// File
	// Assuming it is the first argument
	char* fileString = argv[1];
	ifstream file(fileString); //input file stream
	string line;
	if (!file || !file.good()) {
		// File doesn't exist or some other error
		cerr << "File not found" << endl;
		return 0;
	}

	unsigned MemCyc = 0, BSize = 0, L1Size = 0, L2Size = 0, L1Assoc = 0,
			L2Assoc = 0, L1Cyc = 0, L2Cyc = 0, WrAlloc = 0;

	for (int i = 2; i < 19; i += 2) {
		string s(argv[i]);
		if (s == "--mem-cyc") {
			MemCyc = atoi(argv[i + 1]);
		} else if (s == "--bsize") {
			BSize = atoi(argv[i + 1]);
		} else if (s == "--l1-size") {
			L1Size = atoi(argv[i + 1]);
		} else if (s == "--l2-size") {
			L2Size = atoi(argv[i + 1]);
		} else if (s == "--l1-cyc") {
			L1Cyc = atoi(argv[i + 1]);
		} else if (s == "--l2-cyc") {
			L2Cyc = atoi(argv[i + 1]);
		} else if (s == "--l1-assoc") {
			L1Assoc = atoi(argv[i + 1]);
		} else if (s == "--l2-assoc") {
			L2Assoc = atoi(argv[i + 1]);
		} else if (s == "--wr-alloc") {
			WrAlloc = atoi(argv[i + 1]);
		} else {
			cerr << "Error in arguments" << endl;
			return 0;
		}
	}

	/* init caches: */
	L1.init_class( MemCyc,  BSize ,  L1Size,  L1Assoc,  L1Cyc ,  WrAlloc);
	L2.init_class( MemCyc,  BSize ,  L2Size,  L2Assoc,  L2Cyc ,  WrAlloc);
	/* init stats */
	stats.L1Miss = 0 ;
	stats.L1Hit = 0 ;
	stats.L2Miss = 0 ;
	stats.L2Hit = 0 ;
	stats.AccTime = 0 ;
	stats.AccNum = 0 ;

	while (getline(file, line)) {
		stats.AccNum++; //count commands
		stringstream ss(line);
		string address;
		char operation = 0; // read (R) or write (W)
		if (!(ss >> operation >> address)) {
			// Operation appears in an Invalid format
			cout << "Command Format error" << endl;
			return 0;
		}

		// DEBUG - remove this line
		//cout << "operation: " << operation;

		string cutAddress = address.substr(2); // Removing the "0x" part of the address

		// DEBUG - remove this line
		//cout << ", address (hex)" << cutAddress;

		unsigned long int num = 0;
		num = strtoul(cutAddress.c_str(), NULL, 16);

		// DEBUG - remove this line
		//cout << " (dec) " << num << endl;
		 handle_command( num, operation);

	}

	double L1MissRate = stats.L1Miss/ (stats.L1Miss+stats.L1Hit);
	double L2MissRate = stats.L2Miss/ (stats.L2Miss+stats.L2Hit);
	double avgAccTime = stats.AccTime/stats.AccNum;

	printf("L1miss=%.03f ", L1MissRate);
	printf("L2miss=%.03f ", L2MissRate);
	printf("AccTimeAvg=%.03f\n", avgAccTime);

	return 0;
}
