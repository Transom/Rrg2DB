#include <cassert>
#include "utils.h"

namespace FDP {

	using std::make_pair;

	STR_SIDE_MAP::STR_SIDE_MAP(){
		maps.insert(make_pair("left",LEFT));
		maps.insert(make_pair("right",RIGHT));
		maps.insert(make_pair("top",TOP));
		maps.insert(make_pair("bottom",BOTTOM));
	}
	HIERARCHY_MAP::HIERARCHY_MAP(){
		maps.insert(make_pair("element",ELEMENT));
		maps.insert(make_pair("primitive",PRIMITIVE));
		maps.insert(make_pair("tile",TILE));
		maps.insert(make_pair("arch",ARCH));
	}
	SIDE_MAP::SIDE_MAP(){
		maps.insert(make_pair(TOP,BOTTOM));
		maps.insert(make_pair(BOTTOM,TOP));
		maps.insert(make_pair(LEFT,RIGHT));
		maps.insert(make_pair(RIGHT,LEFT));
	}
	DIR_MAP::DIR_MAP(){
		maps.insert(make_pair("input",Dir_IN));
		maps.insert(make_pair("output",Dir_OUT));
		maps.insert(make_pair("inout",Dir_INOUT));
	}
	name_to_xmltag::name_to_xmltag(){
		maps.insert(std::make_pair("design",DESIGN));
		maps.insert(std::make_pair("library",LIBRARY));
		maps.insert(std::make_pair("port",PORT));
		maps.insert(std::make_pair("cell",CELL));
		maps.insert(std::make_pair("portRef",PORTREF));
		maps.insert(std::make_pair("instance",INSTANCE));
		maps.insert(std::make_pair("net",NET));
		maps.insert(std::make_pair("pad",PAD));
		maps.insert(std::make_pair("package",PACKAGE));
		maps.insert(std::make_pair("path",PATH));
	}
	name_to_xdltag::name_to_xdltag(){
		maps.insert(make_pair("design",XDLDESIGN));
		maps.insert(make_pair("inst",INST));
		maps.insert(make_pair("net",XDLNET));
		maps.insert(make_pair("inpin",INPIN));
		maps.insert(make_pair("outpin",OUTPIN));
	}

	SIDE	STR_SIDE_MAP::operator () (const string& str){
		maps_iter iter = maps.find(str);
		assert(iter != maps.end());
		return iter->second;
	} 
	HIERARCHY	HIERARCHY_MAP::operator () (const string& str){
		maps_iter iter = maps.find(str);
		if(iter == maps.end())	return HIEIGNORE;
		return iter->second;
	}
	SIDE	SIDE_MAP::operator() (SIDE s){
		return maps[s];
	}
	DIRECTION	DIR_MAP::operator() (const string& str){
		maps_iter iter = maps.find(str);
		assert(iter != maps.end());
		return iter->second;
	}
	XML_TAG  name_to_xmltag::operator () (const string& str){
		maps_iter iter = maps.find(str);
		if ( iter == maps.end() )   return XMLTAGIGNORE;
		return iter->second;
	}
	XDL_TAG  name_to_xdltag::operator () (const string& str){
		maps_iter iter = maps.find(str);
		if ( iter == maps.end() )	return XDLTAGIGNORE;
		return iter->second;
	}

	ostream& operator <<( ostream &os,const GEOG_DIRECTION &ge){
		if(ge==NORTH_G)os<<"NORTH";
		else if(ge==EAST_G)os<<"EAST";
		else if(ge==WEST_G)os<<"WEST";
		else if(ge==SOUTH_G)os<<"SOUTH";
		else os<<"OTHER";
		return os;
	}
	ostream& operator <<( ostream &os,const WUTDIRECTONTYPE &ge){
		if(ge==VERTICAL)os<<"VERTICAL";
		else if(ge==HIEIGNORE)os<<"HIEIGNORE";
		else if(ge==LEFTDIAGONAL)os<<"LEFTDIAGONAL";
		else if(ge==RIGHTDIAGONAL)os<<"RIGHTDIAGONAL";
		else os<<"OTHER";
		return os;
	}
	ostream& operator <<( ostream &os,const LINETYPE &ge){
		if(ge==HEX_L)os<<"HEX";
		else if(ge==DOUBLE_L)os<<"DOUBLE";
		else if(ge==LONG_L)os<<"LONG";
		else os<<"OTHER";
		return os;
	}
}