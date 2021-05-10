#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <cstring>

using namespace std;

#define FUNC3 0
#define FUNC2 1
#define FUNC5 2
#define SELECTOR 4

class RegisterTable{

    int64_t _table[32];
    int64_t fakeX0 = 0;

    const int regToInt(string& reg)const{

        int reg_num = 0;

        for(int i = 1; i < reg.size() ;i++){
            reg_num *= 10;
            reg_num += reg[i] - '0';
        }
        return reg_num;
    }

    const static string intToBin(int n){
        
        string s;
        unsigned int un = n;
        while(un){
            s += un % 2 + '0';
            un /= 2;
        }
        for(int i = s.size() ; i < 64;i++){
            s+='0';
        }

        reverse(s.begin(),s.end());
        
        return s;
    }


public:
    RegisterTable(){
        memset(_table,0,sizeof(_table));

    }

    int64_t& operator[](string str){
        if(str == "x0"){
            return fakeX0;
        }
        return _table[regToInt(str)];
    }
    const int64_t operator[](string str) const{
        return _table[regToInt(str)];
    }

    void listRegister(int num = -1){
          cout<<"  register  |          int           |         hex        |"<<endl;
        //cout<<"    x10     |  18446744073709551615  |  FFFFFFFFFFFFFFFF  |";
        if(num == -1){
            num = 32;
        }
        for(int i = 0 ; i < num ;i++){
            if(i<10){
                cout<<"     ";
            }else{
                cout<<"    ";
            }
            cout<<"x"<<dec<<i<<"     |  "<<setw(20)<<dec<<_table[i]<<"  |  "<<setw(16)<<setfill('0')<<hex<<_table[i]<<"  |"<<setfill(' ')<<endl;
        }
    }


};

class BranchPredictor{
    int entry;
    uint8_t** _PBT;

public:
    BranchPredictor(int entry){
        this->entry = entry;
        this->_PBT = new uint8_t*[entry];
        for(int i = 0 ; i < entry ; i++){
            this->_PBT[i] = new uint8_t[5];
            memset(this->_PBT[i],0,sizeof(this->_PBT[i]));
        }
    }

    void update2BC(uint64_t pc,bool result){
        int ent = (pc / 4) % entry;
        
        if(result){
            if(this->_PBT[ent][this->_PBT[ent][SELECTOR]] < 3){
                this->_PBT[ent][this->_PBT[ent][SELECTOR]]++;
            }
        }else{
            if(this->_PBT[ent][this->_PBT[ent][SELECTOR]] >0){
                this->_PBT[ent][this->_PBT[ent][SELECTOR]]--;
            }
        }

        this->_PBT[ent][SELECTOR] = (this->_PBT[ent][SELECTOR] * 2 + result) & 3;
        // cout<<(int)this->_PBT[ent][SELECTOR]<<endl;
    }

    bool doPrediction(uint64_t pc){
        int ent = (pc / 4) % entry;
        
        string bc2[4] = {"SN","WN","WT","ST"};
        string res[2] = {"N","T"};
        cout<<"entry: "<<setw(3)<<left<<ent<<" ";
        cout<<"("<<this->_PBT[ent][SELECTOR] / 2<<this->_PBT[ent][SELECTOR]%2<<", ";
        cout<<bc2[this->_PBT[ent][0]]<<", ";
        cout<<bc2[this->_PBT[ent][1]]<<", ";
        cout<<bc2[this->_PBT[ent][2]]<<", ";
        cout<<bc2[this->_PBT[ent][3]]<<")"<<endl;
        cout<<"guess : "<<res[(this->_PBT[ent][this->_PBT[ent][SELECTOR]] > 1)];
        return this->_PBT[ent][this->_PBT[ent][SELECTOR]] > 1;
    }

    void listPredictor(){
        string bc2[4] = {"SN","WN","WT","ST"};
        string res[2] = {"N","T"};

        for(int ent = 0 ; ent < entry;ent++){
            cout<<"Entry "<<ent<<" : ";
            cout<<"("<<this->_PBT[ent][SELECTOR] / 2<<this->_PBT[ent][SELECTOR]%2<<", ";
            cout<<bc2[this->_PBT[ent][0]]<<", ";
            cout<<bc2[this->_PBT[ent][1]]<<", ";
            cout<<bc2[this->_PBT[ent][2]]<<", ";
            cout<<bc2[this->_PBT[ent][3]]<<")"<<endl;
        }
    }

};

/*
    * * * * * * * * * * * * * * * *
    *   Basic Type of Instuction  *
    * * * * * * * * * * * * * * * *
*/

class Instruction{
public:
    string name;
    string rd;
    string rs1;
    string rs2;
    string opcode;
    int imme;
    const map<string,vector<string> > *insts;
    // [inst name] : {[func3],[func2],[func5]}
    
    Instruction(string name,string rd,string rs1,string rs2,int imme){
        this->name = name;
        this->rd = rd;
        this->rs1 = rs1;
        this->rs2 = rs2;
        this->imme = imme;
    }

    virtual string translateResult() = 0;

    const static string intToBin(int n,int size,bool reversed=true){
        
        string s;
        unsigned int un = n;
        while(un){
            s += un % 2 + '0';
            un /= 2;
        }
        for(int i = s.size() ; i < size;i++){
            s+='0';
        }

        // if need to rever it
        if(reversed){
            // fetch needed part
            s = s.substr(0,size);
            reverse(s.begin(),s.end());
        }
        
        return s.substr(0,size);
    }

    // get register's No. and transfer into binary
    const static string regToBin(string reg){
        int reg_i = 0;
        for(size_t i = 1 ; i < reg.size();i++){
            reg_i *= 10;
            reg_i += reg[i] - '0';
        }
        return intToBin(reg_i,5);
    }

    virtual void run(RegisterTable& rTable,uint64_t& pc) = 0;
};

/*
    * * * * * * * * * * * * * * * * * * *
    *   Six Type of Instruction Format  *
    * * * * * * * * * * * * * * * * * * *
*/

class U_Type_Instruction:public Instruction{
public:
    U_Type_Instruction(string name,string rd,string rs1,string rs2,int imme):Instruction(name,rd,rs1,rs2,imme){}
    string translateResult(){
        string result;

        result += Instruction::intToBin(this->imme,32).substr(0,20); // first 20 bit imm
        result += Instruction::regToBin(this->rd);
        result += this->opcode;
        return result;
    }

};

class UJ_Type_Instruction:public Instruction{
public:
    UJ_Type_Instruction(string name,string rd,string rs1,string rs2,int imme):Instruction(name,rd,rs1,rs2,imme){}
    string translateResult(){
        string result;

        string imm = Instruction::intToBin(this->imme,21,false);

        // imm[20,10:1,11,19:12]
        result += imm[20];
        for(int i = 10 ;i>= 1;i--){
            result += imm[i];
        }
        result += imm[11];
        for(int i = 19 ;i>= 12;i--){
            result += imm[i];
        }


        result += Instruction::regToBin(this->rd);
        result += this->opcode;
        return result;
    }
};

class I_Type_Instruction:public Instruction{
public:
    I_Type_Instruction(string name,string rd,string rs1,string rs2,int imme):Instruction(name,rd,rs1,rs2,imme){}
    string translateResult(){
        string result;
        result += Instruction::intToBin(this->imme,12);
        result += Instruction::regToBin(this->rs1);
        result += (*this->insts->find(this->name)).second[FUNC3];
        result += Instruction::regToBin(this->rd);
        
        result += this->opcode;
        return result;
    }
};

class SB_Type_Instruction:public Instruction{
public:
    SB_Type_Instruction(string name,string rd,string rs1,string rs2,int imme):Instruction(name,rd,rs1,rs2,imme){}
    string translateResult(){
        string result;
        string imm = Instruction::intToBin(imme,13,false);

        // imm[12,10:5]
        result += imm[12];
        for(int i = 10 ; i >= 5 ;i--){
            result += imm[i];
        }

        result += Instruction::regToBin(this->rs1);
        result += Instruction::regToBin(this->rd);
        result += (*this->insts->find(this->name)).second[FUNC3];

        // imm[4:1,11]
        for(int i = 4 ; i >= 1;i--){
            result += imm[i];
        }
        result += imm[11];

        result += this->opcode;
        return result;
    }

};

class S_Type_Instruction:public Instruction{
public:
    S_Type_Instruction(string name,string rd,string rs1,string rs2,int imme):Instruction(name,rd,rs1,rs2,imme){}
    string translateResult(){
        string result;
        string imm = Instruction::intToBin(this->imme,12,false);

        // imm[11:5]
        for(int i = 11 ; i >= 5;i--){
            result += imm[i];
        }

        result += Instruction::regToBin(this->rd);
        result += Instruction::regToBin(this->rs1);
        result += (*this->insts->find(this->name)).second[FUNC3];
        
        // imm[4:0]
        for(int i = 4 ; i >= 0;i--){
            result += imm[i];
        }

        result += opcode;
        return result;
    }

};

class R_Type_Instruction:public Instruction{
public:
    R_Type_Instruction(string name,string rd,string rs1,string rs2,int imme):Instruction(name,rd,rs1,rs2,imme){}
    string translateResult(){
        string result;
        result += (*this->insts->find(this->name)).second[FUNC5];
        result += (*this->insts->find(this->name)).second[FUNC2];
        result += Instruction::regToBin(this->rs2);
        result += Instruction::regToBin(this->rs1);
        result += (*this->insts->find(this->name)).second[FUNC3];
        result += Instruction::regToBin(this->rd);

        result += this->opcode;
        return result;
    }

};

/*
    * * * * * * * * * * * * * * * * * * * * * * * * *
    *   Derivative Instuction from Six Basic Type   *
    * * * * * * * * * * * * * * * * * * * * * * * * *
*/

class LUI_Instruction:public U_Type_Instruction{
    const static map<string,vector<string> > insts_names;
public:
    LUI_Instruction(string name,string rd,string rs1,string rs2,int imme):U_Type_Instruction(name,rd,rs1,rs2,imme){
        this->opcode = "0110111";
        this->insts = &insts_names;
    }

    static const bool isThisType(string inst_name){
        return !(insts_names.find(inst_name) == insts_names.end());
    }

    void run(RegisterTable& rTable,uint64_t& pc){
        rTable[this->rd] = (uint64_t)this->imme << 12;

        pc += 4;
        return;
    }
};
const map<string,vector<string> > LUI_Instruction::insts_names = {
    {"LUI", {}}
};


class AUIPC_Instruction:public U_Type_Instruction{
    const static map<string,vector<string> > insts_names;
public:
    AUIPC_Instruction(string name,string rd,string rs1,string rs2,int imme):U_Type_Instruction(name,rd,rs1,rs2,imme){
        this->opcode = "0010111";
        this->insts = &insts_names;
    }
    static const bool isThisType(string inst_name){
        return !(insts_names.find(inst_name) == insts_names.end());
    }

    void run(RegisterTable& rTable,uint64_t& pc){
        rTable[this->rd] = (this->imme << 12) + pc;

        pc += 4;
        return;
    }
};
const map<string,vector<string> > AUIPC_Instruction::insts_names = {
    {"AUIPC", {}}
};


class JAL_Instruction:public UJ_Type_Instruction{
    const static map<string,vector<string> > insts_names;
public:
    JAL_Instruction(string name,string rd,string rs1,string rs2,int imme):UJ_Type_Instruction(name,rd,rs1,rs2,imme){
        this->opcode = "1101111";
        this->insts = &insts_names;
    }
    static const bool isThisType(string inst_name){
        return !(insts_names.find(inst_name) == insts_names.end());
    }
    
    void run(RegisterTable& rTable,uint64_t& pc){
        rTable[this->rd] = pc + 4;

        pc = pc + this->imme;
        return;
    }
};
const map<string,vector<string> > JAL_Instruction::insts_names = {
    {"JAL", {}}
};


class JALR_Instruction:public I_Type_Instruction{
    const static map<string,vector<string> > insts_names;
public:
    JALR_Instruction(string name,string rd,string rs1,string rs2,int imme):I_Type_Instruction(name,rd,rs1,rs2,imme){
        this->opcode = "1100111";
        this->insts = &insts_names;
    }
    static const bool isThisType(string inst_name){
        return !(insts_names.find(inst_name) == insts_names.end());
    }
    
    void run(RegisterTable& rTable,uint64_t& pc){
        rTable[this->rd] = pc + 4;

        pc = this->imme + pc;
        return;
    }
};
const map<string,vector<string> > JALR_Instruction::insts_names = {
    {"JALR", {"000"}}
};


class Branch_Instruction:public SB_Type_Instruction{
    const static map<string,vector<string> > insts_names;
public:
    Branch_Instruction(string name,string rd,string rs1,string rs2,int imme):SB_Type_Instruction(name,rd,rs1,rs2,imme){
        this->opcode = "1100011";
        this->insts = &insts_names;
    }
    static const bool isThisType(string inst_name){
        return !(insts_names.find(inst_name) == insts_names.end());
    }
    
    void run(RegisterTable& rTable,uint64_t& pc){
        
        if(
            (this->name == "BEQ"  && rTable[this->rd] == rTable[this->rs1] ) ||
            (this->name == "BNE"  && rTable[this->rd] != rTable[this->rs1] ) ||
            (this->name == "BLT"  && rTable[this->rd] < rTable[this->rs1] ) ||
            (this->name == "BGE"  && rTable[this->rd] >= rTable[this->rs1] ) ||
            (this->name == "BLTU" && (uint64_t)rTable[this->rd] == (uint64_t)rTable[this->rs1] ) ||
            (this->name == "BGEU" && (uint64_t)rTable[this->rd] == (uint64_t)rTable[this->rs1] )
        ){
            pc = pc + this->imme;
        }else{
            pc += 4;
        }
        return;
    }
};
const map<string,vector<string> > Branch_Instruction::insts_names = {
    {"BEQ",  {"000"}},
    {"BNE",  {"001"}},
    {"BLT",  {"100"}},
    {"BGE",  {"101"}},
    {"BLTU", {"110"}},
    {"BGEU", {"111"}}
};


class Load_Instruction:public I_Type_Instruction{
    const static map<string,vector<string> > insts_names;
public:
    Load_Instruction(string name,string rd,string rs1,string rs2,int imme):I_Type_Instruction(name,rd,rs1,rs2,imme){
        this->opcode = "0000011";
        this->insts = &insts_names;
    }
    static const bool isThisType(string inst_name){
        return !(insts_names.find(inst_name) == insts_names.end());
    }
    
    void run(RegisterTable& rTable,uint64_t& pc){
        return;
        for(int i = 0 ; i < 8 ; i++){
            
        }
        if(this->name == "LB"){ // 8
        }else if(this->name == "LBU"){ // 8

        }else if(this->name == "LH"){ // 16

        }else if(this->name == "LHU"){ // 16

        }else if(this->name == "LW"){ // 32

        }else if(this->name == "LWU"){ // 32

        }else if(this->name == "LD"){

        }
    }
};
const map<string,vector<string> > Load_Instruction::insts_names = {
    {"LB",  {"000"}},
    {"LH",  {"001"}},
    {"LW",  {"010"}},
    {"LBU", {"100"}},
    {"LHU", {"101"}},
    {"LWU", {"110"}},
    {"LD",  {"011"}},
};


class Save_Instruction:public S_Type_Instruction{
    const static map<string,vector<string> > insts_names;
public:
    Save_Instruction(string name,string rd,string rs1,string rs2,int imme):S_Type_Instruction(name,rd,rs1,rs2,imme){
        this->opcode = "0100011";
        this->insts = &insts_names;
    }
    static const bool isThisType(string inst_name){
        return !(insts_names.find(inst_name) == insts_names.end());
    }
    
    void run(RegisterTable& rTable,uint64_t& pc){
        return;
    }
};
const map<string,vector<string> > Save_Instruction::insts_names = {
    {"SB",{"000"}},
    {"SH",{"001"}},
    {"SW",{"010"}},
    {"SD",{"011"}}
};


class Integer_Imme_Instruction:public I_Type_Instruction{
    const static map<string,vector<string> > insts_names;
public:
    Integer_Imme_Instruction(string name,string rd,string rs1,string rs2,int imme):I_Type_Instruction(name,rd,rs1,rs2,imme){
        this->opcode = "0010011";
        this->insts = &insts_names;
    }
    static const bool isThisType(string inst_name){
        return !(insts_names.find(inst_name) == insts_names.end());
    }
    
    void run(RegisterTable& rTable,uint64_t& pc){
        if(this->name == "ADDI"){
            rTable[this->rd] = rTable[this->rs1] + this->imme;
        }else if(this->name == "SLTI"){
            rTable[this->rd] = rTable[this->rs1] < this->imme;
        }else if(this->name == "SLTIU"){
            rTable[this->rd] = (uint64_t)rTable[this->rs1] < (uint64_t)this->imme;
        }else if(this->name == "XORI"){
            rTable[this->rd] = rTable[this->rs1] ^ (uint64_t)(uint32_t)this->imme;
        }else if(this->name == "ORI"){
            rTable[this->rd] = rTable[this->rs1] | (uint64_t)(uint32_t)this->imme;
        }else if(this->name == "ANDI"){
            rTable[this->rd] = rTable[this->rs1] & (uint64_t)(uint32_t)this->imme;
        }

        pc += 4;
        return;
    }
};
const map<string,vector<string> > Integer_Imme_Instruction::insts_names = {
    {"ADDI",  {"000"}},
    {"SLTI",  {"010"}},
    {"SLTIU", {"011"}},
    {"XORI",  {"100"}},
    {"ORI",   {"110"}},
    {"ANDI",  {"111"}},
};


class Shift_Imme_Instruction:public R_Type_Instruction{
    const static map<string,vector<string> > insts_names;
public:
    Shift_Imme_Instruction(string name,string rd,string rs1,string rs2,int imme):R_Type_Instruction(name,rd,rs1,rs2,imme){
        this->opcode = "0010011";
        this->insts = &insts_names;
        this->rs2 = string("x") + to_string(this->imme);
    }
    static const bool isThisType(string inst_name){
        return !(insts_names.find(inst_name) == insts_names.end());
    }
    
    void run(RegisterTable& rTable,uint64_t& pc){
        if(this->name == "SLLI"){
            rTable[this->rd] = rTable[this->rs1] << this->imme;
        }else if(this->name == "SRLI"){
            rTable[this->rd] = ((uint64_t)rTable[this->rs1]) >> this->imme;
        }else if(this->name == "SRAI"){
            rTable[this->rd] = ((int64_t)rTable[this->rs1]) >> this->imme;
        }

        pc += 4;
        return;
    }
};
const map<string,vector<string> > Shift_Imme_Instruction::insts_names = {
    {"SLLI", {"001","00","00000"}},
    {"SRLI", {"101","00","00000"}},
    {"SRAI", {"101","00","01000"}}
};


class Integer_Register_Instruction:public R_Type_Instruction{
    const static map<string,vector<string> > insts_names;
public:
    Integer_Register_Instruction(string name,string rd,string rs1,string rs2,int imme):R_Type_Instruction(name,rd,rs1,rs2,imme){
        this->opcode = "0110011";
        this->insts = &insts_names;
    }
    static const bool isThisType(string inst_name){
        return !(insts_names.find(inst_name) == insts_names.end());
    }
    
    void run(RegisterTable& rTable,uint64_t& pc){
        if(this->name == "ADD"){
            rTable[this->rd] = rTable[this->rs1] + rTable[this->rs2];
        }else if(this->name == "SUB"){
            rTable[this->rd] = rTable[this->rs1] - rTable[this->rs2];
        }else if(this->name == "SLT"){
            rTable[this->rd] = rTable[this->rs1] < rTable[this->rs2];
        }else if(this->name == "SLTU"){
            rTable[this->rd] = (uint64_t)rTable[this->rs1] < (uint64_t)rTable[this->rs2];
        }else if(this->name == "XOR"){
            rTable[this->rd] = rTable[this->rs1] ^ rTable[this->rs2];
        }else if(this->name == "AND"){
            rTable[this->rd] = rTable[this->rs1] & rTable[this->rs2];
        }else if(this->name == "OR"){
            rTable[this->rd] = rTable[this->rs1] | rTable[this->rs2];
        }

        pc += 4;
        return;
    }
};
const map<string,vector<string> > Integer_Register_Instruction::insts_names = {
    {"ADD",  {"000","00","00000"}},
    {"SUB",  {"000","00","01000"}},
    {"SLT",  {"010","00","00000"}},
    {"SLTU", {"011","00","00000"}},
    {"XOR",  {"100","00","00000"}},
    {"OR",   {"110","00","00000"}},
    {"AND",  {"111","00","00000"}}
};


class Shift_Register_Instruction:public R_Type_Instruction{
    const static map<string,vector<string> > insts_names;
public:
    Shift_Register_Instruction(string name,string rd,string rs1,string rs2,int imme):R_Type_Instruction(name,rd,rs1,rs2,imme){
        this->opcode = "0110011";
        this->insts = &insts_names;
    }
    static const bool isThisType(string inst_name){
        return !(insts_names.find(inst_name) == insts_names.end());
    }
    
    void run(RegisterTable& rTable,uint64_t& pc){
        if(this->name == "SLL"){
            rTable[this->rd] = rTable[this->rs1] << (rTable[this->rs2] & 63);
        }else if(this->name == "SRL"){
            rTable[this->rd] = ((uint64_t)rTable[this->rs1]) >> (rTable[this->rs2] & 63);
        }else if(this->name == "SRA"){
            rTable[this->rd] = ((int64_t)rTable[this->rs1]) >> (rTable[this->rs2] & 63);
        }

        pc += 4;
        return;
    }
};

const map<string,vector<string> > Shift_Register_Instruction::insts_names = {
    {"SLL", {"001","00","00000"}},
    {"SRL", {"101","00","00000"}},
    {"SRA", {"101","00","01000"}}
};


class Multiply_Register_Instruction:public R_Type_Instruction{
    const static map<string,vector<string> > insts_names;
public:
    Multiply_Register_Instruction(string name,string rd,string rs1,string rs2,int imme):R_Type_Instruction(name,rd,rs1,rs2,imme){
        this->opcode = "0110011";
        this->insts = &insts_names;
    }
    static const bool isThisType(string inst_name){
        return !(insts_names.find(inst_name) == insts_names.end());
    }
    
    void run(RegisterTable& rTable,uint64_t& pc){
        if(this->name == "MUL"){
            rTable[this->rd] = (rTable[this->rs1] * rTable[this->rs2]) << 32 >> 32;
        }else if(this->name == "MULH"){
            rTable[this->rd] = (rTable[this->rs1] * rTable[this->rs2]) >> 32;
        }else if(this->name == "MULHU"){
            rTable[this->rd] = ((uint64_t)rTable[this->rs1] * (uint64_t)rTable[this->rs2]) >> 32;
        }else if(this->name == "NULHSU"){
            rTable[this->rd] = ((int64_t)rTable[this->rs1] * (uint64_t)rTable[this->rs2]) >> 32;
        }else if(this->name == "DIV"){
            rTable[this->rd] = rTable[this->rs1] / rTable[this->rs2];
        }else if(this->name == "DIVU"){
            rTable[this->rd] = (uint64_t)rTable[this->rs1] / (uint64_t)rTable[this->rs2];
        }else if(this->name == "REM"){
            rTable[this->rd] = rTable[this->rs1] % rTable[this->rs2];
        }else if(this->name == "REMU"){
            rTable[this->rd] = (uint64_t)rTable[this->rs1] % (uint64_t)rTable[this->rs2];
        }
    }
};
const map<string,vector<string> > Multiply_Register_Instruction::insts_names = {
    {"MUL",    {"000","01","00000"}},
    {"MULH",   {"001","01","00000"}},
    {"MULHSU", {"010","01","00000"}},
    {"MULHU",  {"011","01","00000"}},
    {"DIV",    {"100","01","00000"}},
    {"DIVU",   {"101","01","00000"}},
    {"REM",    {"110","01","00000"}},
    {"REMU",   {"111","01","00000"}},
};


/*
    * * * * * * * * * * * * * * * * * * * * * * * * *
    *   Temporary Instruction Type for saving info  *
    * * * * * * * * * * * * * * * * * * * * * * * * *
    
    Because Instruction is abstract class, It can't be instantiated
*/

class TmpInst{
public:
    string inst_name;
    string rd;
    string rs1;
    string rs2;
    int imme;
    TmpInst(string name,string rd,string rs1,string rs2,int imme){
        this->inst_name = name;
        this->rd = rd;
        this->rs1 = rs1;
        this->rs2 = rs2;
        this->imme = imme;
    }
};



int main(int argc, char* argv[]){

    int entry=-1;
    cout<<"Please input entry(entry > 0):"<<endl;
    cin>>entry;
    while(entry < 0){
        cout<<"Entry should > 0"<<endl;
        cout<<"Please input entry(entry > 0):"<<endl;
        cin>>entry;
    }


    // mTable[0] = 0;
    // mTable[1] = 15;
    // mTable[2] = 0;
    // mTable[3] = 240;
    // cout<<((uint64_t*)mTable)[0]<<endl;


    // check if has input file name
    if(argc <2){
        cout<<"Pleas input file name as parameter"<<endl;
        return -1;
    }


    ifstream in(argv[1]);
    
    // test can open file
    if(!in){
        cout<<"Can not open file"<<endl;
        return -1;
    }


    string line;  // line of instruction
    vector<string> lines;
    map<string,pair<int,int> > label;  // temporary save label's pos, pair(jumper,destination)
    vector<TmpInst> insts; // temporary save resolved insts
    int line_cnt = 0;

    // first int rep jumper line,second rep jump distance line
    while(!in.eof()){
        int i = 0;  // char counter, to iterate current line
        
        // all instruction may have
        string inst_name;
        string rd;
        string rs1="";
        string rs2="";
        int imme=0;
        
        getline(in,line);   // fetch one line
        lines.push_back(line);
        line += "\n";   // add terminal char


        while(line[i] == ' '){// clean space in front of instruction
            i++;
        }

        if(line[i] == '#'){continue;} // check if it is comment

        while(line[i] != ' '){  // get first word, it can be label(if exist) or instruction name
            inst_name += line[i++];
        }

        // clear space between | label <-> instruction name | or | instruction name <-> rd |
        while(line[i] == ' '){i++;}
            
        
        if(inst_name.back() == ':'){    // check first word is label or not
        
            if(label.find(inst_name) == label.end()){   // it is label,so find the pair of it
                label[inst_name] = pair<int,int>(-1,line_cnt);  // add new pair if it isn't found yet
            }else{
                label[inst_name].second = line_cnt; // set pair
            }

            inst_name = "";  // reset string to get instruction name

            while(line[i] != ' '){  // refetch instructioin name
                inst_name += line[i++];
            }

            i++;  // skip space between instruction name and rd
        }

        while(line[i] != ',' && line[i] != ' '){ // get rd
            rd += line[i++];
        }
        i++;  // skip ,

        while(line[i] == ' '){i++;};  // skip space



        if((line[i] >= '0' && line[i] <='9') || line[i] == '-'){ // if second column is number

            /*
                LUI / AUIPC => only have imm

                Lx / Sx => have rs1
            */

            bool isNeg = line[i] == '-';    // check if number is negative
            
            if(line[i] == '-'){i++;}    // skip -

            while(line[i] >= '0' && line[i] <='9'){ // convert string to number
                imme *= 10;
                imme += line[i] - '0';
                i++;
            }

            if(isNeg){  // give sign to it
                imme *= -1;
            }

            if(line[i] == '('){ // check if it has (rs1) 
                i++; // skip (

                while(line[i] != ')'){  // get rs1
                    rs1 += line[i];
                    i++;
                }
            }

        }else if(line[i] == 'x'){   // if second column is register
            
            /*
                Branch / Arithmetic / Logical
            */

            while(line[i] != ','){  // get rs1
                rs1 += line[i++];
            }
            i++;  // skip ,

            if(line[i] == 'x'){  // if third column is register => register to register
                
                while(line[i] != '\n' && line[i] != ' '){ // get rs2
                    rs2 += line[i++];
                }
                
            }else if((line[i] >= '0' && line[i] <='9') || line[i] == '-'){ //if third column is number => register - immediate
                
                bool isNeg = line[i] == '-';  // check is negative
                if(line[i] == '-'){  // skip -
                    i++;
                }
                
                while(line[i] >= '0' && line[i] <='9'){  // convert string to int
                    imme *= 10;
                    imme += line[i] - '0';
                    i++;
                }

                if(isNeg){ // give sign
                    imme *= -1;
                }   

            }else{  // third column is Label
            
                string lab; // get label
                while(line[i] != '\n' && line[i] != ' '){
                    lab += line[i];
                    i++;
                }
                lab += ':';

                // connect label
                if(label.find(lab) == label.end()){
                    label[lab] = pair<int,int>(line_cnt,-1);
                }else{
                    label[lab].first = line_cnt;
                }
            }
        }else{  // if second column is Label
        
            /*
                jal
            */

            // get label name
            string lab;
            while(line[i] != '\n' && line[i] != ' '){
                lab += line[i];
                i++;
            }
            lab += ':';

            // connect label
            if(label.find(lab) == label.end()){
                label[lab] = pair<int,int>(line_cnt,-1);
            }else{
                label[lab].first = line_cnt;
            }
        }

        std::transform(inst_name.begin(), inst_name.end(),inst_name.begin(), ::toupper);  // change instruction name to upper case
        insts.push_back(TmpInst(inst_name,rd,rs1,rs2,imme));  // add to tempory instruction

        line_cnt++;
    }

    // process label's connect
    for(auto i : label){

        /*
            i.second = pair<int,int>
            i.second.first = pair's first => jumper instruction
            i.scond.second = destination 
        */
        if(i.second.first != -1 && i.second.second != -1){
            insts[i.second.first].imme = (i.second.second - i.second.first) * 4;
        }
    }

    vector<Instruction*> compiled_insts;
    int line_c = 1;

    compiled_insts.push_back(new LUI_Instruction("LUI","x0","","",0));


    for(auto i : insts){  // find all instruction to match
        if(LUI_Instruction::isThisType(i.inst_name)){
            compiled_insts.push_back(new LUI_Instruction(i.inst_name,i.rd,i.rs1,i.rs2,i.imme));
        }else if(AUIPC_Instruction::isThisType(i.inst_name)){
            compiled_insts.push_back(new AUIPC_Instruction(i.inst_name,i.rd,i.rs1,i.rs2,i.imme));
        }else if(JAL_Instruction::isThisType(i.inst_name)){
            compiled_insts.push_back(new JAL_Instruction(i.inst_name,i.rd,i.rs1,i.rs2,i.imme));
        }else if(JALR_Instruction::isThisType(i.inst_name)){
            compiled_insts.push_back(new JALR_Instruction(i.inst_name,i.rd,i.rs1,i.rs2,i.imme));
        }else if(Branch_Instruction::isThisType(i.inst_name)){
            compiled_insts.push_back(new Branch_Instruction(i.inst_name,i.rd,i.rs1,i.rs2,i.imme));
        }else if(Load_Instruction::isThisType(i.inst_name)){
            compiled_insts.push_back(new Load_Instruction(i.inst_name,i.rd,i.rs1,i.rs2,i.imme));
        }else if(Save_Instruction::isThisType(i.inst_name)){
            compiled_insts.push_back(new Save_Instruction(i.inst_name,i.rd,i.rs1,i.rs2,i.imme));
        }else if(Integer_Imme_Instruction::isThisType(i.inst_name)){
            compiled_insts.push_back(new Integer_Imme_Instruction(i.inst_name,i.rd,i.rs1,i.rs2,i.imme));
        }else if(Shift_Imme_Instruction::isThisType(i.inst_name)){
            compiled_insts.push_back(new Shift_Imme_Instruction(i.inst_name,i.rd,i.rs1,i.rs2,i.imme));
        }else if(Integer_Register_Instruction::isThisType(i.inst_name)){
            compiled_insts.push_back(new Integer_Register_Instruction(i.inst_name,i.rd,i.rs1,i.rs2,i.imme));
        }else if(Shift_Register_Instruction::isThisType(i.inst_name)){
            compiled_insts.push_back(new Shift_Register_Instruction(i.inst_name,i.rd,i.rs1,i.rs2,i.imme));
        }else if(Multiply_Register_Instruction::isThisType(i.inst_name)){
            compiled_insts.push_back(new Multiply_Register_Instruction(i.inst_name,i.rd,i.rs1,i.rs2,i.imme));
        }else{
            // can't not find instruction, stop processing
            cout<<"** Error: Instruction "<<i.inst_name<<" not implement or does not exist (at line "<<line_c<<")"<<endl;
            cout<<"Process exist";
            exit(-1);
        }
        line_c++;
    }

    RegisterTable rTable;

    BranchPredictor predictor(entry);

    string res[2] = {"N","T"};

    uint64_t pc = 4;
    char skip;
    while(pc / 4 < compiled_insts.size()){

        cout<<"0x"<<right<<hex<<setfill('0')<<setw(4)<<pc - 4<<dec<<"  "<<setfill(' ');
        int tmp_pc = pc;
        cout<<lines[pc/4 - 1]<<endl;
        bool guess = predictor.doPrediction(pc);

        compiled_insts[pc/4]->run(rTable,pc);

        predictor.update2BC(tmp_pc,tmp_pc+4 != pc);
        cout<<"  result: "<<res[tmp_pc + 4 != pc]<<endl;
        // rTable.listRegister();

        if(skip != '!'){
            cout<<"----------"<<endl;
            cout<<"BHT After Update:"<<endl;
            predictor.listPredictor();
            cout<<endl;
            rTable.listRegister(7);
            skip = cin.get();
        }

        cout<<"================================================================="<<endl;

    }
    
    rTable.listRegister();


    for(auto i : compiled_insts){
        delete i;
    }

    /*
    // write to file
    ofstream out("out.txt",ios::out);
    for(auto i: compiled_insts){
        out<<i<<"\n";
    }
    */




    return 0;
}