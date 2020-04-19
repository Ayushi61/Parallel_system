#!/bin/python

#Import libraries for simulation
import tensorflow as tf
import numpy as np
import sys
import time

#Imports for visualization
import PIL.Image
import horovod.tensorflow as hvd


hvd.init()

config = tf.ConfigProto()
config.gpu_options.allow_growth = True
config.gpu_options.visible_device_list = '0'

sess = tf.InteractiveSession() #CPU version
#sess = tf.InteractiveSession(config=config) #Use only for capability 3.0 GPU

def DisplayArray(a,rank ,fmt='jpeg' ,rng=[0,1]):
  """Display an array as a picture."""
  #print("printing n " + str(N))
  h=(float)(1)/N
  f_c=open("lake_c_%d.dat" %(rank),'w')
  for i in range(len(a)):
	for j in range(len(a[i])):
		f_c.write(str(i*h)+" "+str(j*h)+" " +str(a[i][j])+'\n')
  a = (a - rng[0])/float(rng[1] - rng[0])*255
  a = np.uint8(np.clip(a, 0, 255))
  with open("lake_py_%d.jpg" %(rank),"w") as f:
      PIL.Image.fromarray(a).save(f, "jpeg")

sess = tf.InteractiveSession()

# Computational Convenience Functions
def make_kernel(a):
  """Transform a 2D array into a convolution kernel"""
  a = np.asarray(a)
  a = a.reshape(list(a.shape) + [1,1])
  return tf.constant(a, dtype=1)

def simple_conv(x, k):
  """A simplified 2D convolution operation"""
  x = tf.expand_dims(tf.expand_dims(x, 0), -1)
  y = tf.nn.depthwise_conv2d(x, k, [1, 1, 1, 1], padding='SAME')
  return y[0, :, :, 0]

def laplace(x):
  """Compute the 2D laplacian of an array"""
#  5 point stencil #
  five_point =      [[0.0, 0.0, 0.0,  0.0, 0.0,  0.0, 0.0],
                    [0.0, 0.0, 0.0,  0.0, 0.0,  0.0, 0.0],
                    [0.0, 0.0, 0.0,  1.0, 0.0,  0.0, 0.0],
                    [0.0, 0.0, 1.00, -4., 1.00, 0.0, 0.0],
                    [0.0, 0.0, 0.0,  1.0, 0.0,  0.0, 0.0],
                    [0.0, 0.0, 0.0,  0.0, 0.0,  0.0, 0.0],
                    [0.0, 0.0, 0.0,  0.0, 0.0,  0.0, 0.0]]

#  9 point stencil #
  nine_point =      [[0.0, 0.0, 0.0,  0.0, 0.0,  0.0, 0.0],
                    [0.0, 0.0, 0.0,  0.0, 0.0,  0.0, 0.0],
                    [0.0, 0.0, 0.25, 1.0, 0.25, 0.0, 0.0],
                    [0.0, 0.0, 1.00, -5., 1.00, 0.0, 0.0],
                    [0.0, 0.0, 0.25, 1.0, 0.25, 0.0, 0.0],
                    [0.0, 0.0, 0.0,  0.0, 0.0,  0.0, 0.0],
                    [0.0, 0.0, 0.0,  0.0, 0.0,  0.0, 0.0]]
# 13 point stencil #
  thirteen_point=  [[0.0, 0.0, 0.0,  0.125, 0.0,  0.0, 0.0],
                    [0.0, 0.0, 0.0,  0.25,  0.0,  0.0, 0.0],
                    [0.0, 0.0, 0.0,  1.00,  0.0,  0.0, 0.0],
                    [0.125, 0.25, 1.0, -5.50, 1.0, 0.25, 0.125],
                    [0.0, 0.0, 0.0,  1.0,  0.0,  0.0, 0.0],
                    [0.0, 0.0, 0.0,  0.25,  0.0,  0.0, 0.0],
                    [0.0, 0.0, 0.0,  0.125,  0.0,  0.0, 0.0]]
						   
  laplace_k = make_kernel(thirteen_point)
  return simple_conv(x, laplace_k)

# Define the PDE
if len(sys.argv) != 4:
	print "Usage:", sys.argv[0], "N npebs num_iter"
	sys.exit()
	
N = int(sys.argv[1])
npebs = int(sys.argv[2])
num_iter = int(sys.argv[3])

# Initial Conditions -- some rain drops hit a pond

# Set everything to zero
u_init  = np.zeros([N, N], dtype=np.float32)
ut_init = np.zeros([N, N], dtype=np.float32)

# Some rain drops hit a pond at random points
for n in range(npebs):
  a,b = np.random.randint(0, N, 2)
  u_init[a,b] = np.random.uniform()

# Parameters:
# eps -- time resolution
# damping -- wave damping
eps = tf.placeholder(tf.float32, shape=())
damping = tf.placeholder(tf.float32, shape=())

# Create variables for simulation state
U  = tf.Variable(u_init)
Ut = tf.Variable(ut_init)

#recv 3 rows
U_rcv0=tf.Variable(np.zeros([3,N],dtype=np.float32))
Ut_rcv0=tf.Variable(np.zeros([3,N],dtype=np.float32))
U_rcv1=tf.Variable(np.zeros([3,N],dtype=np.float32))
Ut_rcv1=tf.Variable(np.zeros([3,N],dtype=np.float32))

#communicate rows for calculations
bcast=tf.group(tf.assign(U_rcv1,hvd.broadcast(U[N-3:N],0)),tf.assign(Ut_rcv1,hvd.broadcast(Ut[N-3:N],0)),tf.assign(U_rcv0,hvd.broadcast(U[0:3],1)),tf.assign(Ut_rcv0,hvd.broadcast(Ut[0:3],1)))

#combined results
U_comb=tf.Variable(np.zeros([N+3,N],dtype=np.float32))
Ut_comb=tf.Variable(np.zeros([N+3,N],dtype=np.float32))

#concat
concat0=tf.group(U_comb.assign(tf.concat([U,U_rcv0],0)),Ut_comb.assign(tf.concat([Ut,Ut_rcv0],0)))
concat1=tf.group(U_comb.assign(tf.concat([U_rcv1,U],0)),Ut_comb.assign(tf.concat([Ut_rcv1,Ut],0)))




# Discretized PDE update rules
U_ = U_comb + eps * Ut_comb
Ut_ = Ut_comb + eps * (laplace(U_comb) - damping * Ut_comb)



# Operation to update the state
step = tf.group(
  U_comb.assign(U_),
  Ut_comb.assign(Ut_))


#sliced output n*n
U_slice0=tf.group(U.assign(tf.slice(U_comb,[0,0],[N,N])))
Ut_slice0=tf.group(Ut.assign(tf.slice(Ut_comb,[0,0],[N,N])))

U_slice1=tf.group(U.assign(tf.slice(U_comb,[3,0],[N,N])))
Ut_slice1=tf.group(Ut.assign(tf.slice(Ut_comb,[3,0],[N,N])))


# Initialize state to initial conditions
tf.global_variables_initializer().run()



# Run num_iter steps of PDE
start = time.time()
for i in range(num_iter):
  # Step simulation
  bcast.run()
  if hvd.rank()==0:
	concat0.run()
  else:
   	concat1.run()
  step.run({eps: 0.06, damping: 0.03})
  if hvd.rank()==0:
        U_slice0.run()
	Ut_slice0.run()
  else:
        U_slice0.run()
	Ut_slice0.run()


end = time.time()
print('Elapsed time: {} seconds'.format(end - start))  
DisplayArray(U.eval(),hvd.rank(),rng=[-0.1, 0.1])
