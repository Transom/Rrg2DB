#include <string>
#include <cstring>
#include <cstdio>
#include <cctype>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cassert>
#include <map>
#include "io.h"
#include "utils.h"
#include <cctype>

#define Debug 
#define DebugInfo "line: "<<__LINE__<<" file: "<<__FILE__<<endl

namespace FDP { namespace IO {

	using std::string;
	using std::cout;
	using std::endl;
	using std::make_pair;
	using std::istringstream;

	FDP::STR_SIDE_MAP	str_side_map;
	FDP::DIR_MAP		dir_map;
	FDP::HIERARCHY_MAP	hierarchy_map;
	FDP::name_to_xdltag	NameToXdl;
	FDP::name_to_xmltag	NameToXml;


	inline bool EndElement(const string& str){
		return str.find("/") != string::npos;
	}
// 
	//inline bool StartAndEndInOneLine(const string& str){//error 5 
	//	if (!EndElement(str)) return false;
	//	if (str.find(' ') != string::npos)	return true;
	//	return false;
	//}
	inline bool StartElement(const string& str){
		if(EndElement(str)){
			if(str.find("/>") != string::npos) return true;
			return false;
		}
		return true;
	}

	inline bool spaceline(const string& str){
		if (!str.size())	return true;
		string::size_type index;
		for (index=0; index!=str.size();index++)
			if (!isspace(str[index]))	break;
		if (index==str.size())	return true;
		return false;
	}
	inline bool xdl_annotated_line(const string& str){
		if (!str.size())	return false;
		string::size_type index;
		for (index=0; index!=str.size();index++)
			if (!isspace(str[index]))	break;
		if (index==str.size())	return false;
		if (str[index] == '#')	return true;
		return false;
	}

	inline bool EndXdlElement(const string& str){
		return str.find(';') != string::npos;
	}

	inline string  wipeoffword(string& line,string& word){
		string::size_type pos = line.find(word);
		if (pos == string::npos) return line;
		return line.substr(0,pos) + wipeoffword(line.substr(pos+word.size()),word);
	}

	inline string  getletters(string& str){
		return getalnum(str);
	}

	/* Get the string that is following the 'word' from the 'line',which 
	* is bracketed by the 'c'. 
	*eg. line="<portRef name="BEST_LOGIC_OUTS5_INT3" instanceRef="GRM_BRAM"/>" word="instanceRef" c='\"'
	* The return value will be {GRM_BRAM}.
	*/
	inline string  getvalue(const string& line,const string& word,const char c){
		string::size_type idx,left,right;
		idx = line.find(word);
		assert(idx != string::npos);
		left = line.substr(idx).find(c)+idx;
		right = line.substr(left+1).find(c)+left+1;
		assert(line.find(c)!=string::npos&&left<right);
		return line.substr(left+1,right-left-1);
	}

	inline string  getvalue(const string& word,const char c){
		string cc(1,c);
		string::size_type left,right;
		left = word.find_first_of(cc);
		right = word.find_last_of(cc);
		assert(left != string::npos && right != string::npos);
		return word.substr(left+1,right-left-1);
	}

	// The getalnum function do get all the numbers or letters of the given string 
	//until the end or the ending character(not letter or number).
	inline string getalnum(const string &line)
	{
		string::size_type itr;
		string temp("");
		for (itr=0;itr!=line.size();itr++){
			if(isalnum(line[itr])) temp.push_back(line[itr]);
		}
		return temp;
	}
	inline string getalnum(const string &line,char endingcharacter)
	{
		string::size_type itr;
		string temp("");
		if(isspace(endingcharacter)){
			for (itr=0;itr!=line.size();itr++){
				if(isspace(line[itr])) continue;
				else break;
			}
			if(itr==line.size()) return temp;
			for (;itr!=line.size();itr++){
				if(isalnum(line[itr])) temp.push_back(line[itr]);
				else if(isspace(line[itr])) break;
			}
		}
		else{
			for (itr=0;itr!=line.size();itr++){
				if(line[itr]==endingcharacter) continue;
				else break;
			}
			if(itr==line.size()) return temp;
			for (;itr!=line.size();itr++){
				if(isalnum(line[itr])) temp.push_back(line[itr]);
				else if(line[itr]==endingcharacter) break;
			}
		}
		return temp;
	}
	inline XML_TAG getxmltag(const string& line){
		return NameToXml(getalnum(line,' '));
	}
	inline XDL_TAG getxdltag(const string& line){
		char buf[25];
		string line_copy = line;
		istringstream stream(line_copy);
		string word;
		stream>>word;
		sscanf_s(word.c_str(),"%[1-9a-zA-Z]",buf,_countof(buf));//some thing wrong
		assert(strlen(buf) > 0);
		return NameToXdl(string(buf));
	}
// the func get position from a string like  "x,y" "x,y,z" or "z". return point(x,y,z)
	inline Point getposition(const string& str)
	{
		short x,y,z=0;
		char buf[10];
		string::size_type first_comma,last_comma;
		first_comma=str.find_first_of(',');
		last_comma=str.find_last_of(',');
		if (first_comma==string::npos){
			x=0;y=0;
			z=str2num(str);
		}
		else
		{
			sscanf_s(str.c_str(),"%[^,]",buf,_countof(buf));
			x=str2num(string(buf));
			if(first_comma!=last_comma){
				sscanf_s(string(str,first_comma+1).c_str(),"%[^,]",buf,_countof(buf));
				y=str2num(string(buf));
				sscanf_s(string(str,last_comma+1).c_str(),"%[0-9]",buf,_countof(buf));
				z=str2num(string(buf));
			}
			else{
				sscanf_s(string(str,last_comma+1).c_str(),"%[0-9]",buf,_countof(buf));
				y=str2num(string(buf));
			}
		}
	#ifdef Debug
	 if(x<0||x>500||y<0||y>500||z<0||z>20)
	 {
		 cout<<"coordinate  may not be like this x="<<x<<"  y="<<y<<" z="<<z<<endl;
		 system("pause");
	 }
	#endif
		return Point(x,y,z);
	}

	void parsexdldesign(vector<string>& ves,string& line){
		istringstream stream(line);
		string word;
		stream>>word;//ignore the "inpin" beginner
		assert(word == "design");
		stream>>word;
		ves.push_back(getvalue(word,'\"'));
		stream>>word;
		ves.push_back(word);
		stream>>word;
		ves.push_back(wipeoffword(word,string(";")));
	}
	void parsexdlinst(vector<string>& ves,string& line){
		istringstream stream1(line);
		string word;
		stream1 >> word;//ignore the "inst" beginner
		assert(word == "inst");
		stream1 >> word;
		ves.push_back(getvalue(word,'\"'));
		stream1 >> word;
		ves.push_back(getvalue(word,'\"'));
		string::size_type pos = line.find("placed");
		assert(pos != string::npos);
		string sub = line.substr(pos+6);//6 is the size of string: "placed"
		istringstream stream2(sub);
		stream2 >> word;
		ves.push_back(word);
		stream2 >> word;
		ves.push_back(wipeoffword(word,string(",")));
	}
	void parsexdlnet(vector<string>& ves,string& line){
		istringstream stream(line);
		string word;
		stream >> word;//ignore the "net" beginner
		assert(word == "net");
		stream >> word;
		ves.push_back(getvalue(word,'\"'));
		stream >> word;
		ves.push_back(getalnum(word));
	}
	void parsexdlinpin(vector<string>& ves,string& line){
		istringstream stream(line);
		string word;
		stream >> word;//ignore the "inpin" beginner
		assert(word == "inpin");
		stream >> word;
		ves.push_back(getvalue(word,'\"'));
		stream >> word;
		ves.push_back(wipeoffword(word,string(",")));
	}
	void parsexdloutpin(vector<string>& ves,string& line){
		istringstream stream(line);
		string word;
		stream >> word;//ignore the "inpin" beginner
		assert(word == "outpin");
		stream >> word;
		ves.push_back(getvalue(word,'\"'));
		stream >> word;
		ves.push_back(wipeoffword(word,string(",")));
	}

	void ArchFile::parse(Design* design, NLFactory* factory){
		std::ifstream infile(getfilename().c_str());
		assert(infile);
		string		line;
		XML_TAG		tag;
		Port*		curPort;
		Cell*		curCell;
		Net*		curNet;
		Instance*	curInstance;
		Library*	curLibrary;
		using std::cerr;
#ifdef Debug
		int line_lndex=0;
        int count_of_XMLTAGIGNORE=0;
#endif
		while (getline(infile,line)){
			tag = getxmltag(line);
			if (StartElement(line)){
				if (tag == DESIGN){
					design->rename(getvalue(line,"name",'\"').c_str());
					FPGADesign* fpga = static_cast<FPGADesign*>(design);
					fpga->setscale(getposition(getvalue(line,"scale",'\"')));
				}
				else if (tag == LIBRARY){
					string name = getvalue(line,"name",'\"');
					curLibrary = factory->makeLibrary(name.c_str(),const_cast<const Design*>(design));
					stage = hierarchy_map(name);
				}
				else if (tag == CELL){
					if (stage == TILE){
						ArchFactory* af = static_cast<ArchFactory*>(factory);
						curCell = af->makeTile(getvalue(line,"name",'\"').c_str(),getvalue(line,"type",'\"').c_str(),curLibrary);
					}
					else{
						curCell = factory->makeCell(getvalue(line,"name",'\"').c_str(),getvalue(line,"type",'\"').c_str(),curLibrary);
						static_cast<ArchCell*>(curCell)->sethierarchy(stage);
					}
				}
				else if (tag == PORT){
					ArchPort* archport;
					ArchPin*  cellpin;
					string port_name = getvalue(line,"name",'\"');
					string prefix = getletters(port_name);
					if (stage == TILE){
						short msb = str2num(getvalue(line,"msb",'\"'));
						short lsb = str2num(getvalue(line,"lsb",'\"'));
						SIDE  side = str_side_map(getvalue(line,"side",'\"'));
						for (short i = lsb; i <= msb; ++i){
							string single = prefix + num2str(i);
							curPort = factory->makePort(single.c_str(),curCell);
							archport = static_cast<ArchPort*>(curPort);
							cellpin = factory->makePin(single.c_str(),NULL);
							archport->setserial(i);
							archport->setside(side);
							archport->setcellpin(cellpin);
							cellpin->setcellport(curPort);
							curCell->addport(curPort);
						}
					}
					else{
						curPort = factory->makePort(port_name.c_str(),curCell);
						archport = static_cast<ArchPort*>(curPort);
						cellpin = factory->makePin(port_name.c_str(),NULL);
						archport->setdir(dir_map(getvalue(line,"direction",'\"')));
						archport->setcellpin(cellpin);
						cellpin->setcellport(curPort);
						curCell->addport(curPort);
					}
				}
				else if (tag == PATH)
				{
					//more information to add: segregated, cin, cout, r, delay and so on
					ArchCell::const_port_iter from,to;
					if( !curCell->findport(from,getvalue(line,"from",'\"')) ){cerr<<"something wrong here."<<"file: "<<__FILE__<<" line"<<__LINE__<<endl;} //modification 1
					if( !curCell->findport(to,getvalue(line,"to",'\"'))){cerr<<"something wrong here."<<"file: "<<__FILE__<<" line"<<__LINE__<<endl;}
					(static_cast<ArchCell*>(curCell))->creatpath(const_cast<Pin*>((*from)->getcellpin()),const_cast<Pin*>((*to)->getcellpin()));
				}
				else if (tag == INSTANCE)
				{
					string tmp_inst_name = getvalue(line,"name",'\"');
					string cellRef = getvalue(line,"cellRef",'\"');
					string libraryRef = getvalue(line,"libraryRef",'\"');
					Design::const_lib_iter lib;
					Library::const_cell_iter cell;
					bool b1 = design->findlib(lib,libraryRef);
					assert(b1);
					bool b2 = (*lib)->findelem(cell,cellRef);
					assert(b2);
					ArchCell* instof = static_cast<ArchCell*>(*cell);
					if(stage == ARCH)
						curInstance = factory->makeInstance(tmp_inst_name.c_str(),NULL,instof);
					else 
						curInstance = factory->makeInstance(tmp_inst_name.c_str(),curCell,instof);
					if (stage == TILE)
						(static_cast<ArchInstance*>(curInstance))->setposition(getposition(getvalue(line,"z",'\"')));// the third index of pos for slice or GRM.
					else if (stage==ARCH)
						(static_cast<ArchInstance*>(curInstance))->setposition(getposition(getvalue(line,"pos",'\"')));
				}
				else if (tag == NET)
				{
					curNet = factory->makeNet(getvalue(line,"name",'\"').c_str(),curCell);
				}
				else if (tag == PORTREF)
				{
					string::size_type pos = line.find("instanceRef");
					string pname = getvalue(line,"name",'\"');
					if (pos != string::npos)
					{
						string instRef = getvalue(line,"instanceRef",'\"');
						Cell::const_inst_iter		instIter;
						Instance::const_pin_iter	 pinIter;
						bool b1 = curCell->findinst(instIter,instRef);
						assert(b1);
						bool b2 = (*instIter)->findpin(pinIter,pname);
						assert(b2);
						if(!(b1&&b2)){cerr<<"something wrong here"<<"file: "<<__FILE__<<" line"<<__LINE__<<endl;} //modification3
						curNet->addpin(*pinIter);
						(*pinIter)->addnet(curNet);
					}
					else
					{
						Cell::const_port_iter		portIter;
						bool temp=curCell->findport(portIter,pname);assert(temp);
						Pin* cellpin = const_cast<Pin*>((*portIter)->getcellpin());
						curNet->addpin(cellpin);
						cellpin->addnet(curNet);
					}
				}
			}
#ifdef Debug
			++line_lndex;
			if(tag==XMLTAGIGNORE&&line.find("interface")==string::npos&&line.find("content")==string::npos)
			{
				count_of_XMLTAGIGNORE++;
				if(count_of_XMLTAGIGNORE>10)
				{
					cout<<"too many XMLTAGIGNORE lines exits! make sure it is OK and continue"<<endl;
					system("pause");
					count_of_XMLTAGIGNORE=0;
				}
			}
#endif
			if (EndElement(line))
			{
				if (tag == DESIGN){
					cout<<"Read arch complete!"<<endl;
				}
				else if (tag == LIBRARY){
					stage = HIEIGNORE;
					design->addlib(curLibrary);
				}
				else if (tag == CELL){
					curLibrary->addelem(curCell);
				}
				else if (tag == PATH){
					assert(stage == ELEMENT);
				}
				else if (tag == INSTANCE){
					if (stage == ARCH)
						design->addinst(curInstance);
					else
						curCell->addinst(curInstance);
				}
				else if (tag == NET){
						curCell->addnet(curNet);
				}
			}
	}
	infile.close();
}


	void PackageFile::parse(Design* design, NLFactory* factory){
		std::ifstream infile(getfilename().c_str());
		assert(infile);
		string		line;
		XML_TAG		tag;
		PadInfo*	curPad;
		FPGADesign* fpgadesign = dynamic_cast<FPGADesign*>(design);
		assert(fpgadesign);
		ArchFactory* archfactory = dynamic_cast<ArchFactory*>(factory);
		assert(archfactory);

		while (getline(infile,line)){
			tag = getxmltag(line);
			if (StartElement(line)){
				if (tag == PACKAGE){
					fpgadesign->setpackage(getvalue(line,"name",'\"').c_str());
				}
				else if (tag == PAD){
					curPad = archfactory->makePad(getvalue(line,"name",'\"').c_str(),getposition(getvalue(line,"pos",'\"')));
					fpgadesign->addpad(curPad);
				}
			}
			
		}
		infile.close();
	}


	void XDLFile::parse(Design* cktdesign, NLFactory* cktfactory){
		std::ifstream infile(getfilename().c_str());
		assert(infile);
		XDL_TAG		tag;
		string		line;
		Pin*		curPin;
		Port*		curPort;
		Cell*		curCell;
		Net*		curNet;
		Instance*	curInstance;
		NetTrace*	curNettrace;
		SliceConfig* curSliCfg;
		Library*	curLibrary = cktfactory->makeLibrary("primitive",cktdesign);
		cktdesign->addlib(curLibrary);
		
		XDLFactory* xdlfactory = new XDLFactory();

		vector<string>	ves;
	//	int line_index = 0;
		stage	=	CKTSTAGEIGNORE;
		while (getline(infile,line)){
	//		++line_index;
	//		cout << "parsing line: " << line_index << endl;
			if ( spaceline(line) || xdl_annotated_line(line) )	continue;
			tag = getxdltag(line);
			if (tag == XDLDESIGN){
				parsexdldesign(ves,line);
				xdldesign->setname(const_cast<char*>(ves.at(0).c_str()));
				xdldesign->setdevice(const_cast<char*>(ves.at(1).c_str()));
				xdldesign->setxdlversion(const_cast<char*>(ves.at(2).c_str()));
			}
			else if (tag == INST){
				stage = CKTINST;
				parsexdlinst(ves,line);
				curCell = cktfactory->makeCell(ves.at(3).c_str(),ves.at(1).c_str(),curLibrary);
				curInstance = cktfactory->makeInstance(ves.at(0).c_str(),NULL,curCell);
				curSliCfg = xdlfactory->makeSlice(curInstance,ves.at(2).c_str());
				curLibrary->addelem(curCell);
				cktdesign->addinst(curInstance);
			}
			else if (tag == XDLNET){
				stage = CKTNET;
				parsexdlnet(ves,line);
				curNet = cktfactory->makeNet(ves.at(0).c_str(),NULL);
				NETTYPE type = ORDINARY;
				if( !ves.at(1).compare("VCC") || !ves.at(1).compare("vcc") ) type = VCC;
				if( !ves.at(1).compare("GND") || !ves.at(1).compare("gnd") ) type = GND;
				curNettrace = xdlfactory->makeNet(curNet,type);
			}
			else if (tag == OUTPIN){
				parsexdloutpin(ves,line);
				Design::const_inst_iter iter;
				XDLDesign::const_sliceIter sli_iter = xdldesign->findsite(ves.at(0).c_str());
				bool b = cktdesign->findinst(iter,ves.at(0));
				assert(b);
				curPort = cktfactory->makePort(ves.at(1).c_str(),(*iter)->getcell());
				curPin = cktfactory->makePin(ves.at(1).c_str(),*iter);
				(*iter)->getcell()->addport(curPort);
				(*iter)->addpin(curPin);
				curNet->addpin(curPin);
				curNet->addsrc(curPin);
				curNettrace->setsrc(*sli_iter);
			}
			else if (tag == INPIN){
				parsexdlinpin(ves,line);
				Design::const_inst_iter iter;
				XDLDesign::const_sliceIter sli_iter = xdldesign->findsite(ves.at(0).c_str());
				bool b = cktdesign->findinst(iter,ves.at(0));
				assert(b);
				curPort = cktfactory->makePort(ves.at(1).c_str(),(*iter)->getcell());
				curPin = cktfactory->makePin(ves.at(1).c_str(),*iter);
				(*iter)->getcell()->addport(curPort);
				(*iter)->addpin(curPin);
				curNet->addpin(curPin);
				curNet->addsink(curPin);
				curNettrace->addsink(*sli_iter);
			}
			else{ //store the config info of the insts
				if (!EndXdlElement(line)){
					if(stage == CKTINST) curSliCfg->addConfigInfo(wipeoffword(line,string("cfg")));
				}
			}

			if (EndXdlElement(line)){
				if (stage == CKTINST)	xdldesign->addslice(curSliCfg);
				if (stage == CKTNET){
					if(!curNettrace->getnet()->dangle()) xdldesign->addnet(curNettrace); 
				}
				stage = CKTSTAGEIGNORE;
			}
			ves.clear();
		}
		infile.close();
	}

}}