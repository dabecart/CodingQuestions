#include <iostream>
#include <string>
#include <stdint.h>

using namespace std;

bool isPalindrome(int x) {
    string s = to_string(x);
    int len = s.length();

    int i = 0;
    while(i < len/2){
        if(s[i] != s[len-1-i]){
            return false;
        }
        i++;
    }
    return true;
}

int main(){
    cout << isPalindrome(121) << endl;
}