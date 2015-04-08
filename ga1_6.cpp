//***********************************************************************************************
// Team 7       :   Adam Guy, Nabir Dinani, Jonathan Kocmoud, Nicholas Warner
// Date			:	11 March 2015
// Subject		:	CSCE 315-504
// Assignment	:	Project 3
// Updated		:	5 April 2015
//
// File			:	ga.cpp (Genetic Algorithm . cpp)
//
// Description	:	A Genetic Algorithm for finding a circuit that completes
//					a solution for two and three input designs
//
//
// Version		1.0	Creates a population of random circuits of a specified size.
//
//				1.1 Added circuit evaluation. Modifies gate and circuit output
//					after a gate is added.
//
//				1.2	Added evaluation functionality to step back through the entire
//					circuit and modify every gate, in praparation for ciruit breeding.
//
//				1.3 Added fitness function using the formula provided in the
//					instructions.
//
//				1.4	Added breeder function to "splice" children from parents and cut
//					unfit circuits from the breeding pool.
//
//				1.5 -Redid Fitness Function to lower penalty for missing bits and having not gates, but
//					a slightly higher penalty for number of gates.
//					-Corrected gate counting for children, so the fitness function would have more
//					accurate data.
//					-Outputs circuit number on screen during initialization. This is purely so I know how
//					long I have until the breeding starts, where the fitness will be outputed to tell me
//					algorithm progress
//
//				1.6	-Modified Breed function to take the top 2/3 of population, plus clones of the top 1/3
//					of the population. This is to keep the population of circuits relatively stable.
//					-Added functionality to kill circuits larger than 35 gates
//					-Fixed logic error where when breeding, more than two not gates could be had. If an
//					extra not gate is added past that, evalCircuit will remove it.
//					-Every gate output in a circuit will be tested in evalCircuit for a correct answer.
//					If found, the circuit will be cut of every gate after the right answer and returned
//					to the pool for fitness to find when it iterates through again
//
// Things to do :	-Add mutations (throw a gate at the end - 1 in 1000?)
//					-Check to make sure gate numbers are revaluated for children (in evalCircuit)
//					-Make fitness cut off dynamic?
//					-Currently the population baloons out. Needs to be more horizontal growth
//					
// ALWAYS 			- have to debug. The nature of this algorithm, being so long
//					running, requires constant testing for errors. They take a while
//					to uncover.
//					- use good coding style and proven practices. An obscure bug can
//					take a long time to find, and an even longer time to debug.
//					- Aim for correctness before efficiency to prevent bugs.
//					
//***********************************************************************************************

// NOTES:	Not sure, but in breeder, during the actual cutting and swapping, it may attach a gate
//			to a line that is actually greater than a gate. It shouldn't, I don't think, because
//			they're cutting at the same line. So every gate after should connect to some point
//			at the cut or before...but if evalCircuit has troubles, this is probably it.
//			UPDATE (30 Marfch 2015):
//			No problems encountered with this for up to a million circuits. Logic appears sound

//			MAIN AT END OF SOURCE

#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <fstream>
#include <chrono>
#include <bitset>

#include "FLTK_lGraph.h"

using namespace std;
using namespace std::chrono;

/********************************************************************************
								GLOBALS
********************************************************************************/

long double averageFitness1;
long double averageFitness2;
long double averageFitness3;
double averageMissingBits1;
double averageMissingBits2;
double averageMissingBits3;
ofstream ofs;

/********************************************************************************
							DATA STRUCTURES
********************************************************************************/

struct Gate
{
	int outputLine;
	string gate;		// NONE, NOT, AND, OR
	int x;				// first input for not, and, or and none
	int y;				// second input for and, or | -1 for NONE or NOT gate
	bitset<8> A;		// first input for not, and, or and none
	bitset<8> B;
	bitset<8> gateOut;
};

struct Circuit
{
	int id;
	int nots;
	int aNo;
	vector<Gate> gates;
	bitset<8> x;
	bitset<8> y;
	bitset<8> z;
	bitset<8> out1;
	double fitness1;
	double fitness2;
	double fitness3;
};

/********************************************************************************
						PRINTING FUNCTIONALITIES
********************************************************************************/

//function to print the circuit
void printCircuit( Circuit c )
{
	cout << "CIRCUIT ID: " << c.id << "\n";
	ofs << "CIRCUIT ID: " << c.id << "\n";
	for ( int i = 0; i < c.gates.size(); ++i )
	{
		if( c.gates[ i ].gate == "NONE" ) //print the none gate
		{
			cout << c.gates[ i ].outputLine << " " << c.gates[ i ].gate << " "
				 << c.gates[ i ].x << "\t" << c.gates[ i ].gateOut << "\n";
			ofs << c.gates[ i ].outputLine << " " << c.gates[ i ].gate << " "
				<< c.gates[ i ].x << "\t" << c.gates[ i ].gateOut << "\n";
		}
		else if ( c.gates[ i ].gate == "NOT" ) //print the not gate
		{
			cout << c.gates[ i ].outputLine << " " << c.gates[ i ].gate << "  "
				 << c.gates[ i ].x << "\t" << c.gates[ i ].gateOut << "\n";
			ofs << c.gates[ i ].outputLine << " " << c.gates[ i ].gate << "  "
				<< c.gates[ i ].x << "\t" << c.gates[ i ].gateOut << "\n";
		}
		else if ( c.gates[ i ].gate == "AND" ) //print the and gate
		{
			cout << c.gates[ i ].outputLine << " " << c.gates[ i ].gate << "  "
				 << c.gates[ i ].x << " " << c.gates[ i ].y << "\t" << c.gates[ i ].gateOut << "\n";
			ofs << c.gates[ i ].outputLine << " " << c.gates[ i ].gate << "  "
				<< c.gates[ i ].x << " " << c.gates[ i ].y << "\t" << c.gates[ i ].gateOut << "\n";
		}
		else if ( c.gates[ i ].gate == "OR" ) //print the or gate
		{
			cout << c.gates[ i ].outputLine << " " << c.gates[ i ].gate << "   "
				 << c.gates[ i ].x << " " << c.gates[ i ].y << "\t" << c.gates[ i ].gateOut << "\n";
			ofs << c.gates[ i ].outputLine << " " << c.gates[ i ].gate << "   "
				<< c.gates[ i ].x << " " << c.gates[ i ].y << "\t" << c.gates[ i ].gateOut << "\n";
		}
		else
		{
			cout << "Error in circuit read\n";
			ofs << "Error in circuit read\n";
			break;
		}	
	}
	cout << "Circuit Output\nOut 1: " << c.out1 << "\n";
	ofs << "Circuit Output\nOut 1: " << c.out1 << "\n";
	cout << "\n";
	ofs << "\n";
}

//function to print the generation 
void printGeneration( const vector<Circuit> &g )
{
	for ( int i = 0; i < g.size(); ++i )
	{
		printCircuit( g[ i ] );
	}
}

/********************************************************************************
						GENETIC FUNCTIONALITIES
********************************************************************************/

//function to evaluate the gate
void evalGate( Gate &g, int operation )
{
	switch ( operation )
	{
		case 0:		// AND
			g.gateOut = g.A & g.B;
			break;
		case 1:		// OR
			g.gateOut = g.A | g.B;
			break;
		case 2:		// NOT
			g.gateOut = ~(g.A);
			break;
		case 3:		// NONE
			break;
		default:
			cout << "GATE EVALUATION ERROR\n";
			break;
	}
}

//funciton to evaluate the circuit
void evalCircuit( Circuit &c, const bitset<8> &answer1, const bitset<8> &answer2, const bitset<8> &answer3,
					bool flag1, bool flag2, bool flag3, bool type, bool neg )
{
	// at every index, set ouput of that line
	// 		lines will never be without updated parents
	//		as every gate is connected to lines with
	//		a lower id than it, which will have already
	//		been updates
	c.nots = 0;
	c.aNo = 0;
	int op = -1;
	int nots = 0;
	bool reFlag = false;
	int size = c.gates.size();
	for ( int i = 0; i < size; ++i )
	{
		if ( c.gates[ i ].gate == "AND" )
		{
			op = 0;
			++(c.aNo);
		}
		else if ( c.gates[ i ].gate == "OR" )
		{
			op = 1;
			++(c.aNo);
		}
		else if ( c.gates[ i ].gate == "NOT" )
		{
			op = 2;
			++(c.nots);
			++nots;
		}
		else if ( c.gates[ i ].gate == "NONE" )
		{
			op = 3;
		}
		if ( nots > 2 )
		{
			// take out the not gate
			c.gates.erase( c.gates.begin() + i );
			--size;
		}
		else
		{
			evalGate( c.gates[ i ], op );
			c.out1 = c.gates.back().gateOut;
			// check to see if an answer exists in a lower gate
			// if so, cut all the higher gates out and return
			// the fitness function will pick up the correct
			// answer when it cycles through
			if ( c.out1 == answer1 && !flag1 )
			{
				c.gates.erase( c.gates.end() + i + 1, c.gates.end() );
				return;
			}
			else if ( c.out1 == answer2 && type && !flag2 )
			{
				c.gates.erase( c.gates.end() + i + 1, c.gates.end() );
				return;
			}
			else if ( c.out1 == answer3 && neg && !flag3 )
			{
				c.gates.erase( c.gates.end() + i + 1, c.gates.end() );
				return;
			}
		}
		
	}
	if ( reFlag )
	{
		evalCircuit( c, answer1, answer2, answer3, flag1, flag2, flag3, type, neg );
		return;
	}
}

//fitness function to see which circuit to keep and which one to cut off
Circuit fitness( vector<Circuit> &circuits, const bitset<8> &answer1, const bitset<8> &answer2,
				const bitset<8> &answer3, bool &flag1, bool &flag2, bool &flag3 )
{
	averageMissingBits1 = 0;
	flag1 = false;
	flag2 = false;
	flag3 = false;
	Circuit g;
	int numMissingValues1 = 0;
	int numMissingValues2 = 0;
	int numMissingValues3 = 0;
	int size = circuits.size();

	for ( int i = 0; i < size; ++i )
	{
		numMissingValues1 = 0;
		numMissingValues2 = 0;
		numMissingValues3 = 0;
		if ( circuits[ i ].out1 == answer1 )
		{
			flag1 = true;
			g = circuits[ i ];
		}
		else if ( circuits[ i ].out1 == answer2 )
		{
			flag2 = true;
			g = circuits[ i ];
		}
		else if ( circuits[ i ].out1 == answer3 )
		{
			flag3 = true;
			g = circuits[ i ];
		}

		for ( int b = 0; b < 8; ++b )	// find how many bits are missing from the answer
		{
			if ( circuits[ i ].out1[ b ] != answer1[ b ] )
			{
				++numMissingValues1;
			}
			else if ( circuits[ i ].out1[ b ] != answer2[ b ] )
			{
				++numMissingValues2;
			}
			else if ( circuits[ i ].out1[ b ] != answer3[ b ] )
			{
				++numMissingValues3;
			}
		}
		averageMissingBits1 += numMissingValues1;
		averageMissingBits2 += numMissingValues2;
		averageMissingBits3 += numMissingValues3;

		circuits[ i ].fitness1 = 1000000 * numMissingValues1
								+ 10000 * circuits[ i ].nots
								+ 10 * circuits[ i ].aNo;
		averageFitness1 += circuits[ i ].fitness1;
		circuits[ i ].fitness2 = 1000000 * numMissingValues2
								+ 10000 * circuits[ i ].nots
								+ 10 * circuits[ i ].aNo;
		averageFitness2 += circuits[ i ].fitness2;
		circuits[ i ].fitness3 = 1000000 * numMissingValues3
								+ 100000 * circuits[ i ].nots
								+ 10 * circuits[ i ].aNo;
		averageFitness3 += circuits[ i ].fitness3;
	}

	averageMissingBits1 = averageMissingBits1 / size;
	averageMissingBits2 = averageMissingBits2 / size;
	averageMissingBits3 = averageMissingBits3 / size;
	averageFitness1 = averageFitness1 / size;
	averageFitness2 = averageFitness2 / size;
	averageFitness3 = averageFitness3 / size;
	numMissingValues1 = 0;
	numMissingValues2 = 0;
	numMissingValues3 = 0;
	return g;
}

void genCut( vector<Circuit> &circuits, vector<Circuit> &tempGen, bool flag1, bool flag2, bool flag3, bool type, bool neg )
{
	long double cutoff = -1;
	// go through entire population pool and pull out all circuits that meet the fitness cut
	if ( !flag1 ) 	// first answer not found yet
	{
		cutoff = averageFitness1 / 3.0;					// top 2/3 make it + the top 1/3 cloned
		int size = circuits.size();
		for ( int i = 0; i < size; ++i )
		{
			if ( circuits[ i ].gates.size() > 35 )		// kill circuit if larger than 35 gates
			{
				circuits.erase( circuits.begin() + i );
				--size;
			}
			else
			{
				Circuit x = circuits[ i ];
				if ( x.fitness1 > cutoff )				// save top 2/3
				{
					tempGen.push_back( x );
				}
				if ( x.fitness1 > ( cutoff * 2)  )		// clone top third
				{
					tempGen.push_back( x );
				}
			}
		}
	}
	else if ( flag1 && type && !flag2 )		//second answer not found yet
	{
		cutoff = averageFitness2 / 3.0;
		int size = circuits.size();
		for ( int i = 0; i < size; ++i )
		{
			if ( circuits[ i ].gates.size() > 35 )
			{
				circuits.erase( circuits.begin() + i );
				--size;
			}
			else
			{
				Circuit x = circuits[ i ];
				if ( x.fitness2 > cutoff )
				{
					tempGen.push_back( x );
				}
				if ( x.fitness2 > ( cutoff * 2)  )
				{
					tempGen.push_back( x );
				}
			}
		}
	}
	else if ( flag1 && flag2 && neg && !flag3 )
	{
		cutoff = averageFitness3 / 3.0;
		int size = circuits.size();
		for ( int i = 0; i < size; ++i )
		{
			if ( circuits[ i ].gates.size() > 35 )
			{
				circuits.erase( circuits.begin() + i );
				--size;
			}
			else
			{
				Circuit x = circuits[ i ];
				if ( x.fitness3 > cutoff )
				{
					tempGen.push_back( x );
				}
				if ( x.fitness3 > ( cutoff * 2)  )
				{
					tempGen.push_back( x );
				}
			}
		}
	}
}

//function to breed, make new childrens by cross breading two parents 
void breed( vector<Circuit> &circuits, const bitset<8> &answer1, const bitset<8> &answer2,
				const bitset<8> &answer3, bool flag1, bool flag2, bool flag3, bool type, bool neg )
{
	srand( time( NULL ) );
	int cut = -1;
	long double top = -1;
	vector<Circuit> tempGen;
	// decide a cutoff for population fitness acceptable - above two thirds of average?
	genCut( circuits, tempGen, flag1, flag2, flag3, type, neg );
	

	// then randomly select two at a time to breed together, discarding the parents
	// save these children back into "circuits"
	Circuit mother;
	Circuit father;
	Circuit son;
	Circuit daughter;
	int mom;
	int dad;
	vector<Gate> firstGateSet;
	vector<Gate> secondGateSet;
	vector<Gate> thirdGateSet;
	vector<Gate> fourthGateSet;
	circuits.clear();
	// if even number of parents, just pick two at a time until depleted
	if ( tempGen.size() % 2 == 0 )	// even number of parents
	{
		while ( tempGen.size() != 0 )
		{
			mom = rand() % tempGen.size();
			mother = tempGen[ mom ];
			tempGen.erase( tempGen.begin() + mom );
			dad = rand() % tempGen.size();
			father = tempGen[ dad ];
			tempGen.erase( tempGen.begin() + dad );

			if ( mother.gates.size() < father.gates.size() )	// choose random cut line from smallest circuit
			{
				cut = rand() % mother.gates.size();
			}
			else
			{
				cut = rand() % father.gates.size();
			}

			// perform the cut
			firstGateSet.assign( mother.gates.begin() + cut, mother.gates.end() );
			secondGateSet.assign( mother.gates.begin(), mother.gates.end() - cut - 1 );
			thirdGateSet.assign( father.gates.begin() + cut, father.gates.end() );
			fourthGateSet.assign( father.gates.begin(), father.gates.end() - cut - 1 );

			son.gates= secondGateSet;
			for ( int a = 0; a < thirdGateSet.size(); ++a )
			{
				son.gates.push_back( thirdGateSet[ a ] );
			}
			daughter.gates = fourthGateSet;
			for ( int b = 0; b < firstGateSet.size(); ++b )
			{
				daughter.gates.push_back( firstGateSet[ b ] );
			}

			evalCircuit( son, answer1, answer2, answer3, flag1, flag2, flag3, type, neg );
			evalCircuit( daughter, answer1, answer2, answer3, flag1, flag2, flag3, type, neg );
			circuits.push_back( son );
			circuits.push_back( daughter );
			firstGateSet.clear();
			secondGateSet.clear();
			thirdGateSet.clear();
			fourthGateSet.clear();
		}
	}
	else			// if odd number of parents, pick two at a time until the final three - cross all three of those
	{
		Circuit mistress;
		Circuit stepChild;
		int miss;
		vector<Gate> fifthGateSet;
		vector<Gate> sixthGateSet;
		while ( tempGen.size() != 0 )
		{
			mom = rand() % tempGen.size();
			mother = tempGen[ mom ];
			tempGen.erase( tempGen.begin() + mom );
			dad = rand() % tempGen.size();
			father = tempGen[ dad ];
			tempGen.erase( tempGen.begin() + dad );

			if ( mother.gates.size() < father.gates.size() )	// choose random cut line from smallest circuit
			{
				cut = rand() % mother.gates.size();
			}
			else
			{
				cut = rand() % father.gates.size();
			}

			if ( tempGen.size() == 1 )	// the last three parents - two form two children (two mothers, one father)
			{
				miss = rand() % tempGen.size();
				mistress = tempGen[ miss ];
				tempGen.erase( tempGen.begin() + miss );
				if ( mistress.gates.size() < cut )
				{
					cut = rand() % mistress.gates.size();
				}

				firstGateSet.assign( mother.gates.begin() + cut, mother.gates.end() );
				secondGateSet.assign( mother.gates.begin(), mother.gates.end() - cut - 1 );
				thirdGateSet.assign( father.gates.begin() + cut, father.gates.end() );
				fourthGateSet.assign( father.gates.begin(), father.gates.end() - cut - 1 );
				fifthGateSet.assign( mistress.gates.begin() + cut, mistress.gates.end() );
				sixthGateSet.assign(  mistress.gates.begin(), mistress.gates.end() - cut - 1 );

				son.gates= secondGateSet;
				for ( int a = 0; a < thirdGateSet.size(); ++a )
				{
					son.gates.push_back( thirdGateSet[ a ] );
				}
				daughter.gates = fourthGateSet;
				for ( int b = 0; b < thirdGateSet.size(); ++b )
				{
					daughter.gates.push_back( firstGateSet[ b ] );
				}
				stepChild.gates = sixthGateSet;
				for ( int c = 0; c < fifthGateSet.size(); ++c )
				{
					stepChild.gates.push_back( fifthGateSet[ c ] );
				}


				evalCircuit( son, answer1, answer2, answer3, flag1, flag2, flag3, type, neg );
				evalCircuit( daughter, answer1, answer2, answer3, flag1, flag2, flag3, type, neg );
				evalCircuit( stepChild, answer1, answer2, answer3, flag1, flag2, flag3, type, neg );
				circuits.push_back( son );
				circuits.push_back( daughter );
				circuits.push_back( stepChild );
				firstGateSet.clear();
				secondGateSet.clear();
				thirdGateSet.clear();
				fourthGateSet.clear();
				fifthGateSet.clear();
				sixthGateSet.clear();
			}
			else	// not the last three parents
			{
				firstGateSet.assign( mother.gates.begin() + cut, mother.gates.end() );
				secondGateSet.assign( mother.gates.begin(), mother.gates.end() - cut - 1 );
				thirdGateSet.assign( father.gates.begin() + cut, father.gates.end() );
				fourthGateSet.assign( father.gates.begin(), father.gates.end() - cut - 1 );
				son.gates= secondGateSet;
				for ( int a = 0; a < thirdGateSet.size(); ++a )
				{
					son.gates.push_back( thirdGateSet[ a ] );
				}
				daughter.gates = fourthGateSet;
				for ( int b = 0; b < firstGateSet.size(); ++b )
				{
					daughter.gates.push_back( firstGateSet[ b ] );
				}
				evalCircuit( son, answer1, answer2, answer3, flag1, flag2, flag3, type, neg );
				evalCircuit( daughter, answer1, answer2, answer3, flag1, flag2, flag3, type, neg );
				circuits.push_back( son );
				circuits.push_back( daughter );
				firstGateSet.clear();
				secondGateSet.clear();
				thirdGateSet.clear();
				fourthGateSet.clear();
			}
		}
	}
	ofs << "New Population: " << circuits.size() << "\n";
	// write to graphical output
	long num = circuits.size();
	write(pipefd, &num, sizeof(num)); 
}

/********************************************************************************
					CIRCUIT POPULATION INITIALIZATIONS
********************************************************************************/

void initGateBox( vector<Gate> &g, int size )
{
	srand( time( NULL ) );
	int type = -1;
	int nots = 0;
	for ( int i = 0; i < size; ++i )
	{
		Gate X;
		X.outputLine = -1;
		X.x = -1;
		X.y = -1;
		type = rand() % 3 + 1;	// 1, 2 or 3
		switch ( type )
		{
			case 1:
				X.gate = "NOT";
				//++nots;
				break;
			case 2:
				X.gate = "AND";
				break;
			case 3:
				X.gate = "OR";
				break;
			default:
				break;
		}
		g.push_back( X );
	}
}

void initCircuitBox( vector<Circuit> &cBox, vector<Gate> &gBox, bool type, bitset<8> A, bitset<8> B, bitset<8> C )
{
	Gate temp;
	Circuit init;
	init.nots = 0;
	init.aNo = 0;
	init.fitness1 = 0;
	init.fitness2 = 0;
	init.x = A;
	init.y = B;
	init.z = C;
	int gates = -1;

	if ( type ) // two outputs
	{
		gates = 3;
	}
	else
	{
		gates = 2;
	}

	for ( int a = 0; a < gates; ++a )		// first "Gates" are NONE
	{
		cout << a << "\t";
		temp.outputLine = a;
		temp.gate = "NONE";
		temp.x = a;
		temp.y = -1;

		if ( a == 0 )
		{
			temp.gateOut = A;
			temp.A = A;
		}
		else if ( a == 1 )
		{
			temp.gateOut = B;
			temp.A = B;
		}
		else if ( a == 2 )
		{
			temp.gateOut = C;
			temp.A = C;
		}

		init.gates.push_back( temp );
	}
	init.id = 0;
	cBox.push_back( init );

	srand( time( NULL ) );
	int cDex = -1;									// circuit box choose index
	int gDex = -1;									// gate box choose index
	int line1 = -1;									// first line to connect gate to
	int line2 = -1;									// second line to connect gate to
	int size = gBox.size();
	for ( int i = 1; i < size; ++i )				// until the gate box is empty
	{
		// cout << i << "\n";
		cout << i << "\t";
		cDex = rand() % cBox.size();				// will always adjust to choose from full circuit box range as it expands
		gDex = rand() % gBox.size();				// will always adjust to choose from full gate box range as it shrinks
		init = cBox[ cDex ];						// new circuit
		init.id = i;								// circuit number
		line1 = rand() % init.gates.size();			// a random line at the circuit at cDex in cbox
		line2 = rand() % init.gates.size();			// a random line at the circuit at cDex in cbox

		while ( line1 == line2 )					// make sure the lines don't equal each other
		{
			line2 = rand() % init.gates.size() + 1;	// a random line at the circuit at cDex in cbox
		}

		temp.outputLine = init.gates.size();		// line number
		temp.gate = gBox[ gDex ].gate;				// get random gate

		int move = 0;
		if ( temp.gate == "NOT" && init.nots > 2 )
		{
			while ( temp.gate == "NOT" && move < gBox.size() )// prevent a third NOT from being added
			{
				temp.gate = gBox[ move ].gate;
				++move;
				if ( move > gBox.size() )			// prevent a rare memcpy segfault
					if ( gDex % 2 == 0 )
						temp.gate = "AND";
					else
						temp.gate == "OR";
			}
			gBox.erase( gBox.begin() + move );		// delete gate used
		}
		else gBox.erase( gBox.begin() + gDex );		// delete gate used

		// set gate in circuit
		int op = 3; // NONE
		if ( temp.gate == "NOT" )
		{
			temp.x = line1;
			temp.y = -1;
			init.nots += 1;

			temp.A = init.gates[ line1 ].gateOut;
			op = 2; // NOT
		}
		else if ( temp.gate == "AND" )
		{
			temp.x = line1;
			temp.y = line2;
			init.aNo += 1;
			temp.A = init.gates[ line1 ].gateOut;
			temp.B = init.gates[ line2 ].gateOut;
			op = 0;	// AND
		}
		else if ( temp.gate == "OR" )
		{
			temp.x = line1;
			temp.y = line2;
			init.aNo += 1;
			temp.A = init.gates[ line1 ].gateOut;
			temp.B = init.gates[ line2 ].gateOut;
			op = 1;	// OR
		}

		evalGate( temp, op );						// set the output of the newest line (gate)
		init.gates.push_back( temp );				// save to the chosen circuit
		init.out1 = temp.gateOut;					// set the output of the circuit to the newest value
		init.fitness1 = 0.0;
		init.fitness2 = 0.0;
		init.fitness3 = 0.0;
		cBox.push_back( init );						// save new circuit
	}
	cout << "\n\n";
}

/********************************************************************************
							MAIN AND MAIN-HELPERS
********************************************************************************/

//funtion to ask the user for input instead of finding the built in circuit 
void userInput( bitset<8> &A, bitset<8> &B, bitset<8> &C, bitset<8> &answer1,
				bitset<8> &answer2, bitset<8> &answer3, bool &type, bool &neg )
{
	int numOut = -1;
	string input1;
	string input2;
	string input3;
	string output1;
	string output2;
	string output3;
	string def = "";

	cout << "Run Defaults? (y or n): ";
	cin >> def;
	
	if ( def == "n" )
	{	cout << "Number of outputs (1, 2 or 3): ";
		cin >> numOut;
		if ( numOut == 1 )
		{
			cout << "Enter the 8 bits of first input: ";
			cin >> input1;
			cout << "Enter the 8 bits of second input: ";
			cin >> input2;
			cout << "Enter the 8 bits of the output: ";
			cin >> output1;
			bitset<8> A (string(input1)); 
			bitset<8> B (string(input2));
			bitset<8> answer1 (string(output1));
		}
		else if ( numOut == 2 )
		{
			type = true;
			cout << "Enter the 8 bits of first input: ";
			cin >> input1;
			cout << "Enter the 8 bits of second input: ";
			cin >> input2;
			cout << "Enter the 8 bits of third input: ";
			cin >> input3;
			cout << "Enter the 8 bits of the first output: ";
			cin >> output1;
			cout << "Enter the 8 bits of the second output: ";
			cin >> output2;
			bitset<8> A (string(input1)); 
			bitset<8> B (string(input2));
			bitset<8> C (string(input3));
			bitset<8> answer1 (string(output1));
			bitset<8> answer2 (string(output2));
		}
		else if ( numOut == 3 )
		{
			neg = true;
			cout << "Enter the 8 bits of first input: ";
			cin >> input1;
			cout << "Enter the 8 bits of second input: ";
			cin >> input2;
			cout << "Enter the 8 bits of third input: ";
			cin >> input3;
			cout << "Enter the 8 bits of the first output: ";
			cin >> output1;
			cout << "Enter the 8 bits of the second output: ";
			cin >> output2;
			cout << "Enter the 8 bits of the third output: ";
			cin >> output3;
			bitset<8> A (string(input1)); 
			bitset<8> B (string(input2));
			bitset<8> C (string(input3));
			bitset<8> answer1 (string(output1));
			bitset<8> answer2 (string(output2));
			bitset<8> answer3 (string(output3));
		}
		else
		{
			cout << "INVALID INPUT\n";
		}
	}
	else	// run defaults
	{
		int test = -1;
		cout << "Test (1, 2 or 3): ";
		cin >> test;
		if ( test == 1 )		// run the xor test
		{
			bitset<8> A (string("00000011"));
			bitset<8> B (string("00000101"));
			bitset<8> answer1 (string("00000110"));
		}
		else if ( test == 2 )	// run the adder test
		{
			type = true;
			bitset<8> A (string("00001111"));
			bitset<8> B (string("00110011"));
			bitset<8> C (string("01010101"));
			bitset<8> answer1 (string("00010111"));
			bitset<8> answer2 (string("01101001"));
		}
		else if ( test == 3 )	// run the negation test
		{
			neg = true;
			bitset<8> A (string("00001111"));
			bitset<8> B (string("00110011"));
			bitset<8> C (string("01010101"));
			bitset<8> answer1 (string("11110000"));
			bitset<8> answer2 (string("11001100"));
			bitset<8> answer3 (string("10101010"));
		}
		else
		{
			cout << "INVALID INPUT\n";
		}
	}
}

int main ( int argc, char const *argv[] )
{
	int pop = 5000;
	long num = 0;
	vector<Gate> gBox;
	vector<Circuit> cBox;
	bitset<8> A (string("00001111")); 
	bitset<8> B (string("00110011")); 
	bitset<8> C (string("01010101"));
	bitset<8> answer1 (string("00010111"));
	bitset<8> answer2 (string("01101001"));
	bitset<8> answer3 (string("00000000"));
	
	Circuit report;
	Circuit correct1;
	Circuit correct2;
	Circuit correct3;

	bool flag1 = false;	// first output found
	bool flag2 = false;	// second output found
	bool flag3 = false; // third output found
	bool type = false;	// if true, two outputs
	bool neg = false;	// if true, three output

	// create pipe to send values to graphical output process
	int pfd[2];
	if (pipe(pfd) == -1){
		cout << "Failure creating pipe" <<endl;
		exit(EXIT_FAILURE);
	}
	
	// fork graphical output process
	pid_t childpid = fork();

	if (childpid < 0){
		cout << "FORK() FAILED";
		exit(EXIT_FAILURE);
	}
	else if (childpid == 0) { // child process
		close(pfd[1]); // close unused write pipe (child only reads)
		pipefd = pfd[0];
		graph();
		close(pfd[0]);
		exit(EXIT_SUCCESS);
	}
	// parent process
	close(pfd[0]); // close unused read end of pipe
	pipefd = pfd[1];

	ofs.open( "output.txt" );
	ofs << "Population: " << pop << "\n\n";
	
	userInput( A, B, C, answer1, answer2, answer3, type, neg );
	
	initGateBox( gBox, pop );
	initCircuitBox( cBox, gBox, type, A, B, C );
	
	int swch = 0;
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	//for ( int i = 0; i < 30; ++i )
	for ( ; ; )
	{
		if ( cBox.size() == 0 )
		{
			break;
		}
		report = fitness( cBox, answer1, answer2, answer3, flag1, flag2, flag3 );
		if ( flag1 )							// if we find the first answer
		{
			correct1 = report;
			printCircuit( correct1 );
			if ( !type || !neg )				// if there's one output only
			{
				break;							// single output answer found
			}
			else
			{
				cout << "First output found. Switching Context...\n";
				ofs << "\nFirst output found. Switching Context...\n\n";
			}
		}
		else if ( flag2 && ( type || neg ) )	// if we find the second answer
		{
			correct2 = report;
			printCircuit( correct2 );
			if ( !neg && flag1 ) 				// if only two outputs and first found
			{
				break;
			}
			else
			{
				cout << "Second output found. Switching Context...\n";
				ofs << "\nSecond output found. Switching Context...\n\n";
			}
		}
		else if ( flag3 && neg )				// if we find the third answer
		{
			correct3 = report;
			printCircuit( correct3 );
			if ( flag1 && flag2 )				// if the other two answers have been found
			{
				break;
			}
			else
			{
				cout << "Third output found. Switching Context...\n";
				ofs << "\nThird output found. Switching Context...\n\n";
			}
		}
		breed( cBox, answer1, answer2, answer3, flag1, flag2, flag3, type, neg );

		if ( !flag1 )						// single output
		{
			cout << "1Fitness Report: " << averageFitness1 << "\tBit Report: " << averageMissingBits1 << "\n";
			ofs << "1Fitness Report: " << averageFitness1 << "\tBit Report: " << averageMissingBits1 << "\n";
			// write to graphical output
			num = averageFitness1;
			write(pipefd, &num, sizeof(num)); 
			num = averageMissingBits1;
			write(pipefd, &num, sizeof(num));
		}
		else if ( type && !neg && flag1 )	// two output
		{
			cout << "3Fitness Report: " << averageFitness2 << "\tBit Report: " << averageMissingBits2 << "\n";
			ofs << "3Fitness Report: " << averageFitness2 << "\tBit Report: " << averageMissingBits2 << "\n";
			// write to graphical output
			num = averageFitness2;
			write(pipefd, &num, sizeof(num)); 
			num = averageMissingBits2;
			write(pipefd, &num, sizeof(num));
		}
		else if ( neg && flag1 && flag2 )	// three output
		{
			cout << "3Fitness Report: " << averageFitness3 << "\tBit Report: " << averageMissingBits3 << "\n";
			ofs << "3Fitness Report: " << averageFitness3 << "\tBit Report: " << averageMissingBits3 << "\n";
			// write to graphical output
			num = averageFitness3;
			write(pipefd, &num, sizeof(num)); 
			num = averageMissingBits3;
			write(pipefd, &num, sizeof(num)); 
		}
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
		ofs << "Run time: " << duration << "\n\n";
		ofs.close();
		ofs.open( "output.txt", ofstream::app );
	}
	// cout << "wha...\n";
	printCircuit( correct1 );
	ofs.close();

	close(pfd[1]); // close write end of the pipe
	wait(NULL);  // wait for child (child ends when Xming window is closed
	return 0;
}
