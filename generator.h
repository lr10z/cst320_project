/***********************************************************
* Author:					Leander Rodriguez
* Date Created:				05/29/14
* Last Modification Date:	06/05/14
* Lab Number:				CST 320 Final Project
* Filename:					generator.h
*
************************************************************/
#ifndef GENERATOR_H
#define GENERATOR_H

#include <fstream>
#include <string>
#include <stack>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <list>
#include <vector>
#include <iterator>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace std;

struct parserEntry
{
	parserEntry(string value, string type)
	{
		m_value = value;
		m_type = type;
	}

	std::string m_value;
	std::string m_type;
};

class Generator
{
	public:

		Generator(string languageIn);

		// left factor method
		//string leftFactor(string languageIn);
		void leftFactor();

		// grammar methods
		map<string, vector<vector<string>>>::iterator begin() { return m_grammar.begin(); }
		map<string, vector<vector<string>>>::iterator end() { return m_grammar.end(); }

		// first set methods
		void firstSet();
		void firstSet2(map<string, vector<string>>::iterator fs);

		vector<string> getFirsts(string key1) { return m_firstSet[key1]; }

		// follow set methods
		void followSet();
		void followSet2(string currentToken, string terminal, int idx);
		void followSet3();
		void followSet4(string nT, string currentTok, int idx);

		vector<vector<string>> getRules(string nonTerminal) { return m_grammar.at(nonTerminal); }

		// LL(1) table methods
		void LL1Table();
		void LL1Table2();

		list<string> getRule(string key1, string key2) { return m_table[make_pair(key1, key2)]; }

		map<pair<string, string>, list<string>>::iterator beginTbl() { return m_table.begin(); }
		map<pair<string, string>, list<string>>::iterator endTbl() { return m_table.end(); }

	private:

		// variables
		int twin;
		int count;
		bool is_in;
		bool m_lambda;
		string prevFirstTok;
		string prevNonTerminal;
		string non_Terminal;
		string currentToken;
		ofstream myfile;

		// containers
		vector<string> order;
		set<string> terminals;
		vector<string> have_lambdas;
		unordered_set<string> m_nonTerminals;
		map<pair<string, string>, list<string>> m_table;
		map<string, vector<vector<string>>> m_grammar;
		map<string, vector<string>> m_firstSet;
		unordered_map<string, vector<string>> m_followSet;

		// iterators
		map<string, vector<vector<string>>>::iterator grmIter;
		vector<vector<string>>::iterator rulesIter;
};

#endif