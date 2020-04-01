import matplotlib.pyplot as plt
import scipy
import seaborn as sns
import pandas as pd
import numpy as np

fs = 7

def plotComparison(ax, Y, YP):
    ax.scatter(Y.flatten(), YP.flatten(), s=0.1, color='k', alpha=0.05)
   # slope, intercept, r_value, p_value, std_err = scipy.stats.linregress(Y.flatten(),YP.flatten())
   # ax.plot([0, 1], [intercept, slope+intercept], color='k', linewidth=1)
   # regText = "R = {:.2f}\nRMSE = {:.4f} ".format(r_value, RMSE)
   # print(regText)
   # ax.annotate(s=regText, xy=(0.8,0.1), fontsize=fs, xycoords='axes fraction')
    ax.axis('equal') 
    ax.set_xlim([0,0.2])
    ax.set_ylim([0,0.2])
    ax.set_xlabel("Finite element model strain", fontsize=fs)
    ax.set_ylabel("PLS predicted strain", fontsize=fs)
    ax.tick_params(axis='both', which='major', labelsize=fs)

def plotFullRange(ax, YP):
    x = range(YP.shape[0])
    ax.plot(YP[:,::20], x, linewidth=0.3, color='k', alpha=0.9)

Y = np.array(pd.read_csv('data/yvalues.csv', header=None))
YP_plsr = np.array(pd.read_csv('data/node_prediction_YP_plsr.csv', header=None))
YP_cubic = np.array(pd.read_csv('data/node_prediction_YP_cubic.csv', header=None))

fig = plt.figure(figsize=(3,6))
ax1 = fig.add_subplot(211)
ax2 = fig.add_subplot(212)

#plotComparison(ax1, Y, YP_plsr)
#plotComparison(ax2, Y, YP_cubic)


YP_full_plsr = np.array(pd.read_csv('data/full_prediction_YP_plsr.csv', header=None))
YP_full_cubic = np.array(pd.read_csv('data/full_prediction_YP_cubic.csv', header=None))
figg = plt.figure(figsize=(3,6))
axx1 = fig.add_subplot(211)
axx2 = fig.add_subplot(212)
plotFullRange(axx1, YP_full_plsr)
plotFullRange(axx2, YP_full_cubic)

#fig.savefig('Figure_PLS.tif', format='tif', dpi=1200)
plt.show()
