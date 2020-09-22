//#include "stdafx.h"
#include "windows.h"
#include "resource1.h"//#include "ScrnSave.h" �Ѿ���resource1.h������
#include "stdio.h"
#include "String.h"
//���ǰ������Ŀ�����Ըĳ�ANSI�ַ����������UNICODE�����޷��������ⲿ��������
//û���ҵ�ʲô����ĺ÷���Ŀǰ��



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

	//static HBRUSH hBkBrush;//������ˢ
	static SYSTEMTIME st = { 0 };//�洢ʱ��
	static SYSTEMTIME customST = { 0 };
	static UINT uTimer = 0;
	static char tmpStr[54];
	static TEXTMETRIC tm;
	static HFONT hFont,hFont2;
	static HDC storageDC = NULL;//��������
	static HBITMAP hBitMap = NULL;
	LOGBRUSH lb;
	
	static double yP;  // year progress
	static double yesterdayYP;
	static double todayP; // ������˶��
	switch (message)
	{
	case WM_CREATE:
		//������Ļ����RECT
		GetClientRect(GetDesktopWindow(), &scrnRECT);
		//��������
		hFont = CreateFont
			(
			120, 0,    //�߶�20, ��ȡ0��ʾ��ϵͳѡ�����ֵ
			0, 0,    //�ı���б����������б��Ϊ0
			FW_LIGHT,    //����
			0, 0, 0,        //��б�壬���»��ߣ����л���
			DEFAULT_CHARSET,    //�ַ���
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,        //һϵ�е�Ĭ��ֵ
			DEFAULT_PITCH | FF_DONTCARE,
			"Tahoma"    //��������
			);
		hFont2 = CreateFont
			(
			36, 0,    //�߶�20, ��ȡ0��ʾ��ϵͳѡ�����ֵ
			0, 0,    //�ı���б����������б��Ϊ0
			FW_LIGHT,    //����
			0, 0, 0,        //��б�壬���»��ߣ����л���
			DEFAULT_CHARSET,    //�ַ���
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,        //һϵ�е�Ĭ��ֵ
			DEFAULT_PITCH | FF_DONTCARE,
			TEXT("Tahoma")    //��������
			);
		//���ö�ʱ��
		uTimer = SetTimer(hWnd, 1, 30, NULL); //  interval in ms

		hDC = GetDC(hWnd);
		//�õ���Ļ�ļ���dc
		storageDC = CreateCompatibleDC(hDC);
		//hBitMap��ʱû������,ֻ�Ǹ���scrnRECT�Ĵ�С������һ��������DC���ݵ�λͼ 
		hBitMap = (HBITMAP)CreateCompatibleBitmap(hDC, scrnRECT.right, scrnRECT.bottom);//�����������λͼ(Ҳ������Ļ��ʾ����),storageDC�Ͳ���,��ΪstorageDC���ڴ滷��,������Ļ��ʾ
		ReleaseDC(hWnd, hDC);
		SelectObject(storageDC, hBitMap);
		SelectObject(storageDC, GetStockObject(DKGRAY_BRUSH));
		//��¼����Ϊֹ������˶�� progress
		GetLocalTime(&customST);
		customST.wHour = 0;
		customST.wMinute = 0;
		customST.wSecond = 0;
		yesterdayYP = YProgress(&customST);

		break;
	case WM_TIMER:
		GetLocalTime(&st);//��õ�ǰ����ʱ��
		yP = YProgress(&st);
		customST.wDay = st.wDay;
		customST.wMonth = st.wMonth;
		customST.wYear = st.wYear;
		yesterdayYP = YProgress(&customST);
		todayP = ((st.wHour*3600+st.wMinute*60+st.wSecond)*1000+st.wMilliseconds)/ (1000*86400.0);
		barHeight = (scrnRECT.bottom - scrnRECT.top) / 365.0 + 1;
		//����ʵ�
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
		Rectangle(storageDC, scrnRECT.left, scrnRECT.top, scrnRECT.right+1, scrnRECT.bottom+1);//�ı䱳����ɫ
		// ����Ϊֹ���ϵ��µ�Progress
		//DeleteObject(SelectObject(storageDC, CreateSolidBrush(RGB(255, 228, 11))));
		//Rectangle(storageDC, scrnRECT.left, scrnRECT.top
		//	, scrnRECT.right+1, scrnRECT.bottom*yesterdayYP);
		//��������ҵ�Progress
		//DeleteObject(SelectObject(storageDC, CreateSolidBrush(RGB(255, 228, 11))));
		//Rectangle(storageDC, scrnRECT.left, scrnRECT.bottom*yesterdayYP-1
		//	, scrnRECT.right*todayP+1, scrnRECT.bottom*yesterdayYP+barHeight);
		
				//���Ͻǵ���ǰʱ������
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

		//�����һ����bit����Ļ
		hDC = BeginPaint(hWnd, &ps);
		BitBlt(hDC, 0, 0, cx, cy, storageDC, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		//���ټ�ʱ��
		if (uTimer)
			KillTimer(hWnd, uTimer);
		//������������
		DeleteObject(SelectObject(hDC, GetStockObject(BS_SOLID)));
		break;

	default:
		return DefScreenSaverProc(hWnd, message, wParam, lParam);
	}

	return FALSE;
}

//��ʱ��û��Ҫ���û����õĹ��ܣ���ֻ��д������ռ���ӣ����ڲ�������
BOOL WINAPI ScreenSaverConfigureDialog(HWND hDlg,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
	)
{
	float tt;
	int tmp;
	switch (message)   {
	case WM_INITDIALOG:    // �������öԻ��򲢳�ʼ�����ؼ�ֵ   
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
		{    // ���û������뱣�浽ע����� 
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
	// ע�����ⴰ������Զ��崰���� 
	return TRUE;  
}      

