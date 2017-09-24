from matplotlib import pyplot as plot

spdList = []

with open('raw_speed.out','r') as f:
  for line in f:
    spdList.append(float(line))

#print(spdList)
plot.hist(spdList, 40)
plot.savefig('rawspdhist.png')
