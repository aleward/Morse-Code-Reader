#include "BST.h"
#include <string.h>
// Sets up letter tree

BST::BST() 
  : root(new Node(0))
{
  // GOING DOWN FROM THE ROOT AND CONSTRUCTING THE TREE
  
  // Down left side
  // .
  Node* e = new Node('e');
  root->leftChild = e;
  // ..
  Node* i = new Node('i');
  e->leftChild = i;
  // ...
  Node* s = new Node('s');
  i->leftChild = s;
  // ....
  Node* h = new Node('h');
  s->leftChild = h;
  h->leftChild = new Node('5'); // .....
  h->rightChild = new Node('4'); // ....-
  
  // Going back up
  // ...-
  Node* v = new Node('v');
  s->rightChild = v;
  v->rightChild = new Node('3'); // ...--
  // ..-
  Node* u = new Node('u');
  i->rightChild = u;
  // ..-.
  u->leftChild = new Node('f');
  u->rightChild = new Node(0); // nothing, bridge to:
  u->rightChild->rightChild = new Node('2'); // ..---
  // .-
  Node* a = new Node('a');
  e->rightChild = a;
  //.-.
  Node* r = new Node('r');
  a->leftChild = r;
  // .-..
  r->leftChild = new Node('l');
  // .--
  Node* w = new Node('w');
  a->rightChild = w;
  // .--.
  w->leftChild = new Node('p');
  // .---
  Node* j = new Node('j');
  w->rightChild = j;
  j->rightChild = new Node('1');
  

  // Down right side
  // -
  Node* t = new Node('t');
  root->rightChild = t;
  // -.
  Node* n = new Node('n');
  t->leftChild = n;
  // -..
  Node* d = new Node('d');
  n->leftChild = d;
  // -...
  Node* b = new Node('b');
  d->leftChild = b;
  b->leftChild = new Node('6'); // -....
  // -..-
  d->rightChild = new Node('x');
  
  // Going back up
  // -.-
  Node* k = new Node('k');
  n->rightChild = k;
  // -.-.
  k->leftChild = new Node('c');
  // -.--
  k->rightChild = new Node('y');
  
  // --
  Node* m = new Node('m');
  t->rightChild = m;
  //--.
  Node* g = new Node('g');
  m->leftChild = g;
  // --..
  Node* z = new Node('z');
  g->leftChild = z;
  z->leftChild = new Node('7'); //--...
  //--.-
  g->rightChild = new Node('q');
  // ---
  Node* o = new Node('o');
  m->rightChild = o;
  o->leftChild = new Node(0); // nothing, bridge to:
  o->leftChild->leftChild = new Node('8'); // ---..
  o->rightChild = new Node(0); // nothing, bridge to:
  o->rightChild->leftChild = new Node('9'); // ----.
  o->rightChild->rightChild = new Node('0'); // -----
  
}

// A helper for the print function below
void helper(Node* curr, String s) {
  if (curr != NULL) {
    if (curr->letter != 0) {
      String temp = ": " + s;
      Serial.println(curr->letter + temp);
    }
    helper(curr->leftChild, s + '.');
    helper(curr->rightChild, s + '-');
  }
}

// Print all letters encoded in the tree
void BST::printTree() {
  helper(root->leftChild, ".");
  helper(root->rightChild, "-");
}


/* Instead of returning some error character if a search of the tree 
 * doesn't land on a letter, I record the last encountered letter, 
 * and then loop back to the root to find more */
 
/* In other words, this function uses a string of .'s and -'s to 
 * parse the tree for a letter - while also accounting for user 
 * error, or if a space between characters was not detected */
String BST::getTokenLetter(String s) {
  // The letter(s) to return
  String let = "";

  // Starting at the root node, we go down the tree for each 
  // character in the screen
  Node* curr = root;
  Node* prev = root; // To be able to backtrack
  
  for (int i = 0; i < s.length(); i++) {
    // Go left if the character is a .
    if (s[i] == '.') {
      
      // In case of mistakes
      if (curr->leftChild == NULL) {
        if (curr->letter == 0) { // If you land on the 0-char Node
          let += prev->letter;
          i -= 2;
          curr = root;
          prev = root;
          
        } else { // If you land on a null Node
          let += curr->letter;
          curr = root->leftChild;
          prev = root;
        }
      } 
      
      // Normal case
      else {
        prev = curr;
        curr = curr->leftChild;
      }
    } 

    // Go right if the character is a -
    else if (s[i] == '-') {
      
      // In case of mistakes
      if (curr->rightChild == NULL) {
        if (curr->letter == 0) { // If you land on the 0-char Node
          let += prev->letter;
          i -= 2;
          curr = root;
          prev = root;
          
        } else { // If you land on a null Node
          let += curr->letter;
          curr = root->rightChild;
          prev = root;
        }
      } 
      
      // Normal case
      else {
        prev = curr;
        curr = curr->rightChild;
      }
    }
  }

  // Final letter - from last node landed on after parsing the string
  let += curr->letter;

  return let;
}
