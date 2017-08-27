import copy

from matplotlib import pyplot as plot

from numpy import array
from numpy import vstack
from numpy import ones

from numpy.linalg import lstsq

title = ""

dorsalWts = []
ventralWts = []

stateList = []

class State:
  def __init__(self, dorsalWts, ventralWts):
    self.leftDorsalWts = [x[0] for x in dorsalWts]
    self.rightDorsalWts = [x[1] for x in dorsalWts]

    self.leftVentralWts = [x[0] for x in ventralWts]
    self.rightVentralWts = [x[1] for x in ventralWts]

N = 0

with open('./sim_data/sim4.out', 'r') as f:
  for line in f:
    if line == "START SIM\n":
      title = "chemotaxis"
    elif line == "NOSE TOUCH\n":
      title = "nose touch"
    elif line[:4] == "TICK":
      if len(dorsalWts) > 0:
        if N == 10:
          break
        stateList.append(State(dorsalWts, ventralWts))
        dorsalWts = []
        ventralWts = []
        N += 1
    else:
      if len(dorsalWts) < 17:
        dorsalWts.append([float(x) for x in line.split()])
      else:
        ventralWts.append([float(x) for x in line.split()])

  stateList.append(State(dorsalWts, ventralWts))

mLeftList = []
mRightList = []

for i in range(len(stateList)):
  visArray = zip(stateList[i].leftDorsalWts, stateList[i].rightDorsalWts, stateList[i].leftVentralWts, stateList[i].rightVentralWts)
  plot.clf()

  """x = [j for j in range(len(stateList[i].leftDorsalWts))]
  xMat = vstack([x, ones(len(x))]).T
  mLeftDorsal, c = lstsq(xMat, stateList[i].leftDorsalWts)[0]
  mLeftVentral, c = lstsq(xMat, stateList[i].leftVentralWts)[0]
  mRightDorsal, c = lstsq(xMat, stateList[i].rightDorsalWts)[0]
  mRightVentral, c = lstsq(xMat, stateList[i].rightVentralWts)[0]

  mLeft, c = lstsq(xMat, array(stateList[i].leftVentralWts)+array(stateList[i].leftDorsalWts))[0]
  mRight, c = lstsq(xMat, array(stateList[i].rightVentralWts)+array(stateList[i].rightDorsalWts))[0]

  mLeftList.append(mLeft)
  mRightList.append(mRight)

  #print(i, mLeftDorsal, mLeftVentral, mRightVentral, mRightDorsal)
  print(i, mLeft, mRight)"""

  a = plot.imshow(visArray, vmin=-80, vmax=80, interpolation='none', cmap='seismic')
  a.axes.get_xaxis().set_ticks([])
  a.axes.get_yaxis().set_ticks([])
  plot.xlabel('Dorsal Ventral')
  plot.ylabel('Posterior                                           Anterior')
  plot.colorbar()
  numStr = str(i).zfill(2)
  plot.title('Muscle Weights\nNeural Tick ' + numStr + '\n')
  plot.tight_layout()
  plot.savefig('images/muscles'+numStr+'.png')

"""plot.clf()
plot.plot([i for i in range(len(stateList))], mLeftList, 'b')
plot.plot([i for i in range(len(stateList))], mRightList, 'r')
plot.savefig('muscgrad.png')"""
