#include <SeqMaker/SeqMaker.h>

#include <iostream>

using namespace std;

int main(void) {
  auto seq = SeqMaker_Init(0, SEQ_UNIT_ORIGINAL);
  if (seq == nullptr) {
    cout << "Init is Failed." << endl;
    return 0;
  } else {
    cout << "Init is Successed!" << endl;
  }
  char* ngram = SeqMaker_CreateUniGram(seq);
  SeqMaker_DeInit(seq);
  cout << "=== N-gram ===" << endl;
  cout << ngram << endl;
  return 0;
}
