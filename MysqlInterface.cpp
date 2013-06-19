#include "StdAfx.h"
#include "MysqlInterface.h"



MysqlInterface::~MysqlInterface(void)
{
	mysql_close(&sql_obj);
}

void MysqlInterface::writeInitial( void )
{
	string tempDB="test";
	mysql_init(&sql_obj);
	if(mysql_real_connect(&sql_obj,NULL,userName.c_str(),passWord.c_str(),tempDB.c_str(),0,NULL,0)){
		cout<<"mysql connect succeeded!"<<endl;
	}
	else{
		cout<<"mysql connect failed!"<<endl;
	}
	mysqlRealQuery("DROP DATABASE IF EXISTS "+DBName);
	mysqlRealQuery("CREATE DATABASE "+DBName);
	mysql_init(&sql_obj);
	if(mysql_real_connect(&sql_obj,NULL,userName.c_str(),passWord.c_str(),DBName.c_str(),0,NULL,0)){
		cout<<"mysql connect succeeded!"<<endl;
	}
	else{
		cout<<"mysql connect failed!"<<endl;
	}
}

void MysqlInterface::readInitial( void )
{
	mysql_init(&sql_obj);
	if(mysql_real_connect(&sql_obj,NULL,userName.c_str(),passWord.c_str(),DBName.c_str(),0,NULL,0)){
		cout<<"mysql connect succeeded!"<<endl;
	}
	else{
		cout<<"mysql connect failed!"<<endl;
	}
}

void MysqlInterface::mysqlRealQuery( const string & cwd )
{
	if(mysql_real_query(&sql_obj,cwd.c_str(),cwd.length())!=0){
		cout<<"failed to query, command line:\n\t "<<cwd<<endl;
		system("pause");
	}
}