#ifndef DMVIEW_H
#define DMVIEW_H

#include <map>
#include <vector>
#include <string>

namespace DM {
class View
{
    int type;
    std::string name;
    std::vector<std::string> WriteAttributes;
    std::vector<std::string> ReadAttributes;
    std::vector<std::string> AvailableAttributes;
    int readType;
    int writeType;

    std::string idOfDummyComponent;

public:
    View(std::string name);
    View(){}
    void addAttributes(std::string name);
    void getAttributes(std::string name);
    void addAvalibleAttribute(std::string name);
    void addComponent(int Type);
    void getComponent(int Type);

    std::string getName(){return this->name;}
    std::vector<std::string> const & getWriteAttributes  () const {return WriteAttributes;}
    std::vector<std::string> const & getReadAttributes  () const {return ReadAttributes;}
    std::vector<std::string> const & getAvailableAttributes  () const {return AvailableAttributes;}

    int const  & getReadType  () const {return readType;}
    int const  & getWriteType  () const {return writeType;}

    bool reads();
    bool writes();

    std::string getIdOfDummyComponent() {return this->idOfDummyComponent;}
    void setIdOfDummyComponent(std::string id) {this->idOfDummyComponent = id;}

};
}

#endif // DMVIEW_H
