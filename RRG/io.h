#pragma once
#ifndef IO_H
#define IO_H

#include "netlist.h"
#include "archlib.h"
#include "xdlnetlist.h"
#include <vector>

namespace FDP { namespace IO {

	using namespace netlist;
	using namespace archlib;
	using std::map;
	using std::string;
	using std::vector;

	class READER; class WRITER; class FILE; class ArchReader; class ArchFile;
	class XDLReader; class XDLFile; class XDLWriter;

	class FILE
	{
	public:
		FILE(const char* _file):name(_file){}
		virtual ~FILE(){}
		virtual void parse(Design*, NLFactory*) = 0;
	protected:
		string getfilename()	const	{ return name; }
	private:
		string name;
	};

	class ArchFile : public FILE
	{
	public:
		ArchFile(const char* _file):FILE(_file),stage(HIEIGNORE){}
		void parse(Design*, NLFactory*);
	private:
		HIERARCHY stage;
	};

	class PackageFile : public FILE
	{
	public:
		PackageFile(const char* _file):FILE(_file){}
		void parse(Design*, NLFactory*);
	};

	class XDLFile : public FILE
	{
	public:
		XDLFile(const char* _file,XDLDesign* _xdldeisgn):FILE(_file),stage(CKTSTAGEIGNORE),xdldesign(_xdldeisgn){}
		void parse(Design*, NLFactory*);
		XDLDesign* getxdl() { return xdldesign; }
	private:
		XDLCktStage stage;
		XDLDesign*	xdldesign;
	};

	class READER
	{
	public:
		READER(Design* d,FILE* f):_design(d),_file(f){}
		virtual ~READER(){}
		virtual void parse() = 0;
	protected:
		Design*		getdesign()		const	{ return _design; }
		FILE*		getfile()		const	{ return _file; }
	private:
		Design* _design;
		FILE*	_file;
	};

	class WRITER
	{
	public:
		WRITER(FILE* _file):file(_file){}
		virtual ~WRITER(){}
	protected:
		FILE*	getfile()		const		{ return file; }
	private:
		FILE*	file;
	};

	class ArchReader : public READER
	{
	public:
		ArchReader(FPGADesign* d,ArchFile* f):READER(d,f){} //syntax question
		void  parse()	{ 
			NLFactory* factory = new ArchFactory;
			getfile()->parse( getdesign(), factory ); //syntax question
			delete factory;
		}
	};

	class PackageReader : public READER
	{
	public:
		PackageReader(FPGADesign* d,PackageFile* f):READER(d,f){}
		void  parse() {
			NLFactory* factory = new ArchFactory;
			getfile()->parse( getdesign(), factory);
			delete factory;
		}
	private:
	};

	class XDLReader : public READER
	{
	public:
		XDLReader(Design* d,XDLFile* f):READER(d,f){}
		void parse() {
			NLFactory* factory = new NLFactory;
			XDLFile* xdl = static_cast<XDLFile*>(getfile());
			Design* ckt = xdl->getxdl()->getdesign();
			getfile()->parse( ckt, factory );
			delete factory;
		}
	};

	class XDLWriter : public WRITER
	{
	public:
		XDLWriter(XDLFile* xdlfile):WRITER(xdlfile){}
	};

	string			getletters(string&);
	string			getalnum(const string &);
	string			getalnum(const string &,const char );
	string			getvalue(const string&,const string&,const char);
	string			getvalue(const string&,const char);
	string			wipeoff_space_and_symbolics(const string&);
	string			wipeoffword(string&,string&);
	bool			StartElement(const string&);
	bool			EndElement(const string&);
	bool			StartAndEndInOneLine(const string&);
	bool			spaceline(const string&);
	bool			xdl_annotated_line(const string&);
	bool			EndXdlElement(const string&);
	XML_TAG			getxmltag(const string&);
	XDL_TAG			getxdltag(const string&);
	Point			getposition(const string&);

	void			parsexdldesign(vector<string>&,string&);
	void			parsexdlinst(vector<string>&,string&);
	void			parsexdlnet(vector<string>&,string&);
	void			parsexdlinpin(vector<string>&,string&);
	void			parsexdloutpin(vector<string>&,string&);

}}




#endif