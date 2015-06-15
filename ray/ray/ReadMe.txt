========================================================================
    CONSOLE APPLICATION : ray Project Overview
========================================================================

AppWizard has created this ray application for you.  

This file contains a summary of what you will find in each of the files that
make up your ray application.


ray.vcproj
    This is the main project file for VC++ projects generated using an Application Wizard. 
    It contains information about the version of Visual C++ that generated the file, and 
    information about the platforms, configurations, and project features selected with the
    Application Wizard.

ray.cpp
    This is the main application source file.

/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named ray.pch and a precompiled types file named StdAfx.obj.

/////////////////////////////////////////////////////////////////////////////
Other notes:

AppWizard uses "TODO:" comments to indicate parts of the source code you
should add to or customize.

/////////////////////////////////////////////////////////////////////////////
I don't believe that I made any changes to udray.cpp beyond adding two fields to
the Ray struct.

I have fully implemented ambient, diffuse, and specular shading with shading.

Clicking on the image after it has finished rendering will cause a RayTrace to
to be performed for that pixel. Some of the results are then displayed on the 
command line