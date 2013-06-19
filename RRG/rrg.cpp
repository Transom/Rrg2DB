#include <cstring>
#include <cassert>
#include <algorithm>
#include <fstream>
#include "rrg.h"

namespace FDP { namespace RRG {

	using namespace archlib;
	using std::pair;
	using std::make_pair;
	using std::cout;
	using std::endl;

	FDP::SIDE_MAP side_map;

	short	FDP::archlib::transform_within_onetile(SIDE s,short i);

	inline bool islong(string name){
		return (name.find("LV") != string::npos) || (name.find("LH") != string::npos);
	}
	inline bool isclk(string name){
		return name.find("CLK") != string::npos;
	}
	inline bool isdouble(string name){
		return (name.find("W2") != string::npos) || (name.find("E2") != string::npos)
			|| (name.find("S2") != string::npos) || (name.find("N2") != string::npos);
	}
	inline bool ishex(string name){
		return (name.find("W6") != string::npos) || (name.find("E6") != string::npos)
			|| (name.find("S6") != string::npos) || (name.find("N6") != string::npos);
	}

	void combine(vector<HopUnit>& lhs,vector<HopUnit>& rhs){
		vector<HopUnit>::const_iterator beg = rhs.begin(), end = rhs.end();
		while (beg != end)	{ lhs.push_back(*beg);++beg; }
	}

	short findmin(vector<short>& indexs){
		short MIN = HUGEPOSITIVE;
		unsigned short i = 0;
		while(i < indexs.size()){
			if(indexs[i] < MIN) MIN = indexs[i]; ++i;
		}
		return MIN;
	}

	inline short transform_between_tiles(short i, short& x,short& y) {
		assert(i/PORTNUM < SIDENUM);
		SIDE	from,to;
		switch (i/PORTNUM) {
			case 0: from = LEFT; break;
			case 1: from = RIGHT; break;
			case 2: from = BOTTOM; break;
			case 3: from = TOP; break;
		}
		to = side_map(from);
		switch (from) {
			case LEFT:  x = 0, y = -1; break;
			case RIGHT:  x = 0, y = 1; break;
			case TOP:  x = -1, y = 0; break;
			case BOTTOM:  x = 1, y = 0; break;
		}
		short	serial = i%PORTNUM;
		return	transform_within_onetile(to,serial);
	}

	bool	operator==(const HopUnit& lhs,const HopUnit& rhs){ return lhs.x==rhs.x && lhs.y==rhs.y && lhs.index==rhs.index; }
	bool	operator!=(const HopUnit& lhs,const HopUnit& rhs){ return !(lhs==rhs); }
	HopUnit	operator+(const HopUnit& lhs,const HopUnit& rhs){
		return HopUnit(lhs.x+rhs.x, lhs.y+rhs.y, rhs.index);
	}

	inline HopUnit& HopUnit::operator = (const HopUnit& rhs){
		x = rhs.x; y = rhs.y; index = rhs.index;
		return *this;
	}

	bool operator==(const WireTemplate& lhs,const WireTemplate& rhs) {
		if (strcmp(lhs.getname().c_str(),rhs.getname().c_str()) || lhs.size()!=rhs.size()) return false;
		if(lhs.getIndex()!=rhs.getIndex())return false;
		WireTemplate::const_hop_unit_iter beg1 = lhs.seg_begin(),end1 = lhs.seg_end(),beg2 = rhs.seg_begin(),end2 = rhs.seg_end();							
		for (;beg1!=end1 && beg2!=end2;++beg1,++beg2) { if (*beg1 != *beg2) break; }
		if (beg1!=end1) return false;
		WireTemplate::const_hop_unit_iter beg3 = lhs.pip_begin(),end3 = lhs.pip_end(),beg4 = rhs.pip_begin(),end4 = rhs.pip_end();
		for (;beg3!=end3 && beg4!=end4;++beg3,++beg4) { if (*beg3 != *beg4) break; }
		if (beg3!=end3) return false;
		return true;
	}
	bool operator!=(const WireTemplate& lhs,const WireTemplate& rhs) { return !(lhs == rhs); }

	bool operator==(const TileTemplate& lhs,const TileTemplate& rhs) {
		if (lhs.size() != rhs.size())	return false;
		TileTemplate::const_wire_iter beg1 = lhs.begin(), end1 = lhs.end(),beg2 = rhs.begin();
		for (;beg1 != end1;++beg1,++beg2) { if (*beg1 != *beg2) break; }
		if (beg1 != end1) return false;
		return true;
	}
	bool operator!=(const TileTemplate& lhs,const TileTemplate& rhs) { return !(lhs == rhs); }

	//constructors
	HopUnit::HopUnit(short _x,short _y, short _i):x(_x),y(_y),index(_i){}
	HopUnit::HopUnit(const HopUnit& hop){ x = hop.x; y = hop.y; index = hop.index; }
	TileUnit::TileUnit():name(""),tile_type(NULL){}
	TileUnit::TileUnit(const char* _name,TileTemplate* _t):name(_name),tile_type(_t){}
	WireTemplate::WireTemplate(const char* _name):name(_name){}
	//in the default constructor, resize the vector
	TileTemplate::TileTemplate(){ wires.resize(PORTNUM*SIDENUM,NULL); }
	Rrg::Rrg(FPGADesign* _fpga):fpga(_fpga){}

	//destructors
	Rrg::~Rrg() {
		for (const_wire_iter iter = wire_begin(); iter != wire_end(); ++iter)	delete iter->second;
		for (const_tile_iter iter = tile_begin(); iter != tile_end(); ++iter)	delete iter->second;
	}

	string			TileUnit::getwirename(short i)		const		{
		return getwire(i)==NULL ? string("") : getwire(i)->getname();
	}

	short			TileUnit::find_wireindex_by_name(const string& _name)	const {
		short  index = tile_type->find_wireindex_by_name(_name);
		if(index == HUGENEGATIVE){
			iter beg = long_global_nets.begin(), end = long_global_nets.end();
			while (beg != end){
				if(!(beg->second)->getname().compare(_name)) return beg->first;
				++beg;
			}
			index = HUGENEGATIVE;
		}
		return index;
	}

	//return wireTemplate by Index. First find out it belongs to Longline or not.
	WireTemplate*	TileUnit::getwire(short i)			const		{
		if(long_global_nets.count(i))
			return long_global_nets[i];
		else 
			return tile_type->findwire(i);
	}

	bool			WireTemplate::findpip(const HopUnit& h)		const{
		const_hop_unit_iter iter = find(pip_begin(),pip_end(),h);
		return iter != pip_end();
	}

	bool			WireTemplate::findseg(const HopUnit& h)		const{
		const_hop_unit_iter iter = find(seg_begin(),seg_end(),h);
		return iter != seg_end();
	}

	void			WireTemplate::get_children_of_local_tile(vector<HopUnit>& children, const HopUnit& absolute)		const   {
		const_hop_unit_iter _beg = pip_begin(), _end = pip_end();
		while(_beg != _end){
			children.push_back(absolute+*_beg);
			++_beg;
		}
	}

	void			WireTemplate::get_children_of_other_tiles(vector<HopUnit>& children, const HopUnit& absolute)  const {
		const_hop_unit_iter _beg = seg_begin(), _end = seg_end();
		while(_beg != _end){
			children.push_back(absolute+*_beg);
			++_beg;
		}
	}

	void			TileTemplate::addwire( size_t i, WireTemplate* w ){
		try {	wires.at(i) = w;	}
		catch ( std::out_of_range& e ){
			e.what();
			wires.resize(i+100,NULL);
			wires.at(i) = w;
		}
	}

	short			TileTemplate::find_wireindex_by_name(const string& _name) const {
		unsigned short i = 0;
		for (; i < wires.size(); ++i){
			if(!wires[i]) continue;
			if ( !_name.compare(wires[i]->getname()) )  break;
		}
		if(i == wires.size()) {
			return HUGENEGATIVE;
		}
		return i;
	}

	void Rrg::find_children_of_local_tile(vector<HopUnit>& children, const HopUnit& father){
		const TileUnit& tile = get_tile_by_pos(Point(father.x,father.y,0));
		if(tile.getTileTemplate() == NULL || tile.gettilename().empty()) return;
		WireTemplate* tmp = tile.getwire(father.index);
		if(!tmp){ cout << "error exits in rrg here#1!" << endl; exit(0);}
		tmp->get_children_of_local_tile(children,father);
	}

	void Rrg::find_children_of_other_tiles(vector<HopUnit>& children, const HopUnit& father){
		const TileUnit& tile = get_tile_by_pos(Point(father.x,father.y,0));
		if(tile.getTileTemplate() == NULL || tile.gettilename().empty()) return;
		WireTemplate* tmp = tile.getwire(father.index);
		if(!tmp){ cout << "error exits in rrg here#2!" << endl; exit(0);}
		tmp->get_children_of_other_tiles(children,father);
	}

	short Rrg::find_wireindex_by_name_and_pos(const Point& p,const string& _name){
		const TileUnit& tile = get_tile_by_pos(p);
		if(tile.getTileTemplate() == NULL || tile.gettilename().empty()){ 
			return HUGENEGATIVE;
		}
		return tile.find_wireindex_by_name(_name);
	}

	void Rrg::build()
	{
		fpga->settopinfo();
		short XMAX = fpga->getscale().getx(), YMAX = fpga->getscale().gety();
		for (short x = 0; x < XMAX; ++x)
		{
			for (short y = 0; y < YMAX; ++y)
			{
				topinfo[x][y].tile_type = creat_tile_template(x,y);
				topinfo[x][y].name = fpga->get_inst_by_pos(Point(x,y,0))->getname();
				create_pinwire_map(x,y);
			}
		}
		fpga_scale=Point(fpga->getscale().getx(),fpga->getscale().gety(),PORTNUM*SIDENUM+100);
	}

	void Rrg::searchconnection(vector<HopUnit>& ves,short serial,short x,short y){
		short deltax = HUGENEGATIVE, deltay = HUGENEGATIVE;
		vector<short> direct; 
		vector<HopUnit> store; 
		typedef vector<short>::const_iterator dir_iter; 
		typedef vector<HopUnit>::const_iterator pip_iter;
		searchpip(ves,serial,x,y);
		if(serial >= PORTNUM*SIDENUM || cannot_be_driven(serial,x,y)) 
			return;
		find_connection_with_othertiles(store,serial,x,y);
		searchbypass(direct,serial,x,y);
		dir_iter beg = direct.begin(), end = direct.end();
		while(beg!=end)	{ 
			find_connection_with_othertiles(store,*beg,x,y); ++beg; 
		}
		pip_iter _beg = store.begin(), _end = store.end();
		while(_beg != _end){
			ves.push_back(HopUnit(_beg->x-x,_beg->y-y,_beg->index));
			++_beg;
		}
	}

	void Rrg::find_connection_with_othertiles(vector<HopUnit>& ves,short serial,short x,short y){
		short deltax = HUGENEGATIVE, deltay = HUGENEGATIVE;
		short mir = transform_between_tiles(serial,deltax,deltay);
		vector<HopUnit> store;
		collector(store,mir,x+deltax,y+deltay);
		typedef vector<HopUnit>::const_iterator pip_iter;
		pip_iter beg = store.begin(), end = store.end();
		while(beg != end){
			ves.push_back(HopUnit(beg->x,beg->y,beg->index));
			++beg;
		}
	}

	void Rrg::collector(vector<HopUnit>& ves,short serial,short x,short y){
		if(x <0 || y < 0 || x > fpga->getscale().getx() || y > fpga->getscale().gety()) return;
		ArchTile *curtile = gettile(x,y);
		vector<short> _direct;
		typedef vector<short>::const_iterator dir_iter;
		short actutal = -1;
		bool if_bypass = is_bypass(serial,x,y);
		if(if_bypass){
			searchbypass(_direct,serial,x,y);
			actutal = findmin(_direct);
			actutal = actutal < serial ? actutal : serial;
		}
		if(is_driver(serial,x,y) || (cannot_be_driven(serial,x,y) && !if_bypass && (!curtile->getportname(serial).empty()) )) {
			if(!if_bypass) actutal = serial;
			assert(actutal > -1);
			ves.push_back(HopUnit(x,y,actutal));
		}
		dir_iter beg = _direct.begin(), end = _direct.end();
		while(beg != end){
			find_connection_with_othertiles(ves,*beg,x,y);
			++beg;
		}
	}

	void Rrg::searchpip(vector<HopUnit>& ves,short serial,short x,short y){
		ArchTile* cur = gettile(x,y);
		if(!cur) return;
		ArchTile::const_pip_iter beg = cur->pip_begin(serial), end = cur->pip_end(serial);
		vector<short> rec;
		while (beg != end) {
			if(is_bypass(beg->second,x,y)){
				vector<short> direct;
				searchbypass(direct,beg->second,x,y);
				short act = findmin(direct);
				act = act < beg->second ? act : beg->second;
				if(find(rec.begin(),rec.end(),act) == rec.end()){
					ves.push_back(HopUnit(0,0,act));
					rec.push_back(act);
				}
				++beg; continue; 
			}
			ves.push_back(HopUnit(0,0,beg->second));
			++beg; 
		}
	}

	void Rrg::searchbypass(vector<short>& direct,short serial,short x,short y){
		ArchTile* cur = gettile(x,y);
		if(!cur) return;
		ArchTile::const_bypass_iter beg = cur->bypass_begin(serial), end = cur->bypass_end(serial);
		while (beg != end) { direct.push_back(beg->second);++beg;}
	}

	//if there is no actual pip, then it is an bypass port. 
	bool Rrg::is_bypass(short serial,short x,short y){
		ArchTile* cur = gettile(x,y);
		if(!cur) return false;
		ArchTile::const_bypass_iter beg = cur->bypass_begin(serial), end = cur->bypass_end(serial);
		if(beg == end) return false;
		return true;
	}

	bool Rrg::is_driver(short serial,short x,short y){
		ArchTile* cur = gettile(x,y);
		if(!cur) return false;
		ArchTile::const_pip_iter beg = cur->pip_begin(serial), end = cur->pip_end(serial);
		if (beg == end) return false;
		return true;
	}

	//only can be driver or isolated or bypass
	bool Rrg::cannot_be_driven(short serial,short x,short y){
		ArchTile* cur = gettile(x,y);
		if(!cur) return false;
		ArchTile::const_pip_iter beg = cur->reverse_begin(serial), end = cur->reverse_end(serial);
		if (beg == end) {
			return true;
		}
		return false;
	}

	WireTemplate* Rrg::creat_wire_template(short serial,short x,short y){
		vector<HopUnit> fanouts; 
		typedef vector<HopUnit>::const_iterator hopiter;
		ArchTile* cur = gettile(x,y);
		assert(cur);
		searchconnection(fanouts,serial,x,y);
		WireTemplate* wire = new WireTemplate(cur->getportname(serial).c_str());
		wire->setIndex(serial);
		hopiter beg = fanouts.begin(), end = fanouts.end();
		while(beg != end) { 
			if(beg->x || beg->y)
				wire->addseg(*beg);
			else
				wire->addpip(*beg);
			++beg; 
		}
		const_wire_iter iter;
		if (findwire(iter,wire))	{
			delete wire; return iter->second;
		} 
		else{ 
			addwire(wire->getname(),wire);
			return wire;
		}
		
	}

	TileTemplate* Rrg::creat_tile_template(short x,short y){
		TileTemplate* tile = new TileTemplate();
		ArchTile* archtile = gettile(x,y);
		assert(archtile);
		short portVecSize = archtile->portsize();
		vector<bool>	boolean_pool(portVecSize,false);
		ArchTile::const_pip_iter beg = archtile->pip_begin(), end = archtile->pip_end();
		while (beg != end){
			//driver
			if(!boolean_pool[beg->first])
			{
				boolean_pool[beg->first] = true;
				short index;
				if(is_bypass(beg->first,x,y)){
					vector<short> direct;
					searchbypass(direct,beg->first,x,y);
					for (unsigned short i = 0; i < direct.size(); ++i) boolean_pool[direct[i]] = true;
					direct.push_back(beg->first);
					index = findmin(direct);
				}
				else
					index = beg->first;
				if (islong(string(archtile->getportname(index))) || (isclk(string(archtile->getportname(index))) && index > PORTNUM*SIDENUM && string(archtile->getportname(index)).find("PINWIRE") == string::npos))
					topinfo[x][y].long_global_nets.insert(std::make_pair(index,creat_wire_template(index,x,y)));
				else
					tile->addwire(index,creat_wire_template(index,x,y));
			}
			//fanout
			if(!boolean_pool[beg->second]) 
			{
				boolean_pool[beg->second] = true;
				short index;
				if(is_bypass(beg->second,x,y)){
					vector<short> direct;
					searchbypass(direct,beg->second,x,y);
					for (unsigned short i = 0; i < direct.size(); ++i) boolean_pool[direct[i]] = true;
					direct.push_back(beg->second);
					index = findmin(direct);
				}
				else
					index = beg->second;
				if (islong(string(archtile->getportname(index))) || (isclk(string(archtile->getportname(index))) && index > PORTNUM*SIDENUM && string(archtile->getportname(index)).find("PINWIRE") == string::npos))
					topinfo[x][y].long_global_nets.insert(std::make_pair(index,creat_wire_template(index,x,y)));
				else
					tile->addwire(index,creat_wire_template(index,x,y));
			}
			++beg;
		}
		short i = 0;
		for (; i < portVecSize ;++i){
			string portname = archtile->getportname(i);
			if(boolean_pool[i] || portname.empty()) continue;
			tile->addwire(i,creat_wire_template(i,x,y));
			boolean_pool[i] = false;
		}
		const_tile_iter iter;
		if ( findtile(iter,tile) ) { delete tile; return iter->second; }
		else{ addtile(tile->size(),tile); return tile; }
	}

	bool Rrg::findwire( const_wire_iter& i,const WireTemplate* t ) const {
		string name = t->getname();
		const_wire_iter beg = wire_begin(name), end = wire_end(name);
		for (;beg != end;++beg){
			if(*(beg->second) == *t){
				i = beg; return true;
			}
		}
		return false;
	}

	bool Rrg::findtile( const_tile_iter& i,const TileTemplate* t ) const {
		size_t _size = t->size();
		const_tile_iter beg = tile_begin(_size), end = tile_end(_size);
		for (;beg != end;++beg){
			if(*(beg->second) == *t){
				i = beg; return true;
			}
		}
		return false;
	}

	void Rrg::create_pinwire_map(short x, short y) {
		ArchTile* archtile = gettile(x,y);
		assert(archtile);
		archtile->build_pin_to_pinwire_name_map(pinwiremap[x][y]);
		
	}

}}