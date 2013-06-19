#include "archlib.h"
#include <map>
#include <vector>
#include <cstring>
#include <stdexcept>
#define DEBUG

namespace FDP { namespace archlib {

	using std::map;
	using std::string;
	using std::make_pair;
	using std::cout;
	using std::endl;

	inline short transform_within_onetile(SIDE s,short i) {
		return (s-LEFT)*PORTNUM+i;
	}

	Cell* ArchFactory::makeCell(const char* name, const char* type, const Library* owner){
		return new ArchCell(name,type,owner,HIEIGNORE);
	}
	Instance* ArchFactory::makeInstance(const char* name,const Cell* owner,Cell* instof){
		return new ArchInstance(name,static_cast<const ArchCell*>(owner),static_cast<ArchCell*>(instof));
	}
	Port* ArchFactory::makePort(const char* name,const Cell* owner){
		return new ArchPort(name,static_cast<const ArchCell*>(owner));
	}
	Pin* ArchFactory::makePin(const char* name, const Instance* owner){
		return new ArchPin(name,owner);
	}
	ArchTile* ArchFactory::makeTile(const char* name, const char* type, const Library* owner){
		return new ArchTile(name,type,owner);
	}
	PadInfo* ArchFactory::makePad(const char* name,const Point& p){
		return new PadInfo(name,p);
	}

	//constructors
	sitePinToPinwire::sitePinToPinwire(string _sitePinName /* = "" */, string _siteName /* = "" */, string _pinWireName /* = "" */, short _z /* = -1 */):sitePinName(_sitePinName),siteName(_siteName),pinWireName(_pinWireName),z(_z){}
	ArchPIP::ArchPIP(ArchPin* _from, ArchPin* _to):from(_from),to(_to){}
	ArchPort::ArchPort(const char* s,const ArchCell* c):Port(s,c),side(SIDEIGNORE),serial(-1),type(TYPEIGNORE),dir(Dir_DIRIGNORE){}
	ArchInstance::ArchInstance(const char* s, const ArchCell* owner, ArchCell* instof):Instance(s,owner,instof){
		if(instof->gethierarchy() == TILE){ destroy(); clear(); }
	}
	ArchCell::ArchCell(const char* _name, const char* _type, const Library* _owner, HIERARCHY _h):Cell(_name,_type,_owner),hierarchy(_h){}
	ArchTile::ArchTile(const char* _name,const char* _type, const Library* _owner):ArchCell(_name,_type,_owner,TILE),portnum(PORTNUM*SIDENUM)
						{ portnames.resize(portnum,"");}
	FPGADesign::FPGADesign(const char* s):Design(s){
		for (short x=0;x<SCALEX;++x)
			for (short y=0;y<SCALEY;++y)
				topInfo[x][y] = NULL;
	}
	//destructors
	FPGADesign::~FPGADesign(){
		vector<PadInfo*>::iterator beg = pads.begin(), end = pads.end();
		while(beg != end){ delete *beg;++beg; }
	}

	inline void ArchInstance::creatpath(ArchPin* lhs, ArchPin* rhs){
		ArchPIP* pip = new ArchPIP(lhs,rhs);
		paths.push_back(*pip); }
	inline void ArchCell::creatpath(ArchPin* lhs, ArchPin* rhs) { 
		ArchPIP* pip = new ArchPIP(lhs,rhs);
		paths.push_back(*pip); }
	inline void ArchTile::creatpath(ArchPin* lhs, ArchPin* rhs) {
		short in,out;
		ArchPort* left = NULL;
		ArchPort* right = NULL;
		if(!lhs->getowner()) {//for real cellpin 
			left = static_cast<ArchPort*>(const_cast<Port*>(lhs->getcellport()));
			in = transform_within_onetile(left->getside(),left->getserial());
		}
		else{//for fake cellpin
			left = static_cast<ArchPort*>(const_cast<Port*>(lhs->getport()));
			in = left->getserial();
		}
		if (!rhs->getowner()){
			right = static_cast<ArchPort*>(const_cast<Port*>(rhs->getcellport()));
			out = transform_within_onetile(right->getside(),right->getserial());
		} 
		else{
			right = static_cast<ArchPort*>(const_cast<Port*>(rhs->getport()));
			out = right->getserial();
		}
		assert(in > -1 && out > -1);
		const_pip_iter beg = pip_begin(in), end = pip_end(in);
		if(beg == end){
			pips.insert(make_pair(in,out));
			reverse.insert(make_pair(out,in));
			return;
		}
		while(beg != end) {
			if(beg->second == out) break;
			++beg;
		}
		if(beg == end){
			pips.insert(make_pair(in,out));
			reverse.insert(make_pair(out,in));
		}
	}

	//find the ports which belong to the site, these ports under process are from one arch-net
	//this arch-net also belongs to the site
	void ArchCell::relativepins(vector<ArchPin*>& ves,const ArchPin* p){
		Pin::const_net_iter beg = p->net_begin(), end = p->net_end();
		Net::const_pin_iter pb,pe;
		while (beg != end){
			if((*beg)->dangle()) { ++beg; continue; }
			pb = (*beg)->pin_begin(), pe = (*beg)->pin_end();
			while (pb != pe){
				if( !(*pb)->getowner() )	ves.push_back(*pb);
				++pb;
			}
			++beg;
		}
	}
	//find the ports which belong to tile, these ports under process are from one arch-net
	//this arch-net also belongs to the tile
	void ArchTile::relativepins(vector<ArchPin*>& ves,const ArchPin* p){
		Pin::const_net_iter beg = p->net_begin(), end = p->net_end();
		Net::const_pin_iter pb,pe;
		while (beg != end){
			if((*beg)->dangle()) {
				++beg; 
				continue; 
			}
			pb = (*beg)->pin_begin(), pe = (*beg)->pin_end();
			while (pb != pe){
				if( !(*pb)->getowner() ){	
					//find the cellpin of the cell as the owner of cellpin is NULL
					//put the found port in the name-vector
					ArchPort* tmpport = static_cast<ArchPort*>(const_cast<Port*>((*pb)->getcellport()));
					setportname(transform_within_onetile(tmpport->getside(),tmpport->getserial()),(*beg)->getname());
					ves.push_back(*pb);
				}
				++pb;
			}
			++beg;
		}
	}

	void ArchCell::deHierarchy() 
	{
		if (hierarchy == ELEMENT)	return;
		if(!empty())				return;
		const_inst_iter beg = inst_begin(), end = inst_end();
		if (beg == end) return;
		for (;beg != end;++beg){
			build(static_cast<ArchInstance*>(const_cast<Instance*>(*beg)));
		}
	}

	//first find the paths in subcells and transform them into the paths of the current site
	void ArchCell::build(ArchInstance* inst){
		ArchInstance::const_path_iter iter_beg,iter_end,iter;
		if(inst->empty())	inst->build();
		iter_beg = inst->path_begin(), iter_end = inst->path_end();
		for (iter=iter_beg;iter!=iter_end;++iter){
			vector<ArchPin*>				 from,to;
			vector<ArchPin*>::const_iterator outer,inner;
			relativepins(from,iter->getfrom());
			relativepins(to,iter->getto());	
			if(from.empty() || to.empty()) continue; 
			for (outer = from.begin(); outer != from.end(); ++outer){
				for (inner = to.begin(); inner != to.end();++inner){
					creatpath(*outer,*inner);
				}
			}
		}
	}

	//specific version of the arch-tile
	void ArchTile::build(ArchInstance* inst){
	//	cout << "archname:\t"<< getname() << "\t\tinstname:\t" << inst->getname() << endl;
		ArchInstance::const_path_iter iter_beg,iter_end,iter;
		if(inst->empty())	inst->build();
		iter_beg = inst->path_begin(), iter_end = inst->path_end();
		for (iter=iter_beg;iter!=iter_end;++iter){
			vector<ArchPin*> from,to;
			vector<ArchPin*>::const_iterator outer,inner;
			relativepins(from,iter->getfrom());
			relativepins(to,iter->getto());
			//two if-expression aim to add the pin of slices into rrg
			if (from.empty()) 
				increasepoints(from,iter->getfrom());
			if (to.empty())
				increasepoints(to,iter->getto());
			if ( from.empty() || to.empty() ) 
				continue;
			for (outer = from.begin(); outer != from.end(); ++outer){
				for (inner = to.begin(); inner != to.end();++inner){
					creatpath(*outer,*inner);
				}
			}
		}
	}

	//add the ports that belong to the slices to the tile, because these ports are useful in the routing
	void ArchTile::increasepoints(vector<ArchPin*>& ves,const ArchPin* p){
		Pin::const_net_iter beg = p->net_begin(), end = p->net_end();
		Net::const_pin_iter pb,pe;
		while (beg != end){
		//	if((*beg)->dangle()) { ++beg; continue; }
			string inportname = (*beg)->getname();
			short pidx = find_portindex_by_name(inportname);
			if(pidx == -1){ 
				pidx = ++portnum; setportname(pidx,inportname.c_str());
			}
			pb = (*beg)->pin_begin();
			ArchPort* tmpport = static_cast<ArchPort*>(const_cast<Port*>((*pb)->getport()));
			tmpport->setserial(pidx);
			ves.push_back(*pb);
			++beg;
		}
	}

	short ArchTile::find_portindex_by_name(const string& str){
		short i = 0, psize = portsize();
		for (; i < psize ; ++i){
			if(!getportname(i).compare(str)) return i;
		}
		return -1;
	}

	//store the portname(also the arch-net name) in a vector(accessed though index) for further use(creating rrg)
	void ArchTile::setportname( short i,const char* name ){
		try{	portnames.at(i) = name;  }
		catch ( std::out_of_range& e){
			e.what() ;
			portnames.resize(i+100,"");	
			portnames.at(i) = name;
		}
	}

	void ArchTile::build_nets_of_connection_with_tile_and_slice(){
		const_net_iter iter, beg = net_begin(), end = net_end();
		for (iter = beg; iter != end; ++iter){
			if ((*iter)->dangle())	continue;
			string netname = (*iter)->getname();
			ArchNet::const_pin_iter pin_iter, pin_beg = (*iter)->pin_begin(), pin_end = (*iter)->pin_end();
			bool if_qualified = true;
			vector<ArchPin*> cellpins;
			for (pin_iter = pin_beg; pin_iter != pin_end; ++pin_iter){
				if ( !(*pin_iter)->getowner() )	{ cellpins.push_back(*pin_iter); }
				else{
					string sitename = (*pin_iter)->getowner()->getname();
					if(sitename.find("GRM") != string::npos || sitename.find("grm") != string::npos){	
						if_qualified = false; break;
					}
				}
			}
			unsigned short size = (unsigned short)cellpins.size();
			if(if_qualified && size == 1){
				ArchPin* cellpin = *(cellpins.begin());
				ArchPort* tmpport = static_cast<ArchPort*>(const_cast<Port*>(cellpin->getcellport()));
				setportname(transform_within_onetile(tmpport->getside(),tmpport->getserial()),(*iter)->getname());
			}
		}
	}
	
	//find the bypasses of tile, one bypass represents the logical connection but no physical connection
	void ArchTile::setbypass() {
		const_net_iter iter, beg = net_begin(), end = net_end();
		short		cnt = 0;
		vector<ArchPin*>   tmp;
		vector<ArchPin*>::const_iterator outer,inner,tmp_beg,tmp_end;
		ArchPort  *left,*right;
		for (iter = beg; iter != end; ++iter)
		{
			if ((*iter)->dangle())	continue;
			ArchNet::const_pin_iter pin_iter, pin_beg = (*iter)->pin_begin(), pin_end = (*iter)->pin_end();
			for (pin_iter = pin_beg; pin_iter != pin_end; ++pin_iter)
				if ( !(*pin_iter)->getowner() )	{  tmp.push_back(*pin_iter); ++cnt; }
				tmp_beg = tmp.begin(), tmp_end = tmp.end();
				if (cnt >= 2)
				{
					for (outer = tmp_beg; outer != tmp_end; ++outer)
					{
						inner = ++outer;--outer;
						for (; inner != tmp_end; ++inner)
						{
							left = static_cast<ArchPort*>(const_cast<Port*>((*outer)->getcellport()));
							right = static_cast<ArchPort*>(const_cast<Port*>((*inner)->getcellport()));
							bypasses.insert(make_pair(transform_within_onetile(left->getside(),left->getserial()),
								transform_within_onetile(right->getside(),right->getserial())));
							bypasses.insert(make_pair(transform_within_onetile(right->getside(),right->getserial()),
								transform_within_onetile(left->getside(),left->getserial())));
						}
					}
				}
				cnt = 0;
				tmp.clear();
		}
	}

	void ArchTile::build_pin_to_pinwire_name_map(vector<sitePinToPinwire>& vec) const {
		const_net_iter iter, beg = net_begin(), end = net_end();
		for (iter = beg; iter != end; ++iter) {
			if ((*iter)->dangle())	continue;
			ArchNet::const_pin_iter pin_iter, pin_beg = (*iter)->pin_begin(), pin_end = (*iter)->pin_end();
			for (pin_iter = pin_beg; pin_iter != pin_end; ++pin_iter) {
				const Object *site = (*pin_iter)->getowner();
				if (site) {
					string sitename = site->getname();
					if( sitename.find("IOB")!=string::npos) { //只选择IOB里面的pinwire，后面有必要加的话修改。
						const ArchInstance *inst = static_cast<const ArchInstance*>(site);
						sitePinToPinwire one_item = sitePinToPinwire((*pin_iter)->getname(),sitename,(*iter)->getname(),inst->getz());
						vec.push_back(one_item);
					}
				}
			}
		}
	}

	//first find the paths in subcells and transform them into the paths of the current instance
	void ArchInstance::build(){
		ArchCell* subcell = static_cast<ArchCell*>( getcell() );
	//	cout<<"ArchInstance name: "<<this->getname()<<endl;//to be deleted
		if (subcell->empty())	subcell->deHierarchy();
		ArchCell::const_path_iter iter_beg,iter_end,iter;
		iter_beg = subcell->path_begin(), iter_end = subcell->path_end();
		Instance::const_pin_iter from,to;
		for (iter=iter_beg;iter!=iter_end;++iter){
			/*if(!strcmp("CLKV_DCM_DCM0_CLKP0",iter->getfrom()->getname()) && !strcmp("CLKV_DCM_DCM_OUTCLKP0",iter->getto()->getname()))
				cout << "within archinst build#1" << endl;*/
			if (findpin(from,iter->getfrom()->getname()) && findpin(to,iter->getto()->getname())){
		//		cout << "within archinst build#2" << endl;
				creatpath(*from,*to);
			}
			else { std::cerr << "error in arch-instance build operation." << std::endl; exit(0);}
		}
	}

	void FPGADesign::settopinfo() {
		const_lib_iter lib_iter;
		bool b = findlib(lib_iter,string("tile"));
		assert(b);
		Library::cell_iter beg = (*lib_iter)->cell_begin(), end = (*lib_iter)->cell_end();
		while (beg!=end) {
			ArchTile* tile = static_cast<ArchTile*>(*beg);
			tile->deHierarchy();
			tile->setbypass();
			tile->build_nets_of_connection_with_tile_and_slice();
			++beg;
		}
		const_inst_iter beg_inst = getinsts().begin(), end_inst = getinsts().end();
		ArchInstance*	inst = NULL;
		for (;beg_inst != end_inst; ++beg_inst){
			inst = static_cast<ArchInstance*>(*beg_inst);
			assert(inst);
			topInfo[inst->getx()][inst->gety()] = inst;
		}
	}
	
	Point FPGADesign::get_pos_by_tilename(const string& _name)	const {
		for (short x = 0; x < SCALEX; ++x)
			for (short y = 0; y < SCALEY; ++y){
				if(topInfo[x][y] == NULL)	continue;
				if( !_name.compare(topInfo[x][y]->getname()) )
					return Point(x,y,0);
			}
		return Point(-1,-1,-1);
	}
	
	Point FPGADesign::get_pos_by_iobname(const char* _name)		const {
		vector<PadInfo*>::const_iterator beg = pads.begin(), end = pads.end();
		while (beg != end){
			if(!strcmp(_name,((*beg)->name).c_str())) return (*beg)->pos;
			++beg;
		}
		return Point(-1,-1,-1);
	}
	
}
}