#include <bits/stdc++.h>
using namespace std;


class Solution
{
public:
    int roman2Int(string s){
        int result = 0;
        unordered_map<char, int> romInt = {
            {'I',1},
            {'V',5},
            {'X',10},
            {'L',50},
            {'C',100},
            {'D', 500},
            {'M', 1000}

        };
        for (int i = 0; i < s.length(); i++)
        {
            if(romInt[s[i]] < romInt[s[i + 1]]){
                result -= romInt[s[i]];
            }
            else{
                result += romInt[s[i]];
            }
        }
        return result;
    }
};
int main()
{
    Solution slu;
    cout << slu.roman2Int("IV") << endl;
    system("pause");
    return 0;
}

