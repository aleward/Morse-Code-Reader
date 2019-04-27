// BST Class to sort morse letters (left = dots, right = dash)
// Make it possible to return string of letters if a space is 
// not clearly defined
#include "Arduino.h"
//#define NULL nullptr

struct Node {
  Node(char l): letter(l), leftChild(NULL), rightChild(NULL) {}
  // Variables
  char letter; // 0 = null
  Node* leftChild;
  Node* rightChild;
};

class BST {
private:
  Node* root;
  
public:
  BST(); // Constructs tree of letters in .cpp file
  void printTree(); // For testing
  
  // Accesses the tree to find corresponding letter to the code
  String getTokenLetter(String s);
};
