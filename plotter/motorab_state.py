from matplotlib import pyplot as plot

class SimRun:
  def __init__(self):
    self.aVals = []
    self.bVals = []
    self.aTickList = []
    self.bTickList = []

  def add(self, entry, tickNum):
    entryList = list(entry)
    if entryList[0] == 'A':
      self.aVals.append(-1*int(entry[1:]))
      self.aTickList.append(tickNum)
    elif entryList[0] == 'B':
      self.bVals.append(int(entry[1:]))
      self.bTickList.append(tickNum)

  def getSpecials(self):
    specialValList = []
    specialTickList = []
    for i in range(len(self.bVals)):
      if self.bVals[i] in [18, 11, 10, 9]:
        specialValList.append(self.bVals[i])
        specialTickList.append(self.bTickList[i])
    for i in range(len(self.aVals)):
      if self.aVals[i] in [-11, -10, -14]:
        specialValList.append(self.aVals[i])
        specialTickList.append(self.aTickList[i])

    return specialTickList, specialValList

  def getStates(self):
    stateList = []
    for i in range(200):
      stateDict = {}
      aList = [self.aVals[j] for j in range(len(self.aTickList)) if self.aTickList[j] == i]
      bList = [self.bVals[j] for j in range(len(self.bTickList)) if self.bTickList[j] == i]
      for a in aList:
        stateDict['A'+str(-1*a)] = 1
      for b in bList:
        stateDict['B'+str(b)] = 1
      stateList.append(stateDict)

    return stateList
        

simRunList = []

currSimRun = SimRun()
currTick = -1

with open('motorab_state.out', 'r') as f:
  for line in f:
    if line == "START\n" and len(currSimRun.aTickList) > 0:
      simRunList.append(currSimRun)
      currTick = -1
      currSimRun = SimRun()
    elif line == "TICK\n":
      currTick += 1
    else:
      currSimRun.add(line, currTick)

"""p1, = plot.plot(simRunList[0].bTickList, simRunList[0].bVals, 'ob', label='B Motor Neuron')
p2, = plot.plot(simRunList[0].aTickList, simRunList[0].aVals, 'or', label='A Motor Neuron')
plot.legend([p1, p2], ['B Motor Neuron', 'A Motor Neuron'])
plot.title("Motor Neuron Activity\n(Chemotaxis first half, nose-touch second half)")
plot.ylabel("Individual Neurons")
plot.xlabel("Neural Timesteps")"""

x, y = simRunList[0].getSpecials()

print("NUM STATES")
print(len(set(simRunList[0].bTickList)))

avg = 1.0
n = 5.0
avgList = []
statesList = simRunList[0].getStates()
for state in statesList:
  val = 0
  try:
    val += state['B12']
  except KeyError:
    pass
  try:
    val += state['B13']
  except KeyError:
    pass
  try:
    val += state['B9']
  except KeyError:
    pass
  try:
    val += state['B10']
  except KeyError:
    pass
  try:
    val += state['B11']
  except KeyError:
    pass
  try:
    val += -1*state['A10']
  except KeyError:
    pass
  try:
    val += -1*state['A11']
  except KeyError:
    pass
  
  avg = (val + (n*avg))/(n + 1)

  avgList.append(avg)

plot.plot([i for i in range(len(avgList))], avgList, 'b*')
plot.axvline(x=100.0, color='r')
#plot.plot(x, y, 'om')

plot.show()
