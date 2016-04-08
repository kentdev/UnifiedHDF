#ifndef UHDF_INTERFACES_H
#define UHDF_INTERFACES_H

#include <list>
#include <string>

class UHDF_Dataset;
class UHDF_Group;
class UHDF_Attribute;

class UHDF_DatasetHolder
{
public:
    virtual std::list<std::string> getDatasetNames() const = 0;
    virtual UHDF_Dataset openDataset(const std::string &datasetName) const = 0;
};

class UHDF_GroupHolder
{
public:
    virtual std::list<std::string> getGroupNames() const = 0;
    virtual UHDF_Group openGroup(const std::string &groupName) const = 0;
};

class UHDF_AttributeHolder
{
public:
    virtual std::list<std::string> getAttributeNames() const = 0;
    virtual UHDF_Attribute openAttribute(const std::string &attributeName) const = 0;
};

#endif // UHDF_INTERFACES_H
