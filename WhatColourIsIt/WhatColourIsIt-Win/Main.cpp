//#include "stdafx.h"
#include "windows.h"
#include "resource1.h"//#include "ScrnSave.h" 已经被resource1.h包含了
#include "stdio.h"
#include "String.h"
//我是把这个项目的属性改成ANSI字符集，如果用UNICODE会有无法解析的外部符号问题
//没有找到什么解决的好方法目前。




int Hex2Dec(int hexNum)
{
	return (((int)(hexNum/10))* 16 + hexNum % 10);
}
LRESULT CALLBACK ScreenSaverProc(HWND hWnd,UINT message,
	WPARAM wParam,LPARAM lParam)
{
	static int cx = GetSystemMetrics(SM_CXSCREEN);
	static int cy = GetSystemMetrics(SM_CYSCREEN);
	static RECT scrnRECT;
	static PAINTSTRUCT ps = { NULL };
	static HDC hDC = NULL;
	//static HBRUSH hBkBrush;//背景画刷
	static SYSTEMTIME st = { 0 };//存储时间
	static UINT uTimer = 0;
	static char tmpStr[24];
	static TEXTMETRIC tm;
	static HFONT hFont,hFont2;
	static HDC storageDC = NULL;//用来缓冲
	static HBITMAP hBitMap = NULL;

	switch (message)
	{
	case WM_CREATE:
		//建立屏幕区域RECT
		GetClientRect(GetDesktopWindow(), &scrnRECT);
		
		//创建一个计时器
		hFont = CreateFont
			(
			240, 100,    //高度20, 宽取0表示由系统选择最佳值
			0, 0,    //文本倾斜，与字体倾斜都为0
			FW_LIGHT,    //粗体
			0, 0, 0,        //非斜体，无下划线，无中划线
			DEFAULT_CHARSET,    //字符集
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,        //一系列的默认值
			DEFAULT_PITCH | FF_DONTCARE,
			"微软雅黑 Light"    //字体名称
			);
		hFont2 = CreateFont
			(
			60, 25,    //高度20, 宽取0表示由系统选择最佳值
			0, 0,    //文本倾斜，与字体倾斜都为0
			FW_LIGHT,    //粗体
			0, 0, 0,        //非斜体，无下划线，无中划线
			DEFAULT_CHARSET,    //字符集
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,        //一系列的默认值
			DEFAULT_PITCH | FF_DONTCARE,
			TEXT("微软雅黑 Light")    //字体名称
			);
		//设置定时器
		uTimer = SetTimer(hWnd, 1, 1000, NULL);

		hDC = GetDC(hWnd);
		//得到屏幕的兼容dc
		storageDC = CreateCompatibleDC(hDC);
		//hBitMap此时没有内容,只是根据scrnRECT的大小建立了一个与桌面DC兼容的位图 
		hBitMap = (HBITMAP)CreateCompatibleBitmap(hDC, scrnRECT.right, scrnRECT.bottom);//创建桌面兼容位图(也就是屏幕显示兼容),storageDC就不行,因为storageDC是内存环境,不是屏幕显示
		ReleaseDC(hWnd, hDC);
		SelectObject(storageDC, hBitMap);
		SelectObject(storageDC, GetStockObject(DKGRAY_BRUSH));

		break;
	case WM_TIMER:
		//更换用来填充背景的画刷
		GetLocalTime(&st);//获得当前本地时间
		DeleteObject(SelectObject(storageDC, CreateSolidBrush(RGB(Hex2Dec(st.wHour),
			Hex2Dec(st.wMinute), Hex2Dec(st.wSecond))))); 
		//DeleteObject(SelectObject(hDC, CreateSolidBrush(RGB(st.wHour*255/24,
			//st.wMinute * 255 / 60, (st.wSecond*1000+st.wMilliseconds) * 255 / 60000))));
		InvalidateRect(hWnd, &scrnRECT, FALSE);
		break;
	case WM_PAINT:
	
		//Draw BKG
		Rectangle(storageDC, scrnRECT.left, scrnRECT.top, scrnRECT.right, scrnRECT.bottom);//改变背景颜色

		SetBkMode(storageDC, TRANSPARENT);
		SetTextColor(storageDC,RGB(255,255,255));
		
		//This is TimeText
		sprintf_s(tmpStr, "%d%d%s%d%d%s%d%d", 
			(int)(st.wHour / 10), st.wHour%10, ":", 
			(int)(st.wMinute/10),st.wMinute%10, ":", 
			(int)(st.wSecond/10),st.wSecond%10
			);
		SelectObject(storageDC, hFont);
		GetTextMetrics(storageDC, &tm);
		TextOutA(storageDC, cx / 2 - 3.8 * tm.tmAveCharWidth, cy*1/4, tmpStr, lstrlenA(tmpStr));
		
		//This is ColorText
		sprintf_s(tmpStr, "%s%d%d%d%d%d%d", "#",
			(int)(st.wHour / 10), st.wHour % 10,
			(int)(st.wMinute / 10), st.wMinute % 10,
			(int)(st.wSecond / 10), st.wSecond % 10
			);
		SelectObject(storageDC, hFont2);
		GetTextMetrics(storageDC, &tm);
		TextOutA(storageDC, cx / 2 - 4 * tm.tmAveCharWidth, 3 * cy / 4, tmpStr, lstrlenA(tmpStr));

		//画完后一次性bit到屏幕
		hDC = BeginPaint(hWnd, &ps);
		BitBlt(hDC, 0, 0, cx, cy, storageDC, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		//销毁计时器
		if (uTimer)
			KillTimer(hWnd, uTimer);
		//销毁其他东西
		DeleteObject(SelectObject(hDC, GetStockObject(BS_SOLID)));
		break;

	default:
		return DefScreenSaverProc(hWnd, message, wParam, lParam);
	}

	return FALSE;
}

//暂时还没有要给用户设置的功能，故只是写在这里占个坑，现在不起作用
BOOL WINAPI ScreenSaverConfigureDialog(HWND hDlg,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
	)
{
	
	switch (message)   {
	case WM_INITDIALOG:    // 创建设置对话框并初始化各控件值   
		return TRUE;
	case WM_COMMAND:   
		if ( LOWORD( wParam ) == IDOK ) 
		{    // 将用户的输入保存到注册表中 
			EndDialog( hDlg, LOWORD( wParam ) );  
			return TRUE; 
		} 
		else if( LOWORD( wParam ) == IDCANCEL )
		{   
			EndDialog( hDlg, LOWORD( wParam ) ); 
			return TRUE;  
		} 
		break;  
	}
	return FALSE;
}

BOOL WINAPI RegisterDialogClasses(HANDLE hInst)   {  
	// 注册特殊窗口类或自定义窗口类 
	return TRUE;  
}      

