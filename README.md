Useful links:
[OpenSim 3.2 VC12P x86](https://drive.google.com/file/d/0BzMAK5L0QV2PSEkxSUVxSXd3NzA/edit?usp=sharing)

When compiling with OpenSim3.2 in Windows, the following errors may occur:
```
Error	1	error C3646: '__attribute__' : unknown override specifier	c:\opensim3.2\sdk\include\opensim\common\DebugUtilities.h	42	1	Calibration
Error	2	error C2065: 'noreturn' : undeclared identifier	c:\opensim3.2\sdk\include\opensim\common\DebugUtilities.h	42	1	Calibration
Error	3	error C2072: 'OpenSim::DebugUtilities::Fatal_Error' : initialization of a function	c:\opensim3.2\sdk\include\opensim\common\DebugUtilities.h	42	1	Calibration
Error	4	error C2146: syntax error : missing ';' before identifier 'WINAPI'	c:\opensim3.2\sdk\include\opensim\common\LoadOpenSimLibrary.h	43	1	Calibration
Error	5	error C4430: missing type specifier - int assumed. Note: C++ does not support default-int	c:\opensim3.2\sdk\include\opensim\common\LoadOpenSimLibrary.h	43	1	Calibration
Error	6	error C2146: syntax error : missing ';' before identifier 'LoadOpenSimLibrary'	c:\opensim3.2\sdk\include\opensim\common\LoadOpenSimLibrary.h	43	1	Calibration
```

If so, be sure that _WIN32 is used instead of WIN32 in the define directives in OpenSim include files DebugUtilities.h, LoadOpenSimLibrary.h, osimCommonDLL.h (i.e. look where the errors are and change WIN32 in _WIN32 everywhere) 
