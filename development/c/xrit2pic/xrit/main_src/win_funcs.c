/*******************************************************************
 * Copyright (C) 2003 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 ********************************************************************/
/*******************************************************************
 * functions for windows only. 
 * NOTE: windows.h interferes with xrit2pic.h, so must be in sep. file.
 ********************************************************************/

#if __GTK_WIN32__ == 1
#include <windows.h>
int execcmd(char *cmd,char *env,char *dir,int wait)
{
//char dir[100]; strcpy(dir,"c:\\Program Files\\xrit\\");
  STARTUPINFO         si;
  PROCESS_INFORMATION pi;
  ZeroMemory (&si, sizeof(si));
  si.cb=sizeof (si);

  ZeroMemory( &pi, sizeof(pi) );

  if( !CreateProcess( NULL,   // No module name (use command line)
      cmd,            // Command line
      NULL,           // Process handle not inheritable
      NULL,           // Thread handle not inheritable
      FALSE,          // Set handle inheritance to FALSE
      CREATE_NO_WINDOW, // No window
      env,            // NULL: Use parent's environment block
      dir,            // NULL: Use parent's starting directory 
      &si,            // Pointer to STARTUPINFO structure
      &pi )           // Pointer to PROCESS_INFORMATION structure
  ) 
  {
      return -1;
  }
  // Wait until child process exits.
  if (wait)
    WaitForSingleObject( pi.hProcess, INFINITE );

  // Close process and thread handles. 
  CloseHandle( pi.hProcess );
  CloseHandle( pi.hThread );
  return 0;
}
#endif
