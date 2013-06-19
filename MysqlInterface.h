#pragma once
#include <WinSock2.h>
#include"mysql.h"
#include <iostream>
#include <string>
using namespace std;

class MysqlInterface
{
public:
	MysqlInterface(string _userName="root",string _passWord="mysql",string _DBName="Rrg_DB"):userName(_userName),passWord(_passWord),DBName(_DBName)
	{
		
	}
	~MysqlInterface(void);
	void mysqlRealQuery(const string & cwd);
	void writeInitial(void);
	void readInitial(void);
private:
	string userName;
	string passWord;
	string DBName;
	MYSQL  sql_obj;

};
