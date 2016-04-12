#include "UHDF.h"
#include <iostream>
using namespace std;

template <typename T>
void listAttributes(const T &attOwner, int depth)
{
    for (const auto &name : attOwner.getAttributeNames())
    {
        for (int i = 0; i < depth; i++)
            cout << "\t";

        cout << name << ": ";

        try
        {
            const UHDF_Attribute att = attOwner.openAttribute(name);
            if (att.isString())
            {
                cout << "string, '" << att.readAsString() << "'" << endl;
            }
            else
            {
                cout << UHDFTypeName(att.getType()) << ", " << att.getNumElements() << " elements" << endl;
            }
        }
        catch (UHDF_Exception &e)
        {
            cerr << "ERROR OPENING (" << e.what() << ")" << endl;
        }
    }
}

void listGroup(const UHDF_Group &g, int depth)
{
    for (const auto &name : g.getGroupNames())
    {
        for (int i = 0; i < depth; i++)
            cout << "\t";

        cout << "\tGROUP '" << name << "'" << endl;

        try
        {
            const UHDF_Group g = g.openGroup(name);
            listAttributes(g, depth + 1);
            listGroup (g, depth + 1);
        }
        catch (UHDF_Exception &e)
        {
            cerr << "ERROR OPENING (" << e.what() << ")";
        }
    }
    for (const auto &name : g.getDatasetNames())
    {
        for (int i = 0; i < depth; i++)
            cout << "\t";
        cout << "\tFIELD '" << name << "'" << endl;

        try
        {
            const UHDF_Dataset d = g.openDataset(name);
            listAttributes(d, depth + 2);
        }
        catch (UHDF_Exception &e)
        {
            cerr << "ERROR OPENING (" << e.what() << ")";
        }
    }
}

void listContents(const UHDF_File &f)
{
    cout << f.getFileName() << ":" << endl;
    for (const auto &name : f.getGroupNames())
    {
        cout << "\tGROUP '" << name << "'" << endl;

        try
        {
            const UHDF_Group g = f.openGroup(name);
            listGroup (g, 1);
        }
        catch (UHDF_Exception &e)
        {
            cerr << "ERROR OPENING (" << e.what() << ")";
        }
    }
    for (const auto &name : f.getDatasetNames())
    {
        cout << "\tFIELD '" << name << "'" << endl;

        try
        {
            const UHDF_Dataset d = f.openDataset(name);
            listAttributes(d, 1);
        }
        catch (UHDF_Exception &e)
        {
            cerr << "ERROR OPENING (" << e.what() << ")";
        }
    }
}

template <typename T>
void average(const T &dsOwner, const string &field)
{
    const UHDF_Dataset d = dsOwner.openDataset(field);

    const auto &dims = d.getDimensions();

    const vector<double> data = d.readAll<double>();
    double avg = 0;

    for (auto d : data)
    {
        avg += d;
    }
    avg /= data.size();

    cout << "Average value = " << avg << endl;
}

int main (int argc, char *argv[])
{
    UHDF_File testH4("NPP_VMAE_L1.A2003025.0715.hdf", UHDF_READONLY);
    UHDF_File testH5("hdf5_test.h5", UHDF_READONLY);

    listContents(testH4);

    listContents(testH5);

    average(testH4, "Latitude");
    average(testH5, "images/Iceberg");

    return 0;
}
