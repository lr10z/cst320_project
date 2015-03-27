/***********************************************************
* Author:					Leander Rodriguez
* Date Created:				05/29/14
* Last Modification Date:	06/05/14
* Lab Number:				CST 320 Final Project
* Filename:					main.cpp
*
************************************************************/

#include <fstream>

#include "generator.h"

using  namespace  std;

int main()
{

	string filename;

	cout << "Enter text file name: ";
	cin >> filename;
	cout << endl << endl;

	ifstream in(filename);


	if (!in)
	{
		cout << "*** Error opening file '"
			<< filename
			<< "' ***"
			<< endl;

		exit(EXIT_FAILURE);
	}

	std::string languageIn((std::istreambuf_iterator<char>(in)),
		(std::istreambuf_iterator<char>()));

	Generator *g = new Generator(languageIn);

	in.close();

	return  EXIT_SUCCESS;
}