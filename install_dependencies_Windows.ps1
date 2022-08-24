# Create dependencies folder and move into folder
param($DependenciesDir = ".." , [switch]$IsCompileServer )

Set-Location $PSScriptRoot # Move current working directory to directory of this script

if ($DependenciesDir -eq "..") {
    $DependenciesDir = Split-Path -Path $PSScriptRoot -Parent
}

$DependenciesDir = $DependenciesDir + "\ceinms_dependencies"
if (!(Test-Path $DependenciesDir -PathType container)) {
    New-Item -Path $DependenciesDir -ItemType directory
}
cd $DependenciesDir

# Install Chocolately
$pathChocolately = "C:\\ProgramData\\chocolatey"
if (Test-Path $pathChocolately -PathType container) {
    Write-Host "Chocolately found."
} else {
    Write-Host "Chocolately NOT found."
    Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
}

# Sometimes env vars are not reloaded between CI builds
$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")

# Refreash environment variables after installing Chocolately
$env:ChocolateyInstall = Convert-Path "$((Get-Command choco).Path)\..\.."   
Import-Module "$env:ChocolateyInstall\helpers\chocolateyProfile.psm1"
refreshenv

# Install CMake
choco install --limit-output --no-progress --yes cmake --installargs 'ADD_CMAKE_TO_PATH=System'

# Install Git and 7Zip
choco install --limit-output --no-progress --yes git 7zip.install
refreshenv # Refresh environment variables so that Git can be used

# Install Visual Studio 2019 (Community or BuildTools version)
$pathVisualStudio = "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\BuildTools"
if (Test-Path $pathVisualStudio -PathType container) {
    Write-Host "Visual Studio BuildTools 2019 found."
} else {
    $pathVisualStudio = "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community"
    if (Test-Path $pathVisualStudio -PathType container) {
        Write-Host "Visual Studio Community 2019 found."
    } else {

        # Only install build tools, no need for IDE.
        if ($IsCompileServer) {
            Write-Host "Installing Visual Studio 2019 BuildTools."
            Invoke-WebRequest -Uri https://aka.ms/vs/16/release/vs_buildtools.exe -OutFile vs_buildtools.exe
            $process = Start-Process -FilePath vs_buildtools.exe -ArgumentList "--add", "Microsoft.Component.MSBuild", "--add", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "--add", "Microsoft.VisualStudio.Component.VC.CMake.Project", "--add", "Microsoft.VisualStudio.Workload.VCTools", "--add", "Microsoft.VisualStudio.Component.VC.ATL", "--add", "Microsoft.VisualStudio.Component.Windows10SDK.18362", "--add", "Microsoft.VisualStudio.Component.VC.14.28.16.9.x86.x64", "--passive", "--norestart", "--wait" -Wait -PassThru
            Write-Output $process.ExitCode
        } else {
            Write-Host "Installing Visual Studio 2019 Community."

            # TODO: test this includes all components needed.
            Invoke-WebRequest -Uri https://aka.ms/vs/16/release/vs_community.exe -OutFile vs_community.exe
            $process = Start-Process -FilePath vs_community.exe -ArgumentList  "--add", "Microsoft.VisualStudio.Product.Community", "--add", "Microsoft.VisualStudio.Workload.VCTools", "--add", "Microsoft.VisualStudio.Component.VC.ATL", "--add", "Microsoft.VisualStudio.Component.Windows10SDK.18362", "--add", "Microsoft.VisualStudio.Component.VC.14.28.16.9.x86.x64", "--passive", "--norestart", "--wait" -Wait -PassThru
            Write-Output $process.ExitCode
        }
    }
}
refreshenv

# Install OpenSim
where.exe /Q simbody-visualizer.exe 2>$null
if( $LASTEXITCODE -eq 0 ) {
    $pathOpenSim = where.exe simbody-visualizer.exe
    $pathOpenSim = Split-Path -Path $pathOpenSim -Parent
    $pathOpenSim = Split-Path -Path $pathOpenSim -Parent
    Write-Host "Found OpenSim at: " $pathOpenSim
} else {
    Write-Host "Could NOT find OpenSim. Installing..."

    choco install --limit-output --no-progress --yes jdk8 swig

    refreshenv
          
    cd $DependenciesDir
    mkdir opensim_4_3
    cd opensim_4_3
    git clone -q https://github.com/opensim-org/opensim-core/tree/4.3 
    mkdir opensim_dependencies_build
    cd .\opensim_dependencies_build
    cmake ..\opensim-core\dependencies                               `
        -G"Visual Studio 16 2019"                                    `
        -A"x64"													                             `
        -DCMAKE_INSTALL_PREFIX="..\opensim_dependencies_install"     `
    -DSUPERBUILD_ezc3d=ON
    cmake --build . --config RelWithDebInfo -- /maxcpucount:4
    cd ..
    mkdir opensim_build
    cd .\opensim_build
    cmake ..\opensim-core                                            `
        -G"Visual Studio 16 2019"                                    `
        -A"x64"													                             `
        -DCMAKE_INSTALL_PREFIX="..\opensim_install"                  `
        -DOPENSIM_DEPENDENCIES_DIR="..\opensim_dependencies_install" `
        -DBUILD_JAVA_WRAPPING=OFF                                    `
        -DBUILD_PYTHON_WRAPPING=ON                                   `
        -DWITH_BTK=ON                                                `
    -DOPENSIM_C3D_PARSER=ezc3d
    cmake --build . --config RelWithDebInfo -- /maxcpucount:4
    ctest --build-config RelWithDebInfo --parallel 4
    cmake --build . --config RelWithDebInfo --target install -- /maxcpucount:4

    $pathOpenSim = $DependenciesDir + "\opensim_4_2\opensim_install\bin"
    [Environment]::SetEnvironmentVariable("PATH", $Env:PATH + $pathOpenSim, [EnvironmentVariableTarget]::Machine)
}

# Install dependencies (C++) in parallel
# RealTime Biomechanics - Filter
$installRTBFilter = {
    param($DependenciesDir)
    if (Test-Path ($DependenciesDir + "\RTB_Filter") -PathType container) {
        Write-Host "RealTime Biomechanics - Filter found."
    } else {
        Write-Host "RealTime Biomechanics - Filter NOT found."
        cd $DependenciesDir
        git clone -q https://github.com/RealTimeBiomechanics/Filter
        cd Filter
        cmake -DCMAKE_INSTALL_PREFIX="..\RTB_Filter" .
        cmake --build .
        cmake --install .
        cd ..
        Remove-Item Filter -force -Recurse
        [Environment]::SetEnvironmentVariable("Filter_DIR", ($DependenciesDir + "\RTB_Filter"), [EnvironmentVariableTarget]::Machine)
    }
}

# RealTime Biomechanics - Concurrency
$installRTBConcurrency = {
    param($DependenciesDir)
    if (Test-Path ($DependenciesDir + "\RTB_Concurrency") -PathType container) {
        Write-Host "RealTime Biomechanics - Concurrency found."
    } else {
        Write-Host "RealTime Biomechanics - Concurrency NOT found."
        cd $DependenciesDir
        git clone -q https://github.com/RealTimeBiomechanics/Concurrency
        cd Concurrency
        cmake -DCMAKE_INSTALL_PREFIX="..\RTB_Concurrency" .
        cmake --build .
        cmake --install .
        cd ..
        Remove-Item Concurrency -force -Recurse
        [Environment]::SetEnvironmentVariable("Concurrency_DIR", ($DependenciesDir + "\RTB_Concurrency"), [EnvironmentVariableTarget]::Machine)
    }
}

$jobs = @(
    $installRTBFilter,
    $installRTBConcurrency
    )

$jobArray = New-Object -TypeName System.Collections.ArrayList
foreach($job in $jobs)
{
    $jobArray.Add((Start-Job -ScriptBlock $job -ArgumentList $DependenciesDir ))
} 
Write-Host "Started installing dependencies (C++) in parallel." -Verbose
Wait-Job -Job $jobArray
foreach($job in $jobArray){
    Receive-Job -Name $job.Name
}
#Receive-Job $jobArray # this wasn't printing everything, maybe the above will...

# Install CodeSynthesis XSD
if (Test-Path ($DependenciesDir + "\CodeSynthesis XSD 4.0") -PathType container) {
      Write-Host "CodeSynthesis XSD found."
} else {
	if (Test-Path "C:\Program Files (x86)\CodeSynthesis XSD 4.0" -PathType container) {
		Write-Host "CodeSynthesis XSD found [C:\Program Files (x86)]."
	} else {
		Write-Host "CodeSynthesis XSD not found."
		# Download installer
		$XSDDir = $DependenciesDir + "\xsd-4.0.msi"
		Invoke-WebRequest "https://www.codesynthesis.com/download/xsd/4.9/windows/i686/xsd-4.0.msi" -OutFile $XSDDir
		# Run the installer
		Start-Process .\xsd-4.0.msi -Wait
		Remove-Item xsd-4.0.msi -force -Recurse
	}
}

Set-Location $PSScriptRoot # Move current working directory to directory of this script
refreshenv

Write-Host "Dependencies install finished."