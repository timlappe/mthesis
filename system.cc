#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <omp.h>
#include <vector>
#include <math.h>

using namespace std;

// Systemkonstanten
const unsigned int N=5000; // Anzahl Gittermassen
const unsigned int steps=10000000; // Anzahl Zeitschritte
const unsigned int n=1; // Energielevel
const double a=1.0; // Boxlaenge
const double L=a/(N+1); // Abstand Gittermassen
const double h=1.0; // Wirkungsquantum //6.62606957*1e-34;
const double M=1.0; // Teilchenmasse
const double m=1e+4*M; // Gitterteilchenmasse
const double E_0=n*n*h*h/(8*M*a*a); // Energie
const double k=(N+1)*(N+1)*m*E_0/(2*M*a*a);// Federkonstante // (-1)*(1/((cos(n*M_PI/(N+1)) - 1)))*((m*h*h*pow(M_PI,2)*pow(n,4))/(32*M*M*pow(a,4)))

double v_0=0.0; // Anfangsgeschwindigkeit Teilchen
int start_index=1136.0; // Anfangsposition Teilchen
const unsigned int resol=1; // Raeumliche Aufloesung Anfangswerte
double pos_0s[resol]={start_index};
double excitation=0.005; // Anfangsanregung Gitter

// Vektoren und Matrizen
vector<double> EigVals(N);
vector< vector<double> > T(N,vector<double>(N));

// this function runs only once
void TridiagToeplitz() {
	for (int i=0; i<N; i++) {
		EigVals[i]=(2*k/m)*(cos((i+1)*M_PI/(N+1)) - 1);
		
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
	System(double, double, double, double*, double*);
	// destructor
	~System();
	
	// Teilchen
	double pos;
	double v;
	
	// Zeit
	double delta_t;
	
	// aktuelle Gittergeschw.
	double xdot_index;
	
	// Gitter
	double* y;
	double* ydot;
	
	double* w;
	double* wdot;
	
	// Methoden
	double Collision(double, double, double, double);
	double Oscillate(int, double);
	double* Evolve(double*);
};

// initializing
System::System(double pos_0, double v_0, double xdot_index_0, double* y_0, double* ydot_0) {
	pos=pos_0;
	v=v_0;
	
	delta_t=82.0; // mind. 16+2=18
	xdot_index=xdot_index_0;
	
	y=new double[N];
	ydot=new double[N];
	
	w=new double[N];
	wdot=new double[N];

	y=y_0;
	ydot=ydot_0;	
}

// destructor
System::~System() {}

double System::Collision(double m1, double v1, double m2, double v2) {
	return 2*((m1*v1 + m2*v2)/(m1 + m2)) - v1;
	// return 2*v2 - v1;
}

double System::Oscillate(int ind, double del_t) {
	double next=0.0;
	#pragma omp parallel for ordered reduction(+:next)
	for (int i=0; i<N; i++) {
		w[i]=y[i]*cos(sqrt(-EigVals[i])*del_t) + (ydot[i]/(sqrt(-EigVals[i])))*sin(sqrt(-EigVals[i])*del_t); // Watch out: assuming EigVals are negative
		wdot[i]=ydot[i]*cos(sqrt(-EigVals[i])*del_t) - y[i]*(sqrt(-EigVals[i]))*sin(sqrt(-EigVals[i])*del_t);
		
		// compute next lattice velocity
	#pragma ordered
		next+=T[ind][i]*wdot[i];
	}
	
	// interchange up-to-date and previous values
	double* help_ptr;
	double* helpdot_ptr;

	help_ptr=y;
	helpdot_ptr=ydot;

	y=w;
	ydot=wdot;
	
	w=help_ptr;
	wdot=helpdot_ptr;
	
	return next;
}

double* System::Evolve(double* arr) {

	// neue Geschwindigkeiten berechnen
	double w=v; // temporarily copy v
	v=this->Collision(M, v, m, xdot_index);
	
	double ww_pre=xdot_index; // save xdot_index before collision
	xdot_index=this->Collision(m, xdot_index, M, w); // use w
	double ww_post=xdot_index; // save xdot_index after collision but before oscillation
	
	// ydot updaten
	int index=round(pos/L) - 1;
	
	// Spezialfall Enden
	if (index==N) { 
		index-=1; // rechts
	}
	if (index==-1) {
		index+=1; // links
	}
	
	#pragma omp parallel for
	for (int i=0; i<N; i++) {
		ydot[i]+=T[index][i]*(xdot_index - ww_pre); //Transponierung aber nicht notwendig: T ist sym.
	}
	
	// Position updaten
	pos+=v*delta_t;

	// Reflektion
	double B=(N+1)*L; // B=a...
	int b=floor(pos/B);
	if ((b % 2) == 0) { // gerade
		pos=pos - b*B;
		// v bleibt
	}
	else { // ungerade
		pos=B - (pos - b*B);
		v=-v;
	}
	
	// Index updaten
	index=round(pos/L) - 1;
	
	// Spezialfall Enden
	if (index==N) { 
		index-=1; // rechts
	}
	if (index==-1) {
		index+=1; // links
	}
	
	// Gitter weiterentwickeln
	xdot_index=this->Oscillate(index, delta_t);
	
	// Position, resultierenden Index, (resultierende) Geschwindigkeiten des Teilchens und der Gittermasse speichern (v ist dann die Ausgangsgeschw. fuer den folgenden Stoss)
	*arr=b;
	arr++;
	*arr=pos;
	arr++;
	*arr=index;
	arr++;
	*arr=v;
	arr++;
	*arr=ww_pre; // use ww_pre
	arr++;
	*arr=ww_post; // use ww_post
	arr++;
	
	return arr;
	
	
	// Dringend beachten: Das Teilchen fliegt mit der hier gespeicherten Geschw. v vom vorigen Ort zum hier gespeicherten Ort.
	// Der Index gehoert zum vorigen Ort, weil er vor dem Update der Position gesetzt wird.
	// b gehoert hingegen zur aktuellen, weil upgedateten Position.
	
}

//------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------

int main() {

// Matrizen erzeugen
TridiagToeplitz();

// Anfangswerte Gitter
double x_0[N]={};
double xdot_0[N]={};
double yy_0[N]={};
double yydot_0[N]={};

yydot_0[n-1]=excitation;//15.0;//0.1118;//0.005;

	for (int i=0; i<N; i++) {
		for (int j=0; j<N; j++) {
			x_0[i]+=T[i][j]*yy_0[j];
			xdot_0[i]+=T[i][j]*yydot_0[j];
			
		}
		//cout << xdot_0[i] << "\n";
	}

double energie=0;
	for (int i=0; i<N; i++) {
			energie+=0.5*m*xdot_0[i]*xdot_0[i];
	}
	
cout << "Energie: " << energie <<", E_0: " << E_0 << '\n';
cout << '\n';

ofstream system_info;
system_info.open("data/system_info.txt");

system_info << "# N: " << N << '\n';
system_info << "# M: " << M << '\n';
system_info << "# m: " << m << '\n';
system_info << "# v_0: " << v_0 << '\n';
system_info << "# L: " << L << '\n'; 
system_info << "# steps: " << steps << '\n';  
system_info << "# resol: " << resol << '\n'; 

system_info.close();

for (int k=0; k<resol; k++) {
	// must be reset because in the initialization the System pointers are set to these arrays
	double y_0[N]={};
	double ydot_0[N]={};
	ydot_0[n-1]=excitation;//15.0;//0.1118;//0.005;

	double pos_0=pos_0s[k]*L; // Anfangsposition Teilchen
	int index_0=round(pos_0/L) - 1; // Anfangsindex Teilchen
	double xdot_index_0=0.0; // Anfangsgeschwindigkeit erste Gittermasse
	
	for (int j=0; j<N; j++) {
		xdot_index_0+=T[index_0][j]*ydot_0[j];
	}
	
	// number of elements in each file line
	int neefl=6;
	vector<double> particle_data_array(neefl*steps, 0.0); // neefl=number of elements in each file line
	double* ptr;
	ptr=&particle_data_array[0];

	System sys(pos_0, v_0, xdot_index_0, y_0, ydot_0);

	for (int j=0; j<steps; j++) {
		ptr=sys.Evolve(ptr);
	}
	
	// Textdatei anlegen und oeffnen
	ofstream particle_data;
    	ostringstream FileNameStream;
    	FileNameStream << "data/particle_data_" << k << ".txt";
    	string FileName = FileNameStream.str();
    	particle_data.open(FileName.c_str());

	// array einlesen
	for (int i=0; i<steps; i++) {
		for (int j=i*neefl; j<(i+1)*neefl; j++) { // neefl=number of elements in each file line
			particle_data << particle_data_array[j] << '\t'; // possibly use << setw(15) or so
		}
		particle_data << '\n';
	}

	particle_data.close();
	
}

return 0;
}
