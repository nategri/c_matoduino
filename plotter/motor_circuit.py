from matplotlib import pyplot as plot

from numpy import array
from numpy import arange
from numpy import mean

class SimRun:
  def __init__(self, leftPts, rightPts):
    self.leftPts = array([float(x) for x in leftPts])
    self.rightPts = array([float(x) for x in rightPts])
    leftPts = array([1.0e-6 if x == 0 else x for x in self.leftPts])

    self.tickPts = [i for i in range(200)]

    self.ratioPts = self.rightPts / leftPts

simRunList = []

leftPts = []
rightPts = []

with open('./motor_circuits/simulations.out', 'r') as f:
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

# Grab final non-"START\n" terminated run
simRunList.append(SimRun(leftPts, rightPts))

leftAvg = array([0 for i in range(200)])
rightAvg = array([0 for i in range(200)])

for sim in simRunList:
  leftAvg += sim.leftPts
  rightAvg += sim.rightPts

leftAvg = leftAvg/float(len(simRunList))
rightAvg = rightAvg/float(len(simRunList))

print("TOT. SIM RUNS")
print(len(simRunList))

#plot.plot(simRunList[0].tickPts, leftAvg, 'b')
#plot.plot(simRunList[0].tickPts, rightAvg, 'r')

#plot.plot(simRunList[0].tickPts, simRunList[0].leftPts, 'b')
#plot.plot(simRunList[0].tickPts, simRunList[0].rightPts, 'r')

#print(leftAvg)

#plot.plot(simRunList[0].tickPts, rightAvg/leftAvg, 'g')
#for i in range(len(simRunList)):
#plot.plot(simRunList[0].tickPts, simRunList[45].rightPts/simRunList[45].leftPts, 'r')


threshRatioList = arange(1, 7.0+0.1, 0.1)

chemResList = []
noseResList = []

for threshRatio in threshRatioList:
  chemExceedList = []
  noseExceedList = []

  for sim in simRunList:
    chemExceedNum = 0.0
    noseExceedNum = 0.0
    for i in range(0, 100):
      if sim.ratioPts[i] > threshRatio:
        chemExceedNum += 1.0
    for i in range(100, 200):
      if sim.ratioPts[i] > threshRatio:
        noseExceedNum += 1.0
    chemExceedList.append(chemExceedNum)
    noseExceedList.append(noseExceedNum)

  chemResList.append(mean(chemExceedList))
  noseResList.append(mean(noseExceedList))

plot.title('Avg. Number of Neural Cycles Above A/B Motor Neuron Ratio Reversal Thresh.\n(100 Simulations)\n')
p1, = plot.plot(threshRatioList, chemResList, 'ob', label='During Chemotaxis')
p2, = plot.plot(threshRatioList, noseResList, 'or', label='Post-Nose Touch')
p3, = plot.plot(threshRatioList, array(noseResList)-array(chemResList), 'og', label='Difference')
plot.legend([p1, p2, p3], ['During Chemotaxis', 'Post-Nose Touch', 'Difference'])

plot.xlabel('Trial Threshold Value')
plot.ylabel('Avg. Cycles Above Threshold')

plot.tight_layout()

plot.savefig('avgmotorcircuit.png')
