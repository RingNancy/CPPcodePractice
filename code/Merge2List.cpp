# include <bits/stdc++.h>
using namespace std;
struct ListNode {
    int val;
    ListNode *next;
    ListNode() : val(0), next(nullptr) {}
    ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode *next) : val(x), next(next) {}
};

class Solution
{
private:  
public:
    ListNode* MergeTwoList(ListNode *list1, ListNode* list2)
    {
        ListNode *dum = new ListNode(0);
        ListNode *cur = dum;
        while (list1 != nullptr && list2 !=nullptr)
        {
            if(list1->val < list2->val)
            {
                cur->next = list1;
                list1 = list1->next;
            }
            else{  
                cur->next = list2;
                list2 = list2->next;
            }
            cur = cur->next;
        }
        // 最后会剩下一个节点，要么l1空，要么l2 空，
        cur->next = list1 != nullptr ? list1 : list2;
        return dum->next;
    }

};

int main()
{
    // 创建两个有序链表
    ListNode* list1 = new ListNode(1);
    list1->next = new ListNode(3);
    list1->next->next = new ListNode(5);

    ListNode* list2 = new ListNode(2);
    list2->next = new ListNode(4);
    list2->next->next = new ListNode(6);

    // 创建 Solution 对象
    Solution solution;

    // 合并两个有序链表
    ListNode* mergedList = solution.MergeTwoList(list1, list2);

    // 打印合并后的链表
    ListNode* current = mergedList;
    while (current != nullptr) {
        cout << current->val << " ";
        current = current->next;
    }
    cout << endl;

    // 释放内存
    delete list1;
    delete list2;
    delete mergedList;

    system("pause");
    return 0;
}

