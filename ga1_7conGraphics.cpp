//***********************************************************************************************
// Name			:	Nicholas Warner
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
//				(CURRENT)
//				1.7	-Removed functionality to kill circuits larger than 35 gates - fitness handles this
//					pretty well already.
//					-Added mutations to add a random gate (AND or OR) at the end of 20% of the population
//					-Infinite loop finalized to not break until all solutions are found, and update the
//					user on the appropriate fitness and bit state
//					-genCut function added to remove population trimming from breed - also refined to
//					clone as well as kill
//
// Things to do :	-Add graphics
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
//			UPDATE (30 March 2015):
//			No problems encountered with this for up to a million circuits. Logic appears sound

//			MAIN AT END OF SOURCE

#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <fstream>
#include <chrono>
#include <bitset>
#include <unistd.h>

#include "FLTK_lGraph.h"

using namespace std;
using namespace std::chrono;

/********************************************************************************
								GLOBALS
********************************************************************************/

long double AVERAGEFITNESS1;
long double AVERAGEFITNESS2;
long double AVERAGEFITNESS3;
double AVERAGEMISSINGBITS1;
double AVERAGEMISSINGBITS2;
double AVERAGEMISSINGBITS3;
ofstream OFS;

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
}cfound1, cfound2, cfound3;

/********************************************************************************
						PRINTING FUNCTIONALITIES
********************************************************************************/

//function that prints the circuit
void printCircuit( Circuit c )
{
	cout << "CIRCUIT ID: " << c.id << "\n";
	OFS << "CIRCUIT ID: " << c.id << "\n";
	for ( int i = 0; i < c.gates.size(); ++i )
	{
		if( c.gates[ i ].gate == "NONE" ) //prints the none gate
		{
			cout << c.gates[ i ].outputLine << " " << c.gates[ i ].gate << " "
				 << c.gates[ i ].x << "\t" << c.gates[ i ].gateOut << "\n";
			OFS << c.gates[ i ].outputLine << " " << c.gates[ i ].gate << " "
				<< c.gates[ i ].x << "\t" << c.gates[ i ].gateOut << "\n";
		}
		else if ( c.gates[ i ].gate == "NOT" ) //prints the not gate
		{
			cout << c.gates[ i ].outputLine << " " << c.gates[ i ].gate << "  "
				 << c.gates[ i ].x << "\t" << c.gates[ i ].gateOut << "\n";
			OFS << c.gates[ i ].outputLine << " " << c.gates[ i ].gate << "  "
				<< c.gates[ i ].x << "\t" << c.gates[ i ].gateOut << "\n";
		}
		else if ( c.gates[ i ].gate == "AND" ) //prints the and gate
		{
			cout << c.gates[ i ].outputLine << " " << c.gates[ i ].gate << "  "
				 << c.gates[ i ].x << " " << c.gates[ i ].y << "\t" << c.gates[ i ].gateOut << "\n";
			OFS << c.gates[ i ].outputLine << " " << c.gates[ i ].gate << "  "
				<< c.gates[ i ].x << " " << c.gates[ i ].y << "\t" << c.gates[ i ].gateOut << "\n";
		}
		else if ( c.gates[ i ].gate == "OR" ) //prints the or gate
		{
			cout << c.gates[ i ].outputLine << " " << c.gates[ i ].gate << "   "
				 << c.gates[ i ].x << " " << c.gates[ i ].y << "\t" << c.gates[ i ].gateOut << "\n";
			OFS << c.gates[ i ].outputLine << " " << c.gates[ i ].gate << "   "
				<< c.gates[ i ].x << " " << c.gates[ i ].y << "\t" << c.gates[ i ].gateOut << "\n";
		}
		else
		{
			cout << "Error in circuit read\n";
			OFS << "Error in circuit read\n";
			break;
		}	
	}
	cout << "Circuit Output\nOut 1: " << c.out1 << "\n";
	OFS << "Circuit Output\nOut 1: " << c.out1 << "\n";
	cout << "\n";
	OFS << "\n";
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

//evaluate the gate function
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

//evaluate the circuit function
void evalCircuit( Circuit &c, const bitset<8> &answer1, const bitset<8> &answer2, const bitset<8> &answer3,
					bool flag1, bool flag2, bool flag3, bool type, bool neg )
{
	// at every index, set ouput of that line
	// 		lines will never be without updated parents
	//		as every gate is connected to lines with
	//		a lower id than it, which will have already
	//		been updates
	int min = 2;
	if ( neg || type )
	{
		min = 3;
	}
	c.nots = 0;
	c.aNo = 0;
	int op = -1;
	int nots = 0;
	int index = min;
	bool reFlag = false;
	int size = c.gates.size();
	for ( int i = min; i < size; ++i )
	{
		if ( c.gates[ i ].gate == "AND" )
		{
			c.gates[ i ].outputLine = index;
			op = 0;
			++(c.aNo);
		}
		else if ( c.gates[ i ].gate == "OR" )
		{
			c.gates[ i ].outputLine = index;
			op = 1;
			++(c.aNo);
		}
		else if ( c.gates[ i ].gate == "NOT" )
		{
			c.gates[ i ].outputLine = index;
			op = 2;
			++(c.nots);
			++nots;
		}
		else if ( c.gates[ i ].gate == "NONE" )
		{
			c.gates[ i ].outputLine = index;
			op = 3;
		}
		if ( nots > 2 )
		{
			// take out the not gate
			c.gates.erase( c.gates.begin() + i );
			--size;
			--index;
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
				c.gates.erase( c.gates.end() + ( i - min ) + 1, c.gates.end() );
				return;
			}
			else if ( c.out1 == answer2 && type && !flag2 )
			{
				c.gates.erase( c.gates.end() + ( i - min ) + 1, c.gates.end() );
				return;
			}
			else if ( c.out1 == answer3 && neg && !flag3 )
			{
				c.gates.erase( c.gates.end() + ( i - min ) + 1, c.gates.end() );
				return;
			}
		}
		++index;
	}
	if ( reFlag )
	{
		evalCircuit( c, answer1, answer2, answer3, flag1, flag2, flag3, type, neg );
		return;
	}
}


//fitness function for the GA
Circuit fitness( vector<Circuit> &circuits, bitset<8> answer1, bitset<8> answer2,
				bitset<8> answer3, bool &flag1, bool &flag2, bool &flag3, bool type, bool neg )
{
	AVERAGEMISSINGBITS1 = 0;
	AVERAGEMISSINGBITS2 = 0;
	AVERAGEMISSINGBITS3 = 0;
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
			cfound1 = g;
		}
		else if ( circuits[ i ].out1 == answer2 && ( type || neg ) )
		{
			flag2 = true;
			g = circuits[ i ];
			cfound2 = g;
		}
		else if ( circuits[ i ].out1 == answer3 && neg )
		{
			flag3 = true;
			g = circuits[ i ];
			cfound3 = g;
		}

		for ( int b = 0; b < 8; ++b )	// find how many bits are missing from the answer
		{
			if ( circuits[ i ].out1[ b ] != answer1[ b ] )
			{
				++numMissingValues1;
			}
			if ( circuits[ i ].out1[ b ] != answer2[ b ] )
			{
				++numMissingValues2;
			}
			if ( circuits[ i ].out1[ b ] != answer3[ b ] )
			{
				++numMissingValues3;
			}
		}
		AVERAGEMISSINGBITS1 += numMissingValues1;
		AVERAGEMISSINGBITS2 += numMissingValues2;
		AVERAGEMISSINGBITS3 += numMissingValues3;

		circuits[ i ].fitness1 = 10000 * numMissingValues1
								+ 100 * circuits[ i ].nots
								+ 100 * circuits[ i ].aNo;
		AVERAGEFITNESS1 += circuits[ i ].fitness1;
		circuits[ i ].fitness2 = 10000 * numMissingValues2
								+ 100 * circuits[ i ].nots
								+ 100 * circuits[ i ].aNo;
		AVERAGEFITNESS2 += circuits[ i ].fitness2;
		circuits[ i ].fitness3 = 10000 * numMissingValues3
								+ 100 * circuits[ i ].nots
								+ 100 * circuits[ i ].aNo;
		AVERAGEFITNESS3 += circuits[ i ].fitness3;
	}
	AVERAGEMISSINGBITS1 = AVERAGEMISSINGBITS1 / size;
	AVERAGEMISSINGBITS2 = AVERAGEMISSINGBITS2 / size;
	AVERAGEMISSINGBITS3 = AVERAGEMISSINGBITS3 / size;
	AVERAGEFITNESS1 = AVERAGEFITNESS1 / size;
	AVERAGEFITNESS2 = AVERAGEFITNESS2 / size;
	AVERAGEFITNESS3 = AVERAGEFITNESS3 / size;
	numMissingValues1 = 0;
	numMissingValues2 = 0;
	numMissingValues3 = 0;
	return g;
}

//mutation function
void mutate( vector<Circuit> &circuits )
{
	srand( time( NULL ) );
	int size = circuits.size() * 0.2; // 20% of population
	int op = -1;
	for ( int i = 0; i < size; ++i )
	{
		Gate X;
		int index = rand() % circuits.size();
		int type = rand() % 2 + 1;								// 1 or 2

		int line1 = rand() % circuits[ index ].gates.size();	// a random line at the circuit at circuits in cbox
		int line2 = rand() % circuits[ index ].gates.size();	// a random line at the circuit at circuits in cbox
		while ( line1 == line2 )								// make sure the lines don't equal each other
		{
			line2 = rand() % circuits[ index ].gates.size() + 1;// a random line at the circuit at circuits in cbox
		}

		switch ( type )
		{
			case 1:
				X.gate = "AND";
				break;
			case 2:
				X.gate = "OR";
				break;
			default:
				break;
		}

		X.x = line1;
		X.y = line2;
		circuits[ index ].aNo += 1;
		X.A = circuits[ index ].gates[ line1 ].gateOut;
		X.B = circuits[ index ].gates[ line2 ].gateOut;

		if ( X.gate == "AND" )
		{
			op = 0;
		}
		else if ( X.gate == "OR" )
		{
			op = 1;
		}	
		evalGate( X, op );						// set the output of the newest line (gate)
		circuits[ index ].gates.push_back( X );
	}
}

void genCut( vector<Circuit> &circuits, vector<Circuit> &tempGen, bool flag1, bool flag2, bool flag3, bool type, bool neg )
{
	long double cutoff = -1.0;
	long double topoff = -1.0;
	double cut = 0.95;	// 0.7
	double top = 0.4;	// 0.2
	int min = 2;
	if ( type || neg )
	{
		min = 3;
	}

	if ( !flag1 ) 	// first answer not found yet
	{
		cutoff = AVERAGEFITNESS1 * cut;		// top 95% (85%) make it
		topoff = AVERAGEFITNESS1 * top;			// top 40% (20%) cloned
	}
	else if ( flag1 && type && !flag2 )			//second answer not found yet, but first was
	{
		cutoff = AVERAGEFITNESS2 * cut;
		topoff = AVERAGEFITNESS2 * top;
	}
	else if ( flag1 && flag2 && neg && !flag3 )	// third answer not found yet, but second and first was
	{
		cutoff = AVERAGEFITNESS3 * cut;
		topoff = AVERAGEFITNESS3 * top;
	}

	// go through entire population pool and pull out all circuits that meet the fitness cut
	int size = circuits.size();
	for ( int i = 0; i < size; ++i )
	{
		if ( /*circuits[ i ].gates.size() < 35 ||*/ circuits[ i ].gates.size() > min )	// kill circuit if larger than 35 gates or if only NONES
		{
			if ( circuits[ i ].fitness1 < cutoff )		// save top 95%
			{
				tempGen.push_back( circuits[ i ] );
			}
			if ( circuits[ i ].fitness1 < topoff )		// clone top 40%
			{
				tempGen.push_back( circuits[ i ] );
			}
		}
	}
}

//breed function that creates two new children by cross breeding two parents
void breed( vector<Circuit> &circuits, const bitset<8> &answer1, const bitset<8> &answer2,
				const bitset<8> &answer3, bool flag1, bool flag2, bool flag3, bool type, bool neg )
{
	srand( time( NULL ) );
	int cut = -1;
	long double top = -1;
	int cutBase = 2;
	vector<Circuit> tempGen;
	// decide a cutoff for population fitness acceptable - above two thirds of average?
	genCut( circuits, tempGen, flag1, flag2, flag3, type, neg );
	mutate( circuits );
	if ( type || neg )
	{
		cutBase = 3;
	}
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
				cut = rand() % ( mother.gates.size() - cutBase + 2 );
			}
			else
			{
				cut = rand() % ( father.gates.size() - cutBase + 2 );
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
	else if ( tempGen.size() >= 2 )			// if odd number of parents, pick two at a time until the final three - cross all three of those
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
				cut = rand() % ( mother.gates.size() - cutBase + 2 );
			}
			else
			{
				cut = rand() % ( father.gates.size() - cutBase + 2 );
			}

			if ( tempGen.size() == 1 )	// the last three parents - two form two children (two mothers, one father)
			{
				miss = rand() % tempGen.size();
				mistress = tempGen[ miss ];
				tempGen.erase( tempGen.begin() );
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
	OFS << "New Population: " << circuits.size() << "\n";
	// write to graphical output
	long num = circuits.size();
	write(pipefd, &num, sizeof(num));
}

/********************************************************************************
					CIRCUIT POPULATION INITIALIZATIONS
********************************************************************************/

void initGateBox( vector<Gate> &g, int size, bool tell )
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
		if ( nots < 2 && tell )
		{
			type = rand() % 3 + 1;	// 1, 2 or 3
		}
		else	// prevent more than two NOTs being placed in box
		{
			type = rand() % 2 + 2;	// 2 or 3
		}
		switch ( type )
		{
			case 1:
				X.gate = "NOT";
				++nots;
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

void initCircuitBox( vector<Circuit> &cBox, vector<Gate> &gBox, bool type, bool neg, bitset<8> A, bitset<8> B, bitset<8> C )
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

	if ( type || neg ) // two outputs
	{
		gates = 3;
	}
	else
	{
		gates = 2;
	}

	for ( int a = 0; a < gates; ++a )		// first "Gates" are NONE
	{
		//cout << a << "\t";
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
}

/********************************************************************************
							MAIN AND MAIN-HELPERS
********************************************************************************/

void userInput( bitset<8> &A, bitset<8> &B, bitset<8> &C, bitset<8> &answer1,
				bitset<8> &answer2, bitset<8> &answer3, bool &type, bool &neg )
{
	int numOut = -1;
	int input1;
	int input2;
	int input3;
	int output1;
	int output2;
	int output3;
	string def = "";

	cout << "Run Defaults? (y or n): ";
	cin >> def;
	
	if ( def == "n" )
	{	
		cout << "Number of outputs (1, 2 or 3): ";
		cin >> numOut;
		if ( numOut == 1 )
		{
			cout << "Enter the first input (as an integer): ";
			cin >> input1;
			cout << "Enter the second input (as an integer): ";
			cin >> input2;
			cout << "Enter the first output (as an integer): ";
			cin >> output1;
			A = input1;
			B = input2;
			answer1 = output1;
		}
		else if ( numOut == 2 )
		{
			type = true;
			cout << "Enter the first input (as an integer): ";
			cin >> input1;
			cout << "Enter the second input (as an integer): ";
			cin >> input2;
			cout << "Enter the third input (as an integer): ";
			cin >> input3;
			cout << "Enter the first output (as an integer): ";
			cin >> output1;
			cout << "Enter the second output (as an integer): ";
			cin >> output2;
			A = input1;
			B = input2;
			C = input3;
			answer1 = output1;
			answer2 = output2;
		}
		else if ( numOut == 3 )
		{
			neg = true;
			cout << "Enter the first input (as an integer): ";
			cin >> input1;
			cout << "Enter the second input (as an integer): ";
			cin >> input2;
			cout << "Enter the third input (as an integer): ";
			cin >> input3;
			cout << "Enter the first output (as an integer): ";
			cin >> output1;
			cout << "Enter the second output (as an integer): ";
			cin >> output2;
			cout << "Enter the third output (as an integer): ";
			cin >> output3;
			A = input1;
			B = input2;
			C = input3;
			answer1 = output1;
			answer2 = output2;
			answer3 = output3;
		}
		else
		{
			cout << "INVALID INPUT\n";
		}
	}
	else	// run defaults
	{
		int test = -1;
		cout << "1 = XOR, 2 = ADDER, 3 = NEGATION\n";
		cout << "Test (1, 2 or 3): ";
		cin >> test;
		if ( test == 1 )		// run the xor test
		{
			A = 3;					//00000011
			B = 5;					//00000101
			answer1 = 6;			//00000110
		}
		else if ( test == 2 )	// run the adder test
		{
			type = true;
			A = 15;					//00001111
			B = 51;					//00110011
			C = 85;					//01010101
			answer1 = 240;			//11110000
			answer2 = 204;			//11001100
		}
		else if ( test == 3 )	// run the negation test
		{
			neg = true;
			A = 15;					//00001111
			B = 51;					//00110011
			C = 85;					//01010101
			answer1 = 240;			//11110000
			answer2 = 204;			//11001100
			answer3 = 170;			//10101010
		}
		else
		{
			cout << "INVALID INPUT\n";
		}
	}
}

int main ( int argc, char *argv[] )
{
	int pop = 25000;
	long num = 0;
	string fileName = "output.txt";
	vector<Gate> gBox;
	vector<Circuit> cBox;
	bitset<8> A (string("00001111")); 
	bitset<8> B (string("00110011")); 
	bitset<8> C (string("01010101"));
	bitset<8> answer1 (string("00010111"));
	bitset<8> answer2 (string("01101001"));
	bitset<8> answer3 (string("00000000"));
	
	Circuit report;

	bool flag1 = false;	// first output found
	bool flag2 = false;	// second output found
	bool flag3 = false; // third output found
	bool type = false;	// if true, two outputs
	bool neg = false;	// if true, three output
	bool c1 = false;	// for once we find an answer, so we don't keep printing it
	bool c2 = false;
	bool c3 = false;
	bool tell = true;	// tells the gate box whether to throw nots in or not

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

	// COMMENT IN FOR COMMAND LINE ARGUMENTS -f -p FOR FILENAME AND POPULATION SELECTION
	// string inOne = argv[ 1 ];
	// string inTwo = argv[ 2 ];
	// if ( inOne == "-f" )
	// {
	// 	cout << "Input file name: ";
	// 	cin >> fileName;
	// 	if ( inTwo == "-p" )
	// 	{
	// 		cout << "Input population: ";
	// 		cin >> pop;
	// 		cout << "\n";
	// 	}
	// 	else
	// 	{
	// 		cout << "\n";
	// 	}
	// }

	OFS.open( fileName );
	OFS << "Population: " << pop << "\n\n";

	userInput( A, B, C, answer1, answer2, answer3, type, neg );
	
	initGateBox( gBox, pop, tell );
	tell = false;
	initCircuitBox( cBox, gBox, type, neg, A, B, C );

	cout << "type: " << type << "\tneg: " << neg << "\n";
	if ( type )
	{
		cout << "OUT1: " << answer1 << "\n";
		cout << "OUT2: " << answer2 << "\n";
		OFS << "OUT1: " << answer1 << "\n";
		OFS << "OUT2: " << answer2 << "\n\n";
	}
	else if ( neg )
	{
		cout << "OUT1: " << answer1 << "\n";
		cout << "OUT2: " << answer2 << "\n";
		cout << "OUT3: " << answer3 << "\n";
		OFS << "OUT1: " << answer1 << "\n";
		OFS << "OUT2: " << answer2 << "\n";
		OFS << "OUT3: " << answer3 << "\n\n";
	}
	else
	{
		cout << "OUT: " << answer1 << "\n";
		OFS << "OUT: " << answer1 << "\n\n";
	}
	
	int swch = 0;
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	int generation = 0;
	for ( ; ; )
	{
		if ( cBox.size() < ( pop / 2 ) )
		{
			initGateBox( gBox, pop / 3, false );
			initCircuitBox( cBox, gBox, type, neg, A, B, C );
		}
		else if ( cBox.size() == 0 )
		{
			cout << "TOO SMALL\n";
			initGateBox( gBox, pop, false );
			initCircuitBox( cBox, gBox, type, neg, A, B, C );
		}
		report = fitness( cBox, answer1, answer2, answer3, flag1, flag2, flag3, type, neg );

		if ( !type && !neg )						// single output answer
		{
			cout << "1Fitness Report: " << AVERAGEFITNESS1 << "\t\tBit Report: " << AVERAGEMISSINGBITS1 << "\n";
			OFS << "1Fitness Report: " << AVERAGEFITNESS1 << "\t\tBit Report: " << AVERAGEMISSINGBITS1 << "\n";
			// write to graphical output
				num = AVERAGEFITNESS1;
				write(pipefd, &num, sizeof(num)); 
				num = AVERAGEMISSINGBITS1;
				write(pipefd, &num, sizeof(num));
			if ( flag1 )							// if answer found
			{	
				cout << "FIRST FOUND\n";
				OFS << "FIRST FOUND\n";
				break;
			}
		}
		else if ( type )	// if two output answer
		{
			if ( flag1 && !flag2 && !c1 )
			{
				c1 = true;
				cout << "First output found. Searching for second...\n";
				OFS << "First output found. Searching for second...\n";
			}
			else if ( flag2 && !flag1 && !c2 )
			{
				c2 = true;
				cout << "Second output found. Searching for first...\n";
				OFS << "Second output found. Searching for first...\n";
			}
			else if ( flag1 && flag2 )
			{
				cout << "Both answers found.\n";
				OFS << "Both answers found.\n";
				break;
			}

			if ( !flag1 )
			{
				cout << "1Fitness Report: " << AVERAGEFITNESS1 << "\t\tBit Report: " << AVERAGEMISSINGBITS1 << "\n";
				OFS << "1Fitness Report: " << AVERAGEFITNESS1 << "\t\tBit Report: " << AVERAGEMISSINGBITS1 << "\n";
				// write to graphical output
				num = AVERAGEFITNESS1;
				write(pipefd, &num, sizeof(num)); 
				num = AVERAGEMISSINGBITS1;
				write(pipefd, &num, sizeof(num));
			}
			else
			{
				cout << "2Fitness Report: " << AVERAGEFITNESS1 << "\t\tBit Report: " << AVERAGEMISSINGBITS1 << "\n";
				OFS << "2Fitness Report: " << AVERAGEFITNESS1 << "\t\tBit Report: " << AVERAGEMISSINGBITS1 << "\n";
				// write to graphical output
				num = AVERAGEFITNESS1;
				write(pipefd, &num, sizeof(num)); 
				num = AVERAGEMISSINGBITS1;
				write(pipefd, &num, sizeof(num));
			}
		}
		else if ( neg )				// if we find the third answer
		{
			if ( flag1 && ( !flag2 || !flag3 ) && !c1 )
			{
				c1 = true;
				cout << "First output found. Searching for others...\n";
				OFS << "First output found. Searching for others...\n";
			}
			else if ( flag2 && ( !flag1 && !flag3 ) && !c2 )
			{
				c2 = true;
				cout << "Second output found. Searching for others...\n";
				OFS << "Second output found. Searching for others...\n";
			}
			else if ( flag3 && ( !flag1 && !flag2 ) && !c3 )
			{
				c3 = true;
				cout << "Third output found. Searching for others...\n";
				OFS << "Third output found. Searching for others...\n";
			}
			else if ( flag1 && flag2 && flag3 )
			{
				cout << "All three answers found.\n";
				OFS << "All three answers found.\n";
				break;
			}

			if ( !flag1 )
			{
				cout << "1Fitness Report: " << AVERAGEFITNESS1 << "\t\tBit Report: " << AVERAGEMISSINGBITS1 << "\n";
				OFS << "1Fitness Report: " << AVERAGEFITNESS1 << "\t\tBit Report: " << AVERAGEMISSINGBITS1 << "\n";
				// write to graphical output
				num = AVERAGEFITNESS1;
				write(pipefd, &num, sizeof(num)); 
				num = AVERAGEMISSINGBITS1;
				write(pipefd, &num, sizeof(num));
			}
			else if ( flag1 && !flag2 )
			{
				cout << "2Fitness Report: " << AVERAGEFITNESS1 << "\t\tBit Report: " << AVERAGEMISSINGBITS1 << "\n";
				OFS << "2Fitness Report: " << AVERAGEFITNESS1 << "\t\tBit Report: " << AVERAGEMISSINGBITS1 << "\n";
				// write to graphical output
				num = AVERAGEFITNESS1;
				write(pipefd, &num, sizeof(num)); 
				num = AVERAGEMISSINGBITS1;
				write(pipefd, &num, sizeof(num));
			}
			else if ( flag1 && flag2 && !flag3 )
			{
				cout << "3Fitness Report: " << AVERAGEFITNESS1 << "\t\tBit Report: " << AVERAGEMISSINGBITS1 << "\n";
				OFS << "3Fitness Report: " << AVERAGEFITNESS1 << "\t\tBit Report: " << AVERAGEMISSINGBITS1 << "\n";
				// write to graphical output
				num = AVERAGEFITNESS1;
				write(pipefd, &num, sizeof(num)); 
				num = AVERAGEFITNESS1;
				write(pipefd, &num, sizeof(num)); 
			}
		}
		breed( cBox, answer1, answer2, answer3, flag1, flag2, flag3, type, neg );
		++generation;

		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
		OFS << "Run time: " << duration << "\n\n";
		OFS.close();
		OFS.open( fileName, ofstream::app );
	}

	cout << "\n\ntype: " << type << "\tneg: " << neg << "\n";
	cout << "flag1: " << flag1 << "\tflag2: " << flag2 << "\tflag3: " << flag3 << "\n";
	printCircuit( cfound1 );
	if ( type )
	{
		printCircuit( cfound2 );
	}
	else if ( neg )
	{
		printCircuit( cfound2 );
		printCircuit( cfound3 );
	}
	OFS.close();

	close(pfd[1]); // close write end of the pipe
	wait(NULL);  // wait for child (child ends when Xming window is close

	return 0;
}
