#include <iostream>
#include <string>

using namespace std;

string convert(string s, int numRows) {
    if(numRows == 1) return s;
    
    string mat[numRows];
    bool upDirection = true;
    int strSelector = 0;
    int strIndex = 0;
    while(strIndex < s.length()){
        mat[strSelector] += s[strIndex++];

        if(upDirection) strSelector++;
        else strSelector--;

        if(strSelector == 0) upDirection = true;
        else if(strSelector >= (numRows-1)) upDirection = false; 
    }

    string out = "";
    for(int i = 0; i < numRows; i++){
        out += mat[i];
    }
    return out;
}

int main() {
    std::cout << convert("AB", 1) << endl;
    return 0;
}