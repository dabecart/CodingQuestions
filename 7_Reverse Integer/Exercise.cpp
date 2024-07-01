#include <iostream>
#include <string>
#include <stdint.h>

using namespace std;

int32_t reverse(int32_t x) {
    if(x == 0x8000000) return 0;

    int32_t ret = 0;
    bool negative = x < 0;
    if(negative) x = -x;

    do{
        ret *= 10;
        ret += x%10;
        x /= 10;
    }while(x != 0 && ret < (0xEFFFFFFF/10));
    
    if(x!=0) return 0;

    if(negative) return -ret;
    else return ret;
}

int main() {
    std::cout << reverse(1463847412) << endl;
    return 0;
}