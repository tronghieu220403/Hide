/**********************************************************************
 *
 *  Toby Opferman
 *
 *  Example Dynamic Loading a Driver
 *
 *  This example is for educational purposes only.  I license this source
 *  out for use in learning how to write a device driver.
 *
 *
 **********************************************************************/


#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <string>

using namespace std;

const string path = "C:\\Users\\hieu\\Documents\\PFMinifilter\\x64\\Release\\";
const string name = "PFMinifilter";


int _cdecl main(void)
{
    string s = "sc delete " + name;
    system(&s[0]);

    string s1 = "rundll32 syssetup,SetupInfObjectInstallAction DefaultInstall 128 " + path + name + ".inf";
    system(&s1[0]);

    cout << "Press enter to continue " << endl;
    getchar();

    string s2 = "net start " + name;
    system(&s2[0]);

    cout << "Service stop in:" << endl;
    for (int i = 19; i > 0; i--)
    {
        cout << i << endl;
        Sleep(1000);
    }

    string s3 = "net stop " + name;
    system(&s3[0]);

    return 0;
}


