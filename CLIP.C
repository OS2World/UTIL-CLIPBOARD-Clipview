#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include <stdio.h>
#include <string.h>
#include "clip.h"

/*
    This is a pretty primitive Clipboard Viewer utility for OS/2.
    Currently it supports only Text and Bitmaps. Could use some
    optimization and features. But, what the hell, it works! (I think!)

      Program Name.....: Clipview
      Version..........: 1.0
      Written By.......: Jay F. McLain (CompuServe 76701,157)
      Date.............: May 30, 1989 (Clean-up December 10, 1989)
      Operating System.: OS/2 PM Version 1.10
*/

/*
    UPDATES\ENHANCEMENTS:
*/


HAB      hAB;            /* Handle to Application Anchor Block         */
HDC      hDC,            /* Handle to Application Device Context       */
         hMemDC;         /* Handle to Bitmap Memory Device Context     */
HPS      hMemPS;         /* Handle to Memory Presentation Space        */
HWND     hwndFrame,      /* Handle to Application Frame                */
         hwndClient,     /* Handle to Application Client Area          */
         hwndSysMenu,    /* Handle to Application System Menu          */
         hwndSysSubMenu; /* Handle to Application Sub Menu             */
MENUITEM miSysMenu;      /* Menu Item Styles                           */
SHORT    sItem,          /* Loop Variable for Initializing System Menu */
         idSysMenu;      /* ID for System Menu                         */

PSZ      pszData[4]      = { "Display", NULL, NULL, NULL };
SIZEL    sizlPage;
POINTL   aptl[6];

CHAR     szClientClass[] = "Clipview";

CHAR     *szMenuText[2]  = {NULL,
                            "A~bout Clipview..." };

MENUITEM mi[2] = { MIT_END, MIS_SEPARATOR, 0,         0, NULL, NULL,
                   MIT_END, MIS_TEXT,      0, IDM_ABOUT, NULL, NULL };


/****************************** Clipboard MAIN() ******************************/

INT main(VOID) {
  static ULONG flFrameFlags = FCF_TITLEBAR      | FCF_SYSMENU |
                              FCF_SIZEBORDER    | FCF_MINMAX  |
                              FCF_SHELLPOSITION | FCF_ICON;

  HMQ  hMQ;  /* Handle to Application Message Queue */
  QMSG qmsg; /* A Queue Message                     */

  hAB = WinInitialize( NULL );
  hMQ = WinCreateMsgQueue( hAB, 0 );

  WinRegisterClass( hAB, szClientClass, ClipboardWndProc, CS_SIZEREDRAW, 0 );

  hwndFrame = WinCreateStdWindow( HWND_DESKTOP,
                                  WS_VISIBLE,
                                  &flFrameFlags,
                                  szClientClass,
                                  APP_TITLE,
                                  0L,
                                  (HMODULE) NULL,
                                  ID_CLIPBOARD,
                                  &hwndClient );

  while ( WinGetMsg( hAB, &qmsg, NULL, 0, 0 ) )
    WinDispatchMsg( hAB, &qmsg );

  WinDestroyWindow( hwndFrame );
  WinDestroyMsgQueue( hMQ );
  WinTerminate( hAB );
  return 0;
}


/************************* Clipboard Window Procedure *************************/

MRESULT EXPENTRY ClipboardWndProc(HWND hWnd, USHORT msg, MPARAM mp1, MPARAM mp2) {
  switch ( msg ) {
    case WM_CREATE:
      hDC = WinOpenWindowDC(hWnd);

      hMemDC = DevOpenDC( hAB,
                          OD_MEMORY,
                          "*",
                          4L,
                          (PDEVOPENDATA) pszData,
                          hDC );

      hMemPS = GpiCreatePS( hAB,
                            hMemDC,
                            &sizlPage,
                            PU_PELS | GPIA_ASSOC | GPIT_MICRO );

      hwndSysMenu = WinWindowFromID( WinQueryWindow( hWnd, QW_PARENT, FALSE ),
                                     FID_SYSMENU );

      idSysMenu   = SHORT1FROMMR( WinSendMsg( hwndSysMenu,
                                              MM_ITEMIDFROMPOSITION,
                                              NULL,
                                              NULL ) );

      WinSendMsg( hwndSysMenu,
                  MM_QUERYITEM,
                  MPFROM2SHORT( idSysMenu,FALSE ),
                  MPFROMP( &miSysMenu ) );

      hwndSysSubMenu = miSysMenu.hwndSubMenu;

      for (sItem = 0; sItem < 2; sItem++)
        WinSendMsg( hwndSysSubMenu,
                    MM_INSERTITEM,
                    MPFROMP( mi + sItem ),
                    MPFROMP( szMenuText[sItem] ) );

      WinSetClipbrdViewer( hAB, hWnd );
      return 0;

    case WM_COMMAND:
      switch ( COMMANDMSG( &msg )->cmd ) {
        case IDM_ABOUT:
          WinDlgBox( HWND_DESKTOP,
                     hWnd,
                     AboutDlgProc,
                     NULL,
                     IDD_ABOUT,
                     NULL );
          return 0;
      }
      break;

    case WM_PAINT:
    {
      HPS     hPS;         /* Handle to WM_PAINT Presentation Space */
      USHORT  usLen,       /* String Length                         */
              usfInfo;     /* Clipboard Format Information          */
      HBITMAP hClipBitmap; /* Handle to Clipboard Bitmap            */
      SEL     selClipText; /* Memory Selector for Clipboard Text    */
      PCHAR   pchClipText; /* Pointer to Clipboard Text             */
      POINTL  ptlStart;    /* String Display Location               */
      SWP     swp;         /* Application Client Area Location\Size */
      RECTL   rectl;       /* WM_PAINT Invalid Rectangle            */


      hPS = WinBeginPaint( hWnd, NULL, &rectl );

      WinFillRect( hPS, &rectl, CLR_BLACK );

      /* Is Information in Clipboard Text? */
      if ( WinQueryClipbrdFmtInfo( hAB, CF_TEXT, &usfInfo ) ) {
        WinSetWindowText( hwndFrame, "Clipboard -- Text" );
        WinOpenClipbrd( hAB );

        if ( (selClipText =
                (SEL) WinQueryClipbrdData( hAB, CF_TEXT ) ) != NULL ) {
          pchClipText = MAKEP( selClipText, 0 );

          ptlStart.x = 10;
          ptlStart.y = 10;

          /* Find Length of String in Clipboard */
          for (usLen = 0; pchClipText[usLen]; usLen++);

          GpiCharStringAt( hPS, &ptlStart, (LONG) usLen, pchClipText );
        }

        WinCloseClipbrd( hAB );
      }
      else {

      /* Is Information in Clipboard Bitmap? */
        if ( WinQueryClipbrdFmtInfo( hAB, CF_BITMAP, &usfInfo ) ) {
          WinSetWindowText( hwndFrame, "Clipboard -- Bitmap");
          WinOpenClipbrd( hAB );

          if ( ( hClipBitmap =
                   (HBITMAP) WinQueryClipbrdData( hAB, CF_BITMAP ) ) != NULL ) {
            GpiSetBitmap( hMemPS, hClipBitmap );
            WinQueryWindowPos( hWnd, &swp );

            aptl[0].x = (LONG) swp.cx;
            aptl[0].y = (LONG) swp.cy;

            aptl[1].x = (LONG) 0;
            aptl[1].y = (LONG) 0;

            aptl[2].x = (LONG) 0;
            aptl[2].y = (LONG) 0;

            GpiBitBlt( hPS, hMemPS, 3L, aptl, ROP_SRCCOPY, BBO_IGNORE );
          }

          WinCloseClipbrd( hAB );
        }
      }

      WinEndPaint( hPS );
    }
    return 0;

    case WM_DRAWCLIPBOARD:                   /* If New Data in Clipboard      */
      WinInvalidateRect( hWnd, NULL, NULL ); /* Invalidate Client Area.       */
      return 0;                              /* (Send WM_PAINT Message)       */

    case WM_DESTROY:                      /* Application is Terminating.      */
      GpiDestroyPS( hMemPS );             /* So, Destroy Presentation Space   */
      DevCloseDC( hMemDC );               /* and, Close Memory Device Context */
      return 0;
  }

  return WinDefWindowProc( hWnd, msg, mp1, mp2 );
}


/************************* AboutBox Dialog Procedure *************************/

MRESULT EXPENTRY AboutDlgProc(HWND hWnd, USHORT msg, MPARAM mp1, MPARAM mp2) {
  switch ( msg ) {
    case WM_COMMAND:
      switch ( COMMANDMSG( &msg )->cmd ) {
        case DID_OK:
          WinDismissDlg( hWnd, TRUE );
          return 0;
      }

      break;
  }

  return WinDefDlgProc( hWnd, msg, mp1, mp2 );
}
