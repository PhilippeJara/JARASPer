#include <memory>
#include <functional>
#include <iostream>
#include <vector>
#include <bitset>
#include <algorithm>
#include <map>

const size_t max_bits = 16;

std::bitset <max_bits> trim_input (int bits, std::bitset <max_bits> input);
class bus
{
public:
  int bits;
  std::bitset <max_bits> info;
  void set (int arg);
  bus (int bits);
  bus (int inf, int bits);
};
struct memory
{
  size_t const len;
  std::vector<size_t> body;
  std::shared_ptr<bus> addr_bus;
  std::shared_ptr<bus> data_bus;
  memory (size_t mem_size, size_t len);
  memory (size_t mem_size, size_t mem_block_len, size_t abus_len, size_t dbus_len);
};

class regist {
public:
  size_t bits;
  size_t id;
  std::bitset<max_bits> info;  
  std::vector<std::shared_ptr<bus>> in;
  std::vector<std::shared_ptr<bus>> out;
  regist();
  regist(size_t bits,size_t id);
  void link_in(std::shared_ptr<bus> arg);
  void link_out(std::shared_ptr<bus> arg);
  void remove_link_in(std::shared_ptr<bus> arg);
  void remove_link_out(std::shared_ptr<bus> arg);
  void set(int arg);
  void set(std::bitset<max_bits> arg);
  
};

class alu{
public:
  std::shared_ptr<regist> A;
  std::shared_ptr<regist> B;
  std::shared_ptr<regist> Z;
  alu();
  alu(std::shared_ptr<regist> Z,
      std::shared_ptr<regist> B,
      std::shared_ptr<regist> A);
  void add();
  void sub();
  void SHR(size_t id, size_t amnt);
  void SHL(size_t id, size_t amnt);
};

size_t get_operator(std::bitset<max_bits> microcode,
		    size_t operator_size,
		    size_t operand_size,
		    size_t operand_amnt);


size_t get_operand(std::bitset<max_bits> microcode,
		   size_t operand_size,
		   size_t operand_index);




//resolver como tirar o template daqui
class control_unit{
public:
  std::shared_ptr<regist> cu_reg;
  std::map <size_t,
	    std::shared_ptr<bus>> buses;
  std::map <size_t,
	    std::tuple<std::shared_ptr<regist>,
			      bool,
			      bool>> regists_in_out;
  std::map <size_t,
	    std::shared_ptr<alu>> alus; 
  std::map <size_t,
	    std::vector<size_t>> opcodes;
  size_t map_reg_counter;
  size_t map_bus_counter;
  size_t map_alu_counter;
  size_t map_mar_counter;
  size_t map_mdr_counter;
  size_t map_func_counter;
  size_t operator_size;
  size_t operand_size;
  size_t operand_amnt;
  std::map<size_t, size_t> mdrs_id;
  std::map<size_t, size_t> mars_id;
  control_unit(size_t cu_reg_s,
	       size_t operator_s,
	       size_t operand_s,
	       size_t operand_amnt);
  
  size_t make_bus(int bits);
  

  size_t make_regist(int bits);
  //reolver as linkagens para permitir o o input de dados na memoria
  size_t make_mdr(int bits, const std::shared_ptr<memory> &mem);
  
  size_t make_mar(const int bits, const std::shared_ptr<memory> &mem);
  
  size_t make_alu(std::shared_ptr<regist> A, std::shared_ptr<regist> B, std::shared_ptr<regist> Z);
    
  
  std::shared_ptr<regist> get_register(size_t id);
  std::shared_ptr<regist> get_mar(size_t id);
  std::shared_ptr<regist> get_mdr(size_t id);
  bool get_register_in(size_t id);
  bool get_register_out(size_t id);
  std::shared_ptr<bus> get_bus(size_t id);
  std::shared_ptr<alu> get_alu(size_t id);
  void set_in(size_t id);
  void set_out(size_t id);

  control_unit(size_t arg);
  




  
  /* 
     00: halt
     01: assignment
     02: add
     03: read from memory adress
     04: write to memory adress
     05: sub
     06: shl
     07: shr
  */



  
  void assignment(size_t id_reg1, size_t id_reg2);
  
  void add(size_t id);

  void sub(size_t id);
  //no momento só é possível usar SHR  e SHL no primeiro registrador da ALU (A)
  void SHR(size_t id_alu, size_t amnt);
    

  void SHL(size_t id_alu, size_t amnt);

  void read(const std::shared_ptr<regist> &mar,
	    const std::shared_ptr<regist> &mdr,
	    const std::vector<std::shared_ptr<memory>> &memories);
    

  void write(const std::shared_ptr<regist> &mar,
	     const std::shared_ptr<regist> &mdr,
	     const std::vector<std::shared_ptr<memory>> &memories);
    

  void execute(const std::vector<std::shared_ptr<memory>> &memories);
  void opcode_execute(const std::vector<std::shared_ptr<memory>> &);
  void interpret_minst(size_t,const std::vector<std::shared_ptr<memory>> &);
  void reg_out();
  void reg_in();
  void sync_bus();
};
class overseer{
public:
  std::vector <std::shared_ptr<control_unit>> control_units;
  std::vector <std::shared_ptr<memory>> memories;
  overseer();
  void cycle();
  void cycle_old();
};

// trocar os "at()" dos mapas por find().
// reavaliar o uso dos shared_ptrs, principalmente como argumento de funcões.
// implementar controle sobre ins e outs individuais dos registradores.(?)
// avaliar referência de mar's e mdr's à memoria


