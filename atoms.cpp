#include "atoms.h"


using namespace std;

bitset<max_bits> trim_input(int bits, bitset<max_bits> input){
  for (int i = max_bits-1; i > bits-1; i--){input.reset(i);}
  return input;
}


bus::bus(int bits) : bits(bits), info() {}
bus::bus(int inf, int bits) : info(inf), bits(bits) {}
void bus::set(int arg){
  auto tmp = bitset<max_bits>(arg);
  info = trim_input(bits, tmp);
}

  
//zerar após cada ciclo?


memory::memory(size_t mem_size, size_t len ): len(len),
					      body(vector<size_t>(mem_size)),
					      addr_bus(make_shared<bus>(len)),
					      data_bus(make_shared<bus>(len)){}
memory::memory(size_t mem_size,
	       size_t mem_block_len,
	       size_t abus_len,
	       size_t dbus_len): len(mem_block_len),
				 body(vector<size_t>(mem_size)),
				 addr_bus(make_shared<bus>(abus_len)),
				 data_bus(make_shared<bus>(dbus_len)){}



regist::regist() : bits(), info() , in() , out(){}
regist::regist(size_t bits,size_t id) :bits(bits), id(id), info(), in(), out() {}
void regist::link_in(shared_ptr<bus> arg) {in.push_back(arg);}
void regist::link_out(shared_ptr<bus> arg) {out.push_back(arg);}
void regist::remove_link_in(shared_ptr<bus> arg){
  in.erase(find (in.begin(),
		 in.end(),
		 arg));}
void regist::remove_link_out(shared_ptr<bus> arg){
  out.erase(find (out.begin(),
		  out.end(),
		  arg));}  
void regist::set(int arg) {info = trim_input(bits, arg);}
void regist::set(bitset<max_bits> arg) {info = arg;}
  


alu::alu() : A(), B(), Z() {}
alu::alu(shared_ptr<regist> Z,
	 shared_ptr<regist> B,
	 shared_ptr<regist> A) : A(A), B(B), Z(Z){}
void alu::add() {Z->info = trim_input(Z->bits, A->info.to_ulong() + B->info.to_ulong());}
void alu::sub() {Z->info = trim_input(Z->bits, A->info.to_ulong() - B->info.to_ulong());}
void alu::SHR(size_t id, size_t amnt) {
  if (A->id == id){
    (Z->info) = trim_input(Z->bits, (A->info) >>  amnt);}
  else if (B->id == id){
    (Z->info) = trim_input(Z->bits, (B->info) >>  amnt);}
  else {(cout << "id invávlido em SHR" <<endl);}
}
void alu::SHL(size_t id, size_t amnt) {
  if (A->id == id){
    (Z->info) = trim_input(Z->bits, (A->info) <<  amnt);}
  else if (B->id == id){
    (Z->info) = trim_input(Z->bits, (B->info) <<  amnt);}
  else {(cout << "id invávlido em SHL" << endl);}
}




size_t get_operator(bitset<max_bits> microcode,
		    size_t operator_size,
		    size_t operand_size,
		    size_t operand_amnt){
  return (trim_input(operator_size,
		     microcode >>(operand_size*operand_amnt))).to_ulong();
}

size_t get_operand(bitset<max_bits> microcode,
		   size_t operand_size,
		   size_t operand_index){
  return (trim_input(operand_size,
		     microcode >>(operand_size*operand_index))).to_ulong();
}




//resolver como tirar o template daqui

control_unit::control_unit(size_t cu_reg_s,
			   size_t operator_s,
			   size_t operand_s,
			   size_t operand_amnt) : cu_reg(), buses(),
						  regists_in_out(), map_reg_counter(0),
						  map_bus_counter(0), map_alu_counter(0),
						  map_mar_counter(0), map_mdr_counter(0),
						  operator_size(operator_s), operand_size(operand_s),
						  operand_amnt(operand_amnt){
  this->cu_reg = this->get_register(this->make_regist(cu_reg_s));}
  

  
size_t control_unit::make_bus(int bits){
  buses.insert(make_pair(map_bus_counter, make_shared<bus> (bits)));
  map_bus_counter++;
  return map_bus_counter - 1;
}

size_t control_unit::make_regist(int bits){
  regists_in_out.insert(make_pair(map_reg_counter,
				  make_tuple(make_shared<regist>(bits,map_reg_counter),
					     false,false)));
  map_reg_counter++;
  return map_reg_counter - 1;
}
//reolver as linkagens para permitir o o input de dados na memoria
size_t control_unit::make_mdr(int bits, const shared_ptr<memory> &mem){
  auto id = make_regist(bits);
  mdrs_id.insert(make_pair(map_mdr_counter, id));
  get_register(id)->link_in(mem->data_bus);
  get_register(id)->link_out(mem->data_bus);
  map_mdr_counter++;
  return map_mdr_counter - 1;
}
  
size_t control_unit::make_mar(const int bits, const shared_ptr<memory> &mem){
  auto id = make_regist(bits);
  mars_id.insert(make_pair(map_mar_counter, id));
  get_register(id)->link_out(mem->addr_bus);
  map_mar_counter++;
  return map_mar_counter - 1;
}
  
size_t control_unit::make_alu(shared_ptr<regist> A,
			      shared_ptr<regist> B,
			      shared_ptr<regist> Z){
  alu al(A,B,Z);
  alus.insert(make_pair(map_alu_counter, make_shared<alu>(A,B,Z)));
  map_alu_counter++;
  return map_alu_counter -1;
}
  
shared_ptr<regist> control_unit::get_register(size_t id){
  return get<0>(regists_in_out.at(id));}

shared_ptr<regist> control_unit::get_mar(size_t id){return get_register(mars_id.at(id));}
shared_ptr<regist> control_unit::get_mdr(size_t id){return get_register(mdrs_id.at(id));}
bool control_unit::get_register_in(size_t id){return get<1>(regists_in_out.at(id));}
bool control_unit::get_register_out(size_t id){return get<2>(regists_in_out.at(id));}
shared_ptr<bus> control_unit::get_bus(size_t id){return buses.at(id);}
shared_ptr<alu> control_unit::get_alu(size_t id){return alus.at(id);}
void control_unit::set_in(size_t id){get<1>(regists_in_out.at(id)) = true;}
void control_unit::set_out(size_t id){get<2>(regists_in_out.at(id)) = true;}

control_unit::control_unit(size_t arg) : cu_reg(), buses(),
					 regists_in_out(), map_reg_counter(0),
					 map_bus_counter(0), map_alu_counter(0),
					 map_mar_counter(0), map_mdr_counter(0){
  this->cu_reg = this->get_register(this->make_regist(arg));}
  




  
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



  
void control_unit::assignment(size_t id_reg1, size_t id_reg2){
  set_out(id_reg1);
  set_in(id_reg2);
  cout <<get_register_out(id_reg1) << "  "<< get_register_in(id_reg1) << endl
       <<get_register_in(id_reg2) <<"  " <<get_register_out(id_reg1) << endl;
}
  
void control_unit::add(size_t id){
  get_alu(id)->add();
    
  (cout << "A " << get_alu(id)->A->id << ": " << get_alu(id)->A->info << endl);
  (cout << "B " << get_alu(id)->B->id << ": " << get_alu(id)->B->info  << endl);
  (cout << "Z " << get_alu(id)->Z->id << ": " << get_alu(id)->Z->info << endl);
}

void control_unit::sub(size_t id){
  get_alu(id)->sub();
    
  (cout << "A " << get_alu(id)->A->id << ": " << get_alu(id)->A->info << endl);
  (cout << "B " << get_alu(id)->B->id << ": " << get_alu(id)->B->info  << endl);
  (cout << "Z " << get_alu(id)->Z->id << ": " << get_alu(id)->Z->info << endl);
}
//no momento só é possível usar SHR  e SHL no primeiro registrador da ALU (A)
void control_unit::SHR(size_t id_alu, size_t amnt){
  auto A_id = get_alu(id_alu)-> A->id;
  (get_alu(id_alu)->SHR(A_id, amnt));
    
  (cout << "A " << get_alu(id_alu)->A->id << ": " << get_alu(id_alu)->A->info << endl);
  (cout << "Z " << get_alu(id_alu)->Z->id << ": " << get_alu(id_alu)->Z->info << endl);
}

void control_unit::SHL(size_t id_alu, size_t amnt){
  auto A_id = (get_alu(id_alu)-> A->id);
  (get_alu(id_alu)->SHL(A_id, amnt));
      
  (cout << "A " << get_alu(id_alu)->A->id << ": " << get_alu(id_alu)->A->info << endl);
  (cout << "Z " << get_alu(id_alu)->Z->id << ": " << get_alu(id_alu)->Z->info << endl);
      
}

void control_unit::read(const shared_ptr<regist> &mar,
			const shared_ptr<regist> &mdr,
			const vector<shared_ptr<memory>> &memories){
  for(auto& mem:memories){
    if(find(mar->out.begin(), mar->out.end(), mem->addr_bus) != mar->out.end() &&
       find(mdr->in.begin(), mdr->in.end(), mem->data_bus) != mdr->in.end()){
      (mdr->set(mem->body.at(mar->info.to_ulong())));
    }
  }
}

void control_unit::write(const shared_ptr<regist> &mar,
			 const shared_ptr<regist> &mdr,
			 const vector<shared_ptr<memory>> &memories){
  for(auto& mem:memories){
    if(find(mar->out.begin(), mar->out.end(), mem->addr_bus) != mar->out.end() &&
       find(mdr->out.begin(), mdr->out.end(), mem->data_bus) != mdr->out.end()){
      (mem->body.at(mar->info.to_ulong()) = mdr->info.to_ulong());
    }
  }
}

void control_unit::execute(const vector<shared_ptr<memory>> &memories){
  bitset<max_bits> microcode(cu_reg->info);
  //auto m_inst = (microcode >> oper_b * 2).to_ulong();
  auto m_inst = get_operator(microcode, operator_size, operand_size, operand_amnt);
  cout << "m_inst: " << m_inst <<endl
       <<"microcode: " << microcode << endl;
  if (m_inst == 1){
    assignment(get_operand(microcode, operand_size, 0),
	       get_operand(microcode, operand_size, 1));
    (cout<< "assign" << endl);}
  else if(m_inst == 2) {
    add(get_operand(microcode, operand_size, 0));
    (cout<< "add: " << endl);
  }
  else if(m_inst == 3) {
    /// a ID do mdr e do mar utilizada é a sua sequencia em mdrs_id e mars_is, não a sua principal em registers_in_out!!!
    cout << "get_mdr: "
	 << get_operand(microcode, operand_size, 1)
	 << endl;	
    auto mdr = get_mdr(get_operand(microcode, operand_size, 1));
    cout << "get_mar: "
	 << get_operand(microcode, operand_size, 0)
	 << endl;
    auto mar = get_mar(get_operand(microcode, operand_size, 0));
    read(mar, mdr, memories);
    cout << "memread: " << endl;
  }
  else if(m_inst == 4){
    /// a ID do mdr e do mar utilizada é a sua sequencia em mdrs_id e mars_is, não a sua principal em registers_in_out!!!
    cout << "get_mdr: "
	 << get_operand(microcode, operand_size, 1)
	 << endl;	
    auto mdr = get_mdr(get_operand(microcode, operand_size,1));
    cout << "get_mar: "
	 << get_operand(microcode, operand_size, 0)
	 << endl;
    auto mar = get_mar(get_operand(microcode, operand_size, 0));
    write(mar, mdr, memories);
    (cout << "memwrite: " << endl);
  }
  else if(m_inst == 5){
    add(get_operand(microcode, operand_size, 0));
    (cout<< "sub(overflow, carry?): " << endl);
      
  }
  else if(m_inst == 6){
    SHL(get_operand(microcode, operand_size, 0),
	get_operand(microcode, operand_size, 1));
  }
  else if(m_inst == 7){
    SHR(get_operand(microcode, operand_size, 0),
	get_operand(microcode, operand_size, 1));
  }
};

void control_unit::reg_out(){
  vector<shared_ptr<regist>> outs;
    
  for(auto& pair:regists_in_out){
    //auto id = get<0>(pair);
    // cout << "id: " << id << endl
    // 		<< "in:" << get_register_in(id) << endl
    // 		<<"out:" << get_register_out(id) << endl;
    
    if(get<2>(pair.second) == true){
      outs.push_back(get<0>(pair.second));
      get<2>(pair.second) = false;}
  }
        

  //não verifica se tem out repetidos, colocar depois (?)
  //register -> bus
  for(auto& registrador:outs){
    for(auto& bus:registrador->out){
      (cout << "******* REGISTER " << registrador->id <<  "  ->  BUS ********" << endl
       << "bus antes: " <<bus->info << endl);
      (cout << "reg val : " <<registrador->info.to_ulong() << endl);
      bus->set(registrador->info.to_ulong());
      (cout << "bus depois: " <<bus->info << endl);}
  }
}
  
void control_unit::reg_in(){
  vector<shared_ptr<regist>> ins;
  // //bus -> register
  for(auto& pair:regists_in_out){
    if(get<1>(pair.second) == true){
      ins.push_back(get<0>(pair.second));
      get<1>(pair.second) = false;}}

  for(auto& registrador:ins){
    for(auto& bus:registrador->in){
      (cout << "******* BUS -> REGISTER "<< registrador->id <<" ********" << endl
       << "register  antes: " <<registrador->info << endl);
      registrador->set(bus->info.to_ulong());
      (cout << "register depois: " <<registrador->info << endl);
    }
  }
}

  


//************************************************************************************************************************************************


overseer::overseer() : control_units(), memories(){}
void overseer::cycle_old(){
    
  for(auto& cu:control_units){
    cu->execute(memories);
    cu->reg_out();
  }
    
  for(auto& cu:control_units){
    cu->reg_in();
  }
}
void overseer::cycle(){
  for(auto& cu:control_units){
    cu->opcode_execute(memories);
  }
}
// trocar os "at()" dos mapas por find().
// reavaliar o uso dos shared_ptrs, principalmente como argumento de funcões.
// implementar controle sobre ins e outs individuais dos registradores.(?)



