#pragma once
#ifndef ARCHLIB_H
#define ARCHLIB_H

#include <cassert>
#include <string>
#include <hash_map>
#include "utils.h"
#include "netlist.h"

namespace FDP { namespace archlib {
	
	using namespace netlist;
	using std::vector;
	using std::pair;
	using stdext::hash_multimap;
	using std::string;

	typedef ARCHNETTYPE PORTTYPE;

	class ArchPort;class ArchInstance;class ArchCell;class ArchTile;class FPGADesign;struct PadInfo;

	typedef Net ArchNet;
	typedef Pin ArchPin;

	struct sitePinToPinwire {
		sitePinToPinwire(string _sitePinName = "", string _siteName = "", string _pinWireName = "", short _z = -1);
		string	sitePinName;
		string	siteName;
		string	pinWireName;
		short	z;
	};

	class ArchFactory : public NLFactory
	{
	public:
		Cell*		makeCell(const char* name, const char* type, const Library* owner);
		Instance*	makeInstance(const char* name, const Cell* owner, Cell* instof);
		Port*		makePort(const char* name, const Cell* owner);
		Pin*		makePin(const char* name, const Instance* owner = NULL);
		ArchTile*	makeTile(const char* name, const char* type, const Library* owner);
		PadInfo*	makePad(const char* name,const Point& p);
	};

	class ArchPIP
	{
	public:
		ArchPIP(ArchPin* _from, ArchPin* _to);
		void			setfrom(ArchPin* port)			{ from=port; }
		void			setto(ArchPin* port)			{ to=port; }
		const ArchPin*	getfrom()	const				{ return from; }
		const ArchPin*	getto()		const				{ return to; }
	private:
		ArchPin* from;
		ArchPin* to;
		//timing info needs to be added
	};

	class ArchPort : public Port
	{
	public:
		void setside(SIDE s)		{ side = s; }
		void setserial(short s)		{ serial = s; }
		void setdir(DIRECTION d)	{ dir = d; }

		SIDE		getside()	const	{ return side; }
		short		getserial() const	{ return serial; }
		DIRECTION	getdir()	const   { return dir; }
		
		ArchPort(const char* s,const ArchCell* c);
		//	void settype()			{}

	private:
		SIDE		side;
		short		serial;
		PORTTYPE	type;
		DIRECTION	dir;
	};

	class ArchCell : public Cell
	{
	public:
		ArchCell(const char* _name, const char* _type, const Library* _owner, HIERARCHY _h);
		typedef vector<ArchPIP>						pip_type;
		typedef vector<ArchPIP>::const_iterator		const_path_iter;
		typedef vector<ArchPIP>::iterator			path_iter;

		virtual	bool			empty()			const		{ return paths.empty(); }
		HIERARCHY				gethierarchy()	const		{ return hierarchy; }
		void					sethierarchy(HIERARCHY h)	{ hierarchy = h; }
		const_path_iter			path_begin()	const		{ return paths.begin();}
		const_path_iter			path_end()		const		{ return paths.end();}	
		virtual void			relativepins(vector<ArchPin*>&,const ArchPin*);
		virtual void			deHierarchy();
		virtual void			creatpath(ArchPin* lhs, ArchPin* rhs);
		virtual void			build(ArchInstance*);
	private:
		HIERARCHY		hierarchy;
		pip_type		paths;
	};

	class ArchTile : public ArchCell
	{
	public:
		ArchTile(const char* _name, const char* _type, const Library* _owner);
		//store the mapping relationship of the ports, between which no physical pip exists
		typedef hash_multimap<short,short>							bypass_type;
		typedef hash_multimap<short,short>							pip_type;
		typedef pip_type::const_iterator							const_pip_iter;
		typedef const_pip_iter										const_bypass_iter;
		typedef vector<string>										name_type;					

		const pip_type&		getpips() const				{ return pips; }
		void				creatpath(ArchPin* lhs, ArchPin* rhs);
		void				setbypass();
		const_bypass_iter   bypass_begin(short i)	const { return bypasses.lower_bound(i); }
		const_bypass_iter   bypass_end(short i)		const { return bypasses.upper_bound(i); }
		const_pip_iter 		pip_begin()				const { return pips.begin(); }
		const_pip_iter 		pip_end()				const { return pips.end(); }
		const_pip_iter 		pip_begin(short i)		const { return pips.lower_bound(i); }
		const_pip_iter 		pip_end(short i)		const { return pips.upper_bound(i); }
		const_pip_iter		reverse_begin(short i)	const { return reverse.lower_bound(i);}
		const_pip_iter		reverse_end(short i)	const { return reverse.upper_bound(i);}
		string				getportname(short i)	const { return portnames.at(i);}
		void				setportname(short i,const char* name);
		short				find_portindex_by_name(const string&);
		short				portsize()				const { return portnames.size();}
		void				build(ArchInstance*);
		//create pips for connections between sites inside a tile
		void				increasepoints(vector<ArchPin*>&,const ArchPin*);
		void				relativepins(vector<ArchPin*>&,const ArchPin*);
		bool				empty()		const		{ return pips.empty(); }
		void				build_nets_of_connection_with_tile_and_slice();
		void				build_pin_to_pinwire_name_map(vector<sitePinToPinwire>&)		const;

	private:
		//in bypasses, there is no actual pip; in pips, there is
		bypass_type		bypasses;
		pip_type		pips,reverse;
		//store the actual name of each port which belongs to the tile, accessed by the unique index
		name_type		portnames;
		short			portnum;
		//store the ports that only belong to the sites inside the tile
	};

	class ArchInstance : public Instance
	{
	public:
		ArchInstance(const char* s, const ArchCell* owner, ArchCell* instof);

		typedef vector<ArchPIP>						pip_type;
		typedef vector<ArchPIP>::const_iterator		const_path_iter;
		typedef vector<ArchPIP>::iterator			path_iter;

		Point			getposition()	const		{ return pos; }
		void			setposition(Point& p)		{ pos = p; }
		short			getx()		const			{ return pos.getx(); }
		short			gety()		const			{ return pos.gety(); }
		short			getz()		const			{ return pos.getz(); }

		const_path_iter	path_begin()const	{ return paths.begin();}
		const_path_iter	path_end()	const	{ return paths.end();}
		void			creatpath(ArchPin* lhs, ArchPin* rhs);

		void			build();
		bool			empty()		const	{ return paths.empty(); }
	private:
		Point		pos;
		pip_type	paths;
	};

	struct PadInfo
	{
		string  name;
		Point   pos;
		PadInfo(const char* _name,Point _p):name(_name),pos(_p){}
	};

	class FPGADesign : public Design
	{
	public:
		FPGADesign(const char* s);
		~FPGADesign();
		const char*		getpackage()				{ return package.c_str(); }
		const Point&	getscale()					{ return scale; }
		void			setscale(const Point& _p)	{ scale = _p; }
		void			setpackage(const char* pkg) { package = pkg; }
		void			addpad(PadInfo* pad)		{ pads.push_back(pad); }
		void			settopinfo();//reorganize the storage of tile instances to make convenience for fetching inst with Point
		ArchInstance*	get_inst_by_pos(const Point& p) const{
			if(p.getx() > scale.getx() || p.gety() > scale.gety()) return NULL;
			if(p.getx() < 0 || p.gety() < 0) return NULL;
			ArchInstance* inst = topInfo[p.getx()][p.gety()];
			assert(inst);
			return inst;
		}
		Point			get_pos_by_tilename(const string&)	const;
		Point			get_pos_by_iobname(const char*)	const;

	private:
		ArchInstance*		topInfo[SCALEX][SCALEY];
		string				package;
		Point				scale;
		vector<PadInfo*>	pads;
	};

	short transform_within_onetile(SIDE s,short i);
}}



#endif
