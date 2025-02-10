#include "../Includes/Process.hpp"

int main(){

    Config arr_Conf[2];
    arr_Conf[0]= Config("Server1", "0.0.0.0", 8080);
    arr_Conf[1] = Config("Server2", "0.0.0.0", 8081);
    Process Proc;
    if(!Proc.start(arr_Conf, 2)){
        std::cout << "ALl Server setup" << std::endl;
        Proc.mainLoop();
    }
    else
        std::cout << "On Prefere le segfault mdr" << std::endl;
    return 1;
}
