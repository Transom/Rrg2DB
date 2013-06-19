#pragma once
#ifndef RRG_H
#define RRG_H

#include <vector>
#include <hash_map>
#include <cassert>
#include <map>
#include "archlib.h"
#include "utils.h"

namespace FDP { namespace RRG {

	using namespace archlib;
	using std::vector;
	using stdext::hash_map;
	using std::multimap;
	using std::map;
	using std::make_pair;

	class WireTemplate; class TileTemplate; class Rrg;

	struct HopUnit
	{
		HopUnit(short _x = HUGENEGATIVE,short _y = HUGENEGATIVE, short _i = HUGENEGATIVE);
		HopUnit(const HopUnit& hop);
		HopUnit& operator=(const HopUnit& hop);
		short x,y;//related coordinates
		short index;//port index
	};


	struct TileUnit
	{
		TileUnit();
		TileUnit(const char*,TileTemplate*);
		typedef hash_map<short,WireTemplate*>::iterator iter;
		string					getwirename(short i)		const;
		WireTemplate*			getwire(short i)			const;
		string		            gettilename()				const   { return name;}
		TileTemplate*			getTileTemplate()			const	{ return tile_type;}
		short					find_wireindex_by_name(const string&)	const;
		string					name;
		TileTemplate*			tile_type;
		mutable hash_map<short,WireTemplate*>	long_global_nets;
	};

	class WireTemplate
	{
	public:
		WireTemplate(const char*);
		typedef vector<HopUnit>::const_iterator const_hop_unit_iter;
		void					addpip(HopUnit h)		{ fanouts.push_back(h);  }
		void					addseg(HopUnit h)		{ wire_segments.push_back(h);  }
		bool					findpip(const HopUnit& h)	const;
		bool					findseg(const HopUnit& h)	const;
		size_t					size()		const		{ return fanouts.size()+wire_segments.size(); }
		size_t					pipsize()	const		{ return fanouts.size(); }
		size_t					segsize()	const		{ return wire_segments.size(); }
		string					getname()	const		{ return name; }
		const_hop_unit_iter		pip_begin()		const		{ return fanouts.begin(); }
		const_hop_unit_iter		pip_end()		const		{ return fanouts.end();   }
		const_hop_unit_iter		seg_begin()		const		{ return wire_segments.begin(); }
		const_hop_unit_iter		seg_end()		const		{ return wire_segments.end();   }
		void					get_children_of_local_tile(vector<HopUnit>& children, const HopUnit& absolute)	const;//the params are all representing absolute infos
		void					get_children_of_other_tiles(vector<HopUnit>& children, const HopUnit& absolute)	const;
		void					setIndex(const  short &_indxe){ index=_indxe;}
		unsigned short			getIndex()const{ return index;}
	private:
		vector<HopUnit>		fanouts,wire_segments;
		string				name;
		unsigned short		index;
	};

	class TileTemplate
	{
	public:
		TileTemplate();
		typedef vector<WireTemplate*>::const_iterator const_wire_iter;
		void				addwire(size_t i, WireTemplate* w);
		const_wire_iter		begin()				const		{ return wires.begin(); }
		const_wire_iter		end()				const		{ return wires.end(); }
		size_t				size()				const		{ return wires.size(); }
		WireTemplate*		findwire(size_t i)  const		{ return wires.at(i); }
		short				find_wireindex_by_name(const string&)	const;//for net initialization
	private:
		//as vector is implemented using linear array in stl, elements in vector can be accessed through index
		//accessing through index provides much quicker speed
		vector<WireTemplate*> wires;
	};

	class Rrg
	{
	public:
		Rrg(FPGADesign*);
		~Rrg();
		typedef					multimap<string,WireTemplate*>::const_iterator		const_wire_iter;
		typedef					multimap<size_t,TileTemplate*>::const_iterator		const_tile_iter;

		const_wire_iter		wire_begin()		const		{ return wirestore.begin(); }
		const_wire_iter		wire_end()			const		{ return wirestore.end(); }
		const_tile_iter		tile_begin()		const		{ return tilestore.begin(); }
		const_tile_iter		tile_end()			const		{ return tilestore.end(); }
		const_wire_iter		wire_begin(string& name)		const		{ return wirestore.lower_bound(name); }
		const_wire_iter		wire_end(string& name)			const		{ return wirestore.upper_bound(name); }
		const_tile_iter		tile_begin(size_t& size)		const		{ return tilestore.lower_bound(size); }
		const_tile_iter		tile_end(size_t& size)			const		{ return tilestore.upper_bound(size); }

		//construct the routing resource graph
		void					build();
		bool					findwire(const_wire_iter& i,const WireTemplate* t)	const;
		bool					findtile(const_tile_iter& i,const TileTemplate* t)	const;
		void					find_children_of_local_tile(vector<HopUnit>& children, const HopUnit& father);
		void					find_children_of_other_tiles(vector<HopUnit>& children, const HopUnit& father);
		void					addwire(string name,WireTemplate* t)				{ wirestore.insert(make_pair(name,t)); }
		void					addtile(size_t s,TileTemplate* t)				{ tilestore.insert(make_pair(s,t)); }
		void					searchconnection(vector<HopUnit>&,short,short,short);
		void					collector(vector<HopUnit>&,short,short,short);
		void					find_connection_with_othertiles(vector<HopUnit>&,short,short,short);
		void					searchbypass(vector<short>&,short,short,short);
		void					searchpip(vector<HopUnit>&,short,short,short);
		bool					is_driver(short,short,short);
		bool					is_bypass(short,short,short);
		bool					cannot_be_driven(short,short,short);
		ArchTile*				gettile(short x,short y)	const			{
			if(x <0 || y < 0 || x >= fpga->getscale().getx() || y >= fpga->getscale().gety()) return NULL;
			ArchInstance* inst = fpga->get_inst_by_pos(Point(x,y,0));
			assert(inst);
			ArchTile* tile = static_cast<ArchTile*>(inst->getcell());
			assert(tile);
			return tile;
		}
		WireTemplate*			creat_wire_template(short,short,short);
		TileTemplate*			creat_tile_template(short,short);
		const TileUnit&			get_tile_by_pos(const Point& p) const{
			if (p.getx() <0 || p.gety() < 0 || p.getx() >= fpga->getscale().getx() || p.gety() >= fpga->getscale().gety()) return topinfo[SCALEX-1][SCALEY-1];
			return topinfo[p.getx()][p.gety()];
		}
		short							find_wireindex_by_name_and_pos(const Point& p,const string& _name);
		FPGADesign*						getfpga()			const		{ return fpga; }
		void							create_pinwire_map(short x, short y);
		const vector<sitePinToPinwire>& getpinwire_vec  (short x,short y)const{ return pinwiremap[x][y];}
		Point							get_fpga_scale()const{return fpga_scale;}
		size_t							wire_size()const{return wirestore.size();}
	private:
		Point					fpga_scale;
		FPGADesign*				fpga;
		TileUnit				topinfo[SCALEX][SCALEY];
		vector<sitePinToPinwire>		pinwiremap[SCALEX][SCALEY];
		multimap<string,WireTemplate*>	wirestore;//store the only one copy of each wire template
		multimap<size_t,TileTemplate*>	tilestore;//store the only one copy of each tile template
	};

	inline bool operator==(const HopUnit& lhs,const HopUnit& rhs);
	inline bool operator!=(const HopUnit& lhs,const HopUnit& rhs);
	inline HopUnit	operator+(const HopUnit& lhs,const HopUnit& rhs);
	bool operator==(const WireTemplate& lhs,const WireTemplate& rhs);
	bool operator!=(const WireTemplate& lhs,const WireTemplate& rhs);
	bool operator==(const TileTemplate& lhs,const TileTemplate& rhs);
	bool operator!=(const TileTemplate& lhs,const TileTemplate& rhs);

	bool islong(std::string);
	bool isclk(std::string);
	bool isdouble(std::string);
	bool ishex(std::string);
	bool isglobal(std::string);

	short transform_between_tiles(short i, short& x,short& y);

}}







#endif