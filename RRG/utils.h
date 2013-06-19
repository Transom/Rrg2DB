#pragma once
#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <sstream>
#include <iostream>
#include <map>
#include<boost/regex.hpp>

namespace FDP {

	using namespace std;
	const string    __DEBUGFLAG__="__DEBUGFLAG__:: ";
	const short		HUGENEGATIVE = -10000;
	const short		HUGEPOSITIVE = 10000;
	const float     HUGEFLOAT = 1.e30f;
	const short		SCALEX = 100;
	const short		SCALEY = 100;
	const short		SCALEZ = 16;
	const short		PORTNUM = 634;
	const short     SIDENUM = 4;

	const boost::regex long_re("^(LH|LV)");
	const boost::regex hex_re("^\\w6\\w{3}");
	const boost::regex double_re("^\\w2\\w{3}");
	const boost::regex north_re("^N\\d\\w{3}");
	const boost::regex south_re("^S\\d\\w{3}");
	const boost::regex east_re("^E\\d\\w{3}");
	const boost::regex west_re("^W\\d\\w{3}");

	const boost::regex beg_re("^\\w\\dBEG\\d");
	const boost::regex mid_re("^\\w\\dMID\\d");
	const boost::regex end_re("^\\w\\dEND\\d");
	const boost::regex en_d_re("^\\w\\dEND_\\w\\d");

	enum PIPDIRECTION		{VERTICAL_P,HORIZONTAL_P,LEFTDIAGONAL_P,RIGHTDIAGONAL_P,TERM_P,OTHER_P};
	enum WIREDIRECTION		{NORTH_W,SOUTH_W,WEST_W,EAST_W,IOMUX_W,OTHER_W};
	enum GEOG_DIRECTION		{NORTH_G,SOUTH_G,WEST_G,EAST_G,OTHER_G};
	enum WUTDIRECTONTYPE	{VERTICAL,HORIZONTAL,LEFTDIAGONAL,RIGHTDIAGONAL};
	enum LINETYPE			{HEX_L,DOUBLE_L,LONG_L,OTHER_L};

	enum HIERARCHY		{	HIEIGNORE,ELEMENT,PRIMITIVE,TILE,ARCH	};
	enum ARCHNETTYPE	{	TYPEIGNORE,DOUBLE,HEX,LONG,GLOBAL,OTHER	};
	enum SIDE			{	SIDEIGNORE,LEFT,RIGHT,BOTTOM,TOP	};
	enum DIRECTION		{	Dir_DIRIGNORE,Dir_IN,Dir_OUT,Dir_INOUT	};

	enum XML_TAG		{ XMLTAGIGNORE, DESIGN, LIBRARY, PORT, CELL, PORTREF, INSTANCE, PAD, PACKAGE, NET, PATH };
	enum XDL_TAG		{ XDLTAGIGNORE, XDLDESIGN, INST, XDLNET, INPIN, OUTPIN };
	enum XDLCktStage	{ CKTSTAGEIGNORE, CKTINST, CKTNET };

	enum NETTYPE		{ ORDINARY, VCC, GND };

	ostream& operator <<( ostream &,const GEOG_DIRECTION &);
	ostream& operator <<( ostream &,const WUTDIRECTONTYPE &);
	ostream& operator <<( ostream &,const LINETYPE &);

	class STR_SIDE_MAP 
	{
	private:
		map<string,SIDE>	maps;
	public:
		typedef	map<string,SIDE>::const_iterator maps_iter;
		STR_SIDE_MAP();
		SIDE	operator() (const string& str);
	};

	class HIERARCHY_MAP
	{
	private:
		map<string,HIERARCHY>   maps;
	public:
		typedef map<string,HIERARCHY>::const_iterator maps_iter;
		HIERARCHY_MAP();
		HIERARCHY	operator() (const string& str);
	};

	class SIDE_MAP
	{
	public:
		typedef map<SIDE,SIDE>::const_iterator  maps_iter;
		SIDE_MAP();
		SIDE	operator() (SIDE);
	private:
		map<SIDE,SIDE>	maps;
	};

	class DIR_MAP
	{
	public:
		typedef map<string,DIRECTION>::const_iterator maps_iter;
		DIR_MAP();
		DIRECTION	operator() (const string& str);
	private:
		map<string,DIRECTION>   maps;
	};

	class name_to_xmltag
	{
	public:
		typedef	map<string,XML_TAG>::const_iterator maps_iter;
		name_to_xmltag();
		XML_TAG operator() (const string&);
	private:
		map<string,XML_TAG>	maps;
	};
	class name_to_xdltag
	{
	public:
		typedef map<string,XDL_TAG>::const_iterator maps_iter;
		name_to_xdltag();
		XDL_TAG	operator() (const string&);
	private:
		map<string,XDL_TAG>	maps;
	};

	class Point 
	{
	private:
		short x,y,z;
	public:
		Point(short _x=-1, short _y=-1, short _z=-1):x(_x),y(_y),z(_z){}
		Point(const Point& _p):x(_p.getx()),y(_p.gety()),z(_p.getz()){}
		Point& operator=(const Point& _p){ x=_p.getx(); y=_p.gety(); z=_p.getz(); return *this; }
		bool operator==(const Point& rhs) const { return x==rhs.getx() && y==rhs.gety() && z==rhs.getz(); }
		bool operator!=(const Point& rhs) const { return !(operator==(rhs)); }
		bool operator<(const Point& rhs)  const{
			if(x!=rhs.getx())return x<rhs.getx();
			else if(y!=rhs.gety()) return y<rhs.gety();
			else return z<rhs.getz();
		}	
		bool equal_xy(	const Point& p)	const { return (x==p.getx()) && (y==p.gety());}
		bool equal_z(	const Point& p)	const { return z==p.getz(); }
		short		getx()				const { return x;}
		short		gety()				const { return y;}
		short		getz()				const { return z;}
		void		setx(short _x)			  { x=_x;}
		void		sety(short _y)			  { y=_y;}
		void		setz(short _z)			  { z=_z;}
	};

	//function object used as hash_map initializer
	struct point_compare
	{
		enum
		{	// parameters for hash table
			bucket_size = 4,	// 0 < bucket_size
			min_buckets = 8};	// min_buckets = 2 ^^ N, 0 < N

		bool operator()(const Point& lhs, const Point& rhs) const {
			return lhs==rhs;
		}
		std::size_t operator()(const Point& _point) const {
			return _point.getx()*SCALEX + _point.gety();
		}
	};

	//string-to-number transform and number-to-string transform
	inline string num2str(short num){
		stringstream ss; ss << num; return ss.str();
	}
	inline short str2num(string str){
		short num;	
		stringstream ss(str); 
		ss >>num;  
		return num;
	}
}

#endif