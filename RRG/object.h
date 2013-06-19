#pragma once
#ifndef OBJECT_H
#define OBJECT_H

#include <iostream>
using std::string;
namespace FDP { namespace netlist {

	class Object
	{
	public:
		typedef Object		owner_type;
		const char* 		getname()		const { return name.c_str(); }
		void				rename(string s) {	name = s; }
		string				fullname()		const { return name; }
		const owner_type*	getowner()		const { return owner; }

	protected:
		explicit Object(string s = string("unnamed"), const owner_type* _owner = NULL):name(s),owner(_owner){}
		virtual ~Object() {}
	private:
		string name;
		const owner_type* owner;
		Object(const Object&);
		Object& operator=(const Object&);
	};

}
}








#endif