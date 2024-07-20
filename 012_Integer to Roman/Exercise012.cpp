#include <iostream>
#include <string>

using namespace std;

string addSubNumber(int coefficient, char one, char five, char ten){
    if(coefficient == 0) return "";

    string out = "";
    if(coefficient > 4 && coefficient <= 8){
        out += five;
        coefficient -= 5;
    }
    
    if(coefficient > 0 && coefficient <= 3){
        for(int i = 0; i < coefficient; i++){
            out += one;
        }
    }else if(coefficient == 9){
        out += one;
        out += ten;
    }else if(coefficient == 4){
        out += one;
        out += five;
    }
    return out;
}

string intToRoman(int num) {
    string out = "";

    // Get the biggest character.
    int power = 1;
    while(num/power != 0){
        power *= 10;
    }
    power /= 10;

    // Start trimming.
    if(power >= 1000){
        int fourth = num/power;
        for(int i = 0; i < fourth; i++){
            out += 'M';
        }

        num %= power;
        power /= 10;
    }

    if(power >= 100){
        int third = num/power;
        out += addSubNumber(third, 'C', 'D', 'M');
        num %= power;
        power /= 10;
    }

    if(power >= 10){
        int second = num/power;
        out += addSubNumber(second, 'X', 'L', 'C');
        num %= power;
        power /= 10;
    }

    if(power >= 1){
        int first = num/power;
        out += addSubNumber(first, 'I', 'V', 'X');
    }

    return out;
}

int main(){
    int n = 3749;
    cout << n << "  =  " << intToRoman(n) << endl;
}