#include<iostream>
#include<vector>
#include<stdlib.h>
#include<string>

using namespace std;

//holds the values
struct TreeNode {
     int val;
     TreeNode *left;
     TreeNode *right;
     TreeNode(int x) : val(x), left(NULL), right(NULL) {}
};

class Solution {
private:
    void find_max(vector<int>& nums, int beg, int end, int &pos, int &max_val) {
        max_val = nums[beg];
        pos = beg;
        int i;
        #pragma omp parallel for private(i) shared(max_val, pos) 
        for(i=beg+1; i<end;++i) {
          int tmp = nums[i];
          #pragma omp critical 
          {
              if(tmp > max_val) {
                  max_val = tmp;
                  pos = i;
              }
          }
        }
    }
public:
    TreeNode* constructMaximumBinaryTree(vector<int>& nums, int begin, int endin) {
      if(begin >= endin)
        return NULL;
      int pos, max_val, beg, end;
      beg = begin;
      end = endin;
      find_max(nums, beg, end, pos, max_val); 
      TreeNode *max = new TreeNode(max_val);
      #pragma omp parallel sections
      {
          #pragma omp section 
          {
              max->left = constructMaximumBinaryTree(nums, beg, pos);
          } 
          #pragma omp section
          {
              max->right = constructMaximumBinaryTree(nums, pos+1, endin);
          }
      }
      return max;
    }

  void display(TreeNode *res) {
    if(res != NULL) {
      std::cout<< res->val;
      if(res->left != NULL) { std::cout<<" "<<res->left->val; }
                        else { std::cout<<" NULL"; } 
      if(res->right != NULL) { std::cout<<" "<<res->right->val; }
                        else { std::cout<<" NULL"; } 
      std::cout<<std::endl;
      display(res->left);
      display(res->right);
    } 
  }
};

int main(int argc, char *argv[]) {
  vector<int> *x = new vector<int>(0);
  std::cout<<"Printing the vector: ";
#if 1
  for(int i=0;i<atoi(argv[1]);++i) {
    int tmp = rand()%1000;
    std::cout<<"\t "<<tmp;
    x->push_back(tmp);
  }
#else
  x->push_back(383);
  x->push_back(886);
  x->push_back(777);
  x->push_back(915);
  x->push_back(793);
  x->push_back(335);
  x->push_back(386);
  x->push_back(492);
  x->push_back(649);
  x->push_back(421);
#endif
  std::cout<<std::endl;
  Solution hello;
  TreeNode *result = hello.constructMaximumBinaryTree(*x, 0, atoi(argv[1]));
  std::cout<<"Printing the tree"<<std::endl;
  hello.display(result);
}
