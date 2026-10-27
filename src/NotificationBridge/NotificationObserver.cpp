#include "NotificationObserver.h"

void NotificationObserver::update() override{
    //create notification 
    //ie result notice and then send
    //but need to create channels before hand

    //getState().getStatus() for Subject 
    //Describe the body
    
    string state = subject->getState()->getStatus(); 

    switch (state) {
        case ""
    }


    // int i = subject.getData("API"); 
    // switch(i){
    //     case 1: 

    //     case 2: 

    //     case 3: 

    //     default: 

    // };
}