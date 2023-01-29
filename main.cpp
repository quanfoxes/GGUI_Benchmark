#include "ggui.h"

#include <vector>

using namespace std;

// IDEAS:
/*
    Put child render nesting before the parents border rendering.
    Take into consider if the child has also border combine it with parents own borders.

*/

void Main(){

    GGUI::Progress_Bar* bar = GGUI::Main->Get_Elements<GGUI::Progress_Bar>()[0];

    while (bar->Get_Progress() <= 1.01){
        bar->Set_Progress(bar->Get_Progress() + 0.01);

        GGUI::SLEEP(16);
    }

}

int main(int Argument_Count, char** Arguments){
    GGUI::GGUI([=](){

        // Setup enables
        GGUI::Enable_Mouse_Movement();

        GGUI::Progress_Bar* bar = new GGUI::Progress_Bar(
            GGUI::COLOR::CYAN,
            GGUI::COLOR::DARK_BLUE,
            GGUI::Main->Get_Width() - 2
        );
        bar->Show_Border(true);
        GGUI::Main->Add_Child(bar);
        GGUI::Main->Add_Child(new GGUI::Progress_Bar(*bar));

        bar->Set_Position({0, 3});
        bar->Set_Height(bar->Get_Height() + 5);
    });

    Main();

    GGUI::SLEEP(INT64_MAX);
    return 0;
}