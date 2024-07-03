#include <iostream>
#include <string>

using namespace std;

int myAtoi(string s) {
    int sign = 1;
    int num = 0;

    int i = 0;
    while(s[i] == ' '){
        i++;
    }

    if(s[i] == '+'){
        sign = 1;
        i++;
    }else if(s[i] == '-'){
        sign = -1;
        i++;
    }

    for(; i < s.length(); i++){
        if(s[i] < '0' || s[i] > '9')    break;
        if(num > 0x7FFFFFFF/10){
            if(sign ==  1)  return 0x7FFFFFFF;
            if(sign == -1)  return 0x80000000;
        }
        
        num *= 10;

        if(0x7FFFFFFF-num < s[i] - '0'){
            if(sign ==  1)  return 0x7FFFFFFF;
            if(sign == -1)  return 0x80000000;
        }
        num += s[i] - '0';
    }

    return num*sign;
}

int main(){
    cout << myAtoi("   -042") << endl;
}