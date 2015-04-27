#include <windows.h>
#include <conio.h>
#include "utils.h"

bool g_pipe;
HANDLE g_handle;

int GetTime()
{
  return GetTickCount();
}
////////////////////////////////////////////////////////////////////////////////


void Highlight(bool on)
{
  WORD intensity = on? FOREGROUND_INTENSITY : 0;
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
      FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | intensity);
}
////////////////////////////////////////////////////////////////////////////////


void InitIO()
{
  DWORD dw;
  
  g_handle = GetStdHandle(STD_INPUT_HANDLE);
  g_pipe = !GetConsoleMode(g_handle, &dw);
  setbuf(stdout, NULL);
}
////////////////////////////////////////////////////////////////////////////////


bool InputAvailable()
{
  DWORD nchars;
  if (stdin->_cnt > 0) return true;
  if (g_pipe)
  {
    if (!PeekNamedPipe(g_handle, NULL, 0, NULL, &nchars, NULL)) return true;
    return (nchars != 0);
  }
  else
    return _kbhit() != 0;
}
////////////////////////////////////////////////////////////////////////////////

