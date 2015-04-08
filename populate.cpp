//***************************************************
// Name			: Nicholas Warner
// Date			: 11 March 2015
// Subject		: CSCE 315-504
// Assignment	: Project 3
// Updated		: 12 March 2015
// Description	: That's a damn good question
//***************************************************

#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <fstream>
#include <chrono>

using namespace std;
using namespace std::chrono;

struct Gate
{
	int outputLine;
	string gate;		// NONE, NOT, AND, OR
	int x;				// first input for not, and, or and none
	int y;				// second input for and, or | -1 for NONE or NOT gate
};

struct Circuit
{
	int id;
	int nots;
	vector<Gate> gates;
	bitset<8> x;
	bitset<8> y;
	bitset<8> z;
	bitset<8> out1;
	bitset<8> out2;
};

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
		type = rand() % 3 + 1;
		// if ( nots < 2 )
		// {
		// 	type = rand() % 3 + 1;
		// }
		// else	// prevent more than two NOTs being placed in box
		// {
		// 	type = rand() % 2 + 2;
		// }
		//cout << type << "\n";
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

void initCircuitBox( vector<Circuit> &cBox, vector<Gate> &gBox )
{
	Gate temp;
	Circuit init;
	
	for ( int a = 0; a < 3; ++a )		// first three "Gates" are NONE
	{
		temp.outputLine = a;
		temp.gate = "NONE";
		temp.x = a;
		temp.y = -1;
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
		line1 = rand() % init.gates.size() + 1;		// a random line at the circuit at cDex in cbox
		line2 = rand() % init.gates.size() + 1;		// a random line at the circuit at cDex in cbox

		while ( line1 == line2 )					// make sure the lines don't equal each other
		{
			line2 = rand() % init.gates.size() + 1;	// a random line at the circuit at cDex in cbox
		}

		temp.outputLine = init.gates.size() + 1;	// line number
		temp.gate = gBox[ gDex ].gate;				// get random gate
		gBox.erase( gBox.begin() + gDex );			// delete gate used

		// set gate in circuit
		if ( temp.gate == "NOT" )
		{
			temp.x = line1;
			temp.y = -1;
		}
		else if ( temp.gate == "AND" || temp.gate == "OR" )
		{
			temp.x = line1;
			temp.y = line2;
		}

		init.gates.push_back( temp );				// save to the chosen circuit
		cBox.push_back( init );						// save new circuit
	}
}

void printCircuit( Circuit c )
{
	cout << "CIRCUIT ID: " << c.id << "\n";
	for ( int i = 0; i < c.gates.size(); ++i )
	{
		if( c.gates[ i ].gate == "NONE" )
		{
			cout << c.gates[ i ].outputLine << " " << c.gates[ i ].gate << " " << c.gates[ i ].x << "\n";
		}
		else if ( c.gates[ i ].gate == "NOT" )
		{
			cout << c.gates[ i ].outputLine << " " << c.gates[ i ].gate << " " << c.gates[ i ].x << "\n";
		}
		else if ( c.gates[ i ].gate == "AND" )
		{
			cout << c.gates[ i ].outputLine << " " << c.gates[ i ].gate << " " << c.gates[ i ].x << " " << c.gates[ i ].y << "\n";
		}
		else if ( c.gates[ i ].gate == "OR" )
		{
			cout << c.gates[ i ].outputLine << " " << c.gates[ i ].gate << " " << c.gates[ i ].x << " " << c.gates[ i ].y << "\n";
		}
		else
		{
			cout << "Error in circuit read\n";
			break;
		}	
	}
	cout << "\n";
}

void instructionTest()
{
	Gate one, two, three, four, five, six, seven;
	Circuit X;
	X.id = 0;

	one.outputLine = 1;
	one.gate = "NONE";
	one.x = 1;
	one.y = -1;
	X.gates.push_back( one );

	two.outputLine = 2;
	two.gate = "NONE";
	two.x = 2;
	two.y = -1;
	X.gates.push_back( two );

	three.outputLine = 3;
	three.gate = "NOT";
	three.x = 1;
	three.y = -1;
	X.gates.push_back( three );

	four.outputLine = 4;
	four.gate = "NOT";
	four.x = 2;
	four.y = -1;
	X.gates.push_back( four );

	five.outputLine = 5;
	five.gate = "AND";
	five.x = 1;
	five.y = 4;
	X.gates.push_back( five );

	six.outputLine = 6;
	six.gate = "AND";
	six.x = 2;
	six.y = 3;
	X.gates.push_back( six );

	seven.outputLine = 7;
	seven.gate = "OR";
	seven.x = 1;
	seven.y = 6;
	X.gates.push_back( seven );

	printCircuit( X );
}

void testFileRun( vector<Gate> &gBox, vector<Circuit> &cBox, int &max, int pop )
{
	initGateBox( gBox, pop );
	initCircuitBox( cBox, gBox );
	int i = 0;
	for ( ; i < pop; ++i )
	{
		//printCircuit( cBox[ i ] );
		if ( cBox[ i ].gates.size() > max )
			max = cBox[ i ].gates.size();
	}
}

int main ( int argc, char const *argv[] )
{
	//instructionTest();

	ofstream ofs;
	ofs.open( "circuitPopulationAnalysis.txt" );

	vector<Gate> gBox;
	vector<Circuit> cBox;
	double average = 0;
	double averageRun = 0;
	int max = 0;

	ofs << "Population: 10\n";
	for ( int a = 0; a < 10; ++a )
	{
		ofs << "Run: " << a << "\n";
		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		testFileRun( gBox, cBox, max, 10 );
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
		ofs << "Gate Max: " << max << "\nDuration: " << duration << " microseconds\n\n";
		average += max;
		averageRun += duration;
		max = 0;
		gBox.clear();
		cBox.clear();
	}
	ofs << "Average Gate Max: " << average/10 << "\nAverage Run Time: " << averageRun/10 <<"\n\n\n";
	average = 0;
	averageRun = 0;

	ofs << "Population: 100\n";
	for ( int a = 0; a < 10; ++a )
	{
		ofs << "Run: " << a << "\n";
		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		testFileRun( gBox, cBox, max, 100 );
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
		ofs << "Gate Max: " << max << "\nDuration: " << duration << " microseconds\n\n";
		max = 0;
		average += max;
		averageRun += duration;
		gBox.clear();
		cBox.clear();
	}
	ofs << "Average Gate Max: " << average/10 << "\nAverage Run Time: " << averageRun/10 <<"\n\n\n";
	average = 0;
	averageRun = 0;

	ofs << "Population: 1000\n";
	for ( int a = 0; a < 10; ++a )
	{
		ofs << "Run: " << a << "\n";
		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		testFileRun( gBox, cBox, max, 1000 );
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
		ofs << "Gate Max: " << max << "\nDuration: " << duration << " microseconds\n\n";
		max = 0;
		average += max;
		averageRun += duration;
		gBox.clear();
		cBox.clear();
	}
	ofs << "Average Gate Max: " << average/10 << "\nAverage Run Time: " << averageRun/10 <<"\n\n\n";
	average = 0;
	averageRun = 0;

	ofs << "Population: 10000\n";
	for ( int a = 0; a < 10; ++a )
	{
		ofs << "Run: " << a << "\n";
		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		testFileRun( gBox, cBox, max, 10000 );
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
		ofs << "Gate Max: " << max << "\nDuration: " << duration << " microseconds\n\n";
		max = 0;
		average += max;
		averageRun += duration;
		gBox.clear();
		cBox.clear();
	}
	ofs << "Average Gate Max: " << average/10 << "\nAverage Run Time: " << averageRun/10 <<"\n\n\n";
	average = 0;
	averageRun = 0;

	ofs << "Population: 100000\n";
	for ( int a = 0; a < 10; ++a )
	{
		ofs << "Run: " << a << "\n";
		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		testFileRun( gBox, cBox, max, 100000 );
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
		ofs << "Gate Max: " << max << "\nDuration: " << duration << " microseconds\n\n";
		max = 0;
		average += max;
		averageRun += duration;
		gBox.clear();
		cBox.clear();
	}
	ofs << "Average Gate Max: " << average/10 << "\nAverage Run Time: " << averageRun/10 <<"\n\n\n";
	average = 0;
	averageRun = 0;
	
	ofs.close();

	// cout << "Beginning gate box initialization.\n";
	// initGateBox( gBox, 1000000 );
	// cout << "Gate box initialization complete.\n";

	// int max = 0;

	// cout << "Beginning circuit box initialization.\n";
	// initCircuitBox( cBox, gBox );
	// cout << "Circuit box initialization completed.\n";

	// int i = 0;
	// for ( ; i < 1000000; ++i )
	// {
	// 	//printCircuit( cBox[ i ] );
	// 	if ( cBox[ i ].gates.size() > max )
	// 		max = cBox[ i ].gates.size();
	// }
	// cout << "max gate size: " << max << "\nnumber circuits: " << i;

	return 0;
}