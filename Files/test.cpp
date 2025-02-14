#include <Windows.h>
#include <iostream>
using namespace std;

int main() {
  cout << "Hello World!";
  return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
  PSTR lpCmdLine, int nCmdShow)
{
  main();
}
