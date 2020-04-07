import matplotlib.pyplot as plt
import scipy
import seaborn as sns
import pandas as pd
import numpy as np

fs = 7

def plotComparison(ax, Y, YP):
    ax.scatter(Y.flatten(), YP.flatten(), s=0.2, facecolors='k', edgecolors='none', alpha=0.1)
   # ax.plot([0, 1], [intercept, slope+intercept], color='k', linewidth=1)
   # regText = "R = {:.2f}\nRMSE = {:.4f} ".format(r_value, RMSE)
   # print(regText)
   # ax.annotate(s=regText, xy=(0.8,0.1), fontsize=fs, xycoords='axes fraction')
   # ax.axis('equal') 
    diffY = Y.flatten() - YP.flatten()
    rmse = np.sqrt((diffY**2.0).mean())
    slope, intercept, r_value, p_value, std_err = scipy.stats.linregress(Y.flatten(),YP.flatten())
    regText = "R = {:.2f}\nRMSE = {:.4f} ".format(r_value, rmse)
    print(regText)
    ax.annotate(s=regText, xy=(0.5,0.1), fontsize=fs, xycoords='axes fraction')
    ax.set_xlim([0,0.2])
    ax.set_ylim([0,0.2])
    ax.set_xlabel("Finite element model strain", fontsize=fs)
    ax.tick_params(axis='both', which='major', labelsize=fs)
    sns.despine(fig=None, ax=ax, right=True, top=True)

def plotFullRange(ax, YP):
    x = np.array(range(YP.shape[0]))
    x = x/x.max()*2.0
    ax.plot(YP[:,::3], x, linewidth=0.1, color='k', alpha=0.1)
    ax.tick_params(axis='both', which='major', labelsize=fs)

def runLeaveOneOutAnalysis():
    Y = np.array(pd.read_csv('data/yvalues.csv', header=None))
    YP_plsr = np.array(pd.read_csv('data/leave1out_prediction_YP_plsr.csv', header=None))
    YP_cubic = np.array(pd.read_csv('data/leave1out_prediction_YP_cubic.csv', header=None))
    YP_basis = np.array(pd.read_csv('data/leave1out_prediction_YP_basis.csv', header=None))

    
    fig, ax = plt.subplots(1,3,sharex=True, sharey=True, figsize=(8,3))
    plotComparison(ax[0], Y, YP_plsr)
    ax[0].set_ylabel("Surrogate model strain", fontsize=fs)
    ax[0].set_title("Partial Least Square regression", fontsize=fs)

    plotComparison(ax[1], Y, YP_cubic)
    ax[1].set_title("Cubic spline interpolation", fontsize=fs)
    plotComparison(ax[2], Y, YP_basis)
    ax[2].set_title("Basis spline interpolation", fontsize=fs)
   # fig.suptitle("Leave one out fitting", fontsize=fs)
    fig.savefig('LeaveOneOut.tiff', format='tiff', dpi=1200, pil_kwargs={"compression": "tiff_lzw"})
  #  fig.savefig('LeaveOneOut.tiff', format='tiff', dpi=1200)
   
    plt.show()

def runFullRangeAnalysis():
    YP_plsr = np.array(pd.read_csv('data/full_prediction_YP_plsr.csv', header=None))
    YP_cubic = np.array(pd.read_csv('data/full_prediction_YP_cubic.csv', header=None))
    YP_basis = np.array(pd.read_csv('data/full_prediction_YP_basis.csv', header=None))

    fig, ax = plt.subplots(1,3,sharex=True, sharey=True, figsize=(8,3))
    plotFullRange(ax[0], YP_plsr)
    plotFullRange(ax[1], YP_cubic)
    plotFullRange(ax[2], YP_basis)
    ax[0].set_ylabel("tendon force MVC normalised", fontsize=fs)
    ax[0].set_xlabel("Principal component strain", fontsize=fs)
    ax[0].set_title("PLS", fontsize=fs)
    ax[1].set_xlabel("Principal component strain", fontsize=fs)
    ax[1].set_title("Cubic Spline", fontsize=fs)
    ax[2].set_xlabel("Principal component strain", fontsize=fs)
    ax[2].set_title("Basis Spline", fontsize=fs)
    ax[0].set_xlim([0,0.2])
 
    fig.suptitle("Force strain relationship for each Gauss point in the model", fontsize=fs)
    plt.show()

runFullRangeAnalysis()
runLeaveOneOutAnalysis()
