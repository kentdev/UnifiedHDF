#ifndef UHDF_GROUP_H
#define UHDF_GROUP_H

#include "UHDF_Types.h"
#include "UHDF_Interfaces.h"
#include "UHDF_Dataset.h"

#include <list>
#include <string>


// groups don't exist in HDF4, so all UHDF_Groups are HDF5
class UHDF_Group// : UHDF_DatasetHolder, UHDF_AttributeHolder
{
    friend class UHDF_File;

public:
    ~UHDF_Group()
    {
        if (id.h5id >= 0)
            H5Gclose(id.h5id);
    }

    const std::string &getName() const
    {
        return groupname;
    }

    std::list<std::string> getGroupNames() const
    {
        return getObjNames(H5G_GROUP);
    }

    std::list<std::string> getDatasetNames() const
    {
        return getObjNames(H5G_DATASET);
    }

    std::list<std::string> getAttributeNames() const
    {
        std::list<std::string> names;

        const int numAttrs = H5Aget_num_attrs(id.h5id);
        if (numAttrs < 0)
            throw UHDF_Exception("Error retrieving the number of attributes in group '" + groupname + "'");

        for (int i = 0; i < numAttrs; i++)
        {
            const ssize_t nameLength = H5Aget_name_by_idx(id.h5id, ".", H5_INDEX_NAME, H5_ITER_NATIVE, i, NULL, 0, H5P_DEFAULT);
            if (nameLength < 0)
                throw UHDF_Exception("Error getting name of attribute " + boost::lexical_cast<std::string>(i) + " of group '" + groupname + "'");
            if (nameLength == 0)
                continue;

            std::unique_ptr<char[]> name(new char[nameLength + 1]);
            memset(name.get(), 0, nameLength + 1);

            if (H5Aget_name_by_idx(id.h5id, ".", H5_INDEX_NAME, H5_ITER_NATIVE, i, name.get(), nameLength + 1, H5P_DEFAULT) < 0)
                throw UHDF_Exception("Error getting name of attribute " + boost::lexical_cast<std::string>(i) + " of group '" + groupname + "'");

            names.push_back(std::string(name.get()));
        }

        return names;
    }

    UHDF_Group openGroup(const std::string &groupName) const
    {
        try
        {
            // allow specifying a dataset in a subgroup (eg, "group1/group2/dataset")
            const size_t delimiterPos = groupName.find("/");
            if (delimiterPos == std::string::npos)
            {
                return UHDF_Group(id, groupName);
            }
            else
            {
                const std::string groupName = groupName.substr(0, delimiterPos);
                return UHDF_Group(id, groupName).openGroup(groupName.substr(delimiterPos+1, std::string::npos));
            }
        }
        catch (const UHDF_Exception &e)
        {
            throw UHDF_Exception("Couldn't open group " + groupName + " in group " + groupname + ": " + e.what());
        }

        // shouldn't reach here
        throw UHDF_Exception("Error opening group " + groupName);
    }

    UHDF_Dataset openDataset(const std::string &datasetName) const
    {
        try
        {
            // allow specifying a dataset in a subgroup (eg, "group1/group2/dataset")
            const size_t delimiterPos = datasetName.find("/");
            if (delimiterPos == std::string::npos)
            {
                return UHDF_Dataset(UHDF_HDF5, id, datasetName);
            }
            else
            {
                const std::string groupName = datasetName.substr(0, delimiterPos);
                return UHDF_Group(id, groupName).openDataset(datasetName.substr(delimiterPos+1, std::string::npos));
            }
        }
        catch (const UHDF_Exception &e)
        {
            throw UHDF_Exception("Couldn't open dataset " + datasetName + " in group " + groupname + ": " + e.what());
        }

        // shouldn't reach here
        throw UHDF_Exception("Error opening dataset " + datasetName);
    }

    UHDF_Attribute openAttribute(const std::string &attributeName) const
    {
        try
        {
            return UHDF_Attribute(UHDF_HDF5, id, attributeName);
        }
        catch (const UHDF_Exception &e)
        {
            throw UHDF_Exception("Couldn't open attribute " + attributeName + " in group " + groupname + ": " + e.what());
        }
    }

private:
    UHDF_Identifier id;
    std::string groupname;

    UHDF_Group( UHDF_Identifier ownerId, const std::string &groupName)
    {
        groupname = groupName;

        id.h5id = H5Gopen2(ownerId.h5id, groupname.c_str(), H5P_DEFAULT);
        if (id.h5id < 0)
            throw UHDF_Exception("Couldn't open group name '" + groupname + "'");
    }

    std::list<std::string> getObjNames(const H5G_obj_t objType) const
    {
        std::list<std::string> names;

        hsize_t numObjs;
        if (H5Gget_num_objs(id.h5id, &numObjs) < 0)
            throw UHDF_Exception("Error getting number of objects from group " + groupname);

        for (hsize_t i = 0; i < numObjs; i++)
        {
            if (H5Gget_objtype_by_idx(id.h5id, i) == objType)
            {
                char name[256];
                if (H5Gget_objname_by_idx(id.h5id, i, name, 255) < 0)
                    throw UHDF_Exception("Error getting name of object " + boost::lexical_cast<std::string>(i) + " from group " + groupname);

                names.push_back(name);
            }
        }

        return names;
    }
};

#endif
