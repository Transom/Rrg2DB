#pragma once
#ifndef XDLNETLIST_H
#define XDLNETLIST_H

#include "netlist.h"
#include "rrg.h"
#include "utils.h"
#include <vector>
#include <list>
#include <cmath>

namespace FDP {	namespace netlist {

	using namespace RRG;
	using std::vector;
	using std::list;
	using std::string;
	using std::ostream;

	class XDLDesign; class SliceConfig; class NetTrace; class XDLFactory;

	class XDLFactory
	{
	public:
		SliceConfig*	makeSlice(Instance* inst,const char* clbname);
		NetTrace*		makeNet(Net* net,NETTYPE t);
	};

	class SliceConfig
	{
	public:
		SliceConfig(Instance*,const char*);
		void		clear()								{ cfgInfo.clear(); }
		string		getclbname()		const			{ return clbname; }
		void		addConfigInfo(const string& str)	{ cfgInfo.push_back(str); }
		Instance*	getinst()		const				{ return inst; }
		friend ostream&	operator<<(ostream&,const SliceConfig&);

	protected:
	private:
		vector<string>	cfgInfo;
		Instance*		inst;
		string			clbname;		
	};
	
	class BoundingBox
	{
	public:
		BoundingBox(short _xmin = HUGENEGATIVE,short _xmax = HUGENEGATIVE,short _ymin = HUGENEGATIVE,short _ymax = HUGENEGATIVE);
		BoundingBox(const BoundingBox&);
		BoundingBox& operator=(const BoundingBox& rhs);
		bool		outside(const HopUnit& hop,short expandBB)	  const{
			return (hop.x > (xmax+expandBB) ) || (hop.x < (xmin-expandBB)) || (hop.y > (ymax+expandBB)) || (hop.y < (ymin-expandBB-1));
		}
	private:
		short xmin,xmax,ymin,ymax;
	};

	class NetTrace
	{
	public:
		typedef		list<HopUnit>::const_iterator		const_trace_iter;
		typedef		list<HopUnit>::iterator				trace_iter;
		typedef		vector<HopUnit>::const_iterator		const_sink_iter;
		typedef		vector<SliceConfig*>::const_iterator	const_slicfg_iter;
		NetTrace(Net*,NETTYPE);
		bool		dangle()		const				{ return net->dangle(); }
		bool		empty()			const				{ return net->empty(); }
		void		clear()								{ traceInfo.clear(); }
		trace_iter	addTraceInfo(trace_iter iter,const HopUnit& hop)	{ return traceInfo.insert(iter,hop); }
		Net*		getnet()		const				{ return net; }
		ostream&	print(ostream&,Rrg* rrg,int& length);
		void		setsrc(HopUnit h)					{ src = h; }
		void		setsrc(SliceConfig* slice)			{ src_slice = slice; }
		HopUnit				getsrc()				const			{ return src; }
		SliceConfig*		getsrcslice()			const			{ return src_slice; }
		SliceConfig*		findcfg(const Instance* inst)	const	{
			const_slicfg_iter beg = sinkcfg_const_begin(), end = sinkcfg_const_end();
			while (beg != end){
				if((*beg)->getinst() == inst) return *beg;
				++beg;
			}
			return NULL;
		}
		const_trace_iter	trace_const_begin()		const			{ return traceInfo.begin(); }
		const_trace_iter	trace_const_end()		const			{ return traceInfo.end(); }
		trace_iter			trace_begin()							{ return traceInfo.begin(); }
		trace_iter			trace_end()								{ return traceInfo.end(); }
		const_sink_iter		sink_const_begin()		const			{ return sinks.begin(); }
		const_sink_iter		sink_const_end()		const			{ return sinks.end(); }
		const_slicfg_iter	sinkcfg_const_begin()	const			{ return sink_slices.begin(); }
		const_slicfg_iter	sinkcfg_const_end()		const			{ return sink_slices.end(); }
		void				addsink(HopUnit h)						{ sinks.push_back(h); }
		void				addsink(SliceConfig* slice)				{ sink_slices.push_back(slice); }
		void				setboundingbox();
		const BoundingBox&	getboundingbox()		const			{ return boundingbox; }
		NETTYPE				getnettype()			const			{ return type; }

		//calc the manhatan distance between sink and src
		short		manhatanDistFromSrc(const HopUnit& rhs) const		{
			return abs(src.x-rhs.x) + abs(src.y-rhs.y);
		}
		void		sortsinks();

	private:
		list<HopUnit>	traceInfo;
		HopUnit			src;
		vector<HopUnit>	sinks;
		Net*			net;
		SliceConfig*	src_slice;
		vector<SliceConfig*>	sink_slices;
		BoundingBox		boundingbox;
		NETTYPE			type;
	};

	class XDLDesign
	{
	public:
		XDLDesign();
		~XDLDesign();

		typedef		vector<SliceConfig*>::const_iterator		const_sliceIter;
		typedef		vector<NetTrace*>::const_iterator			const_netIter;
		typedef		vector<NetTrace*>::iterator					net_iter;

		const_sliceIter		slice_begin()	   const			{ return slices.begin(); }
		const_sliceIter		slice_end()		   const			{ return slices.end(); }
		const_netIter		net_begin()		   const			{ return nets.begin(); }
		const_netIter		net_end()		   const			{ return nets.end(); }
		net_iter			net_modifiable_begin()				{ return nets.begin(); }
		net_iter			net_modifiable_end()				{ return nets.end(); }
		const_sliceIter		findsite(const char*)	const;
		const_netIter		findnettr(const char*)	const;
		void		addslice(SliceConfig* _slice)		{ slices.push_back(_slice); }
		void		addnet(NetTrace* _net)				{ nets.push_back(_net); }
		ostream&	print(ostream&,Rrg* rrg,int& length);
		void		setdevice(const string& _dev)				{ deviceparameters = _dev; }
		void		setxdlversion(const string& _ver)			{ xdl_version = _ver; }
		void		setname(const string& _name)				{ name = _name; }
		Design*		getdesign()						const		{ return cktdesign; }
		void		deleteNet(NetTrace* net);
	protected:
	private:
		string					deviceparameters;
		string					xdl_version;
		string					name;
		vector<SliceConfig*>	slices;
		vector<NetTrace*>		nets;
		Design*					cktdesign;
	};


}
}





#endif