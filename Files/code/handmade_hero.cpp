#include <Windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
  PSTR lpCmdLine, int nCmdShow)
{
  MessageBox(0, "The program works!", "Hello World!", MB_OK|MB_ICONINFORMATION);
  return 0;
}
