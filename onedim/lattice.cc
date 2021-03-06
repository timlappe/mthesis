#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;

// Systemkonstanten
const unsigned int N=30;

// Energielevel
const unsigned int n=1;

// Boxlaenge
const double a=1.0;

// Wirkungsquantum
const double h=1.0;//6.62606957*1e-34;

// Teilchenmasse
const double M=1.0;

// Energie
const double E_0=n*n*h*h/(8*M*a*a);

// Gitterteilchenmasse
const double m=1000*M;

const double L=a/(N+1);
const double k=(-1)*(1/((cos(M_PI/(N+1)) - 1)))*((m*h*h*pow(M_PI,2)*pow(n,4))/(32*M*M*pow(a,4)));

// Matrizen
double T[N][N];
double D[N][N];

void TridiagToeplitz() {
	for (int i=0; i<N; i++) {
		D[i][i]=(2*k/m)*(cos((i+1)*M_PI/(N+1)) - 1);
		//cout << D[i][i] << endl;
		
		// Wie kann ich diesen extra loop vermeiden?
		double norm=0;
		for (int j=0; j<N; j++) {
			norm+=sin((j+1)*M_PI*(i+1)/(N+1))*sin((j+1)*M_PI*(i+1)/(N+1));
		}
		norm=sqrt(norm);
		for (int j=0; j<N; j++) {
			T[j][i]=sin((j+1)*M_PI*(i+1)/(N+1))/norm;
		}
	}
}

class System {
   public:
   	// constructor
	System(double, double*, double*);
	
	// Teilchen
	double delta_t;
	
	// Gitter
	double x[N];
	double xdot[N];
	
	double y[N];
	double ydot[N];
	
	// Methoden
	void Oscillate();
	void Evolve(ofstream&, ofstream&);
};

// initializing double delta_t, double pos, double v, double* x, double* xdot
System::System(double time_step, double* x_0, double* xdot_0) {
	delta_t=time_step;
	for (int i=0; i<N; i++) {
		x[i]=*x_0;
		x_0++;
		
		xdot[i]=*xdot_0;
		xdot_0++;
	}
}

void System::Oscillate() {
	double w1[N]={};
	double w2[N]={};
	for (int i=0; i<N; i++) {
		w1[i]=y[i]*cos(sqrt(-D[i][i])*delta_t) + ydot[i]/(sqrt(-D[i][i]))*sin(sqrt(-D[i][i])*delta_t); // ACHTUNG: assuming matrix D is neg. def.
		w2[i]=ydot[i]*cos(sqrt(-D[i][i])*delta_t) - y[i]*(sqrt(-D[i][i]))*sin(sqrt(-D[i][i])*delta_t);
	}
		
	for (int i=0; i<N; i++) {
		x[i]=0;
		xdot[i]=0;
		for (int j=0; j<N; j++) {
			x[i]+=T[i][j]*w1[j];
			xdot[i]+=T[i][j]*w2[j];
		}
	}
}
// argument files are particle_data, only_lattice_positions, only_lattice_velocities
void System::Evolve(ofstream& file2, ofstream& file3) {

	// Gitter weiterentwickeln
	for (int i=0; i<N; i++) {
		y[i]=0;
		ydot[i]=0;
		for (int j=0; j<N; j++) {
			y[i]+=T[i][j]*x[j];
			ydot[i]+=T[i][j]*xdot[j];
			
			// Gittermassenpositionen mit abgreifen (vor Evolution)
			if (i == 0) {
				file2 << x[j] << "   ";
			}
			
			// Gittermassengeschwindigkeiten mit abgreifen (nach Stoss, aber vor Evolution)
			if (i == 0) {
				file3 << xdot[j] << "   ";
			}
		}
	}
	
	// Zeilenumbruch in file2 ergaenzen
	file2 << endl;
	
	// Zeilenumbruch in file3 ergaenzen
	file3 << endl;
	
	this->Oscillate(); // sets x and xdot to new values
	
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

int main() {

// Matrizen erzeugen
TridiagToeplitz();

/*
for (int i=0; i<N; i++) {
	for (int j=0; j<N; j++) {
		cout << T[i][j] << " ";
	}
	cout << endl;
}
*/

// Textdateien anlegen und oeffnen
ofstream only_lattice_positions, only_lattice_velocities;
only_lattice_positions.open("only_lattice_positions.txt");
only_lattice_velocities.open("only_lattice_velocities.txt");

double x_0[N]={};
double xdot_0[N]={};
double y_0[N]={};
double ydot_0[N]={};

ydot_0[1]=10.0;

/*
// Anfangswerte Gitter
for (int i=0; i<N; i++) {
	y_0[i]=0.1;
	ydot_0[i]=0.0;
}
*/

	for (int i=0; i<N; i++) {
		for (int j=0; j<N; j++) {
			x_0[i]+=T[i][j]*y_0[j];
			xdot_0[i]+=T[i][j]*ydot_0[j];
		}
	}



// "Frequenz"
double time_step=0.1;

System sys(time_step, x_0, xdot_0);

int steps=500;
for (int i=0; i<steps; i++) {
	sys.Evolve(only_lattice_positions, only_lattice_velocities);
}

only_lattice_positions.close();
only_lattice_velocities.close();

return 0;
}
