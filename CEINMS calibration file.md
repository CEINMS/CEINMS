# CEINMS calibration file (CEINMS 0.21)

The new CEINMS calibration allows to create composite objective functions in the form of:
$$
f_{obj} = w_1f_1^{k_1}+w_2f_2^{k_2}+...+w_pf_p^{k_p}
$$
no limit is present regarding the number of n objective functions that can be added.

The following objective functions can currently be configure in the calibration XML file (CEINMS v0.20) 

## torqueErrorNormalised

This objective function calculates the error between the experimental (e.g., from inverse dynamics) and the predicted joint moments, normalised by the torque variance of the trial.
$$
f = \sqrt{\frac{1}{N\cdot var(\tau)}\sum_{n}^{N}(\hat{\tau}_n-\tau_n)^2}
$$
where, $$\hat\tau$$ is the experimental joint moment, $$\tau$$ is the predicted joint moment, $$N$$ is the number of data point, and $$var(\tau)$$ is the variance of the joint moment in the trial. $$f$$ is calculate for all trials and all degrees of freedom listed in the calibration file.

### XML configuration

```xml
 <torqueErrorNormalised>
     <targets>dofName1 dofName2</targets>
	<!--do not change weight or exponent-->
     <weight>1</weight>
     <exponent>1</exponent>
</torqueErrorNormalised>
```

You can use the keyword `all` in  the`<targets>` tag to select all the degrees of freedom available in the model. 

```xml
 <torqueErrorNormalised>
     <targets>all</targets>
	<!--do not change weight or exponent-->
     <weight>1</weight>
     <exponent>1</exponent>
</torqueErrorNormalised>
```


## maxPeak

This objective function calculates the maximum value of a target variable (state variable or output variable). 
$$
f = max(v_{1...N})
$$
 where $$v_{1...N}$$ is a time series with  $$N$$ points.

### XML configuration

Available `targetsType` are the following (apologies for the spelling in British English):

* `torque` the calculated joint moment

* `activation` the muscle activation calculated from the Excitation to Activation model

* `normalisedFibreLength` the calculated normalised length of the muscle fibres

* `pennationAngle` the calculated pennation angle of the muscle fibres

* `scalarContactForce` the contact force calculated with a planar knee mechanism (Winby et al.)

* `genericContactForce` the contact force calculated via OpenSim's joint reaction analysis in CEINMS

The `targets` are the name of the degrees of freedom, or the name of the muscles that relate to the `targetsType`, so if you want to mimimise the peak of activation during calibration, the `targetsName` tag needs to contain the names of all the muscles for which you want to minimise the peak o f activation. 

The following provides an example to minimise the hip contact force using a generic contact model.

```xml
<maxPeak>
    <targets>hip_r</targets>
    <targetsType>genericContactForce</targetsType>
    <weight>0.01</weight>
    <exponent>2</exponent>
</maxPeak>
```
The following provides an example to minimise the knee contact force using a planar model.

```xml
  <maxPeak>
      <targets>medial_condyle_r lateral_condyle_r</targets>
      <targetsType>scalarContactForce</targetsType>
      <weight>0.01</weight>
      <exponent>2</exponent>
  </maxPeak>
```



## penalty

This objective function is a simple implementation of Lagrangian multipliers to add constraints to the problem. 
$$
f = \left\{ 
\begin{array}{ll} 
	(\hat{x} - x)^k , x < \hat{x} \\
	(x- \hat{x})^k , x >  \hat{x} 
\end{array}
\right.
$$

### XML configuration

Available `targetsType` are the same available to **maxPeak**:

* `torque` the calculated joint moment

* `activation` the muscle activation calculated from the Excitation to Activation model

* `normalisedFibreLength` the calculated normalised length of the muscle fibres

* `pennationAngle` the calculated pennation angle of the muscle fibres

* `scalarContactForce` the contact force calculated with a planar knee mechanism (Winby et al.)

* `genericContactForce` the contact force calculated via OpenSim's joint reaction analysis in CEINMS

The following configuration penalises the values of normalised fibre length that are lower than 0.5 or greater than 1.5 for all muscles in the model.

```xml
<penalty>
    <targets>all</targets>
    <targetsType>normalisedFibreLength</targetsType>
    <weight>100</weight>
    <exponent>2</exponent>
    <range>0.5 1.5</range>
</penalty>
```

However, you can also personalised this penalty for specific muscles, if you have any a priori knowledge of how the muscle should behave (e.g., experimental data)

```xml
<penalty>
    <targets>soleus_r</targets>
    <targetsType>normalisedFibreLength</targetsType>
    <weight>100</weight>
    <exponent>2</exponent>
    <range>0.8 1.3</range>
</penalty>
```

## Combining multiple objective functions

You can combine any number of objective functions,

```xml
<objectiveFunction>
    <torqueErrorNormalised>
        <targets>all</targets>
        <!--  <targetsType></targetsType> -->
        <weight>1</weight>
        <exponent>1</exponent>
    </torqueErrorNormalised>
    <maxPeak>
        <targets>medial_condyle_r lateral_condyle_r</targets>
        <targetsType>scalarContactForce</targetsType>
        <weight>0.01</weight>
        <exponent>2</exponent>
    </maxPeak>
    <penalty>
        <targets>medial_condyle_r lateral_condyle_r</targets>
        <targetsType>scalarContactForce</targetsType>
        <weight>100</weight>
        <exponent>2</exponent>
        <range>100 10000</range>
    </penalty>
    <penalty>
        <targets>all</targets>
        <targetsType>normalisedFibreLength</targetsType>
        <weight>100</weight>
        <exponent>2</exponent>
        <range>0.6 1.4</range>
    </penalty>
</objectiveFunction>
```

