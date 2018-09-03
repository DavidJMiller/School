/*##################################################################
#                            David Mille                           #
#                             Program 4                            #
#                         ----------------                         #
#     Commands to compile: (must have config.txt in directory)     #
#        With reservation stations, RB, and regs status :          #
#        1st time) g++ main.cpp && ./a.out -v < trace.dat          #
#             all other times) ./a.out -v <trace.dat               # 
#       Without reservation stations, RB, and regs status :        #
#          1st time) g++ main.cpp && ./a.out < trace.dat           #
#              all other times) ./a.out < trace.dat                #
##################################################################*/

#include <iostream> 
#include <iomanip>        
#include <algorithm>     /* Sort regs alphabetically */
#include <set>           /* Keep track of instrs and rs without redundancy */
#include <fstream>       /* File I/O */
#include <stdio.h>       /* char* comparison */
#include <string.h>      /* char* comparison */
#include <sstream>       /* Tokenize lines of files */
#include <vector>        /* MIPS instruction data struct */

#define CONFIG_FILE (char*)"config.txt"    /* Change to name of config file */

using namespace std;

/*=================================FRAMEWORK==================================*/

/* Struct that stores all nexessary information for instructions */
struct instruction
{
    char op[32];                   /* Op code */
    string regs[3], full_line;     /* Sest, source1, source 2 in that order */
    int num,                       /* Instruction number (top is first) */
        latency,                   /* Num of cycles needed to carry out exec */
        issue,                     /* Cycle instr was issued */
        exec_start,                /* Cycle instr stared exec */
        exec_end,                  /* Cycle instr ended exec */
        mem_read,                  /* Cycle instr read from mem */
        write_res,                 /* Cycle instr wrote result to mem */
        commit;                    /* Cycle instr was commited */
    char state;                    /* State of instruction */
    int RBS;                       /* ROB entry if instr kicked from rs */

    /* Default constructor */
    instruction()
    {
        op[0] =  '\0';
        regs[0] = ""; 
        regs[1] = "";
        regs[2] = "";
        num = 0;
        latency = 0;
        issue = 0;
        exec_start = 0;
        exec_end = 0;
        mem_read = 0;
        write_res = 0;
        commit = 0;
        state = '\0'; 
    }

    /* Returns true if load instruction, false otherwise */
    bool is_load() 
    {
        return !(strcmp(this->op, "LW") && strcmp(this->op, "L.S"));
    }

    /* Returns true if store instruction, false otherwise */
    bool is_store() 
    {
        return !(strcmp(this->op, "SW") && strcmp(this->op, "S.S"));
    }

    /* Returns true if int add or sub instruction, false otherwise */
    bool is_int() 
    {
        return !(strcmp(this->op, "DADD") && strcmp(this->op, "DSUB"));
    }

    /* Returns true if branch instruction, false otherwise */
    bool is_branch() 
    {
        return !(strcmp(this->op, "BEQ") && strcmp(this->op, "BNE"));
    }

    /* Returns true if fp add or fp sub instruction, false otherwise */
    bool is_fp_add_or_sub() 
    {
        return !(strcmp(this->op, "ADD.S") && strcmp(this->op, "SUB.S"));
    }

    /* Returns true if fp mul or fp div instruction, false otherwise */
    bool is_fp_mul_or_div()
    {
        return !(strcmp(this->op, "MUL.S") && strcmp(this->op, "DIV.S"));
    }

    /* Sets the state of the instruction */
    void set_state(char new_state) 
    {
        this->state = new_state;
    }

    /* Sets how many cycles instruction takes to execute */
    void set_latency(int *latencies) 
    {
        if (!strcmp(this->op, "ADD.S"))
            this->latency = latencies[0];
        else if(!strcmp(this->op, "SUB.S"))
            this->latency = latencies[1];
        else if(!strcmp(this->op, "MUL.S"))
            this->latency = latencies[2];
        else if(!strcmp(this->op, "DIV.S"))
            this->latency = latencies[3];
        else
            this->latency = 1;
    }
};

/* Struct that stores all necessary information for reservation stations */
struct reservation_station
{
    instruction *current_instr;                /* Points to inst in rs */
    int remaining_cycles,                      /* Cycles left for exec */
        rob_spot;                              /* Spot in ROB */
    bool busy,                                 /* Indicates if rs is in use */ 
         skip;                                 /* Flag for fp instrs */
    string qj,                                 /* Qj dependency */ 
           qk;                                 /* Qk dependency */

    /* Default constructor */
    reservation_station()
    {
        remaining_cycles = 0;
        current_instr = NULL;
        busy = false;    
        skip = false;  
        rob_spot = 0;
    }

    /* Reserve a spot in reservation stations */
    void reserve(instruction *instr, int n, vector<instruction *> &v, int x)
    {
        /* Keep track of instr kicked out that hasnt been committed */
        if(this->current_instr != NULL){
            v.push_back(this->current_instr);
            this->current_instr->RBS = x;
        }

        this->current_instr = instr;
        this->remaining_cycles = instr->latency;
        this->busy = true;
        this->skip = false;
        this->rob_spot = n+1;
        this->qj = '\0';
        this->qk = '\0';
    }

    /* Return state of instr in rs */
    char get_state()
    {
        return this->current_instr->state;
    }

    /* Returns if rs is available, returns false otherwise */
    bool is_available()
    {
        return this->busy;
    }

    /* Unreserve rs (tell program its not busy) */
    void unreserve()
    {
        this->busy = false;      
    }
};

/* Struct that stores all necessary information for Reorder Buffer */
struct ROB_entry
{
    bool busy,                        /* Indiciates if ROB spot is busy */
         val_available,               /* Indicates if val is ready to be used */
         skipped;                     /* Flag for printing (executing/ed) */     
    instruction *instr;               /* Pointer to instr in ROB spot */
    char state;                       /* State of ROB spot */
    char dest[4];                     /* Dest reg of ROB spot */

    /* Default constructor */
    ROB_entry()
    {
        busy = false;
        val_available = false;
        instr = NULL;
        state = '\0';
        dest[0] = '\0';
        skipped = false;
    }
};

int buffers[5], latencies[4];            /* Hold config and trace data */
vector<instruction> data;                /* Hold instrs (v[0] is first instr) */
vector<instruction *> to_be_commited;    /* Hold instrs kicked from rs */
int ROB_delays = 0,                      /* Reorder buffer delays */
    RS_delays = 0,                       /* Reservation station delays */
    DMC_delays = 0,                      /* Data memory conflicts */
    TD_delays = 0;                       /* True depend delays */

/* Function prototypes; refer to declerations at end for information */
void read_config_file(char*);
void read_instructions();
void print_stats();
reservation_station **create_reserves();
void init_reorder_buffer(ROB_entry *);
instruction *fetch_instr(vector<instruction> &, int);
bool assign_buffers(instruction *, reservation_station **, ROB_entry *);
void set_dependency(instruction *, reservation_station **,ROB_entry*, int, int);
void remove_dependencies(reservation_station **, set<int> &);
void remove_reservations(reservation_station **, vector<int> &);
void tick(reservation_station **, ROB_entry *, int);
bool can_commit(int);
bool const done(vector<instruction> &);

/* Prints reservation station and ROB at each cycle */
void print_rs(reservation_station **, int);
void print_rob(ROB_entry *);
void print_reg_staus(ROB_entry *rob);

/* ===============================END FRAMEWORK============================== */


/* ==================================MAIN==================================== */

int main(int argc, char *argv[]) 
{
    cout << "Configuration" << endl;
    cout << "-------------" << endl;

    /* Get config data and instructions from files */
    read_config_file(CONFIG_FILE);
    read_instructions();

    /* Main data structs for reservation stations and ROB */
    reservation_station **RS = create_reserves();
    ROB_entry ROB[buffers[4]];
    init_reorder_buffer(ROB);

    /* Forward simulation (all is initialized to empty and instrs are fed in) */
    int cycle = 0;
    while(!done(data))
    {   
        /* At each cycle, always increment counter and initialize not stall */
        cycle++;
        bool stall = false;

        /* Execute a cycle */
        tick(RS, ROB, cycle);

        /* Fetch an instruction if there are still some left */
        instruction *fetch = fetch_instr(data, cycle);
       
        if(fetch != NULL)
        {
            /* Theres a spot in a reservation station and ROB */
            if(assign_buffers(fetch, RS, ROB))
            {
                fetch->set_state('i');
                fetch->issue = cycle;
            }
            /* Stall if we couldnt find spots */
            else
            {
                stall = true;
            }
        }

        /* Print cycle detils if compiler flag is used */
        if(argc == 2 && !strcmp(argv[1],"-v"))
        {
            print_rs(RS, cycle);
            print_rob(ROB);
            print_reg_staus(ROB);
        }

        /* Prevent infinite loop */
        int MAX_CYCLES = 50;
        if(cycle > MAX_CYCLES)
        {
            cout << "Preventing infinite loop : Exiting now." << endl;
            cout << "If program is supposed to exceed " << MAX_CYCLES;
            cout << " cycles please change value at the end of main." << endl;
            return 0;
        }
    }

    cout << "                    Pipeline Simulation                    \n";
    cout << "-----------------------------------------------------------\n";
    cout << "                                      Memory Writes        \n";
    cout << "     Instruction      Issues Executes  Read  Result Commits\n";
    cout << "--------------------- ------ -------- ------ ------ -------\n";
    print_stats();

    /* Delete dynamically allocated reservation stations */
    for(int i = 0; i < 3; i++)
        delete[] RS[i];
    delete[] RS;

    return 0;
}

/* ================================END MAIN================================== */


/* ================================FUNCTIONS================================= */

/* Read/print config file and extract config info in buffers[] & latencies[] */
void read_config_file(char* file_name)
{
    ifstream f;
    f.open(file_name);

    /* Parse config file line by line and store the parameters */
    string line, word;
    int val, i = 0;
    while(getline(f, line))
    {   
        if(line.size() == 0)
            continue;

        stringstream linestream(line);
        while(!linestream.eof())
            linestream >> word;
        
        /* Found an integer at the end of the line */
        if(stringstream(word) >> val) 
        {
            i < 5 ? buffers[i] = val:latencies[i-5] = val; 
            i++;
        }   
    }

    f.close();

    /* Print what was read from config file */
    cout << "buffers:" << endl;
    cout << setw(13) << "eff addr: " << buffers[0] << endl;
    cout << setw(13) << "fp adds: " << buffers[1] << endl;
    cout << setw(13) << "fp muls: " << buffers[2] << endl;
    cout << setw(13) << "ints: " << buffers[3] << endl;
    cout << setw(13) << "reorder: " << buffers[4] << endl << endl;
    cout << "latencies:" << endl;
    cout << setw(11) << "fp add: " << latencies[0] << endl;
    cout << setw(11) << "fp sub: " << latencies[1] << endl;
    cout << setw(11) << "fp mul: " << latencies[2] << endl;
    cout << setw(11) << "fp div: " << latencies[3] << endl;

    return;
}

/* Read MIPS instructions from stdin and parse them */
void read_instructions()
{
    string reg, line;
    int i_num = 0;

    /* While there are still instrs to parse */
    while(getline(cin, line))
    {
        instruction temp;
        temp.full_line = line;
        stringstream linestream(line);
        linestream >> temp.op;

        /* LW or L.S instruction */
        if(temp.is_load())
        {
            getline(linestream, line, ',');
            line.erase(0,line.find_first_not_of(' '));
            temp.regs[0].assign(line);

            getline(linestream, line, '(');

            getline(linestream, line, ')');
            temp.regs[1].assign(line);
        }   
        /* SW or S.S instruction */
        else if(temp.is_store())
        {
            getline(linestream, line, ',');
            line.erase(0,line.find_first_not_of(' '));
            temp.regs[1].assign(line);

            getline(linestream, line, ':');
            temp.regs[0].assign(line);
        }
        /* Instruction that is not LW, L.S, SW, or S.S */
        else 
        {
            int i = 0;
            while(getline(linestream, line, ','))
            {
                line.erase(0,line.find_first_not_of(' '));
                temp.regs[i].assign(line);
                i++;
            };
        }

        /* Set instr num and latency and push on main instr data struct */
        temp.num = i_num;
        temp.set_latency(latencies);
        /* Store instruction */
        data.push_back(temp);
        i_num++;
    }

    return;
}

/* Initialize the main reservation station data struct */
reservation_station **create_reserves()
{
    /*  
    rs[0] = {effaddr1, ....}
    rs[1] = {fpadd1, ....}
    rs[2] = {fpmul, ....}
    rs[3] = {int1, ...}
    */
    reservation_station** result = new reservation_station*[4];

    for(int i = 0; i < 4; i++)
    {
        result[i] = new reservation_station[buffers[i]];
        reservation_station empty;
        for(int j = 0; j < buffers[i]; j++)
        {
            result[i][j] = empty;
        }
    }

    return result;
}

/* Initialize the main ROB data struct with empty spots */
void init_reorder_buffer(ROB_entry *r)
{
    for(int i = 0; i < buffers[4]; i++)
    {
        ROB_entry empty;
        r[i] = empty;
    }

    return;
}

/* Fetches an instruction to be issued, returns empty instr if no spots */
instruction *fetch_instr(vector<instruction> &v, int cycle)
{
    vector<instruction>::iterator itr = v.begin();
    for(itr; itr != v.end(); ++itr)
    {
        /* Found instr not read or was fetched but wasnt assigned to rs/ROB */
        if(itr->state == '\0' || itr->state == 'f')
        {
            itr->set_state('f');
            return &*itr;
        }
    }

    return NULL;
}

/* Takes an instruction an assigns it to ROB and reservation station */
bool assign_buffers(instruction* ins, reservation_station **rs, ROB_entry* rob)
{
    /* Check for appropriate available reservation station */
    int rs_i, rs_j = -1, rob_i = -1, qj, qk;
    bool found_empty = false, rob_found = false;

    /* Determine type of reservation station to go to */
    if(ins->is_load() || ins->is_store())
        rs_i = 0;
    else if(ins->is_fp_add_or_sub())
        rs_i = 1;
    else if(ins->is_fp_mul_or_div())
        rs_i = 2;
    else
        rs_i = 3;

    /* Check for not busy rs */
    for(int j = 0; j < buffers[rs_i]; j++)
    {
        if(rs[rs_i][j].busy == false)
        {
            rs_j = j;
            break;
        }
    }

    /* All rs are busy, this is a reservation station delay */
    if(rs_j == -1)
        RS_delays++;

    /* Check for available ROB entry */
    for(int i = 0; i < buffers[4]; i++)
    {
        /* Found an empty spot (was never assigned) */
        if(rob[i].instr == NULL)
        {
            rob_i = i;
            rob_found = true;
            found_empty = true;
            break;
        }
    }

    /* Didnt find an empty ROB spot, check for non busy spot */
    if(!found_empty)
    {
        for(int i = 0; i < buffers[4]; i++)
            {
            if(rob[i].busy == false)
            {
                rob_found = true;
                rob_i = i;
                break;
            }
        }
    }

    /* Couldnt find an empty/non busy ROB spot, this is Reorder buffer delay */
    if(!rob_found)
        ROB_delays++;

    /* We found both a ROB spot and an approriate reservation station */
    if(rob_i != -1 & rs_j != -1)
    {
        /* Reserve a reservation station */
        rs[rs_i][rs_j].reserve(ins, rob_i, to_be_commited, 
                               rs[rs_i][rs_j].rob_spot);

        /* Assign ROB spot */
        rob[rob_i].busy = true;
        rob[rob_i].instr = ins;
        rob[rob_i].state = 'i';
        rob[rob_i].val_available = false;
        strcpy(rob[rob_i].dest,ins->regs[0].c_str());

        /* Set Qj and Qk for new entry */
        set_dependency(ins, rs, rob, rs_i, rs_j);

        return true;
    }
    else
    {
        return false;
    }
}

/* Determine Qj and Qk dependencies for reservation station entries */
void set_dependency(instruction * ins, reservation_station **rs, ROB_entry *rob,
                    int row, int col)
{
    /* Fp, Int, or branch instructions */
    if(!ins->is_store() && !ins->is_load())
    {
        /* Check if there are reg vals needed that arent ready */
        for(int i = 0; i < buffers[4]; i++)
        {
            /* Bunch of string crap for comparing */
            stringstream ss;
            string res;
            bool found1 = !strcmp(rob[i].dest,ins->regs[1].c_str());
            bool val1_available = rob[i].val_available;
            bool found2 = !strcmp(rob[i].dest,ins->regs[2].c_str());
            bool val2_available = rob[i].val_available;
            if(found1 && !val1_available)
            {
                ss << i + 1;
                ss >> res;
                rs[row][col].qj.assign(res);
            }
            if(found2 && !val2_available)
            {
                ss << i + 1;
                ss >> res;
                rs[row][col].qk.assign(res);
            }
        }
    }

    /* Store instructions */
    if(ins->is_store())
    {
        /* Check if there is a reg val needed that isnt ready */
        for(int i = 0; i < buffers[4]; i++)
        {
            /* Bunch of string crap for comparing */
            stringstream ss;
            string res;
            bool found = !strcmp(rob[i].dest,ins->regs[1].c_str());
            bool val_available = rob[i].val_available;
            if(found && !val_available)
            {
                ss << i + 1;
                ss >> res;
                rs[row][col].qk.assign(res);
                cout << rs[row][col].qj << " -- " << rs[row][col].qk << endl;
            }
        }
    }

    /* Load instructions */
    if(ins->is_load())
    {
        /* Check if there is a reg val needed that isnt ready */
        for(int i = 0; i < buffers[4]; i++)
        {
            /* Bunch of string crap for comparing */
            stringstream ss;
            string res;
            bool found = !strcmp(rob[i].dest,ins->regs[1].c_str());
            bool val_available = rob[i].val_available;
            if(found && !val_available)
            {
                ss << i + 1;
                ss >> res;
                rs[row][col].qk.assign(res);
            }
        }
    }

    return;
}

/* Resolves dependency issues; removes if reg val is available */
void remove_dependencies(reservation_station **rs, set<int> &s)
{
    /* More string manipulation... */
    stringstream ss;
    string rob_num;
    set<int>::iterator itr;

    /* If there were dependecies that were freed up */
    if(!s.empty())
    {
        for(itr = s.begin(); itr != s.end(); itr++)
        {
            ss << *itr;
            ss >> rob_num;
            /* Check if rs instrs had dependency that was freed up */
            for(int j = 0; j < 4; j++)
            {
                for(int k = 0; k < buffers[j]; k++)
                {
                    if(rs[j][k].qj == rob_num)
                        rs[j][k].qj = '\0';
                    if(rs[j][k].qk == rob_num)
                        rs[j][k].qk = '\0';
                }
            }
        }
    }

    return;
}

/* Remove instrs from reservation stations; i and j val of data struct used */
void remove_reservations(reservation_station **rs, vector<int> &v)
{
    for(int i = 0; i < v.size(); i++)
    {
        rs[v[i]][v[i+1]].unreserve();
        i++;
    }

    return;
}

/* Simulates a cycle exectution; ~~~~This is where all the magic happens~~~~ */
void tick(reservation_station **rs, ROB_entry *rob, int cycle)
{
    bool go = true,                 /* Flag only allows one commit per cycle */
         can_write = true;          /* Flag only allows one write per cycle */
    set<int> available_regs;        /* Regs that have vals avail next cycle */
    vector<int> unreserve_these;    /* Unreserve the rs in here next cycle */

    /* Instructions that were kicked from a res station but not commited */
    for(int i = 0; i < to_be_commited.size(); i++)
    {
        if(to_be_commited[i] != NULL)
        {
            /* Just left to commit if kicked out of rs */
            if(to_be_commited[i]->is_load())
            {
                if(to_be_commited[i]->state == 'w')
                {
                    available_regs.insert(to_be_commited[i]->RBS);
                    if(can_commit(to_be_commited[i]->num))
                    {
                        /* Can commit but one already did (data mem conflict) */
                        if(!go)
                        {
                            DMC_delays++;
                        }
                        else
                        {
                            to_be_commited[i]->set_state('c');
                            to_be_commited[i]->commit = cycle;
                            rob[to_be_commited[i]->RBS-1].busy = false;
                            available_regs.insert(to_be_commited[i]->RBS);
                            to_be_commited.erase(to_be_commited.begin()+i);
                            go = false;
                        }
                    } 
                }
            }

            /* Just left to commit if kicked out of rs */
            if(to_be_commited[i]->is_int())
            {
                if(to_be_commited[i]->state == 'w')
                {
                    available_regs.insert(to_be_commited[i]->RBS);
                    if(can_commit(to_be_commited[i]->num))
                    {
                        /* Can commit but one already did (data mem conflict) */
                        if(!go)
                        {
                            DMC_delays++;
                        }
                        else
                        {
                            to_be_commited[i]->commit = cycle;
                            to_be_commited[i]->set_state('c');
                            rob[to_be_commited[i]->RBS-1].busy = false;
                            to_be_commited.erase(to_be_commited.begin()+i);
                            go = false;
                        }
                    }
                }
            }
            /* Just left to commit if it was kicked out of rs */
            if(to_be_commited[i]->is_fp_add_or_sub() || 
               to_be_commited[i]->is_fp_mul_or_div())
            {
                available_regs.insert(to_be_commited[i]->RBS);
                if(can_commit(to_be_commited[i]->num))
                {
                    /* Can commit but one already did (data mem conflict) */
                    if(!go)
                    {
                        DMC_delays++;
                    }
                    else
                    {
                        to_be_commited[i]->commit = cycle;
                        to_be_commited[i]->set_state('c');
                        rob[to_be_commited[i]->RBS-1].busy = false;
                        to_be_commited.erase(to_be_commited.begin()+i);
                        go = false;
                    }
                }
            }
        }
    }

    /* Instructions in the reservation stations */
    for(int i = 0; i < 4; i++)
    {
        for(int j = 0; j < buffers[i] && rs[i][j].current_instr != NULL; j++)
        {
            /* Load instructions */
            if(rs[i][j].current_instr->is_load())
            {
                /* Issued last cycle => Execute now */
                if(rs[i][j].current_instr->state == 'i')
                {
                    rs[i][j].current_instr->set_state('e');
                    rs[i][j].current_instr->exec_start = cycle;
                    rs[i][j].current_instr->exec_end = cycle;
                }
                /* Executed last cycle => Read from mem now */
                else if(rs[i][j].current_instr->state == 'e')
                {
                    rs[i][j].current_instr->set_state('m');
                    rs[i][j].current_instr->mem_read = cycle;
                }
                /* Mem_read/couldnt write last cycle => Write if available */
                else if(rs[i][j].current_instr->state == 'm')
                {
                    /* Can write but one already did (data mem conflict) */
                    if(!can_write)
                    {
                        DMC_delays++;
                    }
                    else
                    {
                        can_write = false;
                        unreserve_these.push_back(i);
                        unreserve_these.push_back(j);
                        rs[i][j].current_instr->set_state('w');
                        rs[i][j].current_instr->write_res = cycle;
                    }
                }
                /* Mem_wrote/couldn't commit last cycle => Commit if available*/
                else if(rs[i][j].current_instr->state == 'w')
                {
                    available_regs.insert(rs[i][j].rob_spot);
                    if(can_commit(rs[i][j].current_instr->num))
                    {
                        /* Can commit but one already did (data mem conflict) */
                        if(!go)
                        {
                            DMC_delays++;
                        }
                        else
                        {
                            rs[i][j].current_instr->set_state('c');
                            rs[i][j].current_instr->commit = cycle;
                            rob[rs[i][j].rob_spot-1].busy = false;
                            available_regs.insert(rs[i][j].rob_spot);
                            go = false;
                        }
                    }
                }
            }

            /* Remove any dependencies if a reg had a val loaded last cycle */
            remove_dependencies(rs, available_regs);
            available_regs.clear();

            /* Store instructions */
            if(rs[i][j].current_instr->is_store())
            {
                /* Issued last cycle => Execute now */
                if(rs[i][j].current_instr->state == 'i')
                {
                    if(rs[i][j].qj[0] == '\0' && rs[i][j].qk[0] == '\0')
                    {
                        rs[i][j].current_instr->exec_start = cycle;
                        rs[i][j].current_instr->exec_end = cycle;
                        rs[i][j].current_instr->set_state('e');
                    }
                }
                /* Executed/couldnt commit last cycle => Commit if available */
                else if(rs[i][j].current_instr->state == 'e')
                {
                    if(can_commit(rs[i][j].current_instr->num))
                    {
                        /* Can commit but one already did (data mem conflict) */
                        if(!go)
                        {
                            DMC_delays++;
                        }
                        else
                        {
                            unreserve_these.push_back(i);
                            unreserve_these.push_back(j);
                            rs[i][j].current_instr->set_state('c');
                            rs[i][j].current_instr->commit = cycle;
                            rob[rs[i][j].rob_spot-1].busy = false;
                            go = false;
                        }
                    }
                }
            }

            /* Integer instructions */
            if(rs[i][j].current_instr->is_int())
            {
                /* Issued last cycle => Execute now */
                if(rs[i][j].current_instr->state == 'i')
                {
                    if(rs[i][j].qj[0] == '\0' && rs[i][j].qk[0] == '\0')
                    {
                        rs[i][j].current_instr->set_state('e');
                        rs[i][j].current_instr->exec_start = cycle;
                        rs[i][j].current_instr->exec_end = cycle;
                    }
                }
                /* Executed/coulnt write last cycle => Write if available */
                else if(rs[i][j].current_instr->state == 'e')
                {
                    rs[i][j].current_instr->set_state('w');
                    rs[i][j].current_instr->write_res = cycle;
                    can_write = false;
                    unreserve_these.push_back(i);
                    unreserve_these.push_back(j);
                    available_regs.insert(rs[i][j].rob_spot);
                }
                /* Mem_wrote/couldnt commit last cycle => Commit if available */
                else if(rs[i][j].current_instr->state == 'w')
                { 
                    available_regs.insert(rs[i][j].rob_spot);
                    if(can_commit(rs[i][j].current_instr->num))
                    {
                        /* Can commit but one already did (data mem conflict) */
                        if(!go)
                        {
                            DMC_delays++;
                        }
                        else
                        {
                            rs[i][j].current_instr->commit = cycle;
                            rs[i][j].current_instr->set_state('c');
                            rob[rs[i][j].rob_spot-1].busy = false;
                            go = false;
                        }
                    }
                }
                    
            }
            
            /* Floating Point Instruction */
            if(rs[i][j].current_instr->is_fp_add_or_sub() || 
               rs[i][j].current_instr->is_fp_mul_or_div())
            {
                /* Issued last cycle => Execute now */
                if(rs[i][j].current_instr->state == 'i')
                {
                    if(rs[i][j].qj[0] == '\0' && rs[i][j].qk[0] == '\0')
                    {
                        rs[i][j].current_instr->set_state('e');
                        rs[i][j].remaining_cycles--;
                        rs[i][j].current_instr->exec_start = cycle;
                    }
                }
                /* Executing last cycle => Continue exec or go to write stage */
                else if(rs[i][j].current_instr->state == 'e')
                {
                    if(rs[i][j].remaining_cycles == 1) 
                    {
                        rs[i][j].current_instr->set_state('w');
                        rs[i][j].current_instr->exec_end = cycle;
                        rob[rs[i][j].rob_spot-1].skipped = true;
                        rs[i][j].skip = true;
                    }
                    else
                    {
                        rs[i][j].remaining_cycles--;
                    }
                }
                else if(rs[i][j].current_instr->state == 'w')
                {
                    /* Needed for consistency (my structure is complicated) */
                    if(rs[i][j].skip)
                    {
                        /* Executed last cycle => Write if available */
                        if(can_write)
                        {
                            rs[i][j].current_instr->write_res = cycle;
                            can_write = false;
                            unreserve_these.push_back(i);
                            unreserve_these.push_back(j);
                        }
                        /* Can write but one already did (data mem conflict)  */
                        else
                        {
                            DMC_delays++;
                        }
                        rob[rs[i][j].rob_spot-1].skipped = false;
                        rs[i][j].skip = false;
                    }
                    /* Mem_wrote/couldnt commit last cycle => Commit if avail */
                    else
                    {
                        available_regs.insert(rs[i][j].rob_spot);
                        if(can_commit(rs[i][j].current_instr->num) && go)
                        {
                            rs[i][j].current_instr->commit = cycle;
                            rs[i][j].current_instr->set_state('c');
                            rob[rs[i][j].rob_spot-1].busy = false;
                            go = false;
                        }
                    }
                }
            }

            /* Branch instructions */
            if(rs[i][j].current_instr->is_branch())
            {
                /* Issued last cycle => Execute now */
                if(rs[i][j].current_instr->state == 'i')
                {
                    rs[i][j].current_instr->set_state('e');
                    rs[i][j].current_instr->exec_start = cycle;
                    rs[i][j].current_instr->exec_end = cycle;
                }
                /* Executed/couldnt commit last cycle => Commit if available */
                else if(rs[i][j].current_instr->state == 'e')
                {
                    unreserve_these.push_back(i);
                    unreserve_these.push_back(j);
                    if(can_commit(rs[i][j].current_instr->num) && go)
                    {
                        rs[i][j].current_instr->commit = cycle;
                        rs[i][j].current_instr->set_state('c');
                        rob[rs[i][j].rob_spot-1].busy = false;
                        go = false;
                    }
                }
            }
        }
    }

    /* Remove dependencies and rs spots for next cycle based on current cycle */
    remove_dependencies(rs, available_regs);
    available_regs.clear();
    remove_reservations(rs, unreserve_these);
    unreserve_these.clear();

    return;
}

/* Check if previous assignments have commited to preserve data flow */
bool can_commit(int n)
{
    for(int i = 0; i < n; i++)
        if(data[i].state != 'c')
            return false;

    return true;
}

/* Checks if all instructions have been commited */
bool const done(vector<instruction> &v) 
{
    for(int i = 0; i < v.size(); i++)
        if(data[i].state != 'c')
            return false;

    return true;
}

/* Prints the results of execution; pretty self explanatory */
void print_stats()
{
    for(int i = 0; i < data.size(); i++)
        TD_delays += data[i].exec_start - data[i].issue - 1;

    for(int i = 0; i < data.size(); i++)
    {
        cout << left << setw(22) << data[i].full_line;
        cout << right << setw(6) << data[i].issue << " ";
        cout << right << setw(3) << data[i].exec_start << " - ";
        cout << right << setw(2) << data[i].exec_end << " ";

        if(data[i].is_load())
            cout << right << setw(6) << data[i].mem_read << " ";
        else
            cout << right << setw(6) << "" << " ";

        if(!data[i].is_store() && !data[i].is_branch())
            cout << right << setw(6) << data[i].write_res << " ";
        else
            cout << right << setw(6) << "" << " ";

        cout << right << setw(7) << data[i].commit << endl;
    }
    cout << endl << endl;

    cout << "Delays" << endl;
    cout << "------" << endl;
    cout << "reorder buffer delays: " << ROB_delays << endl;
    cout << "reservation station delays: " << RS_delays << endl;
    cout << "data memory conflict delays: " << DMC_delays << endl;
    cout << "true depend delays: " << TD_delays << endl;

    return;
}

/* Prints reserve stations and their details; Prints with "-v" compiler flag */
void print_rs(reservation_station **rs, int cycle)
{
    cout << endl << "Cycle: " << cycle << endl << endl;
    cout << "      Reservation stations      " << endl;
    cout << "--------------------------------" << endl;
    cout << "  Name   Busy  Op   Qj  Qk  Dest" << endl;
    cout << "-------- ---- ----- --- --- ----" << endl;

    string names[4] = {"effaddr", "fpadd", "fpmul", "int"};
    for(int i = 0; i < 4; i++)
    {
        for(int j = 0; j < buffers[i]; j++)
        {
            cout << left << setw(7) << names[i] << j << " ";

            if(!rs[i][j].busy)
            {
                cout << left << setw(4) << "no" << endl;
                continue;
            }

            if(rs[i][j].busy)
                cout << left << setw(4) << "yes" << " ";
            else
                cout << left << setw(4) << "no" << " ";
            
            if(rs[i][j].current_instr->op == NULL)
                cout << left << setw(5) << "" << " "; 
            else
                cout << left << setw(5) << rs[i][j].current_instr->op << " ";
            
            if(rs[i][j].qj[0] != '\0')
                cout << "#" << left << setw(2) << rs[i][j].qj << " ";
            else
                cout << left << setw(3) << "" << " ";
            
            if(rs[i][j].qk[0] != '\0')
                cout << "#" << left << setw(2) << rs[i][j].qk << " ";
            else
                cout << left << setw(3) << "" << " ";
            
            if(rs[i][j].rob_spot != 0)
                cout << left << setw(1) << "#" << rs[i][j].rob_spot << endl;
            else
                cout << left << setw(1) << "" << endl;
        }
    }
    cout << endl;
    return;
}

/* Prints ROB and its details; Prints with "-v" compiler flag */
void print_rob(ROB_entry *rob)
{
    cout << "                     Reorder buffer                     " << endl;
    cout << "--------------------------------------------------------" << endl;
    cout << "Entry Busy      Instruction         State    Destination" << endl;
    cout << "----- ---- --------------------- ----------- -----------" << endl;

    for(int i = 0; i < buffers[4]; i++)
    {
        cout << right << setw(5) << i+1 << " ";

        if(rob[i].busy)
            cout << left << setw(4) << "yes" << " ";
        else 
            cout << left << setw(4) << "no" << " ";

        if(rob[i].instr != NULL)
            cout << left << setw(21) << rob[i].instr->full_line << " ";
        else
            cout << left << setw(21) << "" << " ";

        if(rob[i].instr != NULL)
        {
            if(rob[i].instr->state == 'i')
                cout << left << setw(11) << "issued" << " ";
            else if(rob[i].instr->state == 'e')
                cout << left << setw(11) << "executing" << " ";
            else if(rob[i].instr->state == 'w' && rob[i].skipped)
                cout << left << setw(11) << "executed" << " ";
            else if(rob[i].instr->state == 'm')
                cout << left << setw(11) << "memread" << " ";
            else if(rob[i].instr->state == 'w')
                cout << left << setw(11) << "wroteresult" << " ";
            else if(rob[i].instr->state == 'c')
                cout << left << setw(11) << "commited" << " ";
        }
        else
            cout << left << setw(11) << "" << " ";

        if(rob[i].dest[0] != '\0' && !rob[i].instr->is_branch())
            cout << left << setw(11) << rob[i].dest << endl;
        else
            cout << left << setw(11) << "" << endl;
    }
    cout << endl;
    return;
} 

/* Prints regs statuses in quasi-alpha order; Prints with "-v" compiler flag */
void print_reg_staus(ROB_entry *rob)
{
    vector<string> rv, fv;

    cout << "register status" << endl;
    cout << "---------------" << endl;

    for(int i = 0; i < buffers[4]; i++)
    {
        if(rob[i].busy && !rob[i].instr->is_store() && 
           rob[i].instr->regs[0][0] == 'R')
        {
            rv.push_back(rob[i].instr->regs[0]);
        }
        else if(rob[i].busy && !rob[i].instr->is_store() && 
                rob[i].instr->regs[0][0] == 'F')
        {
            fv.push_back(rob[i].instr->regs[0]);
        }
    }

    sort(rv.begin(), rv.end());
    reverse(rv.begin(), rv.end());
    sort(fv.begin(), fv.end());
    reverse(fv.begin(), fv.end());

    /* Remove duplicates from int registers */
    for(int i = 0; i < rv.size(); i++)
        for(int j = i+1; j < rv.size(); j++)
            if(rv[i].compare(rv[j]) == 0)
                rv.erase(rv.begin() + i);

    /* Remove duplicates from fp registers */
    for(int i = 0; i < fv.size(); i++)
        for(int j = i+1; j < fv.size(); j++)
            if(fv[i].compare(fv[j]) == 0)
                fv.erase(fv.begin() + i);

    /* Print int registers first */
    for(int i = rv.size()-1; i >= 0; i--)
    {
        for(int j = 0; j < buffers[4]; j++)
        {
            if(rob[j].instr->regs[0] == rv.back())
            {
                cout << rob[j].instr->regs[0] << "#=" << j+1 << " ";
                rv.pop_back();
                break;
            }
        }
    }

    /* Then print fp registers */
    for(int i = fv.size()-1; i >= 0; i--)
    {
        for(int j = 0; j < buffers[4]; j++)
        {
            if(rob[j].instr->regs[0] == fv.back())
            {
                cout << rob[j].instr->regs[0] << "#=" << j+1 << " ";
                fv.pop_back();
                break;
            }
        }
    }

    cout << endl << endl;
    return;
}

/* =============================END FUNCTIONS================================ */

/* =================================NOTES==================================== */

/*
    ~~~~~CODE~~~~~
    -> Fix cycles off by one sometimes when kicked from rs and still to commit
    -> Implement data memory conflict counters
        -> Picks random instruction to execute rather than instruction order
    -> Fix regs are one cycle off sometimnes in reg status
        -> (Finished) Print exactly how Dr. Whalley prints regs 

    ~~~~~STYLE~~~~~
    -> Stop using 4 as number of rs types. Use a variable (but no global?).
*/