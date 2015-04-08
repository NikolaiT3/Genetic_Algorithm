//***********************************************************************************************
// Team 7       :   Adam Guy, Nabir Dinani, Jonathan Kocmoud, Nicholas Warner
// Date			:	11 March 2015
// Subject		:	CSCE 315-504
// Assignment	:	Project 3
// Updated		:	4 April 2015
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
// Things to do :	--Redo fitness function (lower penalty for missing bits, higher for nots and and/ors)
//					-Add mutations (throw a gate at the end - 1 in 1000?)
//					-Have a max gate size limit (100?)
//					--Check to make sure gate numbers are revaluated for children (in evalCircuit)
//					-Make fitness cut off dynamic?
//					-Implement a gate size max
//					-This should run for Adder and Negation, not Adder and XOR. Must change selection system
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

using namespace std;
using namespace std::chrono;

/********************************************************************************
								GLOBALS
********************************************************************************/

long double averageFitness1;
long double averageFitness2;
int delimiter;	// the length of the first solution
double averageMissingBits;
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
	bitset<8> out2;
	double fitness1;
	double fitness2;
};

/********************************************************************************
						PRINTING FUNCTIONALITIES
********************************************************************************/

void printCircuit( Circuit c )
{
	cout << "CIRCUIT ID: " << c.id << "\n";
	ofs << "CIRCUIT ID: " << c.id << "\n";
	for ( int i = 0; i < c.gates.size(); ++i )
	{
		if( c.gates[ i ].gate == "NONE" )
		{
			cout << c.gates[ i ].outputLine << " " << c.gates[ i ].gate << " "
				 << c.gates[ i ].x << "\t" << c.gates[ i ].gateOut << "\n";
			ofs << c.gates[ i ].outputLine << " " << c.gates[ i ].gate << " "
				<< c.gates[ i ].x << "\t" << c.gates[ i ].gateOut << "\n";
		}
		else if ( c.gates[ i ].gate == "NOT" )
		{
			cout << c.gates[ i ].outputLine << " " << c.gates[ i ].gate << "  "
				 << c.gates[ i ].x << "\t" << c.gates[ i ].gateOut << "\n";
			ofs << c.gates[ i ].outputLine << " " << c.gates[ i ].gate << "  "
				<< c.gates[ i ].x << "\t" << c.gates[ i ].gateOut << "\n";
		}
		else if ( c.gates[ i ].gate == "AND" )
		{
			cout << c.gates[ i ].outputLine << " " << c.gates[ i ].gate << "  "
				 << c.gates[ i ].x << " " << c.gates[ i ].y << "\t" << c.gates[ i ].gateOut << "\n";
			ofs << c.gates[ i ].outputLine << " " << c.gates[ i ].gate << "  "
				<< c.gates[ i ].x << " " << c.gates[ i ].y << "\t" << c.gates[ i ].gateOut << "\n";
		}
		else if ( c.gates[ i ].gate == "OR" )
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
	cout << "Circuit Output\nOut 1: " << c.out1 << "\nOut 2 : " << c.out2 << "\n";
	ofs << "Circuit Output\nOut 1: " << c.out1 << "\nOut 2 : " << c.out2 << "\n";
	cout << "\n";
	ofs << "\n";
}

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

void evalGate( Gate &g, int operation )
{
	//cout << "eGGG\n";
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
	//cout << "//eGGG\n\n\n";
}

void evalCircuit( Circuit &c )
{
	// at every index, set ouput of that line
	// 		lines will never be without updated parents
	//		as every gate is connected to lines with
	//		a lower id than it, which will have already
	//		been updates
	//cout << "eCCC\n";
	c.nots = 0;
	c.aNo = 0;
	int op = -1;
	for ( int i = 0; i < c.gates.size(); ++i )
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
		}
		else if ( c.gates[ i ].gate == "NONE" )
		{
			op = 3;
		}
		evalGate( c.gates[ i ], op );
	}
	c.out1 = c.gates.back().gateOut;
	//cout << "//eCCC\n\n\n";
}

Circuit fitness( vector<Circuit> &circuits, const bitset<8> &answer1, const bitset<8> &answer2, bool &flag1, bool &flag2 )
{
	//cout << "fit\n";
	averageMissingBits = 0;
	flag1 = false;
	flag2 = false;
	Circuit g;
	int numMissingValues1 = 0;
	int numMissingValues2 = 0;
	int size = circuits.size();

	for ( int i = 0; i < size; ++i )
	{
		// cout << "15\n";
		//printCircuit( circuits[ i ] );
		if ( circuits[ i ].out1 == answer1 )
		{
			flag1 = true;
			g = circuits[ i ];
			delimiter = circuits[ i ].gates.size();
		}
		else if ( circuits[ i ].out2 == answer1 )
		{
			flag1 = true;
			g = circuits[ i ];
			delimiter = circuits[ i ].gates.size();
		}
		else if ( circuits[ i ].out1 == answer2 )
		{
			flag2 = true;
			g = circuits[ i ];
			delimiter = circuits[ i ].gates.size();
		}
		else if ( circuits[ i ].out2 == answer2 )
		{
			flag2 = true;
			g = circuits[ i ];
			delimiter = circuits[ i ].gates.size();
		}

		// compare missing bit values
		for ( int b = 0; b < 8; ++b )	// find how many bits are missing from the answer
		{
			if ( circuits[ i ].out1[ b ] != answer1[ b ] )
			{
				++numMissingValues1;
			}
			else if ( circuits[ i ].out2[ b ] != answer2[ b ] )
			{
				++numMissingValues2;
			}
		}
		averageMissingBits += numMissingValues1;
		// cout << "16\n\n";
		// cout << "17\n";
		circuits[ i ].fitness1 = 10000 * numMissingValues1
								+ 1000 * circuits[ i ].nots
								+ 100 * circuits[ i ].aNo;
		averageFitness1 += circuits[ i ].fitness1;
		circuits[ i ].fitness2 = 10000 * numMissingValues2
								+ 1000 * circuits[ i ].nots
								+ 100 * circuits[ i ].aNo;
		averageFitness2 += circuits[ i ].fitness2;
		// cout << "18\n\n";
	}
	averageMissingBits = averageMissingBits / size;
	averageFitness1 = averageFitness1 / size;
	averageFitness2 = averageFitness2 / size;
	numMissingValues1 = 0;
	numMissingValues2 = 0;
	return g;
	//cout << "//fit\n\n\n";
}

void breed( vector<Circuit> &circuits, bool flag1 )
{
	//cDex = rand() % cBox.size();
	//line1 = rand() % init.gates.size();

	srand( time( NULL ) );
	int cut = -1;
	long double cutoff = -1;
	vector<Circuit> tempGen;
	// decide a cutoff for population fitness acceptable - above two thirds of average?
	if ( !flag1 ) 	// first answer not found yet
	{
		//run off averageFitness1
		// go through entire population pool and pull out all circuits that meet the fitness cut
		cutoff = averageFitness1 / 3;	// top 2/3 make it
		for ( int i = 0; i < circuits.size(); ++i )
		{
			if ( circuits[ i ].fitness1 > cutoff )
			{
				tempGen.push_back( circuits[ i ] );
			}
		}
	}
	else
	{
		//run off averageFitness2
		// go through entire population pool and pull out all circuits that meet the fitness cut
		cutoff = averageFitness2 / 3;	// top 2/3 make it
		for ( int i = 0; i < circuits.size(); ++i )
		{
			if ( circuits[ i ].fitness2 > cutoff )
			{
				tempGen.push_back( circuits[ i ] );
			}
		}
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
				cut = rand() % mother.gates.size() + delimiter;
			}
			else
			{
				cut = rand() % father.gates.size() + delimiter;
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

			evalCircuit( son );
			evalCircuit( daughter );
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
				cut = rand() % mother.gates.size() + delimiter;
			}
			else
			{
				cut = rand() % father.gates.size() + delimiter;
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


				evalCircuit( son );
				evalCircuit( daughter );
				evalCircuit( stepChild );
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
				evalCircuit( son );
				evalCircuit( daughter );
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
}

void rePopulate()
{
	// once an answer is found for a two output circuit, create a new population
	// based off that correct circuit and continue with the algorithm to find
	// the remaining answer
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
	delimiter = 0;
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
		cBox.push_back( init );						// save new circuit
	}
	cout << "\n\n";
}

/********************************************************************************
							MAIN AND MAIN-HELPERS
********************************************************************************/

void userInput( bitset<8> &A, bitset<8> &B, bitset<8> &C, bitset<8> &answer1, bitset<8> &answer2, bool &type )
{
	int numOut = -1;
	string input1;
	string input2;
	string input3;
	string output1;
	string output2;
	string def = "";

	cout << "Run Defaults? (y or n): ";
	cin >> def;
	
	if ( def == "n" )
	{	cout << "Number of outputs (1 or 2): ";
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
			bitset<8> answer1 (string(answer1));
		}
		else if ( numOut == 2 )
		{
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
			bitset<8> answer1 (string(answer1));
			bitset<8> answer2 (string(answer2));
		}
		else
		{
			cout << "INVALID INPUT\n";
		}
	}
	else
	{
		int test = -1;
		cout << "Test (1 or 2): ";
		cin >> test;
		if ( test == 2 )		// run the adder test
		{
			type = true;
		}
		else if ( test == 1 )	// run the xor test
		{
			bitset<8> A (string("00000011"));
			bitset<8> B (string("00000101"));
			bitset<8> C (string("00000000"));
			bitset<8> answer1 (string("00000110"));
			bitset<8> answer2 (string("00000000"));
		}
		else
		{
			cout << "INVALID INPUT\n";
		}
	}
}

int main ( int argc, char const *argv[] )
{
	int pop = 100000;
	vector<Gate> gBox;
	vector<Circuit> cBox;
	bitset<8> A (string("00001111")); 
	bitset<8> B (string("00110011")); 
	bitset<8> C (string("01010101"));
	bitset<8> answer1 (string("00010111"));
	bitset<8> answer2 (string("01101001"));
	
	Circuit correct1;

	bool flag1 = false;	// first output found
	bool flag2 = false;	// second output found
	bool type = false;	// if true, two outputs. else, one output

	ofs.open( "output.txt" );
	ofs << "Population: " << pop << "\n\n";

	userInput( A, B, C, answer1, answer2, type );
	
	initGateBox( gBox, pop );
	initCircuitBox( cBox, gBox, type, A, B, C );
	
	int swch = 0;
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	//for ( int i = 0; i < 30; ++i )
	for ( ; ; )
	{
		// cout << "1\n";
		if ( cBox.size() == 0 )
		{
			break;
		}
		correct1 = fitness( cBox, answer1, answer2, flag1, flag2 );
		// cout << "2\n";
		if ( flag1 )				// if we find the first answer
		{
			// cout << "3\n";
			if ( !type )			// if there's one output only
			{
				// cout << "4\n";
				break;				// single output answer found
			}
			else					// there's two outputs, so re-populate off this
			{
				// cout << "5\n";
				rePopulate();
			}
		}
		else if ( flag2 && type )	// if we find the second answer and this is two outputs
		{
			// cout << "6\n";
			if ( !flag1 )			// if second answer found but still need first
			{
				// cout << "7\n";
				rePopulate();
			}
		}
		else if ( flag1 && flag2 )	// if we've found the completed circuit
		{
			// cout << "8\n";
			break;
		}
		else						// no answer found - breed
		{
			// cout << "breed\n";
			breed( cBox, flag1 );
			// cout << "//breed\n\n";
		}


		if ( !type )				// single output
		{
			cout << "1Fitness Report: " << averageFitness1 << "\tBit Report: " << averageMissingBits << "\n";
			ofs << "1Fitness Report: " << averageFitness1 << "\tBit Report: " << averageMissingBits << "\n";
		}
		else						// two output
		{
			if ( !flag1 ) 			// first output not found yet
			{
				cout << "2Fitness Report: " << averageFitness1 << "\n";
				ofs << "2Fitness Report: " << averageFitness1 << "\n";
			}
			else if ( flag1 && swch == 0 )
			{
				cout << "First output found. Switching Context...\n";
				ofs << "\nFirst output found. Switching Context...\n\n";
				printCircuit( correct1 );
				++swch;
				// cout << "9\n";
			}
			else if ( flag2 && swch == 0 )
			{
				cout << "Second output found. Switching Context...\n";
				ofs << "\nSecond output found. Switching Context...\n\n";
				printCircuit( correct1 );
				++swch;
				// cout << "10\n";
			}
			else
			{
				cout << "3Fitness Report: " << averageFitness1 << "\n";
				ofs << "3Fitness Report: " << averageFitness1 << "\n";
			}
		}
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
		ofs << "Run time: " << duration << "\n\n";
	}
	// cout << "wha...\n";
	printCircuit( correct1 );
	ofs.close();
	return 0;
}