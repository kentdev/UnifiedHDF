#ifndef UHDF_FILE_H
#define UHDF_FILE_H

#include <string>
#include <list>
#include <memory>

#include "UHDF_Dataset.h"

#include <boost/lexical_cast.hpp>


class UHDF_File
{
public:
    UHDF_File( const std::string &fileName, UHDF_FileAccess accessMode)
    {
        filename = fileName;

        if (Hishdf(fileName.c_str()) != 0)
        {
            fileType = UHDF_HDF4;
            fileId.h4id = -1;

            int32 h4Access;
            switch (accessMode)
            {
            case UHDF_READONLY:
                h4Access = DFACC_RDONLY;
                break;
            }

            fileId.h4id = SDstart(fileName.c_str(), h4Access);
            if (fileId.h4id < 0)
                throw UHDF_Exception("Unable to open " + filename);

        }
        else if (H5Fis_hdf5(fileName.c_str()) != 0)
        {
            fileType = UHDF_HDF5;
            fileId.h5id = -1;
            H5RootGroupId = -1;

            unsigned int flags;
            switch(accessMode)
            {
            case UHDF_READONLY:
                flags = H5F_ACC_RDONLY;
                break;
            }

            fileId.h5id = H5Fopen(fileName.c_str(), flags, H5P_DEFAULT);
            if (fileId.h5id < 0)
                throw UHDF_Exception("Unable to open " + filename);

            H5RootGroupId = H5Gopen2( fileId.h5id, "/", H5P_DEFAULT);
            if (H5RootGroupId < 0)
                throw UHDF_Exception("Couldn't open root group of file " + filename);
        }
        else
        {
            throw UHDF_Exception(fileName + " is not an HDF4 or HDF5 file");
        }
    }

    ~UHDF_File()
    {
        switch(fileType)
        {
        case UHDF_HDF4:
            if (fileId.h4id >= 0)
                SDend(fileId.h4id);
            break;
        case UHDF_HDF5:
            if (H5RootGroupId >= 0)
                H5Gclose(H5RootGroupId);
            if (fileId.h5id >= 0)
                H5Fclose(fileId.h5id);
            break;
        }
    }

    const std::string &getFileName() const
    {
        return filename;
    }

    std::list<std::string> getDatasetNames() const
    {
        std::list<std::string> datasetNames;

        switch(fileType)
        {
        case UHDF_HDF4:
        {
            int32 numDatasets;
            int32 numAttributes;

            if (SDfileinfo( fileId.h4id, &numDatasets, &numAttributes) < 0)
                throw UHDF_Exception("Error getting file info from " + filename);

            for (int32 i = 0; i < numDatasets; i++)
            {
                const int32 sdsid = SDselect( fileId.h4id, i);
                if (sdsid < 0)
                    throw UHDF_Exception("Error opening dataset #" + boost::lexical_cast<std::string>(i) + " from file " + filename);

                char sdsName[MAX_NC_NAME + 1];
                int32 sdsRank;
                int32 sdsDimSizes[MAX_VAR_DIMS];
                int32 sdsType;
                int32 sdsNumAttrs;

                if (SDgetinfo( sdsid, sdsName, &sdsRank, sdsDimSizes, &sdsType, &sdsNumAttrs) < 0)
                    throw UHDF_Exception("Error getting dataset info from dataset #" + boost::lexical_cast<std::string>(i) + " from file " + filename);

                datasetNames.push_back(std::string(sdsName));

                SDendaccess(sdsid);
            }
            break;
        }
        case UHDF_HDF5:
        {
            hsize_t numObjs;
            if (H5Gget_num_objs(H5RootGroupId, &numObjs) < 0)
                throw UHDF_Exception("Error getting number of objects from root group of " + filename);

            for (hsize_t i = 0; i < numObjs; i++)
            {
                if (H5Gget_objtype_by_idx(H5RootGroupId, i) == H5G_DATASET)
                {
                    char name[256];
                    if (H5Gget_objname_by_idx(H5RootGroupId, i, name, 255) < 0)
                        throw UHDF_Exception("Error getting name of object " + boost::lexical_cast<std::string>(i) + " from root group of " + filename);

                    datasetNames.push_back(name);
                }
            }
            break;
        }
        }

        return datasetNames;
    }

    std::list<std::string> getGroupNames() const
    {
        std::list<std::string> groupNames;

        switch(fileType)
        {
        case UHDF_HDF4:
            break;
        case UHDF_HDF5:
        {
            hsize_t numObjs;
            if (H5Gget_num_objs(H5RootGroupId, &numObjs) < 0)
                throw UHDF_Exception("Error getting number of objects from root group of " + filename);

            for (hsize_t i = 0; i < numObjs; i++)
            {
                if (H5Gget_objtype_by_idx(H5RootGroupId, i) == H5G_GROUP)
                {
                    char name[256];
                    if (H5Gget_objname_by_idx(H5RootGroupId, i, name, 255) < 0)
                        throw UHDF_Exception("Error getting name of object " + boost::lexical_cast<std::string>(i) + " from root group of " + filename);

                    groupNames.push_back(name);
                }
            }
            break;
        }
        }

        return groupNames;
    }

    std::unique_ptr<UHDF_Dataset> openDataset(const std::string &datasetName) const
    {
        try
        {
            switch (fileType)
            {
            case UHDF_HDF4:
                return std::unique_ptr<UHDF_Dataset>(new UHDF_Dataset(fileType, fileId, datasetName));
            case UHDF_HDF5:
            {
                UHDF_Identifier id;
                id.h5id = H5RootGroupId;

                return std::unique_ptr<UHDF_Dataset>(new UHDF_Dataset(fileType, id, datasetName));
            }
            }
        }
        catch (const UHDF_Exception &e)
        {
            throw UHDF_Exception("Couldn't open dataset " + datasetName + " in file " + filename + ": " + e.what());
        }

        // shouldn't reach here
        throw UHDF_Exception("Error opening dataset " + datasetName);
    }

private:
    std::string filename;
    UHDF_FileType fileType;
    UHDF_Identifier fileId;

    hid_t H5RootGroupId;
};

#endif
