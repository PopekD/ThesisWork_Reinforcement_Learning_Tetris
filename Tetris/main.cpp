
#include "Train.h"
#include "RL_Play.h"

int main()
{   
    Train trainer(4, 7, 100000, 10000000);
    trainer.train();


    //RL_Play env;
    //env.play();
    

    return 0;

}
