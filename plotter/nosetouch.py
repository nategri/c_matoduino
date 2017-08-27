from matplotlib import pyplot as plot

from numpy import array

class SimRun:
  def __init__(self, leftPts, rightPts):
    self.leftPts = array(leftPts)
    self.rightPts = array(rightPts)
    self.tickPts = [i for i in range(200)]

simRunList = []

leftPts = []
rightPts = []

with open('simulations.dat', 'r') as f:
  for line in f: 
    if line == "START\n":
      if len(leftPts) > 0:
        simRunList.append(SimRun(leftPts, rightPts))
      leftPts = []
      rightPts = []
      continue
    nums = [float(x) for x in line.split()]
    leftPts.append(nums[0])
    rightPts.append(nums[1])

leftAvg = array([0 for i in range(200)])
rightAvg = array([0 for i in range(200)])

for sim in simRunList:
  leftAvg += sim.leftPts
  rightAvg += sim.rightPts

leftAvg = leftAvg/len(simRunList)
rightAvg = rightAvg/len(simRunList)

plot.plot(simRunList[0].tickPts, leftAvg, 'b')
plot.plot(simRunList[0].tickPts, rightAvg, 'r')

plot.savefig('avgnosttouch.png')
