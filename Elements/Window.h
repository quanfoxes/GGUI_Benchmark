#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <string>
#include <vector>

#include "Element.h"

//GGUI uses the ANSI escape code
//https://en.wikipedia.org/wiki/ANSI_escape_code
namespace GGUI{
    class Window : public Element{
        std::string Title = "";  //if this is empty then no title
    public:
        Window(){}

        Window(std::string title, std::vector<std::string> classes = {});

        Window(std::map<std::string, VALUE*> css) : Element(css){}
        
        Window(std::string title, std::map<std::string, VALUE*> css) : Element(css), Title(title){}

        void Set_Title(std::string t);

        std::string Get_Title();

        std::vector<UTF> Render() override;
        
        void Add_Overhead(Element* w, std::vector<UTF>& Result) override;

        std::string Get_Name() override;

        Element* Copy() override;
    };
}

#endif