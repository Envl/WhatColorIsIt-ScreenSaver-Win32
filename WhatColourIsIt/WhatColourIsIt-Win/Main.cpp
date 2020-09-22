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

int Date2Day(int yy,int mm,int dd) 
{
	int z, y, m;
	float		j;
	z = 14 - mm;
	z /= 12;
	y = yy + 4800 - z;
	m = mm + 12 * z - 3;
	j = 153 * m + 2;
	j = j / 5 + dd + y * 365 + y / 4 - y / 100 + y / 400 - 2472633;
	return j;
}

double YProgress(SYSTEMTIME* st)
{
	double ADay = 86400;
	double AnHour = 3600;
	double AMin = 60;
	int yy = st->wYear % 100;
	double days = Date2Day(yy, st->wMonth, st->wDay) - Date2Day(yy, 1, 1);
	//double days = Date2Day(yy, 12, 31) - Date2Day(yy, 1, 1); // for validation
	double seconds = (days * ADay) + (st->wHour*AnHour) + (st->wMinute*AMin) + st->wSecond;
	//double seconds = (days * ADay) + (23*AnHour) + (59*AMin) + st->wSecond;   // for validation
	double ms = seconds * 1000 + st->wMilliseconds;

	int year = st->wYear;
	int FIX = ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0);
	int DAYs = 365 + FIX;
	double YearInMs = DAYs * ADay * 1000;
	double p = ms / YearInMs;
	return p;
}

LRESULT CALLBACK ScreenSaverProc(HWND hWnd,UINT message,
	WPARAM wParam,LPARAM lParam)
{
	static int cx = GetSystemMetrics(SM_CXSCREEN);
	static int cy = GetSystemMetrics(SM_CYSCREEN);
	int leftMargin = 0;
	static int barHeight = 0; // the bar of TODAY
	static RECT scrnRECT,todayTxtRect;
	static PAINTSTRUCT ps = { NULL };
	static HDC hDC = NULL;

	//static HBRUSH hBkBrush;//背景画刷
	static SYSTEMTIME st = { 0 };//存储时间
	static SYSTEMTIME customST = { 0 };
	static UINT uTimer = 0;
	static char tmpStr[54];
	static TEXTMETRIC tm;
	static HFONT hFont,hFont2;
	static HDC storageDC = NULL;//用来缓冲
	static HBITMAP hBitMap = NULL;
	LOGBRUSH lb;
	
	static double yP;  // year progress
	static double yesterdayYP;
	static double todayP; // 今天过了多久
	switch (message)
	{
	case WM_CREATE:
		//建立屏幕区域RECT
		GetClientRect(GetDesktopWindow(), &scrnRECT);
		//创建字体
		hFont = CreateFont
			(
			120, 0,    //高度20, 宽取0表示由系统选择最佳值
			0, 0,    //文本倾斜，与字体倾斜都为0
			FW_LIGHT,    //粗体
			0, 0, 0,        //非斜体，无下划线，无中划线
			DEFAULT_CHARSET,    //字符集
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,        //一系列的默认值
			DEFAULT_PITCH | FF_DONTCARE,
			"Tahoma"    //字体名称
			);
		hFont2 = CreateFont
			(
			36, 0,    //高度20, 宽取0表示由系统选择最佳值
			0, 0,    //文本倾斜，与字体倾斜都为0
			FW_LIGHT,    //粗体
			0, 0, 0,        //非斜体，无下划线，无中划线
			DEFAULT_CHARSET,    //字符集
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,        //一系列的默认值
			DEFAULT_PITCH | FF_DONTCARE,
			TEXT("Tahoma")    //字体名称
			);
		//设置定时器
		uTimer = SetTimer(hWnd, 1, 30, NULL); //  interval in ms

		hDC = GetDC(hWnd);
		//得到屏幕的兼容dc
		storageDC = CreateCompatibleDC(hDC);
		//hBitMap此时没有内容,只是根据scrnRECT的大小建立了一个与桌面DC兼容的位图 
		hBitMap = (HBITMAP)CreateCompatibleBitmap(hDC, scrnRECT.right, scrnRECT.bottom);//创建桌面兼容位图(也就是屏幕显示兼容),storageDC就不行,因为storageDC是内存环境,不是屏幕显示
		ReleaseDC(hWnd, hDC);
		SelectObject(storageDC, hBitMap);
		SelectObject(storageDC, GetStockObject(DKGRAY_BRUSH));
		//记录昨天为止今年过了多久 progress
		GetLocalTime(&customST);
		customST.wHour = 0;
		customST.wMinute = 0;
		customST.wSecond = 0;
		yesterdayYP = YProgress(&customST);

		break;
	case WM_TIMER:
		GetLocalTime(&st);//获得当前本地时间
		yP = YProgress(&st);
		customST.wDay = st.wDay;
		customST.wMonth = st.wMonth;
		customST.wYear = st.wYear;
		yesterdayYP = YProgress(&customST);
		todayP = ((st.wHour*3600+st.wMinute*60+st.wSecond)*1000+st.wMilliseconds)/ (1000*86400.0);
		barHeight = (scrnRECT.bottom - scrnRECT.top) / 365.0 + 1;
		//跨年彩蛋
		if (st.wDay == 31 && st.wMonth == 12)
		{
			yesterdayYP = todayP;
			todayP = (st.wSecond*1000.0+st.wMilliseconds)/60000.0;
			barHeight = 1;
		}

		//DeleteObject(SelectObject(storageDC, CreateSolidBrush(RGB(Hex2Dec(st.wHour),
			//Hex2Dec(st.wMinute), Hex2Dec(st.wSecond))))); 
		InvalidateRect(hWnd, &scrnRECT, FALSE);
		break;
	case WM_PAINT:
		DeleteObject(SelectObject(storageDC, CreatePen(PS_NULL,1,RGB(255, 255, 255))));
		//Draw BG
		//DeleteObject(SelectObject(storageDC, CreateSolidBrush(RGB(35, 31, 32))));
		DeleteObject(SelectObject(storageDC, CreateSolidBrush(RGB(0, 0, 0))));
		Rectangle(storageDC, scrnRECT.left, scrnRECT.top, scrnRECT.right+1, scrnRECT.bottom+1);//改变背景颜色
		// 昨天为止从上到下的Progress
		//DeleteObject(SelectObject(storageDC, CreateSolidBrush(RGB(255, 228, 11))));
		//Rectangle(storageDC, scrnRECT.left, scrnRECT.top
		//	, scrnRECT.right+1, scrnRECT.bottom*yesterdayYP);
		//今天从左到右的Progress
		//DeleteObject(SelectObject(storageDC, CreateSolidBrush(RGB(255, 228, 11))));
		//Rectangle(storageDC, scrnRECT.left, scrnRECT.bottom*yesterdayYP-1
		//	, scrnRECT.right*todayP+1, scrnRECT.bottom*yesterdayYP+barHeight);
		
				//左上角到当前时间点的线
		lb.lbColor = RGB(88, 88, 88);
		lb.lbHatch = 0;
		lb.lbStyle = BS_SOLID;
		//DeleteObject(SelectObject(storageDC, CreatePen(PS_SOLID, 3, )));
		//SetGraphicsMode()
		// shadow
		DeleteObject(SelectObject(storageDC, ExtCreatePen(PS_COSMETIC, 1, &lb, 0, NULL)));
		MoveToEx(storageDC, 0, 0, NULL);
		LineTo(storageDC, scrnRECT.right*todayP, scrnRECT.bottom*yesterdayYP + barHeight - 3);
		MoveToEx(storageDC, 0, 0, NULL);
		LineTo(storageDC, scrnRECT.right*todayP + 2, scrnRECT.bottom*yesterdayYP + barHeight - 5);
		// shadow 2
		lb.lbColor = RGB(177, 177, 177);
		DeleteObject(SelectObject(storageDC, ExtCreatePen(PS_COSMETIC, 1, &lb, 0, NULL)));
		MoveToEx(storageDC, 0, 0, NULL);
		LineTo(storageDC, scrnRECT.right*todayP, scrnRECT.bottom*yesterdayYP + barHeight - 4);
		MoveToEx(storageDC, 0, 0, NULL);
		LineTo(storageDC, scrnRECT.right*todayP + 1, scrnRECT.bottom*yesterdayYP + barHeight - 5);
		// main line
		lb.lbColor = RGB(255, 255, 255);
		DeleteObject(SelectObject(storageDC, ExtCreatePen(PS_COSMETIC, 1, &lb, 0, NULL)));
		MoveToEx(storageDC, 0, 0, NULL);
		LineTo(storageDC, scrnRECT.right*todayP + 1, scrnRECT.bottom*yesterdayYP + barHeight-4);
		//circle
		//DeleteObject(SelectObject(storageDC, CreatePen(PS_NULL, 1, RGB(255, 255, 255))));
		DeleteObject(SelectObject(storageDC, CreateSolidBrush(RGB(255, 255, 255))));
		Ellipse(storageDC, scrnRECT.right*todayP - 7, scrnRECT.bottom*yesterdayYP - 7
			, scrnRECT.right*todayP + 7, scrnRECT.bottom*yesterdayYP + 7);

		//Draw Text
		DeleteObject(SelectObject(storageDC, CreatePen(PS_NULL, 1, RGB(255, 255, 255))));
		SetBkMode(storageDC, TRANSPARENT);
		//SetBkColor(storageDC, RGB(255, 228, 11));
		//SetTextColor(storageDC, RGB(255, 255, 255));
		SetTextColor(storageDC, RGB(255, 255, 255));

		//This is Progress
		sprintf_s(tmpStr, "%.7lf%s",yP*100,"%");
		SelectObject(storageDC, hFont);
		GetTextMetrics(storageDC, &tm);
		leftMargin=cx / 6 - 3.8 * tm.tmAveCharWidth;
		TextOutA(storageDC, leftMargin, cy * 3 / 4, tmpStr, lstrlenA(tmpStr));
		
		//This is Text of year Progress
		sprintf_s(tmpStr, "of %d has past",st.wYear);
		SelectObject(storageDC, hFont2);
		GetTextMetrics(storageDC, &tm);
		TextOutA(storageDC, leftMargin, 8 * cy / 9, tmpStr, lstrlenA(tmpStr));
		// Text of day progress
		leftMargin = scrnRECT.right*todayP *0.9;
		sprintf_s(tmpStr, "%.7lf%s", todayP * 100, "%");
		TextOutA(storageDC, leftMargin, scrnRECT.bottom*yesterdayYP + 10, tmpStr, lstrlenA(tmpStr));
		TextOutA(storageDC, leftMargin, scrnRECT.bottom*yesterdayYP + 45, "of today has passed", lstrlenA("of today has passed"));

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
	float tt;
	int tmp;
	switch (message)   {
	case WM_INITDIALOG:    // 创建设置对话框并初始化各控件值   
		SYSTEMTIME customST;
		RECT r;
		GetClientRect(GetDesktopWindow(), &r);
		 tmp = (r.bottom-r.top) / 365;
		GetLocalTime(&customST);
		tt = ((customST.wHour * 3600 + customST.wMinute * 60 + customST.wSecond) * 1000 + customST.wMilliseconds) / (1000 * 86400.0);
		tt = YProgress(&customST);
		customST.wHour = 0;
		customST.wMinute = 0;
		customST.wSecond = 0;
		tt = YProgress(&customST);
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

