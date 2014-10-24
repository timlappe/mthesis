from numpy import *
from math import *
import matplotlib
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages

matplotlib.rcParams["agg.path.chunksize"]=20000

#---------------------------------------------------------------------------------
#---------------------------------------------------------------------------------

def AddInterval(bin_width,c,d,liste,weight,weights): # Interval [c,d]
	for k in xrange(int(floor(c/bin_width)),int(ceil(d/bin_width))):
		liste.append(k*bin_width + bin_width/2)
		weights.append(weight)
	return 0

#---------------------------------------------------------------------------------

constants=[]
'''
L=float(constants[4][2])
N=int(constants[0][2])
steps=int(constants[5][2])
resol=int(constants[6][2])
'''
N=0
L=0
bin_width=0

bins=50

burn_in=100000

loc_prob1=[]
weights1=[]
loc_prob2=[]
weights2=[]
loc_prob3=[]
weights3=[]

particle_positions=[]
particle_velocities=[]
lattice_velocities_pre=[]
#lattice_velocities_post=[]
with open('particle_data.txt') as f:
	counter=0
	initial=0
	for k, line in enumerate(f):
		if line.startswith("#"):
			strang=line.strip().split()
			constants.append(strang)
		else:	
			string=line.strip().split()
			particle_positions.append(float(string[2])) # 1 for real positions, 2 for indices
			particle_velocities.append(float(string[3]))
			lattice_velocities_pre.append(float(string[5]))
			#lattice_velocities_post.append(float(string[5]))
		
			if k < (counter*int(constants[5][2]) + burn_in + 9):
				continue
				
			if ((k-9) % int(constants[5][2])) == 0:
				counter+=1
				continue
			#print k
				
			b=int(string[0])
			final=float(string[1])
			v=float(string[3])
			
			weight=1/abs(v)
			
			if initial == 0:
				initial=final
				continue
				
			#---------------------------------------------------------------------------------
			N=int(constants[0][2])
			L=float(constants[4][2])
			bin_width=(N+1)*L/bins
			if b < 0:
				if (b % 2) != 0: # ungerade
					AddInterval(bin_width,0,initial,loc_prob2,weight,weights2)
					AddInterval(bin_width,0,final,loc_prob2,weight,weights2)
					for k in xrange(0,abs(b)-1): # ganze Strecken
						AddInterval(bin_width,0,(N+1)*L,loc_prob2,weight,weights2)
			
				else: # gerade
					AddInterval(bin_width,0,initial,loc_prob2,weight,weights2)
					AddInterval(bin_width,final,(N+1)*L,loc_prob2,weight,weights2)
					for k in xrange(0,abs(b)-1): # ganze Strecken
						AddInterval(bin_width,0,(N+1)*L,loc_prob2,weight,weights2)
			
	
			if b > 0:
				if (b % 2) != 0: # ungerade
					AddInterval(bin_width,initial,(N+1)*L,loc_prob3,weight,weights3)
					AddInterval(bin_width,final,(N+1)*L,loc_prob3,weight,weights3)
					for k in xrange(0,abs(b)-1): # ganze Strecken
						AddInterval(bin_width,0,(N+1)*L,loc_prob3,weight,weights3)
			
				else: # gerade
					AddInterval(bin_width,initial,(N+1)*L,loc_prob3,weight,weights3)
					AddInterval(bin_width,0,final,loc_prob3,weight,weights3)
					for k in xrange(0,abs(b)-1): # ganze Strecken
						AddInterval(bin_width,0,(N+1)*L,loc_prob3,weight,weights3)
			
			if b == 0:
				if initial < final:
					AddInterval(bin_width,initial,final,loc_prob1,weight,weights1)
				else:
					AddInterval(bin_width,final,initial,loc_prob1,weight,weights1)
			#---------------------------------------------------------------------------------
			
			# Nicht vergessen:
			initial=final
	f.close()
	
constants=constants[:-2]
print constants

#---------------------------------------------------------------------------------
#---------------------------------------------------------------------------------

pp=PdfPages('output.pdf')		

#---------------------------------------------------------------------------------
# histogram
fig1=plt.figure()
plt.hist([loc_prob1,loc_prob2,loc_prob3], bins=[q for q in arange(0,(N+1)*L + bin_width,bin_width)], normed=False, weights=[weights1,weights2,weights3], stacked=True)
plt.title('Location probability')
plt.xlabel('pos')
plt.ylabel('#')
plt.grid(True)
#plt.ylim(0.0, 0.7e+5)
pp.savefig(fig1)
#plt.clf()

#---------------------------------------------------------------------------------
time_plots=1
time_start=0
time_end=time_start + 500
markersize=3

scatter_plots=0
scatter_start=800000
scatter_end=900000
#---------------------------------------------------------------------------------

if time_plots==1:
	#---------------------------------------------------------------------------------
	# positions
	y_axis=particle_positions[time_start:time_end]	
	t_axis=xrange(0, len(y_axis))

	fig2=plt.figure(figsize=(40,5))
	plt.title('Trajectory')
	plt.xlabel('t')
	plt.ylabel('pos')
	plt.plot(t_axis,y_axis,marker='o',markersize=markersize)
	#plt.axis([time_start,time_end,0,1])
	plt.grid(True)
	pp.savefig(fig2)

	#---------------------------------------------------------------------------------
		# positions
	y_axis=particle_positions[time_end:2*time_end]	
	t_axis=xrange(0, len(y_axis))

	fig2=plt.figure(figsize=(40,5))
	plt.title('Trajectory')
	plt.xlabel('t')
	plt.ylabel('pos')
	plt.plot(t_axis,y_axis,marker='o',markersize=markersize)
	#plt.axis([time_start,time_end,0,1])
	plt.grid(True)
	pp.savefig(fig2)

	#---------------------------------------------------------------------------------
		# positions
	y_axis=particle_positions[2*time_end:3*time_end]	
	t_axis=xrange(0, len(y_axis))

	fig2=plt.figure(figsize=(40,5))
	plt.title('Trajectory')
	plt.xlabel('t')
	plt.ylabel('pos')
	plt.plot(t_axis,y_axis,marker='o',markersize=markersize)
	#plt.axis([time_start,time_end,0,1])
	plt.grid(True)
	pp.savefig(fig2)

	#---------------------------------------------------------------------------------
	# velocities
	y_axis=particle_velocities[time_start:time_end]
	t_axis=xrange(0, len(y_axis))

	fig3=plt.figure(figsize=(40,5))
	plt.title('Velocity (post)')
	plt.xlabel('t')
	plt.ylabel('v')
	plt.plot(t_axis,y_axis)
	plt.grid(True)
	pp.savefig(fig3)

	#---------------------------------------------------------------------------------
		# velocities
	y_axis=particle_velocities[time_end:2*time_end]
	t_axis=xrange(0, len(y_axis))

	fig3=plt.figure(figsize=(40,5))
	plt.title('Velocity (post)')
	plt.xlabel('t')
	plt.ylabel('v')
	plt.plot(t_axis,y_axis)
	plt.grid(True)
	pp.savefig(fig3)

	#---------------------------------------------------------------------------------
		# velocities
	y_axis=particle_velocities[2*time_end:3*time_end]
	t_axis=xrange(0, len(y_axis))

	fig3=plt.figure(figsize=(40,5))
	plt.title('Velocity (post)')
	plt.xlabel('t')
	plt.ylabel('v')
	plt.plot(t_axis,y_axis)
	plt.grid(True)
	pp.savefig(fig3)

	#---------------------------------------------------------------------------------
	# lattice velocities pre and post
	y_axis_1=lattice_velocities_pre[time_start:time_end]	
	#y_axis_2=lattice_velocities_post
	t_axis=xrange(0, len(y_axis_1))

	fig3=plt.figure(figsize=(40,5))
	plt.title('Current lattice velocity')
	plt.xlabel('t')
	plt.ylabel('xdot')
	plt.plot(t_axis,y_axis_1)
	#plt.plot(t_axis,y_axis_2)
	plt.grid(True)
	pp.savefig(fig3)

	#---------------------------------------------------------------------------------

#---------------------------------------------------------------------------------

if scatter_plots==1:
	#---------------------------------------------------------------------------------
	# recursion plot positions
	liste1=particle_positions[:-1]
	liste2=particle_positions[1:]
	liste1=liste1[scatter_start:scatter_end]
	liste2=liste2[scatter_start:scatter_end]

	fig4=plt.figure()
	plt.scatter(liste1,liste2,s=0.1)
	plt.title('Recursion plot - positions of collisions')
	plt.xlabel('pos_i')
	plt.ylabel('pos_i+1')
	plt.axis([0,1,0,1])
	plt.grid(True)
	pp.savefig(fig4)
	#---------------------------------------------------------------------------------
	# recursion plot velocities
	liste1=particle_velocities[:-1]
	liste2=particle_velocities[1:]
	liste1=liste1[scatter_start:scatter_end]
	liste2=liste2[scatter_start:scatter_end]

	fig5=plt.figure()
	plt.title('Recursion plot - velocities (post)')
	plt.xlabel('v_i')
	plt.ylabel('v_i+1')
	plt.scatter(liste1,liste2,s=0.1)
	plt.grid(True)
	pp.savefig(fig5)
	#---------------------------------------------------------------------------------
	# recursion plot lattice velocities
	liste1=lattice_velocities_pre[:-1]
	liste2=lattice_velocities_pre[1:]
	liste1=liste1[scatter_start:scatter_end]
	liste2=liste2[scatter_start:scatter_end]

	fig6=plt.figure()
	plt.scatter(liste1,liste2,s=0.1)
	plt.title('Recursion plot - velocity of current lattice point (post)')
	plt.xlabel('xdot_i')
	plt.ylabel('xdot_i+1')
	plt.grid(True)
	pp.savefig(fig6)
	#---------------------------------------------------------------------------------
	# scatter plot mixed velocities
	liste1=particle_velocities
	liste2=lattice_velocities_pre
	liste1=liste1[scatter_start:scatter_end]
	liste2=liste2[scatter_start:scatter_end]

	fig6=plt.figure()
	plt.scatter(liste1,liste2,s=0.1)
	plt.title('Scatter plot - velocity of particle (post) and current lattice mass')
	plt.xlabel('v_i')
	plt.ylabel('xdot_i')
	plt.grid(True)
	pp.savefig(fig6)
#---------------------------------------------------------------------------------

pp.close()

#---------------------------------------------------------------------------------
#---------------------------------------------------------------------------------
