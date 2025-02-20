#include <Windows.h>
#include <iostream>
using namespace std;

void main() {
  int t = 16;
  int *p = &t;
  cout << "Hello World!";
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
  PSTR lpCmdLine, int nCmdShow)
{
  cout << "here!";
}
