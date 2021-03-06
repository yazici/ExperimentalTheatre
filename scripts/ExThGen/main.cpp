#include <exception>
#include <iostream>
using namespace std;

#include <CellarWorkbench/Misc/Log.h>
using namespace cellar;

#include <Scaena/ScaenaApplication/Application.h>
#include <Scaena/ScaenaApplication/QGlWidgetView.h>
#include <Scaena/Play/Play.h>
#include <Scaena/Play/Act.h>
using namespace scaena;

#include "PROJECTNAMECharacter.h"


int main(int argc, char** argv) try
{
    getApplication().init(argc, argv);
    
    
    std::shared_ptr<Character> character(new PROJECTNAMECharacter());
    std::shared_ptr<Act> act(new Act("MainAct"));
    act->addCharacter(character);
    
    std::shared_ptr<QGlWidgetView> view(
        new QGlWidgetView("MainView"));
    view->setGlWindowSpace(800, 600);
    view->centerOnScreen();
    view->show();
    
    std::shared_ptr<Play> play(new Play("PROJECTNAME"));
    play->appendAct(act);
    play->addView(view);

    getApplication().setPlay(play);
    return getApplication().execute();    
}
catch(exception& e)
{
    cerr << "Exception caught : " << e.what() << endl;
}
catch(...)
{
    cerr << "Exception passed through.." << endl;
    throw;
}
