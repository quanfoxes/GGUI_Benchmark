#include "Element.h"

#include "../Renderer.h"

#include <algorithm>
#include <vector>

#undef min
#undef max


GGUI::Element::Element(std::string Class){
    Add_Class("default");

    Name = std::to_string((unsigned long long)this);

    Add_Class(Class);

    Parse_Classes();
}

GGUI::Element::Element() {
    Add_Class("default");
    Name = std::to_string((unsigned long long)this);
    Parse_Classes();
}

GGUI::Element::Element(std::map<std::string, VALUE*> css){
    Add_Class("default");
    Style = css;
    Name = std::to_string((unsigned long long)this);
    Parse_Classes();
}

GGUI::RGB GGUI::Element::Get_RGB_Style(std::string style_name){
    return At<RGB_VALUE>(style_name)->Value;
}

int GGUI::Element::Get_Number_Style(std::string style_name){
    return At<NUMBER_VALUE>(style_name)->Value;
}

bool GGUI::Element::Get_Bool_Style(std::string style_name){
    return At<BOOL_VALUE>(style_name)->Value;
}

GGUI::VALUE* GGUI::Element::Get_Style(std::string style_name){
    return At<VALUE>(style_name);
}

void GGUI::Element::Set_Style(std::string style_name, VALUE* value){
    *At<VALUE>(style_name) = *value;
}

void GGUI::Element::Parse_Classes(){

    //Go through all classes and their styles and accumulate them.
    for(auto& Class : Classes){

        // The class wantwd has not been yet constructed.
        if (GGUI::Classes.find(Class) == GGUI::Classes.end()){
            Dirty.Dirty(STAIN_TYPE::CLASS);
        }

        std::map<std::string, VALUE*> Current = GGUI::Classes[Class];

        for (auto& Current_Style : Current){

            Style[Current_Style.first] = Current_Style.second;

        }
    }
}

void GGUI::Element::Add_Class(std::string class_name){
    if(Class_Names.find(class_name) != Class_Names.end()){
        Classes.push_back(Class_Names[class_name]);
    }
    else{
        Classes.push_back(GGUI::Get_Free_Class_ID(class_name));
    }
}

bool GGUI::Element::Has(std::string s){
    //first convert the string to the ID
    int id = Class_Names[s];

    //then check if the element has the class
    for(int i = 0; i < Classes.size(); i++){
        if(Classes[i] == id){
            return true;
        }
    }

    return false;
}

void GGUI::Element::Show_Border(bool b){
    if (b != At<BOOL_VALUE>(STYLES::Border)->Value){
        At<BOOL_VALUE>(STYLES::Border)->Value = b;
        Dirty.Dirty(STAIN_TYPE::EDGE);
        Update_Frame();
    }
}

bool GGUI::Element::Has_Border(){
    return At<BOOL_VALUE>(STYLES::Border)->Value;
}

void GGUI::Element::Add_Child(Element* Child){
    if (Child->At<COORDINATES_VALUE>(STYLES::Position)->Value.X + Child->At<NUMBER_VALUE>(STYLES::Width)->Value >= At<NUMBER_VALUE>(STYLES::Width)->Value || 
    Child->At<COORDINATES_VALUE>(STYLES::Position)->Value.Y + Child->At<NUMBER_VALUE>(STYLES::Height)->Value >= At<NUMBER_VALUE>(STYLES::Height)->Value){
        if (Child->Resize_To(this) == false){

            GGUI::Report(
                "Window exeeded bounds\n "
                "Starts at: {" + std::to_string(Child->At<COORDINATES_VALUE>(STYLES::Position)->Value.X) + ", " + std::to_string(Child->At<COORDINATES_VALUE>(STYLES::Position)->Value.Y) + "}\n "
                "Ends at: {" + std::to_string(Child->At<COORDINATES_VALUE>(STYLES::Position)->Value.X + Child->At<NUMBER_VALUE>(STYLES::Width)->Value) + ", " + std::to_string(Child->At<COORDINATES_VALUE>(STYLES::Position)->Value.Y + Child->At<NUMBER_VALUE>(STYLES::Height)->Value) + "}\n "
                "Max is at: {" + std::to_string(At<NUMBER_VALUE>(STYLES::Width)->Value) + ", " + std::to_string(At<NUMBER_VALUE>(STYLES::Height)->Value) + "}\n "
            );

            return;
        }
    }

    Dirty.Dirty(STAIN_TYPE::DEEP);
    Child->Parent = this;

    Element_Names.insert({Child->Name, Child});

    Childs.push_back(Child);
    Update_Frame();
}

std::vector<GGUI::Element*>& GGUI::Element::Get_Childs(){
    return Childs;
}

bool GGUI::Element::Remove(Element* handle){
    for (int i = 0; i < Childs.size(); i++){
        if (Childs[i] == handle){
            //If the mouse if focused on this about to be deleted element, change mouse position into it's parent Position.
            if (Focused_On == Childs[i]){
                Mouse = Childs[i]->Parent->At<COORDINATES_VALUE>(STYLES::Position)->Value;
            }

            Childs.erase(Childs.begin() + i);
            Update_Parent(handle);
            return true;
        }
    }
    return false;
}

void GGUI::Element::Update_Parent(Element* New_Element){
    //normally elements dont do anything
    if (!New_Element->Is_Displayed()){
        Dirty.Stain_All();
    }

    if (Parent){
        Parent->Update_Parent(New_Element);
    }
    else{
        Dirty.Stain_All();
        Update_Frame(); //the most top (Main) will not flush all the updates to render.
    }
}

void GGUI::Element::Display(bool f){
    // Check if the to be displayed is true and the element wasnt already displayed.
    if (f != Show){
        if (f){
            Dirty.Stain_All();
            Show = true;
        }
        else{
            Dirty.Stain_All();
            Show = false;
        }
    }

    Update_Frame();
}

bool GGUI::Element::Is_Displayed(){
    return Show;
}

bool GGUI::Element::Remove(int index){
    if (index > Childs.size() - 1){
        return false;
    }
    Element* tmp = Childs[index];

    //If the mouse if focused on this about to be deleted element, change mouse position into it's parent Position.
    if (Focused_On == tmp){
        Mouse = tmp->Parent->At<COORDINATES_VALUE>(STYLES::Position)->Value;
    }

    Childs.erase(Childs.begin() + index);
    Update_Parent(tmp);
    return true;
}

void GGUI::Element::Remove(){
    if (Parent){
        //tell the parent what is about to happen.
        //you need to update the parent before removing the child, 
        //otherwise the code cannot erase it when it is not found!
        Parent->Remove(this);
    }
    else{
        Report(
            std::string("Cannot remove ") + Get_Name() + std::string(", with no parent\n")
        );
    }
}

void GGUI::Element::Set_Dimensions(int width, int height){
    if (width != At<NUMBER_VALUE>(STYLES::Width)->Value || height != At<NUMBER_VALUE>(STYLES::Height)->Value){    
        At<NUMBER_VALUE>(STYLES::Width)->Value = width;
        At<NUMBER_VALUE>(STYLES::Height)->Value = height;
        Dirty.Stain_All();
        Update_Frame();
    }
}

int GGUI::Element::Get_Width(){
    return At<NUMBER_VALUE>(STYLES::Width)->Value;
}

int GGUI::Element::Get_Height(){
    return At<NUMBER_VALUE>(STYLES::Height)->Value;
}

void GGUI::Element::Set_Width(int width){
    if (width != At<NUMBER_VALUE>(STYLES::Width)->Value){
        At<NUMBER_VALUE>(STYLES::Width)->Value = width;
        Dirty.Stain_All();
        Update_Frame();
    }
}

void GGUI::Element::Set_Height(int height){
    if (height != At<NUMBER_VALUE>(STYLES::Height)->Value){
        At<NUMBER_VALUE>(STYLES::Height)->Value = height;
        Dirty.Stain_All();
        Update_Frame();
    }
}


void GGUI::Element::Set_Position(Coordinates c){
    At<COORDINATES_VALUE>(STYLES::Position)->Value = c;
    if (Parent){
        Report(
            "Cannot move element who is orphan."
        );
    }
    Parent->Dirty.Dirty(STAIN_TYPE::STRECH);
    Update_Frame();
}

GGUI::Coordinates GGUI::Element::Get_Position(){
    return At<COORDINATES_VALUE>(STYLES::Position)->Value;
}

GGUI::Coordinates GGUI::Element::Get_Absolute_Position(){
    Coordinates Result = {0, 0};
    
    Element* current_element = this;
    Result = current_element->At<COORDINATES_VALUE>(STYLES::Position)->Value;
    current_element = current_element->Parent;

    while (current_element != nullptr){
        Result.X += current_element->At<COORDINATES_VALUE>(STYLES::Position)->Value.X;
        Result.Y += current_element->At<COORDINATES_VALUE>(STYLES::Position)->Value.Y;
        Result.Z += current_element->At<COORDINATES_VALUE>(STYLES::Position)->Value.Z;

        current_element = current_element->Parent;
    }

    return Result;
}

std::pair<unsigned int, unsigned int> GGUI::Element::Get_Fitting_Dimensions(Element* child){
    GGUI::Element tmp = *child;
    tmp.At<NUMBER_VALUE>(STYLES::Width)->Value = 0;
    tmp.At<NUMBER_VALUE>(STYLES::Height)->Value = 0;

    int Border_Size = Has_Border() * 2;

    while (true){
        if (tmp.At<COORDINATES_VALUE>(STYLES::Position)->Value.X + tmp.At<NUMBER_VALUE>(STYLES::Width)->Value < At<NUMBER_VALUE>(STYLES::Width)->Value - Border_Size){
            tmp.At<NUMBER_VALUE>(STYLES::Width)->Value++;
        }
        
        if (tmp.At<COORDINATES_VALUE>(STYLES::Position)->Value.Y + tmp.At<NUMBER_VALUE>(STYLES::Height)->Value < At<NUMBER_VALUE>(STYLES::Height)->Value - Border_Size){
            tmp.At<NUMBER_VALUE>(STYLES::Height)->Value++;
        }
        else if (tmp.At<COORDINATES_VALUE>(STYLES::Position)->Value.X + tmp.At<NUMBER_VALUE>(STYLES::Width)->Value >= At<NUMBER_VALUE>(STYLES::Width)->Value - Border_Size && tmp.At<COORDINATES_VALUE>(STYLES::Position)->Value.X + tmp.At<NUMBER_VALUE>(STYLES::Height)->Value >= At<NUMBER_VALUE>(STYLES::Height)->Value - Border_Size){
            break;
        }
        
        for (auto c : Childs){
            if (Collides(child, c)){
                //there are already other childs occupying this area so we can stop here.
                return {tmp.At<NUMBER_VALUE>(STYLES::Width)->Value, tmp.At<NUMBER_VALUE>(STYLES::Height)->Value};
            }
        }

    }

    return {tmp.At<NUMBER_VALUE>(STYLES::Width)->Value, tmp.At<NUMBER_VALUE>(STYLES::Height)->Value};
}

void GGUI::Element::Set_Back_Ground_Color(RGB color){
    At<RGB_VALUE>(STYLES::Back_Ground_Color)->Value = color;
    if (At<RGB_VALUE>(STYLES::Border_Back_Ground_Color)->Value == At<RGB_VALUE>(STYLES::Back_Ground_Color)->Value)
        At<RGB_VALUE>(STYLES::Border_Back_Ground_Color)->Value = color;
    Dirty.Dirty(STAIN_TYPE::COLOR);
    Update_Frame();
}

GGUI::RGB GGUI::Element::Get_Back_Ground_Color(){
    return At<RGB_VALUE>(STYLES::Back_Ground_Color)->Value;
}

void GGUI::Element::Set_Border_Colour(RGB color){
    At<RGB_VALUE>(STYLES::Border_Colour)->Value = color;
    Dirty.Dirty(STAIN_TYPE::COLOR);
    Update_Frame();
}

GGUI::RGB GGUI::Element::Get_Border_Colour(){
    return At<RGB_VALUE>(STYLES::Border_Colour)->Value;
}

void GGUI::Element::Set_Border_Back_Ground_Color(RGB color){
    At<RGB_VALUE>(STYLES::Border_Back_Ground_Color)->Value = color;
    Dirty.Dirty(STAIN_TYPE::COLOR);
    Update_Frame();
}

GGUI::RGB GGUI::Element::Get_Border_Back_Ground_Color(){
    return At<RGB_VALUE>(STYLES::Border_Back_Ground_Color)->Value;
}

void GGUI::Element::Set_Text_Color(RGB color){
    At<RGB_VALUE>(STYLES::Text_Color)->Value = color;
    Dirty.Dirty(STAIN_TYPE::COLOR);
    Update_Frame();
}

GGUI::RGB GGUI::Element::Get_Text_Color(){
    return At<RGB_VALUE>(STYLES::Text_Color)->Value;
}

//Returns nested buffer of AST window's
std::vector<GGUI::UTF> GGUI::Element::Render(){
    std::vector<GGUI::UTF> Result = Render_Buffer;

    if (Dirty.is(STAIN_TYPE::CLASS)){
        Parse_Classes();

        Dirty.Clean(STAIN_TYPE::CLASS);
    }

    if (Dirty.is(STAIN_TYPE::STRECH)){
        Result.clear();
        Result.resize(At<NUMBER_VALUE>(STYLES::Width)->Value * At<NUMBER_VALUE>(STYLES::Height)->Value);
        Dirty.Clean(STAIN_TYPE::STRECH);
    }

    //Apply the color system to the resized result list
    if (Dirty.is(STAIN_TYPE::COLOR))
        Apply_Colors(this, Result);

    //This will add the borders if nessesary and the title of the window.
    if (Dirty.is(STAIN_TYPE::EDGE))
        Add_Overhead(this, Result);

    //if inned children have changed whitout this changing, then this will trigger.
    if (Children_Changed()){
        Dirty.Dirty(STAIN_TYPE::DEEP);
    }

    //This will add the child windows to the Result buffer
    if (Dirty.is(STAIN_TYPE::DEEP)){
        Dirty.Clean(STAIN_TYPE::DEEP);
        for (auto& c : this->Get_Childs()){
                Nest_Element(this, c, Result, c->Render());
        }
    }

    Render_Buffer = Result;

    return Result;
}

//These are just utility functions for internal purposes, dont need to sed Dirty.
void GGUI::Element::Apply_Colors(Element* w, std::vector<UTF>& Result){
    Dirty.Clean(STAIN_TYPE::COLOR);

    for (auto& utf : Result){
        utf.Pre_Fix = w->Compose_All_Text_RGB_Values();
        
        if (utf.Pre_Fix != "")
            utf.Post_Fix = Constants::RESET_Text_Color + Constants::RESET_Back_Ground_Color;
    }
}

void GGUI::Element::Add_Overhead(GGUI::Element* w, std::vector<GGUI::UTF>& Result){
    if (!w->Has_Border())
        return;

    Dirty.Clean(STAIN_TYPE::EDGE);

    for (int y = 0; y < At<NUMBER_VALUE>(STYLES::Height)->Value; y++){
        for (int x = 0; x < At<NUMBER_VALUE>(STYLES::Width)->Value; x++){
            //top left corner
            if (y == 0 && x == 0){
                Result[y * At<NUMBER_VALUE>(STYLES::Width)->Value + x] = GGUI::UTF(SYMBOLS::TOP_LEFT_CORNER, w->Compose_All_Border_RGB_Values());
            }
            //top right corner
            else if (y == 0 && x == At<NUMBER_VALUE>(STYLES::Width)->Value - 1){
                Result[y * At<NUMBER_VALUE>(STYLES::Width)->Value + x] = GGUI::UTF(SYMBOLS::TOP_RIGHT_CORNER, w->Compose_All_Border_RGB_Values());
            }
            //bottom left corner
            else if (y == At<NUMBER_VALUE>(STYLES::Height)->Value - 1 && x == 0){
                Result[y * At<NUMBER_VALUE>(STYLES::Width)->Value + x] = GGUI::UTF(SYMBOLS::BOTTOM_LEFT_CORNER, w->Compose_All_Border_RGB_Values());
            }
            //bottom right corner
            else if (y == At<NUMBER_VALUE>(STYLES::Height)->Value - 1 && x == At<NUMBER_VALUE>(STYLES::Width)->Value - 1){
                Result[y * At<NUMBER_VALUE>(STYLES::Width)->Value + x] = GGUI::UTF(SYMBOLS::BOTTOM_RIGHT_CORNER, w->Compose_All_Border_RGB_Values());
            }
            //The title will only be written after the top left corner symbol until top right corner symbol and will NOT overflow
            // else if (y == 0 && x < w->Get_Title().size()){
            //     Result[y * At<NUMBER_VALUE>(STYLES::Width)->Value + x] = GGUI::UTF(w->Get_Title()[x - 1], w->Get_Text_Color(), COLOR::RESET);
            // }
            //The roof border
            else if (y == 0 || y == At<NUMBER_VALUE>(STYLES::Height)->Value - 1){
                Result[y * At<NUMBER_VALUE>(STYLES::Width)->Value + x] = GGUI::UTF(SYMBOLS::HORIZONTAL_LINE, w->Compose_All_Border_RGB_Values());
            }
            //The left border
            else if (x == 0 || x == At<NUMBER_VALUE>(STYLES::Width)->Value - 1){
                Result[y * At<NUMBER_VALUE>(STYLES::Width)->Value + x] = GGUI::UTF(SYMBOLS::VERTICAL_LINE, w->Compose_All_Border_RGB_Values());
            }
        }
    }
}

void GGUI::Element::Nest_Element(GGUI::Element* Parent, GGUI::Element* Child, std::vector<GGUI::UTF>& Parent_Buffer, std::vector<GGUI::UTF> Child_Buffer){
    
    unsigned int Max_Allowed_Height = Parent->At<NUMBER_VALUE>(STYLES::Height)->Value - Parent->Has_Border();    //remove bottom borders from calculation
    unsigned int Max_Allowed_Width = Parent->At<NUMBER_VALUE>(STYLES::Width)->Value - Parent->Has_Border();     //remove right borders from calculation

    unsigned int Min_Allowed_Height = 0 + Parent->Has_Border();                        //add top borders from calculation
    unsigned int Min_Allowed_Width = 0 + Parent->Has_Border();                         //add left borders from calculation

    unsigned int Child_Start_Y = Min_Allowed_Height + Child->At<COORDINATES_VALUE>(STYLES::Position)->Value.Y;
    unsigned int Child_Start_X = Min_Allowed_Width + Child->At<COORDINATES_VALUE>(STYLES::Position)->Value.X;

    unsigned int Child_End_Y = Min(Child_Start_Y + Child->At<NUMBER_VALUE>(STYLES::Height)->Value, Max_Allowed_Height);
    unsigned int Child_End_X = Min(Child_Start_X + Child->At<NUMBER_VALUE>(STYLES::Width)->Value, Max_Allowed_Width);

    for (int y = Child_Start_Y; y < Child_End_Y; y++){
        for (int x = Child_Start_X; x < Child_End_X; x++){
            Parent_Buffer[y * Child->At<NUMBER_VALUE>(STYLES::Width)->Value + x] = Child_Buffer[(y - Child_Start_Y) * Child->At<NUMBER_VALUE>(STYLES::Height)->Value + (x - Child_Start_X)];
        }
    }
}
//End of utility functions.

void GGUI::Element::On_Click(std::function<void(GGUI::Event* e)> action){
    Action* a = new Action(
        Constants::ENTER,
        [=](GGUI::Event* e){
            if (Collides(this, Mouse)){
                action(e);

                //action succesfully executed.
                return true;
            }
            //action failed.
            return false;
        },
        this
    );
    GGUI::Event_Handlers.push_back(a);
}

GGUI::Element* GGUI::Element::Copy(){
    Element* new_element = new Element();

    *new_element = *this;

    //now also update the event handlers.
    for (auto& e : GGUI::Event_Handlers){

        if (e->Host == this){
            //copy the event and make a new one
            Action* new_action = new Action(*e);

            //update the host
            new_action->Host = new_element;

            //add the new action to the event handlers list
            GGUI::Event_Handlers.push_back(new_action);
        }

    }

    return new_element;
}

bool GGUI::Element::Children_Changed(){
    if (Dirty.Type != STAIN_TYPE::CLEAN)
        return true;

    for (auto& e : Childs){
        if (e->Children_Changed())
            return true;
    }

    return false;
}

GGUI::Element* GGUI::Element::Get_Element(std::string name){
    Element* Result = nullptr;

    if (Element_Names.find(name) != Element_Names.end()){
        Result = Element_Names[name];
    }

    return Result;
}

