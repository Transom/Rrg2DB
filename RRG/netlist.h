#pragma once
#ifndef NETLIST_H
#define NETLIST_H

#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <cstring>
#include "object.h"

namespace FDP { namespace netlist {

	using std::vector;
	using std::string;
	using std::cout;
	using std::endl;

	class Library; class Cell; class Port; class Net; class Design; class Instance;class Pin;
	
	class NLFactory {
	public:
		virtual ~NLFactory() {}
		virtual Library*  makeLibrary(const char* name, const Design* owner); //why virtual?
		virtual Cell*     makeCell(const char* name, const char* type, const Library* owner);
		virtual Instance* makeInstance(const char* name, const Cell* owner, Cell* instof);
		virtual Net*      makeNet(const char* name, const Cell* owner);
		virtual Port*     makePort(const char* name, const Cell* owner);
		virtual Pin*	  makePin(const char* s,const Instance* owner);
	};

	class Pin : public Object
	{
	public:
		typedef vector<Net*>					net_type;
		typedef vector<Net*>::const_iterator	const_net_iter;

		const_net_iter		net_begin()			const			{ return nets.begin(); }
		const_net_iter		net_end()			const			{ return nets.end(); }
		void				addnet(Net* n)						{ nets.push_back(n); }
		void				setport(Port* p)					{ port = p; }
		void				setcellport(Port* p)				{ assert(!getowner()); cellport = p; }
		const Port*			getport()			const			{ return port; }
		const Port*			getcellport()		const			{ assert(!getowner()); return cellport; }

		Pin(const char* s, const Instance* _owner = NULL);//the owner of Pin is Instance
	private:
		Port*		port;
		Port*		cellport;		
		net_type	nets;
	};


	class Port : public Object
	{
	public:
		Port(const char *s, const Cell* _owner);
		const Pin*  getcellpin()	const		{	return cellpin; }
		void		setcellpin(Pin* p)			{	cellpin = p; }
		virtual ~Port();
	private:
		Pin*   cellpin;
		/*****************************/
		//timing info to be added here//
		/*****************************/
	};

	class Instance : public Object
	{
	public:
		typedef vector<Pin*>					pin_type;
		typedef vector<Pin*>::const_iterator	const_pin_iter;
		typedef vector<Pin*>::iterator			pin_iter;

		Cell*		getcell()			const	{
			return ptr;
		}
		bool	findpin(const_pin_iter& iter,const Pin* p)		const		{
			const_pin_iter beg = pin_begin(), end = pin_end();
			for (;beg != end; ++beg)	
				if ( !strcmp((*beg)->getname(),p->getname()) )  { iter = beg; return true;}
				return false;
		}
		bool	findpin(const_pin_iter& iter,const string& pname)		const		{
			const_pin_iter beg = pin_begin(), end = pin_end();
			for (;beg != end; ++beg)	
				if ( !strcmp((*beg)->getname(),pname.c_str()) )  { iter = beg; return true;}
				return false;
		}

		const_pin_iter		pin_begin()		const	{ return pinVec.begin(); }
		const_pin_iter		pin_end()		const	{ return pinVec.end(); }
		pin_iter			pin_begin_m()			{ return pinVec.begin(); }
		pin_iter			pin_end_m()				{ return pinVec.end(); }

		void				addpin(Pin* p)		{ pinVec.push_back(p); }
		void				destroy();
		void				clear()				{ pinVec.clear(); }

		Instance(const char *s,const Cell* _owner,Cell* _ptr);
		virtual ~Instance();

	private:
		Cell*		ptr;
		pin_type	pinVec;
	};

	class Net : public Object {
	public:
		typedef vector<Pin*>				pin_type;
		typedef pin_type::const_iterator	const_pin_iter;

		Net(const char *s, const Cell* _owner);

		bool dangle() const {	
			if ( pinVec.size() < 2 ) return true; 
			else return false;
		}
		bool empty() const { return src.size() == 0 || sinks.size() == 0; }

		const_pin_iter		pin_begin()		const	{ return pinVec.begin(); }
		const_pin_iter		pin_end()		const	{ return pinVec.end(); }
		const_pin_iter		src_begin()		const	{ return src.begin(); }
		const_pin_iter		src_end()		const	{ return src.end(); }
		const_pin_iter		sink_begin()	const	{ return sinks.begin(); }
		const_pin_iter		sink_end()		const	{ return sinks.end(); }

		void				addpin(Pin* p)			  { pinVec.push_back(p); }
		void				addsrc(Pin* p)			  { src.push_back(p); }
		void				addsink(Pin* p)			  { sinks.push_back(p);}
		size_t				pinsize()		const	  { return pinVec.size(); }

	private:
		pin_type	pinVec;
		pin_type	src,sinks;
	};

	class Cell : public Object
	{
	public:
		typedef vector<Port*>					port_type;
		typedef vector<Instance*>				inst_type;
		typedef vector<Net*>					net_type;
		typedef	vector<Port*>::const_iterator		const_port_iter;
		typedef vector<Instance*>::const_iterator	const_inst_iter;
		typedef vector<Net*>::const_iterator		const_net_iter;
		typedef vector<Port*>::iterator			port_iter;
		typedef vector<Instance*>::iterator		inst_iter;
		typedef vector<Net*>::iterator			net_iter;

		bool no_ports() const { return portVec.empty(); }
		bool no_insts() const { return instVec.empty(); }
		bool no_nets()	const { return netVec.empty(); }

		const_port_iter		port_begin()	const { return portVec.begin(); }
		const_net_iter		net_begin()		const { return netVec.begin(); }
		const_inst_iter		inst_begin()	const { return instVec.begin(); }

		const_port_iter		port_end()	const { return portVec.end(); }
		const_net_iter		net_end()	const { return netVec.end(); }
		const_inst_iter		inst_end()	const { return instVec.end(); }

		bool findport(const_port_iter& i, const Port* p) const {
			const_port_iter iter, beg = port_begin(), end = port_end();
			for (iter = beg; iter != end; ++iter)	if ( *iter == p) { i = iter; return true;}
			return false;
		}

		bool findport(const_port_iter& i, const string& pname) const { //error4 
			const_port_iter iter, beg = port_begin(), end = port_end();
			for (iter = beg; iter != end; ++iter)
			{
				string temp((*iter)->getname());
				if (strcmp((*iter)->getname(),pname.c_str()) ==0) //! int is  not adaptable
				{ i = iter; return true;}
			}
			return false;
		}

		bool findinst(const_inst_iter& i, const Instance* inst) const {
			const_inst_iter iter, beg = inst_begin(), end = inst_end();
			for (iter = beg; iter != end; ++iter)	if (*iter == inst) { i = iter; return true;}
			return false;
		}

		bool findinst(const_inst_iter& i, const string& iname) const {
			const_inst_iter iter, beg = inst_begin(), end = inst_end();
			for (iter = beg; iter != end; ++iter)	if ( !strcmp((*iter)->getname(),iname.c_str()) ) { i = iter; return true;}
			return false;
		}

		bool findnet(const_net_iter& i, const Net* net) const {
			const_net_iter iter, beg = net_begin(), end = net_end();
			for (iter = beg; iter != end; ++iter)	if (*iter == net) { i = iter; return true;}
			return false;
		}

		bool findnet(const_net_iter& i, const string& nname) const {
			const_net_iter iter, beg = net_begin(), end = net_end();
			for (iter = beg; iter != end; ++iter)	if ( !strcmp((*iter)->getname(),nname.c_str()) ) { i = iter; return true;}
			return false;
		}

		void			addinst(Instance* inst)			{ instVec.push_back(inst); }
		void			addnet(Net* net)				{ netVec.push_back(net); }
		void			addport(Port* p)				{ portVec.push_back(p); }

		const inst_type		getinsts() const { return instVec; }
		const net_type		getnets()  const { return netVec; }
		const port_type		getports() const { return portVec; }
		string				gettype()  const { return type; }

		Cell(const char* s,const char* _type, const Library* _owner);
		virtual ~Cell();

	private:
		port_type		portVec;
		inst_type		instVec;
		net_type		netVec;
		string			type;
	};
	
	class Library : public Object {
	public:
		typedef vector<Cell*>				cell_type;
		typedef cell_type::const_iterator	const_cell_iter;
		typedef cell_type::iterator			cell_iter;
		Library(const char *s,const Design* _owner);
		~Library();

		const_cell_iter		const_cell_begin()		const	{ return eleVec.begin(); }
		const_cell_iter		const_cell_end()		const	{ return eleVec.end(); }
		cell_iter			cell_begin()					{ return eleVec.begin(); }
		cell_iter			cell_end()						{ return eleVec.end(); }

		bool findelem(const_cell_iter& i, const Cell* c) const {
			const_cell_iter iter, beg = const_cell_begin(), end = const_cell_end();
			for (iter = beg; iter != end; ++iter)	if (*iter == c) { i = iter; return true; }
			return false;
		}

		bool findelem(const_cell_iter& i, const string& ename) const {
			const_cell_iter iter, beg = const_cell_begin(), end = const_cell_end();
			for (iter = beg; iter != end; ++iter)	if ( !strcmp((*iter)->getname(),ename.c_str()) ) { i = iter; return true; }
			return false;
		}

		void addelem(Cell* c)	{ eleVec.push_back(c);	} 

	private:
		cell_type eleVec;
	};

	class Design : public Object {
	public:
		Design(const char *s);
		virtual ~Design();

		typedef vector<Instance*>			inst_type;
		typedef vector<Library*>			lib_type;
		typedef inst_type::const_iterator	const_inst_iter;
		typedef inst_type::iterator			inst_iter;
		typedef lib_type::const_iterator	const_lib_iter;
		typedef lib_type::iterator			lib_iter;
		
		bool findinst(const_inst_iter& i, const Instance* inst) const {
			for (const_inst_iter iter = top.begin(); iter != top.end(); ++iter)
				if (*iter == inst) { i = iter; return true;}
			return false;
		}

		bool findinst(const_inst_iter& i, const string& iname) const {
			for (const_inst_iter iter = top.begin(); iter != top.end(); ++iter)
				if ( !strcmp((*iter)->getname(),iname.c_str()) ) { i = iter; return true;}
			return false;
		}

		bool findlib(const_lib_iter& i, const Library* lib) const {
			for (const_lib_iter iter = librarys.begin(); iter != librarys.end();++iter) 
				if (*iter==lib) { i = iter; return true; }
			return false;
		}

		bool findlib(const_lib_iter& i, const string& lname) const {
			for (const_lib_iter iter = librarys.begin(); iter != librarys.end();++iter) 
				if ( !strcmp((*iter)->getname(),lname.c_str()) ) { i = iter; return true; }
				return false;
		}

		const_lib_iter lib_begin()		const		{ return librarys.begin(); }
		const_lib_iter lib_end()		const		{ return librarys.end(); }

		void addinst(Instance* inst)		{ top.push_back(inst);   }
		void addlib(Library* lib)			{ librarys.push_back(lib); }
		const inst_type& getinsts() const	{ return top; }

	private:
		inst_type	top;
		lib_type	librarys;
	};

}
}



#endif