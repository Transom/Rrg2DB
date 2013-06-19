#include "xdlnetlist.h"
#include <map>
#include <cassert>
#include <cstring>
#include <vector>
#include <hash_map>
#include <algorithm>
#include <cmath>

namespace FDP { namespace netlist {

	using std::string;
	using std::ostream;
	using std::cout;
	using std::cerr;
	using std::endl;
	using std::map;
	using stdext::hash_map;
	using stdext::hash_multimap;
	using std::vector;
	using std::make_pair;

	SliceConfig*	XDLFactory::makeSlice(Instance* inst,const char* clbname)	{	return new SliceConfig(inst,clbname); }
	NetTrace*		XDLFactory::makeNet(Net* net,NETTYPE t)			{ return new NetTrace(net,t); }
	
	//constructors
	BoundingBox::BoundingBox(short _xmin,short _xmax,short _ymin,short _ymax):xmin(_xmin),xmax(_xmax),ymin(_ymin),ymax(_ymax){}
	BoundingBox::BoundingBox(const BoundingBox& rhs):xmin(rhs.xmin),xmax(rhs.xmax),ymin(rhs.ymin),ymax(rhs.ymax){}
	SliceConfig::SliceConfig(Instance* _inst,const char* _clbname):inst(_inst),clbname(_clbname){}
	NetTrace::NetTrace(Net* _net,NETTYPE _t):net(_net),type(_t){}
	XDLDesign::XDLDesign():deviceparameters(""),xdl_version(""),name(""),cktdesign(0){
		cktdesign = new Design("");
	}

	//destructors
	XDLDesign::~XDLDesign(){
		const_sliceIter beg1 = slice_begin(), end1 = slice_end();
		const_netIter   beg2 = net_begin(),   end2 = net_end();
		while ( beg1 != end1 ){	delete *beg1; ++beg1; }
		while ( beg2 != end2 ){ delete *beg2; ++beg2; }
		delete cktdesign;
	}

	BoundingBox& BoundingBox::operator = (const BoundingBox& rhs){
		xmin = rhs.xmin;
		xmax = rhs.xmax;
		ymin = rhs.ymin;
		ymax = rhs.ymax;
		return *this;
	}

	ostream& operator<<(ostream& os, const SliceConfig& slice){
		vector<string>::const_iterator beg = slice.cfgInfo.begin(), end = slice.cfgInfo.end();
		os << "inst\t\"" << slice.getinst()->getname() << "\"\t\"" 
		   << slice.getinst()->getcell()->gettype() << "\"\t,placed\t"
		   << slice.getclbname() << "\t" << slice.getinst()->getcell()->getname() << "\t," << endl;
		os << " cfg  ";
		while ( beg != end )  { os << "\t" << *beg << endl; ++beg; }
		os << ";" << endl;
		return os;
	}

	ostream& NetTrace::print(ostream& os, Rrg* rrg, int& length){
		const TileUnit* tile;
		NETTYPE _type = getnettype();
		string  __type;
		if(_type == VCC) __type = "vcc";
		else if(_type == GND) __type = "gnd";
		else __type = "";
		cout << "trace size: " << traceInfo.size() << endl;
		list<HopUnit>::iterator pipbeg = traceInfo.begin(), pipend = traceInfo.end(),cur;
		Net::const_pin_iter outbeg = getnet()->src_begin(), outend = getnet()->src_end();
		Net::const_pin_iter inbeg = getnet()->sink_begin(), inend = getnet()->sink_end();
		os << "net\t\"" << getnet()->getname() << "\"\t" << __type << "," << endl;
		while (outbeg!=outend){
			os << "\toutpin\t\"" << (*outbeg)->getowner()->getname() << "\"\t" << (*outbeg)->getname() << "\t," <<endl;
			++outbeg;
		}
		while (inbeg!=inend){
			os << "\tinpin\t\"" << (*inbeg)->getowner()->getname() << "\"\t" << (*inbeg)->getname() << "\t," <<endl;
			++inbeg;
		}
		while (pipbeg != pipend){
			cur = pipbeg; ++pipbeg;
			if (pipbeg == pipend) break;
			tile = &(rrg->get_tile_by_pos(Point(cur->x,cur->y,0)));
	//		nexttile = &(rrg->get_tile_by_pos(Point(pipbeg->x,pipbeg->y,0)));
			if (cur->x == pipbeg->x && cur->y == pipbeg->y){
				os << "\tpip\t" << tile->name << "\t" << tile->getwirename(cur->index)
				   << "\t->\t" << tile->getwirename(pipbeg->index) << "\t," << endl;
				/*cout << "\tpip\t" << tile->name << "\t" << tile->getwirename(cur->index)
					<< "\t->\t" << tile->getwirename(pipbeg->index) << "\t," << endl;*/
			}
			else{
				/*cout << "\tpip\t" << "(" << tile->name << ")" << "\t" << tile->getwirename(cur->index)
					<< "\t->\t" << "(" << nexttile->name << ")" << "\t" << nexttile->getwirename(pipbeg->index) << "\t," << endl;*/
				int deltax = abs(cur->x-pipbeg->x), deltay = abs(cur->y-pipbeg->y);
				length += (deltax+deltay);
			}
		}
		os << ";" << endl;
		return os;
	}

	void NetTrace::setboundingbox(){
		short xmin = HUGEPOSITIVE, ymin = HUGEPOSITIVE, xmax = HUGENEGATIVE, ymax = HUGENEGATIVE;
		if(src.x < xmin)	xmin = src.x;
		if(src.x > xmax)	xmax = src.x;
		if(src.y < ymin)	ymin = src.y;
		if(src.y > ymax)	ymax = src.y;
		vector<HopUnit>::const_iterator beg = sinks.begin(), end = sinks.end();
		while(beg!=end){
			if(beg->x < xmin)	xmin = beg->x;
			if(beg->x > xmax)	xmax = beg->x;
			if(beg->y < ymin)	ymin = beg->y;
			if(beg->y > ymax)	ymax = beg->y;
			++beg;
		}
		boundingbox = BoundingBox(xmin,xmax,ymin,ymax);
	}

	void NetTrace::sortsinks(){
		hash_multimap<short,HopUnit>			disthash;
		hash_multimap<short,HopUnit>::iterator	up,lower;
		vector<short>			distVec;
		vector<short>::const_iterator	distIter,disBeg,disEnd;
		vector<HopUnit>::const_iterator beg = sinks.begin(), end = sinks.end();
		while (beg!=end) {
			short dist = manhatanDistFromSrc(*beg);
			disthash.insert(make_pair(dist,*beg));
			distIter = find(distVec.begin(),distVec.end(),dist);
			if(distIter == distVec.end()) distVec.push_back(dist);
			++beg;
		}
		sort(distVec.begin(),distVec.end());
		sinks.clear();
		disBeg = distVec.begin(); disEnd = distVec.end();
		while(disBeg!=disEnd){
			up = disthash.upper_bound(*disBeg), lower = disthash.lower_bound(*disBeg);
			while(lower!=up){
				sinks.push_back(lower->second);
				++lower;
			}
			++disBeg;
		}
	}

	ostream& XDLDesign::print(ostream& os,Rrg* rrg,int& length){
		os << "design\t\"" << name << "\"\t" << deviceparameters << "  " << xdl_version <<"\t;" <<endl;
		os << endl;
		os << "####inst info" << endl;
		os << endl;
		const_sliceIter sli_beg = slice_begin(), sli_end = slice_end();
		const_netIter	n_beg = net_begin(), n_end = net_end();
		while (sli_beg != sli_end){
			os << **sli_beg;
			++sli_beg;
		}
		os << endl;
		os << "####net info" << endl;
		os << endl;
		while (n_beg != n_end){
			(*n_beg)->print(os,rrg,length);
			++n_beg;
		}
		return os;
	}

	XDLDesign::const_sliceIter XDLDesign::findsite(const char* name)	const {
		const_sliceIter beg = slice_begin(), end = slice_end();
		for (;beg!=end;++beg)
			if ( !strcmp((*beg)->getinst()->getname(),name) )
				return beg; 
		return end;
	}

	XDLDesign::const_netIter XDLDesign::findnettr(const char* name)	const {
		const_netIter beg = net_begin(), end = net_end();
		for (;beg != end;++beg) {
			if ( !strcmp((*beg)->getnet()->getname(),name) ){
				return beg;
			}
		}
		return end;
	}

	void XDLDesign::deleteNet(NetTrace* net){
		net_iter beg = net_modifiable_begin(), end = net_modifiable_end();
		while (beg != end){
			if(*beg == net) break;
			++beg;
		}
		if(beg==end) { cerr << "cannot find net in xdldesign!!!" << endl; exit(0); }
		nets.erase(beg);
	}


}}