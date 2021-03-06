// droplet class file
#ifndef DROPLET_H
#define DROPLET_H

#include <math.h>
#include <stdlib.h>
#include <algorithm> // max, min

#include "constants.hh"

using namespace std;
using namespace Constants;

// ------------------------------------------------------------------------------------------------

class Droplet {
	public:
		 Droplet(int, int, unsigned int, double, double*, double*); // constructor
		~Droplet(); // destructor

		int p; // droplet number
		int rep; // repetition number

		unsigned int Steps; // number of steps
		double dt; // recursion time
		unsigned int T_col; // collision time

		double* R; // coordinates
		double* V; // velocities

		// methods
		double Dot(double*, double*);
		double* Normed(double*);
		double* Cross(double*, double*, double*);
		void Reflect(double*);
		bool Hit(double*, double*);
		bool Hit(unsigned int);
		double* Collide(double, double*, double*);
		void Crashed();
		void Touched();
		double* Update(Verlet*, double*, double, double, double*);
		double F(double, double);
		double* TheForce(Verlet*, int*, double*);
		void Evolve(Verlet*, double*);
};

Droplet::Droplet(int P, int Rep, unsigned int Steps_0, double dt_0, double* R_0, double* V_0) {
	p=P; rep=Rep;
	Steps=Steps_0; dt=dt_0;
	T_col=(1/f)*(1/dt) + 0.5; // is this reliable?
	if (rep == 0) cout << " --- T_col is: " << T_col << "\n";
	R=new double[3];
	V=new double[3];

	if (rep==0) {
		for (int i=0; i<3; ++i) {
			R[i]=R_0[i];
			V[i]=V_0[i];
		}
	}
	else {
		ifstream state_data;
		ostringstream FileNameStream;
		FileNameStream << _DATA_ << "/init/" << system_type << "_" << p << "_init_chunk_" << rep << ".dat";
		string FileName=FileNameStream.str();
		state_data.open(FileName.c_str());
		state_data.precision(15);
		int i=0;
		double RR; double VV;
		while (state_data >> RR >> VV) {
			R[i]=RR; V[i]=VV;
			++i;
		}

		state_data.close();
	}
}

Droplet::~Droplet() {
	// save current state of the system
	ofstream state_data;
	ostringstream FileNameStream;
	FileNameStream << _DATA_ << "/init/" << system_type << "_" << p << "_init_chunk_" << rep + 1 << ".dat";
	string FileName=FileNameStream.str();
	state_data.open(FileName.c_str());
	state_data.precision(15);
	// write droplet state to file
	for (int i=0; i<3; ++i) {
		state_data << R[i] << '\t';
		state_data << V[i] << '\t';
		state_data << '\n';
	}

	state_data.close();
}

// ------------------------------------------------------------------------------------------------
// Vector analysis

double Droplet::Dot(double* vec1, double* vec2) {
	return (vec1[0]*vec2[0] + vec1[1]*vec2[1] + vec1[2]*vec2[2]);
}

double* Droplet::Normed(double* vec) {
	double norm_vec=sqrt(this->Dot(vec, vec));
	for (int i=0; i<3; ++i) vec[i]=vec[i]/norm_vec;
	return vec;
}

double* Droplet::Cross(double* vec1, double* vec2, double* res) {
	res[0]=vec1[1]*vec2[2] - vec1[2]*vec2[1];
	res[1]=vec1[2]*vec2[0] - vec1[0]*vec2[2];
	res[2]=vec1[0]*vec2[1] - vec1[1]*vec2[0];
	return res;
}
// ------------------------------------------------------------------------------------------------

void Droplet::Reflect(double* n) {
	// normalize normal vector
	n=this->Normed(n);

	// reflect V
	double V_dot_n=this->Dot(V, n);
	for (int i=0; i<3; ++i) {
		V[i]=V[i]-2*V_dot_n*n[i];
	}
}

// ------------------------------------------------------------------------------------------------
bool Droplet::Hit(double* r1, double* r2) {
	// see green notebook, 10.12.14
	double* r2_minus_r1=new double[3]; // include lib or write vector addition
	for (int i=0; i<3; ++i) {
		r2_minus_r1[i]=r2[i]-r1[i];
	}
	double l=sqrt(this->Dot(r2_minus_r1, r2_minus_r1));
	return l <= (D/2 + d/2);
}

bool Droplet::Hit(unsigned int tt) {
	return (tt % T_col) == 0;
}

// this function is confirmed to be working properly; set 3x3 grid for inspection
double* Droplet::Collide(double m, double* r, double* v) {
	// see green notebook, 08.12.14
	double* p=new double[3];
	double* t=new double[3];
	double* n=new double[3];

	for (int i=0; i<3; ++i) {
		p[i]=R[i]-r[i];
	}
	p=this->Normed(p);

	n=this->Cross(R, r, n);
	t=this->Cross(n, p, t);
	t=this->Normed(t);

	double V_Dot_p=this->Dot(V, p);
	double v_Dot_p=this->Dot(v, p);
	double c_factor=2*(M*V_Dot_p + m*v_Dot_p)/(M+m);

	double V_Dot_t=this->Dot(V, t);
	double v_Dot_t=this->Dot(v, t);

	for (int i=0; i<3; ++i) {
		//cout << v[i] << '\n';
		V[i]=V_Dot_t*t[i] + (c_factor - V_Dot_p)*p[i];
		v[i]=v_Dot_t*t[i] + (c_factor - v_Dot_p)*p[i];
	}
/*
	double v_length=0.1;
	V=this->Normed(V);
	//cout << V[0] << '\t';
	for (int i=0; i<3; ++i) V[i]=v_length*V[i];
	//cout << V[0] << '\n';
*/
	return v;
}

// make report if droplet crashed somewhere
void Droplet::Crashed() {
	ofstream crash;
	ostringstream FileNameStream;
	FileNameStream << _DATA_ << "/crashed" << ".dat";
	string FileName=FileNameStream.str();
	crash.open(FileName.c_str());
	crash << "Droplet has crashed!" << '\n';
	crash.close();
}

// make report if droplet touched the wall inside one of the slits
void Droplet::Touched() {
	ofstream touch;
	ostringstream FileNameStream;
	FileNameStream << _DATA_ << "/touched" << ".dat";
	string FileName=FileNameStream.str();
	touch.open(FileName.c_str());
	touch << "Droplet has touched!" << '\n';
	touch.close();
}

double* Droplet::Update(Verlet* obj, double* dat, double e, double e_grid, double* f) {
	// evolve droplet
	for (unsigned int i=0; i<3; ++i) {
		R[i]=R[i] + dt*V[i] + dt*dt*f[i]/M;
		V[i]=V[i] + dt*f[i]/M; // velocity must be updated, too!
		e+=0.5*M*V[i]*V[i]; // energy --- and the potential energy??

		if (i<dim) {
			*dat=R[i];
			++dat;
		}
	}
	*dat=e; ++dat;

	// evolve grid
	e_grid=obj->Step();
	*dat=e_grid; ++dat;
	
	return dat;
}

double Droplet::F(double alpha, double dist) {
	return alpha/(dist*dist);
}

double* Droplet::TheForce(Verlet* obj, int* rr, double* FF) {
	//cout << "================================" << '\n';
	for (int dx=-cutoff; dx<=cutoff; ++dx) {
		// take care of boundary - grid masses at the boundary do not exert a force
		if (((rr[0] + dx) <= 0) || ((rr[0] + dx) >= (N_[0]-1))) continue;
		for (int dy=-cutoff; dy<=cutoff; ++dy) {
			// take care of boundary - grid masses at the boundary do not exert a force
			if (((rr[1] + dy) <= 0) || ((rr[1] + dy) >= (N_[1]-1))) continue;			
			
			//cout << rr[0]+dx << "   " << rr[1]+dy << '\n';
			
			double* r_hat=new double[3]; // connecting vector
			double* vv=new double[3]; // velocity
			for (int i=0; i<3; ++i) {
				int index=obj->Index(rr[0]+dx, rr[1]+dy, rr[2], i);
				r_hat[i]=R[i] - obj->r1[index]; // these vectors point *from* the respective grid point *to* the droplet
				vv[i]=obj->rdot[index];
			}
			double norm=sqrt(this->Dot(r_hat, r_hat));
			double v_squared=this->Dot(vv, vv);
			r_hat=this->Normed(r_hat);
			
			for (int i=0; i<3; ++i) {
				FF[i]+=F(-1e-2/(v_squared + 1e0), norm)*r_hat[i];
				//cout << v_squared << '\t';
			} //cout << '\n';
		}
	}
	//cout << "================================" << '\n';
	return FF; // returns the total force
}

// underlying assumption: displacement of grid points is small enough such that only collisions with the nearest grid point actually occur
void Droplet::Evolve(Verlet* Obj, double* datarr) {
	for (unsigned int t=0; t<Steps; t++) {
		double E=0; // droplet energy --- should be initialized to 0: icc handles it anyway, g++ does not ...
		double E_grid=0; // grid energy
		
		// determine grid point nearest to droplet position
		int* r=new int[3];
		for (int i=0; i<3; ++i) {
			r[i]=round(R[i]/L); // NOTE: factor of 1/L
			}
			
		// ------------------------------------------------------------------------------------------------
		// force
		double* force=new double[3];
		for (int i=0; i<3; ++i) force[i]=0;
		//if (t > 30e3) force=this->TheForce(Obj, r, force);
		// ------------------------------------------------------------------------------------------------
		
		if (t == 30e3) V[0]=10.0;
		
		// ------------------------------------------------------------------------------------------------
		if (wall && stop_if_crashed) {
			if (r[0] == (N_[0]-1)) {
				cout << "Arrived!" << '\n';
				break;
			}
		}
		// ------------------------------------------------------------------------------------------------

		// exlcude collisions with outer grid points and make droplet stay in the box
		double* n=new double[3];

		double* n_wall=new double[3]; n_wall[0]=1; n_wall[1]=0; n_wall[2]=0;
		double* n_slit=new double[3]; n_slit[0]=0; n_slit[1]=1; n_slit[2]=0;

		if ((r[0]==N_[0]-1 || r[0]==0) || (r[1]==N_[1]-1 || r[1]==0) || ((N_[2]!=1) && (r[2]==N_[2]-1 || r[2]==0))) {
			for (int i=0; i<3; ++i) n[i]=min(L*(N_[i]-1) - R[i], R[i]); // n[2] is always zero... NOTE: factor of L
			if ((N_[2]==1) && (n[2]==0)) n[2]=max(n[0], n[1]) + 1; // just make n[2] the biggest
			double s=min(min(n[0], n[1]), n[2]);
			for (int i=0; i<3; ++i) {
				double temp=n[i];
				n[i]=0;
				if (temp == s) {
					n[i]=(1 - (N_[i]-1 - r[i])*2/(N_[i]-1))*fabs(temp); // orientation outwards
				}
			}

			// reflect
			if ((s <= 0) && (this->Dot(n, V) > 0)) {
				if (wall && stop_if_crashed) {
					this->Crashed();
					break;
				}
				this->Reflect(n); // second condition is to avoid that droplet gets stuck in the corner
			}

			if (wall) {
				// make sure particle cannot slip through at the boundary
				if ((L*left_face_pos <= R[0]) && (R[0] <= L*right_face_pos)) {
					if (stop_if_crashed) {
						this->Crashed();
						break;
					}
					this->Reflect(n_wall);
				}
			}

			// evolve
			datarr=this->Update(Obj, datarr, E, E_grid, force);
			continue;
		}

		// ------------------------------------------------------------------------------------------------
		// simplify this part
		if (wall) {
			if ((L*left_face_pos < R[0]) && (R[0] < L*right_face_pos)) { // between the wall faces

				if ((r[1] == slit_1_lower) || (r[1] == slit_1_upper)) {
					double s=min(R[1] - L*slit_1_lower, L*slit_1_upper - R[1]);
					if (r[1] == slit_1_lower) n_slit[1]=-1;

					// reflect
					if ((s <= 0) && (this->Dot(n_slit, V) > 0)) {
						cout << "Slit 1: " << R[1] << "   " << V[1] << '\n';
						if (stop_if_crashed) {
							this->Touched();
							//break;
						}
						this->Reflect(n_slit); // second condition is to avoid that droplet gets stuck in the corner
					}

					// evolve
					datarr=this->Update(Obj, datarr, E, E_grid, force);
					continue;
				}

				if ((r[1] == slit_2_lower) || (r[1] == slit_2_upper)) {
					double s=min(R[1] - L*slit_2_lower, L*slit_2_upper - R[1]);
					if (r[1] == slit_2_lower) n_slit[1]=-1;

					// reflect
					if ((s <= 0) && (this->Dot(n_slit, V) > 0)) {
						cout << "Slit 2: " << R[1] << "   " << V[1] << '\n';
						if (stop_if_crashed) {
							this->Touched();
							//break;
						}
						this->Reflect(n_slit); // second condition is to avoid that droplet gets stuck in the corner
					}

					// evolve
					datarr=this->Update(Obj, datarr, E, E_grid, force);
					continue;
				}

				if (!(((L*slit_1_lower < R[1]) && (R[1] < L*slit_1_upper)) || ((L*slit_2_lower < R[1]) && (R[1] < L*slit_2_upper)))) { // if not inside one of the slits

					// if the droplet touches the boundary and would be reflected, the given run can be discarded
					cout << "Crashed into the wall! "<< R[1]<< "   " << V[0] << "   " << R[0] << '\n';
					if (stop_if_crashed) {
						this->Crashed();
						break;
					}

					this->Reflect(n_wall);

					// evolve
					datarr=this->Update(Obj, datarr, E, E_grid, force);
					continue;
				}
			}
		}
		// ------------------------------------------------------------------------------------------------

		// determine actual position and velocity
		double* r_nearest=new double[3];
		double* rdot_nearest=new double[3];
		for (int i=0; i<3; ++i) {
			int index=Obj->Index(r[0], r[1], r[2], i);
			r_nearest[i]=Obj->r1[index];
			rdot_nearest[i]=Obj->rdot[index];
		}

		double temp=0;
///*	
		if ((t%800)==0) {
			//cout << "Excited! " << '\n';
			// make an excitation
			/*
			for (int i=-20; i<=20; ++i) {
				Obj->r1[52330 + N_[1]*i]+=L*1.0;
				Obj->r1[52331 + N_[1]*i]+=L*1.0;
				Obj->r1[52329 + N_[1]*i]+=L*1.0;
			}*/
			
			Obj->r1[52330]+=L*1.0; //30275, 52330, 10150
			//Obj->r1[213131]+=L*1.0;
		}
//*/		
		// ------------------------------------------------------------------------------------------------
		// check if it is time
		if (this->Hit(t)) {
			//cout << "Hit!" << "\n";
			//temp=0.2;
///*
			// make collision
			if (t > 30e3) rdot_nearest=this->Collide(m, r_nearest, rdot_nearest);

			// update r1 according to new velocity: r1=dt*rdot + r0
			for (int i=0; i<3; ++i) {
				int index=Obj->Index(r[0], r[1], r[2], i);
				Obj->rdot[index]=rdot_nearest[i];
				Obj->r1[index]=dt*rdot_nearest[i] + Obj->r0[index];
			}
//*/
			

/*
			for (int i=0; i<2; ++i) {
				int index=Obj->Index(r[0], r[1], r[2], i);
				//cout << index << "\n";
				Obj->r1[index]+=L*1.0;
			}
*/
		}
		// ------------------------------------------------------------------------------------------------

		// evolve
		datarr=this->Update(Obj, datarr, E, E_grid, force);
	}
}
// ------------------------------------------------------------------------------------------------
#endif
