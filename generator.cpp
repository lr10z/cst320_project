/***********************************************************
* Author:					Leander Rodriguez
* Date Created:				05/29/14
* Last Modification Date:	06/05/14
* Lab Number:				CST 320 Final Project
* Filename:					generator.cpp
*
************************************************************/
#include <cctype>
#include <locale>

#include "generator.h"


Generator::Generator(string languageIn)
{
	// local variables
	string	separator,
			rule;

	count = 0;
	twin = 0;

	// 
	// open file to where LL(1) table will be displayed
	//
	myfile.open("output.txt");

	vector<string> rules;

	// string buffer created
	istringstream iss(languageIn);

	// parses string into new lines
	while (getline(iss, languageIn, '\n'))
	{
		// find and display all comments
		size_t pos = languageIn.rfind("//");

		if (pos != string::npos)
		{
			cout << languageIn << endl;
			myfile << languageIn << endl;
			continue;
		}

		// line buffer created
		stringstream ss(languageIn);

		// inserts non-Terminals into containers as keys
		ss >> non_Terminal >> separator;

		m_nonTerminals.insert(non_Terminal);

		bool found = find(order.begin(), order.end(), non_Terminal) != order.end();
		if (!found)
		{
			order.push_back(non_Terminal);
		}

		// stores nonTerminals and associated rule
		while (ss >> rule)
		{
			if (rule == "lambda")
			{
				m_lambda = true;
			}

			rules.push_back(rule);
		}

		//store grammar
		m_grammar[non_Terminal].push_back(rules);
		rules.clear();
	}

	// left factor grammar if needed
	leftFactor();

	// normalize order of non-Terminals once more to include additional
	// terminals if grammar was left factored
	for (auto n = m_nonTerminals.begin(); n != m_nonTerminals.end(); ++n)
	{
		bool found = find(order.begin(), order.end(), *n) != order.end();
		if (!found)
		{
			order.push_back(*n);
		}
	}
	
	// prep LL(1) table
	LL1Table();

	// creates the first set
	firstSet();
}

//string Generator::leftFactor(string languageIn)
void Generator::leftFactor()
{
	// loop through grammar, recgonize left factoring
	// 
	string firstTok;
	vector<vector<string>>::iterator reverseIter;

	// loops through non-terminals keys in map container
	for (grmIter = begin(); grmIter != end(); ++grmIter)
	{
		// loops through each of the current non-terminal's rules
		for (rulesIter = grmIter->second.begin(); rulesIter != grmIter->second.end(); ++rulesIter)
		{
			// store first token and iterator location
			firstTok = *rulesIter->begin();
			reverseIter = rulesIter;

			if (prevFirstTok.empty() && prevNonTerminal.empty())
			{
				prevNonTerminal = grmIter->first;
				prevFirstTok = firstTok;
				twin = 0;
			}

			if (prevNonTerminal != grmIter->first)
			{
				prevNonTerminal = grmIter->first;
				prevFirstTok = firstTok;
				twin = 0;
			}

			if (prevFirstTok == firstTok)
			{	
				++twin;
				if (twin == 2)
				{
					--twin;
					vector<string> tempV;

					// get tokens to factor out
					--reverseIter;
					for (unsigned idx = 1; idx < reverseIter->size(); ++idx)
					{
						tempV.push_back(reverseIter->at(idx));
					}

					// go back to previous rule and adjust it
					reverseIter->clear();
					reverseIter->push_back(firstTok);

					// generate a random letter to create new non-Terminal
					string ntLetter;

					for (char ch = 'D'; ch <= 'Z'; ch++)
					{
						string letter(1, ch);

						// check if ch is an existing nonterminal
						if (!(m_nonTerminals.find(letter) != m_nonTerminals.end()))
						{
							ntLetter = letter;
							break;
						}
					}

					reverseIter->push_back(ntLetter);
					m_nonTerminals.insert(ntLetter);

					// add new non-Terminal to grammar
					// add tempV rule to new non-Terminal
					m_grammar[ntLetter].push_back(tempV);

					// clear temporary storage of rules
					tempV.clear();

					// loop through current rulesIter and get tokens to factor out
					for (unsigned idx = 1; idx < rulesIter->size(); ++idx)
					{
						tempV.push_back(rulesIter->at(idx));
					}

					// add those as new rule to new non-Terminal
					m_grammar[ntLetter].push_back(tempV);

					// clear temporary storage of rules
					tempV.clear();

					// clear all remaining rules after the first rule
					// of the current non-Terminal
					for (unsigned e = 1; e < grmIter->second.size(); ++e)
					{
						if ( grmIter->second.at(e) != grmIter->second.back() )
						{
							// check if next rule exists for the current non-Terminal
							// by incrementing iterator to see next rule
							auto nextIter = rulesIter + 1;

							if (!nextIter->empty())
							{
								// next rule does exist, therefore
								// check if its firstToken is the same as the current firstTok
								if (*nextIter->begin() == firstTok)
								{
									// if it is, copy the remaining tokens in rule
									for (unsigned idx = 1; idx < nextIter->size(); ++idx)
									{
										tempV.push_back(nextIter->at(idx));
									}

									// if there are no remaining tokens, then remaining token will be lambda
									if (tempV.empty())
									{
										tempV.push_back("lambda");
									}

									// pop back rule
									grmIter->second.pop_back();

									// adjust iterator?
								}

							}
						}

						grmIter->second.pop_back();
					}

					// add those as new rule to new non-Terminal
					m_grammar[ntLetter].push_back(tempV);

					// set current rulesIter equal to reverseIter
					rulesIter = reverseIter;
				}
			}
		}
	}
}

void Generator::firstSet()
{
	// local variables
	bool next = true;

	// loops through non-terminals keys in map container
	for (grmIter = begin(); grmIter != end(); ++grmIter)
	{
		// loops through each of the current non-terminal's rules
		for (rulesIter = grmIter->second.begin(); rulesIter != grmIter->second.end(); ++rulesIter)
		{
			// loops through each terminal/non-terminal of current rule
			for (unsigned idx = 0; idx < rulesIter->size(); ++idx)
			{
				if (next == false )
				{
					continue;
				}

				// stores current terminal/non-terminal
				currentToken = rulesIter->at(idx);

				//
				// if token is a non-terminal, go to non-terminal and find its first token
				//
				if (m_nonTerminals.find(currentToken) != m_nonTerminals.end())
				{
					//
					// gets first set from non-terminal if the 
					// non-terminal is already in the first set container
					//
					vector<string> nonT = getFirsts(currentToken);
					vector<string>::iterator nonIt = nonT.begin();

					if (nonT.empty())
					{
						// go to that non-terminal and get first token
						m_firstSet[grmIter->first].push_back(currentToken);
						continue;
					}
					// else token is a terminal, terminal gets added to first set
					// and breaks out of loop to check for the next first terminal
					else
					{
						for (nonIt; nonIt != nonT.end(); ++nonIt)
						{
							if (*nonIt != "lambda")
							{
								m_firstSet[grmIter->first].push_back(*nonIt);
								next = false;

								// if token is not lambda, add current non-Terminal, 
								// current token and associated rule to LL(1) table
								if (currentToken != "lambda")
								{
									// adds rule to table
									for (unsigned idx2 = 0; idx2 < rulesIter->size(); ++idx2)
									{
										// if current token is an indirect non-Terminal
										if (m_nonTerminals.find(*nonIt) != m_nonTerminals.end())
										{
											// get terminals of appropriate/direct non-Terminal
											auto gf = getFirsts(*nonIt);
											for (auto gfIter = gf.begin(); gfIter != gf.end(); ++gfIter)
											{
												m_table[make_pair(grmIter->first, *gfIter)].push_back(rulesIter->at(idx2));
											}
										}
										else
										{
											m_table[make_pair(grmIter->first, *nonIt)].push_back(rulesIter->at(idx2));
										}
									}
								}
							}
							else
							{
								next = true;
							}
						}
					}
				}
				else
				{
					// keep track of what non-Terminals have a lambda rule
					if (currentToken == "lambda")
					{
						have_lambdas.push_back(grmIter->first);
					}

					// add terminal to first set of current non-Terminal
					m_firstSet[grmIter->first].push_back(currentToken);
					
					// if token is not lambda, add current non-Terminal, 
					// current token and associated rule to LL(1) table
					if (currentToken != "lambda")
					{
						// adds rule to table
						for (unsigned idx2 = 0; idx2 < rulesIter->size(); ++idx2)
						{
							m_table[make_pair(grmIter->first, currentToken)].push_back(rulesIter->at(idx2));
						}
					}

					break;
				}
			}
		}
	}

	
	map<string, vector<string>>::iterator fs = m_firstSet.begin();

	// loops through first set map container
	for (fs; fs != m_firstSet.end(); fs)
	{
		firstSet2(fs);

		++fs;
	}


	// if lambda exists in any grammar rule, create follow set
	// before creating LL1 table
	if (m_lambda)
	{
		followSet();
		followSet3();
	}
	
	LL1Table2();
}

void Generator::firstSet2(map<string, vector<string>>::iterator fs)
{
	// local variables
	vector<string> tokens = fs->second;
	vector<string> ntFirsts;
	vector<string>::iterator fsIter;

	for (unsigned i = 0; i < tokens.size(); ++i)
	{
		//
		// returns true if current token of the non-Terminal 
		// rules is a non-Terminal that exists in the first set
		//
		is_in = m_firstSet.find(tokens.at(i)) != m_firstSet.end();

		//
		// if true was returned, replace non-Terminal with first set of that non-Terminal
		//
		if (is_in)
		{
			// returns first set of current non-terminal
			ntFirsts = getFirsts(tokens.at(i));

			// add current non-Terminal, current token, and associated rule to LL(1) table
			for (auto fr = ntFirsts.begin(); fr != ntFirsts.end(); ++fr)
			{
				bool ck = m_table[make_pair(fs->first, *fr)].empty();

				if (m_table[make_pair(fs->first, *fr)].empty())
				{
					m_table[make_pair(fs->first, *fr)].push_back(tokens.at(i));
				}
			}
			
			// removes first non-Terminal in container
			fs->second.erase(remove(fs->second.begin(), fs->second.end(), tokens.at(i)), fs->second.end());

			for (fsIter = ntFirsts.begin(); fsIter != ntFirsts.end(); ++fsIter)
			{
				if (*fsIter == "lambda")
				{
					// checks if token already exists in first set
					// of current non-Terminal
					is_in = find(fs->second.begin(), fs->second.end(), *fsIter) != fs->second.end();

					if (is_in)
					{
						// since current token is lambda, recursive function call 
						// is made to see if next token needs to be handled
						firstSet2(fs);
					}
					else
					{
						// adds lambda to first set of current non-Terminal
						fs->second.push_back(*fsIter);

						// since current token is lambda, recursive function call 
						// is made to see if next token needs to be handled
						firstSet2(fs);
					}
				}
				else
				{
					// checks if token already exists in first set
					// of current non-Terminal
					is_in = find(fs->second.begin(), fs->second.end(), *fsIter) != fs->second.end();

					if (is_in)
					{
						continue;
					}
					else
					{
						fs->second.push_back(*fsIter);
					}
				}
			}
		}
	} 
}

void Generator::followSet()
{
	// fill follow set with non-Terminals
	unordered_set<string>::iterator sIter = m_nonTerminals.begin();

	for (sIter; sIter != m_nonTerminals.end(); ++sIter)
	{
		m_followSet[*sIter];
	}

	//
	// case 1
	// add blank space to the start symbol's follow set
	// equivalent to adding $
	//
	//m_followSet.begin()->second.push_back("$");

	//
	// case 2
	// if there is a terminal following a non-Terminal
	// ...Ab
	//

	// loop through grammar
	for (grmIter = begin(); grmIter != end(); ++grmIter)
	{
		// loops through each of the current non-terminal's rules
		for (rulesIter = grmIter->second.begin(); rulesIter != grmIter->second.end(); ++rulesIter)
		{
			// loops through each terminal/non-terminal of current rule
			for (unsigned idx = 0; idx < rulesIter->size(); ++idx)
			{
				// gets current token
				currentToken = rulesIter->at(idx);

				// if current token is a non-Terminal
				if (m_nonTerminals.find(currentToken) != m_nonTerminals.end())
				{
					if (idx + 1 == rulesIter->size())
					{
						break;
					}

					// gets next token in rule
					string nxtToken = rulesIter->at(idx+1);

					//
					// if next token in rule is a non-Terminal, check for lambda
					// 
					if (m_nonTerminals.find(nxtToken) != m_nonTerminals.end())
					{
						// handle
						// add first set of nxtToken to the follow set of current token
						auto gf = getFirsts(nxtToken);

						for (auto gfIter = gf.begin(); gfIter != gf.end(); ++gfIter)
						{
							if (*gfIter == "lambda")
							{
								continue;
							}
							else
							{
								m_followSet[currentToken].push_back(*gfIter);
							}
						}

						
						//
						// check if current non-Terminal has a lambda rule
						//
						bool has_lambda = false;

						auto gr = getRules(currentToken);

						for (auto grIter = gr.begin(); grIter != gr.end(); ++grIter)
						{
							if (*grIter->begin() == "lambda")
							{
								has_lambda = true;
							}
						}


						if (has_lambda)
						{
							// check if prev token exists
							if (currentToken == *rulesIter->begin())
							{
								continue;
							}
							else
							{
								// prev token exists, get prev token
								string prevTok;
								prevTok = rulesIter->at(idx-1);

								// check if prev token is non-Terminal
								if (m_nonTerminals.find(prevTok) != m_nonTerminals.end())
								{
									// if yes, handle
									// add first set of nxtToken to the follow set of current token
									auto gf = getFirsts(nxtToken);

									for (auto gfIter = gf.begin(); gfIter != gf.end(); ++gfIter)
									{
										if (*gfIter == "lambda")
										{
											continue;
										}
										else
										{
											m_followSet[prevTok].push_back(*gfIter);
										}
									}

								}
								continue;
							}
						}
					}
					// else next token is a terminal
					else
					{
						is_in = false;

						// checks if token already exists in follow set of current non-Terminal
						is_in = find(m_followSet[currentToken].begin(), m_followSet[currentToken].end(), nxtToken) != m_followSet[currentToken].end();

						if (is_in)
						{
							continue;
						}
						else
						{
							// terminal added to the follow set of current non-Terminal
							m_followSet[currentToken].push_back(nxtToken);
						}

						// check if current non-Terminal token goes to lambda
						followSet2(currentToken, nxtToken, idx);
					}
				}
			}
		}
	}

	followSet3();
}

void Generator::followSet2( string currentToken, string terminal, int idx)
{
	// get current non-Terminal token's rules
	auto gr = getRules(currentToken);
	
	// loop through rule tokens
	for (auto grIter = gr.begin(); grIter != gr.end(); ++grIter)
	{
		// check if non-Terminal goes to lambda
		if (*grIter->begin() == "lambda")
		{
			// if it does, check the token before that one
			// check if prev token exists
			if (currentToken == *rulesIter->begin())
			{
				continue;
			}
			else
			{
				// prev token exists, get prev token
				string prevTok;
				prevTok = rulesIter->at(idx - 1);

				// check if prev token is non-Terminal
				if (m_nonTerminals.find(prevTok) != m_nonTerminals.end())
				{
					// if true, then add terminal to that new prev non-Terminal
					m_followSet[prevTok].push_back(terminal);

					// and check again, recursive function call
					followSet2(prevTok, terminal, idx);
				}
			}
		}
	}
}

void Generator::followSet3()
{
	//
	// case 4
	// if current non-Terminal is a unit-production or 
	// if the rule ends with a non-Terminal
	// ...B
	//

	// loop through grammar
	for (grmIter = begin(); grmIter != end(); ++grmIter)
	{
		// gets current non-Terminal and amount of non-Terminal's rules
		non_Terminal = *grmIter->first.begin();

		// loops through each of the current non-terminal's rules
		for (rulesIter = grmIter->second.begin(); rulesIter != grmIter->second.end(); ++rulesIter)
		{
			// loops through each terminal/non-terminal of current rule backwards
			for (unsigned idx = rulesIter->size() - 1; idx != 0; --idx)
			{
				// gets current token
				currentToken = rulesIter->at(idx);

				// if current token is the last token in rule
				if (currentToken == rulesIter->back())
				{
					// if last token is a non-Terminal
					if (m_nonTerminals.find(currentToken) != m_nonTerminals.end())
					{
						// then anything that follows current non-Terminal
						// also follows current non-Terminal token
						followSet4(non_Terminal, currentToken, idx);
					}
				}
			}
		}
	}
}

void Generator::followSet4(string non_Terminal, string currentToken, int idx)
{

	// anything that follows current non-Terminal
	// also follows current non-Terminal token
	is_in = false;

	auto fs = m_followSet[non_Terminal].begin();

	for (fs; fs != m_followSet[non_Terminal].end(); ++fs)
	{
		// checks if token already exists in follow set of current non-Terminal
		is_in = find(m_followSet[currentToken].begin(), m_followSet[currentToken].end(), *fs) != m_followSet[currentToken].end();

		if (is_in)
		{
			continue;
		}
		else
		{
			m_followSet[currentToken].push_back(*fs);
		}
	}

	
	if (is_in == false)
	{
		string prevTok;

		// checks if previous token exists
		if (idx - 1 >= 0)
		{
			// gets previous token
			prevTok = rulesIter->at(idx - 1);

			// if previous token is a non-Terminal token
			if (m_nonTerminals.find(prevTok) != m_nonTerminals.end())
			{
				// must then check if current non-Terminal token goes to lambda
				bool has_lambda = false;

				auto lmda = m_grammar[currentToken];
				auto lmda2 = lmda.begin();

				for (lmda2; lmda2 != lmda.end(); ++lmda2)
				{
					// if current non-terminal token goes to lambda
					if (*lmda2->begin() == "lambda")
					{
						has_lambda = true;

						// then check previous token, recursively
						followSet4(non_Terminal, prevTok, idx-1);
					}
				}
			}
		}
	}
}

void Generator::LL1Table()
{
	//terminals.insert("$");

	// loop through grammar to find all terminals
	for (grmIter = begin(); grmIter != end(); ++grmIter)
	{
		auto rls = grmIter->second.begin();

		for (rls; rls != grmIter->second.end(); rls)
		{
			// get current non-Terminal's rule
			auto ruleToks = rls->begin();

			// loop through rule tokens
			for (ruleToks; ruleToks != rls->end(); ruleToks)
			{
				// if token is a terminal
				if (!(m_nonTerminals.find(*ruleToks) != m_nonTerminals.end()))
				{
					// store terminal
					if (*ruleToks != "lambda")
					{
						terminals.insert(*ruleToks);
					}
				}

				++ruleToks;
			}

			++rls;
		}
	}

	
	grmIter = begin();
	set<string>::iterator terminalsIter;

	// loop through grammar
	for (grmIter; grmIter != end(); grmIter)
	{
		// reset terminals iterator
		terminalsIter = terminals.begin();

		// loop through terminals container
		for (terminalsIter; terminalsIter != terminals.end(); ++terminalsIter)
		{
			// add terminals to table for non-Terminal and terminal relationships
			m_table[make_pair(grmIter->first, *terminalsIter)];
		}

		++grmIter;
	}
}

void Generator::LL1Table2()
{
	vector<string>::iterator hlIter = have_lambdas.begin();
	unordered_map<string, vector<string>>::iterator fsIter = m_followSet.begin();

	// loop through non-Terminals that contain lambda rules
	for (hlIter; hlIter != have_lambdas.end(); hlIter)
	{
		// loop through each terminal in non-Terminal's follow set
		for (unsigned idx = 0; idx < m_followSet[*hlIter].size(); ++idx)
		{
			// add lambda rule to table for terminal
			m_table[make_pair(*hlIter, m_followSet[*hlIter].at(idx))].push_back("lambda");
		}

		++hlIter;
	}

	//
	// output table to file
	//
	//ofstream myfile;
	//myfile.open("output.txt");

	// display table based on order
	vector<string>::iterator vIter = order.begin();
	set<string>::iterator tIter;
	list<string> l;
	list<string>::iterator lIt;

	// iterate through non-Terminals in order
	for (vIter; vIter != order.end(); vIter)
	{
		tIter = terminals.begin();

		// iterator through terminals in order
		for (tIter; tIter != terminals.end(); tIter)
		{
			// display non-Terminal and token
			cout << *vIter << " " << *tIter << " ";
			myfile << *vIter << " " << *tIter << " ";
			l = getRule(*vIter, *tIter);
			lIt = l.begin();

			// display rule to assciate non-Terminal to token
			for (lIt; lIt != l.end(); ++lIt)
			{
				cout << *lIt << " ";
				myfile << *lIt << " ";
			}

			cout << endl;
			myfile << endl;

			++tIter;
		}

		++vIter;
	}

	myfile.close();
}