#include <iostream>
#include <string>

using namespace std;

string longestPalindrome(string s) {
    string out = s.substr(0,1);
    for(int i = 0; i < s.length(); i++){
        int kernelIndex = i;
        int palLength = 1;
        while((kernelIndex+1) < s.length() && s[kernelIndex+1] == s[kernelIndex]){
            kernelIndex++;
            palLength++;
        }
        int substrRadius = 1;
        while((i-substrRadius >= 0) && (kernelIndex+substrRadius) < s.length() &&
              s[i-substrRadius] == s[kernelIndex+substrRadius]) {
            substrRadius++;
            palLength += 2;
        }
        
        if(palLength > out.length()){
            substrRadius--;
            out = s.substr(i-substrRadius, palLength);
        }
    }
    return out;
}

int main() {
    std::cout << longestPalindrome("aacabdkacaa") << endl;
    return 0;
}