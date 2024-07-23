import matplotlib.pyplot as plt
import numpy as np

secret0 = np.fromfile("ctswap_s0-1.txt", dtype=int, sep="\n")
secret1 = np.fromfile("ctswap_s1-1.txt", dtype=int, sep="\n")

plt.hist(secret0, bins=70, alpha=0.5, label='secret=0', range=(500, 1000))
plt.hist(secret1, bins=70, alpha=0.5, label='secret=1', range=(500, 1000))

plt.xlabel('Latency (Cycles)')
plt.ylabel('Frequency')
plt.legend()

plt.show()
