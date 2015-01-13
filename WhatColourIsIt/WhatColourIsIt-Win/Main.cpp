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
LRESULT CALLBACK ScreenSaverProc(HWND hWnd,UINT message,
	WPARAM wParam,LPARAM lParam)
{
	static int cx = GetSystemMetrics(SM_CXSCREEN);
	static int cy = GetSystemMetrics(SM_CYSCREEN);
	static RECT scrnRECT;
	static PAINTSTRUCT ps = { NULL };
	static HDC hDC = NULL;
	//static HBRUSH hBkBrush;//������ˢ
	static SYSTEMTIME st = { 0 };//�洢ʱ��
	static UINT uTimer = 0;
	static char tmpStr[24];
	static TEXTMETRIC tm;
	static HFONT hFont,hFont2;
	static HDC storageDC = NULL;//��������
	static HBITMAP hBitMap = NULL;

	switch (message)
	{
	case WM_CREATE:
		//������Ļ����RECT
		GetClientRect(GetDesktopWindow(), &scrnRECT);
		
		//����һ����ʱ��
		hFont = CreateFont
			(
			240, 100,    //�߶�20, ��ȡ0��ʾ��ϵͳѡ�����ֵ
			0, 0,    //�ı���б����������б��Ϊ0
			FW_LIGHT,    //����
			0, 0, 0,        //��б�壬���»��ߣ����л���
			DEFAULT_CHARSET,    //�ַ���
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,        //һϵ�е�Ĭ��ֵ
			DEFAULT_PITCH | FF_DONTCARE,
			"΢���ź� Light"    //��������
			);
		hFont2 = CreateFont
			(
			60, 25,    //�߶�20, ��ȡ0��ʾ��ϵͳѡ�����ֵ
			0, 0,    //�ı���б����������б��Ϊ0
			FW_LIGHT,    //����
			0, 0, 0,        //��б�壬���»��ߣ����л���
			DEFAULT_CHARSET,    //�ַ���
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,        //һϵ�е�Ĭ��ֵ
			DEFAULT_PITCH | FF_DONTCARE,
			TEXT("΢���ź� Light")    //��������
			);
		//���ö�ʱ��
		uTimer = SetTimer(hWnd, 1, 1000, NULL);

		hDC = GetDC(hWnd);
		//�õ���Ļ�ļ���dc
		storageDC = CreateCompatibleDC(hDC);
		//hBitMap��ʱû������,ֻ�Ǹ���scrnRECT�Ĵ�С������һ��������DC���ݵ�λͼ 
		hBitMap = (HBITMAP)CreateCompatibleBitmap(hDC, scrnRECT.right, scrnRECT.bottom);//�����������λͼ(Ҳ������Ļ��ʾ����),storageDC�Ͳ���,��ΪstorageDC���ڴ滷��,������Ļ��ʾ
		ReleaseDC(hWnd, hDC);
		SelectObject(storageDC, hBitMap);
		SelectObject(storageDC, GetStockObject(DKGRAY_BRUSH));

		break;
	case WM_TIMER:
		//����������䱳���Ļ�ˢ
		GetLocalTime(&st);//��õ�ǰ����ʱ��
		DeleteObject(SelectObject(storageDC, CreateSolidBrush(RGB(Hex2Dec(st.wHour),
			Hex2Dec(st.wMinute), Hex2Dec(st.wSecond))))); 
		//DeleteObject(SelectObject(hDC, CreateSolidBrush(RGB(st.wHour*255/24,
			//st.wMinute * 255 / 60, (st.wSecond*1000+st.wMilliseconds) * 255 / 60000))));
		InvalidateRect(hWnd, &scrnRECT, FALSE);
		break;
	case WM_PAINT:
	
		//Draw BKG
		Rectangle(storageDC, scrnRECT.left, scrnRECT.top, scrnRECT.right, scrnRECT.bottom);//�ı䱳����ɫ

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
	
	switch (message)   {
	case WM_INITDIALOG:    // �������öԻ��򲢳�ʼ�����ؼ�ֵ   
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

