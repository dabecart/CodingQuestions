#include <iostream>
#include <cmath>
#include <vector>

using namespace std;

int maxArea(vector<int>& height) {
    int a = 0, b = height.size()-1;
    int maxArea = min(height[a], height[b])*(b-a);

    bool changed = true;
    while(a < b){
        int area = min(height[a], height[b]) * (b-a);
        if(area > maxArea){
            maxArea = area;
        }

        if(height[b] > height[a])    a++;
        else                         b--;
    }
    return maxArea;
}

int main(){
    vector<int> height = {1,8,6,2,5,4,8,3,7};
    cout << maxArea(height) << endl;
}