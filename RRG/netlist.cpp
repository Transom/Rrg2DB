#include <cassert>
#include "netlist.h"

namespace FDP { namespace netlist {

	//netlist factory
	Library*  NLFactory::makeLibrary(const char* name, const Design* owner)					{ return new Library(name, owner); }
	Cell*     NLFactory::makeCell(const char* name, const char* type, const Library* owner)
	{ 
		assert(owner); 
		return new Cell(name, type, owner);
	}
	Instance* NLFactory::makeInstance(const char* name, const Cell* owner, Cell* instof)	{ return new Instance(name, owner, instof); }
	Net*      NLFactory::makeNet(const char* name, const Cell* owner)						{ return new Net(name, owner); }
	Port*     NLFactory::makePort(const char* name, const Cell* owner)						{ return new Port(name, owner); }
	Pin*	  NLFactory::makePin(const char* name,const Instance* owner)					{ return new Pin(name,owner); }//cellpin have no owner

	//constructors
	Pin::Pin(const char* s, const Instance* _owner):Object(s,_owner),port(0),cellport(0){}
	Port::Port(const char *s, const Cell* _owner):Object(s,_owner),cellpin(NULL){}
	Cell::Cell(const char* s,const char* _type, const Library* _owner):Object(s,_owner),type(_type){}
	Net::Net(const char *s, const Cell* _owner):Object(s,_owner){}
	Library::Library(const char *s,const Design* _owner):Object(s,_owner){}
	Design::Design(const char *s):Object(s,NULL){}
	Instance::Instance(const char *s,const Cell* _owner,Cell* instof):Object(s,_owner),ptr(instof){
		Cell::const_port_iter beg = instof->port_begin(), end = instof->port_end();
		while (beg != end) {
			Pin* p = new Pin((*beg)->getname(), this);
			p->setport(*beg);
			addpin(p); ++beg;
		}
	}

	//destructors
	Port::~Port(){	delete cellpin; }
	Cell::~Cell(){
		for (inst_iter iter = instVec.begin(); iter != instVec.end(); ++iter)	delete *iter;
		for (port_iter iter = portVec.begin(); iter != portVec.end(); ++iter)	delete *iter;
		for (net_iter iter = netVec.begin(); iter != netVec.end(); ++iter)		delete *iter;
	}
	Instance::~Instance(){
		destroy();
	}
	Library::~Library(){	
		for (cell_iter iter = eleVec.begin(); iter != eleVec.end(); ++iter)   delete *iter;
	}
	Design::~Design(){
		for (inst_iter iter = top.begin(); iter != top.end();++iter)   delete *iter;
		for (lib_iter iter = librarys.begin(); iter != librarys.end(); ++iter) delete *iter;
	}

	void Instance::destroy(){
		if(pinVec.empty()) return;
		for (pin_iter iter = pinVec.begin(); iter != pinVec.end(); ++iter)   delete *iter;
	}
	
}}
