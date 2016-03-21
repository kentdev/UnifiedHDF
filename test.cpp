#include "UHDF.h"
#include <iostream>
using namespace std;

void listContents(const UHDF_File &f)
{
    cout << f.getFileName() << ":" << endl;
    for (const auto &name : f.getGroupNames())
    {
        cout << "\tGROUP " << name << endl;
    }
    for (const auto &name : f.getDatasetNames())
    {
        cout << "\tFIELD " << name << endl;
    }
}

int main (int argc, char *argv[])
{
    UHDF_File testH4("NPP_VMAE_L1.A2003025.0715.hdf", UHDF_READONLY);
    UHDF_File testH5("hdf5_test.h5", UHDF_READONLY);

    listContents(testH4);
    listContents(testH5);

    return 0;
}

