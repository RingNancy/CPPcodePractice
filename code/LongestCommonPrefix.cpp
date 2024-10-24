#include<bits/stdc++.h>
#include <vector>
using namespace std;
class Solution
{
public:
    string longestCommonPrefix(vector<string> &strs){
        if (strs.empty()) return "";
        // minmax_element找出最小最大值，会返回一个first和一个second，分别对应的是最小值和最大值
        const auto p = minmax_element(strs.begin(), strs.end());
        for (int i = 0; i < p.first->size(); i++)
        {
            if(p.first->at(i) != p.second->at(i))
            {
                return p.first->substr(0, i);
            }
        }
        return *p.first;
    }
};

int main()
{
    Solution slu;
    vector<string> strs = {"flower", "flow", "flight"};
    cout<< slu.longestCommonPrefix(strs) << endl;
    system("pause");
}
