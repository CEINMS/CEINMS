Preparing your experimental data
################################

- to use ceinms you need: emg, lmt, ma, external torques (see conf files and ceinms/ceinms-calibrate)
- Collect data (suggested: static pose to scale subject, mandatory for now: MVCs)
- Export in c3d
- Use MoToNMS
- [ Specify here the folder structure that we intend to use ]
- Scale subject (provide sample setup.xml or link to Opensim docs and, when the time is ripe, to the muscleOptimizer)
- convert subject using osimToXML (need a template with the model you are studying?)
- use claudio's scripts to:
    - perform ik with opensim
    - perform id with opensim
    - perform muscle analysis with opensim
    - prepare inputData.xml

**Don't forget to sanity-check your results!**