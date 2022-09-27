#include "Renderer.h"

using namespace std;

int main(){
    GGUI::Pause_Renderer();
    
    GGUI::Window* Main = GGUI::Init_Renderer();
    
    GGUI::Text_Field* Text = new GGUI::Text_Field(
        "A AA AAA AAAA AAAAA AAAAAA AAAAAAA AAAAAAAA \n"
        "B BB BBB BBBB BBBBB BBBBBB BBBBBBB BBBBBBBB \n"
        "C CC CCC CCCC CCCCC CCCCCC CCCCCCC CCCCCCCC \n"
    );
    Text->Set_Back_Ground_Color(GGUI::COLOR::DARK_MAGENTA);
    Text->Set_Border_Back_Ground_Color(GGUI::COLOR::LIGHT_BLUE);
    Text->Set_Text_Color(GGUI::COLOR::CYAN);
    Text->Show_Border(true);
    
    Main->Add_Child(Text);

    GGUI::Resume_Renderer();

    while(true){ 
        //Your programm here :D
    }

}