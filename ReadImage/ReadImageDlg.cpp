#include "stdafx.h"
#include "ReadImage.h"
#include "ReadImageDlg.h"
#include "afxdialogex.h"

#include <sys/stat.h>

#include <wininet.h>
#pragma comment(lib, "wininet.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include <baseapi.h>
#include <allheaders.h>

using namespace tesseract;

#define FileApp		L"ConvertImageToText.exe"
#define FileBMP		L"file_ocr.bmp"

/* clone definition of WIN32_FILE_ATTRIBUTE_DATA from WINAPI header */
typedef struct file_info_struct
{
	DWORD    dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	DWORD    nFileSizeHigh;
	DWORD    nFileSizeLow;
} FILE_INFO;

/* function pointer to GetFileAttributesEx */
typedef BOOL(WINAPI *GET_FILE_ATTRIBUTES_EX)(LPCWSTR lpFileName, int fInfoLevelId, LPVOID lpFileInformation);


// CReadImageDlg dialog

CReadImageDlg::CReadImageDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_READIMAGE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CReadImageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, MAIN_TXT_BROWSE, m_txt_browse);
	DDX_Control(pDX, MAIN_TXT_RESULT, m_txt_result);
	DDX_Control(pDX, MAIN_TXT_TRANSLATE, m_txt_translate);
}

BEGIN_MESSAGE_MAP(CReadImageDlg, CDialogEx)
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(MAIN_BTN_BROWSE, &CReadImageDlg::OnBnClickedBtnBrowse)
	ON_BN_CLICKED(MAIN_BTN_OPEN, &CReadImageDlg::OnBnClickedBtnOpen)
	ON_BN_CLICKED(MAIN_BTN_CONVERT, &CReadImageDlg::OnBnClickedBtnConvert)
	ON_BN_CLICKED(MAIN_BTN_TRANSLATE, &CReadImageDlg::OnBnClickedBtnTranslate)
	ON_BN_CLICKED(MAIN_BTN_GOOGLE_TRANSLATE, &CReadImageDlg::OnBnClickedBtnGoogleTranslate)
	ON_BN_CLICKED(MAIN_BTN_GOOGLE_SEARCH, &CReadImageDlg::OnBnClickedBtnGoogleSearch)
	ON_BN_CLICKED(MAIN_BTN_CLOSE, &CReadImageDlg::OnBnClickedBtnClose)	
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CReadImageDlg message handlers

BOOL CReadImageDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();	

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	szpathDefault = GetPathFolder(FileApp);
	m_txt_browse.SetCueBanner(L"Kích chọn tệp tin hình ảnh cần đọc...");
	iMouse_X = 0, iMouse_Y = 0;
	iMsClick = 0;	

	CString szPathImage = LoadFileDefault();
	if (szPathImage != L"")
	{
		m_txt_browse.SetWindowText(szPathImage);
		OnBnClickedBtnConvert();
	}

	SetTimer(1, 800, NULL);	// 1000ms = 1 second

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CReadImageDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CReadImageDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CReadImageDlg::PreTranslateMessage(MSG* pMsg)
{
	int iMes = (int)pMsg->message;
	int iWPar = (int)pMsg->wParam;
	HWND pHWnd = pMsg->hwnd;
	CWnd* pWndWithFocus = GetFocus();

	if (iMes == WM_KEYDOWN)
	{
		if (iWPar == VK_ESCAPE)
		{
			OnBnClickedBtnClose();
			return TRUE;
		}
	}
	else if (iMes == WM_CHAR)
	{
		TCHAR chr = static_cast<TCHAR>(iWPar);
		switch (chr)
		{
		case 0x04:
		{
			if (pWndWithFocus == GetDlgItem(MAIN_TXT_RESULT))
			{
				OnBnClickedBtnTranslate();
				return TRUE;
			}
			break;
		}

		default:
			break;
		}
	}

	// Sự kiện click chuột vào ô text-box
	if (m_txt_result.m_hWnd == pHWnd && iWPar == VK_LBUTTON)
	{
		POINT ptMouse;
		GetCursorPos(&ptMouse);
		int X = (int)ptMouse.x;
		int Y = (int)ptMouse.y;

		if (iMouse_X == 0) iMouse_X = X;
		if (iMouse_Y == 0) iMouse_Y = Y;

		// Kiểm tra cùng vị trí chuột
		if (iMouse_X == X && iMouse_Y == Y)
		{
			CheckMouseClick();
			if (iMsClick >= 2) return TRUE;
		}
	}

	return FALSE;
}


void CReadImageDlg::OnBnClickedBtnBrowse()
{
	try
	{
		AFileFilter	ff(L"File image|*.jpg;*.bmp;*.png|All files|*.*||");
		CFileDialog dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_NOCHANGEDIR,
			L"File image|*.jpg;*.bmp;*.png|All files|*.*||");
		dlgFile.m_ofn.lpstrTitle = _T("Select the image file to use...");

		if (dlgFile.DoModal() != IDOK) return;

		CString szpath = dlgFile.GetPathName();
		m_txt_browse.SetWindowText(szpath);
		m_txt_result.SetWindowText(L"");
		m_txt_translate.SetWindowText(L"");
	}
	catch (...) {}
}

void CReadImageDlg::OnBnClickedBtnOpen()
{
	CString szPathImage = L"";
	m_txt_browse.GetWindowTextW(szPathImage);
	if (FileExists(szPathImage) == 1)
	{
		ShellExecute(NULL, L"open", szPathImage, NULL, NULL, SW_SHOWMAXIMIZED);
	}
}

void CReadImageDlg::OnBnClickedBtnConvert()
{
	CString szPathImage = L"";
	m_txt_browse.GetWindowTextW(szPathImage);

	CString szResult = L"";
	if (FileExists(szPathImage) == 1)
	{
		CString szPreprocessedFile = TesseractPreprocess(szPathImage);
		szResult = TesseractOCR(szPreprocessedFile);
		DeleteFile(szpathDefault + FileBMP);
	}

	szResult.Trim();
	szResult.Replace(L"\n", L" ");
	m_txt_result.SetWindowText(szResult);
	GotoDlgCtrl(GetDlgItem(MAIN_TXT_RESULT));
}

void CReadImageDlg::OnBnClickedBtnTranslate()
{
	try
	{
		if (CheckConnection() == true)
		{
			CString szTranslte = L"";
			int iStart = -1, iEnd = -1;
			m_txt_result.GetSel(iStart, iEnd);
			m_txt_result.GetWindowTextW(szTranslte);

			if (iStart >= 0 && iEnd > iStart)
			{
				szTranslte = szTranslte.Mid(iStart, iEnd - iStart);
			}

			szTranslte.Trim();
			szTranslte.Replace(L".", L"@@@");
			szTranslte.Replace(L";", L"@@@");
			szTranslte.Replace(L"\n", L" ");

			if (szTranslte != L"")
			{
				CString sURLLink = L"";
				CString szKey = ReplateUTF8Web(szTranslte);
				sURLLink.Format(L"https://translate.googleapis.com/translate_a/single?"
					L"client=gtx&sl=en&tl=vi&dt=t&dj=1&q=%s&ie=UTF-8&oe=UTF-8", szKey);
				szTranslte = ReadingWebsite(sURLLink);
			}

			szTranslte.Replace(L"@@@", L".");
			CString szch = L"~`!#$%^&()=[]{};:\"'|,<>?©®";
			CString szen[26] = { L"\\u007e", L"\\u0060", L"\\u0021", L"\\u0023", L"\\u0024", L"\\u0025", L"\\u005e",
				L"\\u0026", L"\\u0028", L"\\u0029", L"\\u003d", L"\\u005b", L"\\u005d", L"\\u007b", L"\\u007d",
				L"\\u003b", L"\\u003a", L"\\u0022", L"\\u0027", L"\\u007c", L"\\u002c", L"\\u003c", L"\\u003e",
				L"\\u003f", L"\\u00a9", L"\\u00ae" };

			for (int i = 0; i < 24; i++) szTranslte.Replace(szen[i], szch.Mid(i, 1));
			szTranslte.Replace(L"\\u003E", L">");

			m_txt_translate.SetWindowText(szTranslte);
			GotoDlgCtrl(GetDlgItem(MAIN_TXT_RESULT));
			m_txt_result.SetSel(iStart, iEnd);
		}		
	}
	catch (...){}
}

void CReadImageDlg::OnBnClickedBtnGoogleTranslate()
{
	CString szResult = L"";
	m_txt_result.GetWindowTextW(szResult);
	if (szResult != L"")
	{
		szResult = ReplateUTF8Web(szResult);
		CString szURLLink = L"https://translate.google.com/#view=home&op=translate&sl=en&tl=vi&text=";
		szURLLink += szResult;
		if (CheckConnection() == true) ShellExecute(NULL, L"open", szURLLink, NULL, NULL, SW_SHOWMAXIMIZED);
	}
}

void CReadImageDlg::OnBnClickedBtnGoogleSearch()
{
	CString szResult = L"";
	m_txt_result.GetWindowTextW(szResult);
	if (szResult != L"")
	{
		szResult = ReplateUTF8Web(szResult);
		CString szURLLink = L"https://www.google.com/search?q=";
		szURLLink += szResult;
		if (CheckConnection() == true) ShellExecute(NULL, L"open", szURLLink, NULL, NULL, SW_SHOWMAXIMIZED);
	}
}

void CReadImageDlg::OnBnClickedBtnClose()
{
	CDialogEx::OnCancel();
}

char* CReadImageDlg::ConvertCstringToChars(CString szvalue)
{
	wchar_t* wszString = new wchar_t[szvalue.GetLength() + 1];
	wcscpy(wszString, szvalue);
	int num = WideCharToMultiByte(CP_UTF8, NULL, wszString, wcslen(wszString), NULL, 0, NULL, NULL);
	char* chr = new char[num + 1];
	WideCharToMultiByte(CP_UTF8, NULL, wszString, wcslen(wszString), chr, num, NULL, NULL);
	chr[num] = '\0';
	return chr;
}

CString CReadImageDlg::ConvertCharsToCstring(char* chr)
{
	int num = MultiByteToWideChar(CP_UTF8, 0, chr, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[num + 1];
	MultiByteToWideChar(CP_UTF8, 0, chr, -1, wstr, num);
	wstr[num] = '\0';
	CString szval = L"";
	szval.Format(L"%s", wstr);
	return szval;
}

int CReadImageDlg::FileExists(CString szPath)
{
	try
	{
		struct stat fileInfo;
		if (szPath.Right(1) == L"\\") szPath = szPath.Left(szPath.GetLength() - 1);
		if (!stat(ConvertCstringToChars(szPath), &fileInfo)) return 1;
		return -1;
	}
	catch (...) { return -1; }
	return 0;
}

CString CReadImageDlg::GetPathFolder(CString szPath)
{
	try
	{
		HMODULE hModule;
		hModule = GetModuleHandle(szPath);
		TCHAR szFileName[1024];
		DWORD dSize = 1024;
		GetModuleFileName(hModule, szFileName, dSize);
		CString szSource = szFileName;
		for (int i = szSource.GetLength() - 1; i > 0; i--)
		{
			if (szSource.GetAt(i) == '\\')
			{
				szSource = szSource.Left(szSource.GetLength() - (szSource.GetLength() - i) + 1);
				break;
			}
		}
		return szSource;
	}
	catch (...) {}
	return L"";
}

CString CReadImageDlg::GetDesktopDir()
{
	CString szpathDesktop = L"";
	char* szpath = new char[MAX_PATH + 1];
	if (SHGetSpecialFolderPathA(HWND_DESKTOP, szpath, CSIDL_DESKTOP, FALSE))
	{
		szpathDesktop = ConvertCharsToCstring(szpath);
		if (szpathDesktop.Right(1) != L"\\") szpathDesktop += L"\\";
	}
	return szpathDesktop;
}

CString CReadImageDlg::TesseractPreprocess(CString szPathImage)
{
	try
	{
		BOOL perform_negate = TRUE;
		l_float32 dark_bg_threshold = 0.5f;

		int perform_scale = 1;
		l_float32 scale_factor = 3.5f;

		int perform_unsharp_mask = 1;
		l_int32 usm_halfwidth = 5;
		l_float32 usm_fract = 2.5f;

		int perform_otsu_binarize = 1;
		l_int32 otsu_sx = 2000;
		l_int32 otsu_sy = 2000;
		l_int32 otsu_smoothx = 0;
		l_int32 otsu_smoothy = 0;
		l_float32 otsu_scorefract = 0.0f;

		l_int32 status = 1;
		l_float32 border_avg = 0.0f;
		PIX *pixs = NULL;
		char *ext = NULL;

		pixs = pixRead(ConvertCstringToChars(szPathImage));
		pixs = pixConvertRGBToGray(pixs, 0.0f, 0.0f, 0.0f);

		if (perform_negate)
		{
			PIX *otsu_pixs = NULL;
			status = pixOtsuAdaptiveThreshold(pixs, otsu_sx, otsu_sy, otsu_smoothx, otsu_smoothy, otsu_scorefract, NULL, &otsu_pixs);

			border_avg = pixAverageOnLine(otsu_pixs, 0, 0, otsu_pixs->w - 1, 0, 1);                               // Top 
			border_avg += pixAverageOnLine(otsu_pixs, 0, otsu_pixs->h - 1, otsu_pixs->w - 1, otsu_pixs->h - 1, 1); // Bottom 
			border_avg += pixAverageOnLine(otsu_pixs, 0, 0, 0, otsu_pixs->h - 1, 1);                               // Left 
			border_avg += pixAverageOnLine(otsu_pixs, otsu_pixs->w - 1, 0, otsu_pixs->w - 1, otsu_pixs->h - 1, 1); // Right 
			border_avg /= 4.0f;

			pixDestroy(&otsu_pixs);

			if (border_avg > dark_bg_threshold) pixInvert(pixs, pixs);
		}

		if (perform_scale) pixs = pixScaleGrayLI(pixs, scale_factor, scale_factor);
		if (perform_unsharp_mask) pixs = pixUnsharpMaskingGray(pixs, usm_halfwidth, usm_fract);
		if (perform_otsu_binarize) status = pixOtsuAdaptiveThreshold(pixs, otsu_sx, otsu_sy, otsu_smoothx, otsu_smoothy, otsu_scorefract, NULL, &pixs);

		CString szpathBitmap = szpathDefault + FileBMP;
		status = pixWriteImpliedFormat(ConvertCstringToChars(szpathBitmap), pixs, 0, 0);

		return szpathBitmap;
	}
	catch (...) {}
	return L"";
}

CString CReadImageDlg::TesseractOCR(CString szPreprocessedFile)
{
	try
	{
		Pix *image = pixRead(ConvertCstringToChars(szPreprocessedFile));
		TessBaseAPI *api = new TessBaseAPI();

		CString szPathData = szpathDefault + L"data\\";
		if (FileExists(szPathData) == 1)
		{
			api->Init(ConvertCstringToChars(szPathData), "eng");
			api->SetPageSegMode(PSM_AUTO_OSD);
			api->SetImage(image);

			return ConvertCharsToCstring(api->GetUTF8Text());
		}		
	}
	catch (...) {}
	return L"";
}

CString CReadImageDlg::LoadFileDefault()
{
	try
	{
		vector<CString> vecImageFiles;
		CString szpathDesktop = GetDesktopDir();		
		GetAllFileInFolder(szpathDesktop, vecImageFiles);

		CString szPathImage = L"";
		int itotal = (int)vecImageFiles.size();
		if (itotal > 0)
		{
			time_t currentTime;
			time(&currentTime);
			struct tm *localTime = localtime(&currentTime);

			CString szTimenow = L"";
			int iday = localTime->tm_mday;
			int imonth = 1 + (int)localTime->tm_mon;			
			szTimenow.Format(L"%02d/%02d/%04d", iday, imonth, 1900 + (int)localTime->tm_year);

			
			int pos = -1;
			long LMax = 0;
			CString szInfo = L"";
			CString szNumDate = L"", szNumTime = L"";
			for (int i = 0; i < itotal; i++)
			{
				szInfo = GetInfoFileInFolder(vecImageFiles[i]);
				pos = szInfo.Find(L"|");
				if (pos > 0)
				{
					if (szInfo.Left(pos) == szTimenow)
					{
						szInfo = szInfo.Right(szInfo.GetLength() - pos - 1);
						if (_wtol(szInfo) > LMax)
						{
							LMax = _wtol(szInfo);
							szPathImage = vecImageFiles[i];
						}
					}
				}				
			}
		}

		vecImageFiles.clear();
		return szPathImage;
	}
	catch (...) {}
	return L"";
}

CString CReadImageDlg::ReplateUTF8Web(CString szChr)
{
	try
	{
		CString scharutf8[161] = { L"%",L" ",
		L"à",L"á",L"ả",L"ã",L"ạ",L"ă",L"ằ",L"ắ",L"ẳ",L"ẵ",L"ặ",L"â",L"ầ",L"ấ",L"ẩ",L"ẫ",L"ậ",
		L"À",L"Á",L"Ả",L"Ã",L"Ạ",L"Ă",L"Ằ",L"Ắ",L"Ẳ",L"Ẵ",L"Ặ",L"Â",L"Ầ",L"Ấ",L"Ẩ",L"Ẫ",L"Ậ",
		L"ò",L"ó",L"ỏ",L"õ",L"ọ",L"ô",L"ố",L"ồ",L"ổ",L"ỗ",L"ộ",L"ơ",L"ờ",L"ớ",L"ở",L"ỡ",L"ợ",
		L"Ò",L"Ó",L"Ỏ",L"Õ",L"Ọ",L"Ô",L"Ồ",L"Ố",L"Ổ",L"Ỗ",L"Ộ",L"Ơ",L"Ờ",L"Ớ",L"Ở",L"Ỡ",L"Ợ",
		L"è",L"é",L"ẻ",L"ẽ",L"ẹ",L"ê",L"ề",L"ế",L"ể",L"ễ",L"ệ",L"È",L"É",L"Ẻ",L"Ẽ",L"Ẹ",L"Ê",L"Ề",L"Ế",L"Ể",L"Ễ",L"Ệ",
		L"ù",L"ú",L"ủ",L"ũ",L"ụ",L"ư",L"ừ",L"ứ",L"ử",L"ữ",L"ự",L"Ù",L"Ú",L"Ủ",L"Ũ",L"Ụ",L"Ư",L"Ừ",L"Ứ",L"Ử",L"Ữ",L"Ự",
		L"ì",L"í",L"ỉ",L"ĩ",L"ị",L"Ì",L"Í",L"Ỉ",L"Ĩ",L"Ị",
		L"ỳ",L"ý",L"ỷ",L"ỹ",L"ỵ",L"Ỳ",L"Ý",L"Ỷ",L"Ỹ",L"Ỵ",L"đ",L"Đ",
		L"±",L"÷",L"Ø",L"ø",L"‰",L"&",L"$",L"#",L"\"",L"^",
		L"²",L"³",L"½",L"=",L"?",L"@",L";",L"+",L":",L"/",L"©",L"®",L"[",L"]",L"|" };

		CString scharweb[161] = { L"%25",L"%20",
			L"%C3%A0",L"%C3%A1",L"%E1%BA%A3",L"%C3%A3",L"%E1%BA%A1",L"%C4%83",L"%E1%BA%B1",L"%E1%BA%AF",
			L"%E1%BA%B3",L"%E1%BA%B5",L"%E1%BA%B7",L"%C3%A2",L"%E1%BA%A7",L"%E1%BA%A5",L"%E1%BA%A9",L"%E1%BA%AB",L"%E1%BA%AD",
			L"%C3%A0",L"%C3%A1",L"%E1%BA%A3",L"%C3%A3",L"%E1%BA%A1",L"%C4%83",L"%E1%BA%B1",L"%E1%BA%AF",L"%E1%BA%B3",L"%E1%BA%B5",
			L"%E1%BA%B7",L"%C3%A2",L"%E1%BA%A7",L"%E1%BA%A5",L"%E1%BA%A9",L"%E1%BA%AB",L"%E1%BA%AD",L"%C3%B2",L"%C3%B3",L"%E1%BB%8F",
			L"%C3%B5",L"%E1%BB%8D",L"%C3%B4",L"%E1%BB%91",L"%E1%BB%93",L"%E1%BB%95",L"%E1%BB%97",L"%E1%BB%99",L"%C6%A1",L"%E1%BB%9D",
			L"%E1%BB%9B",L"%E1%BB%9F",L"%E1%BB%A1",L"%E1%BB%A3",L"%C3%B2",L"%C3%B3",L"%E1%BB%8F",L"%C3%B5",L"%E1%BB%8D",L"%C3%B4",
			L"%E1%BB%91",L"%E1%BB%93",L"%E1%BB%95",L"%E1%BB%97",L"%E1%BB%99",L"%C6%A1",L"%E1%BB%9D",L"%E1%BB%9B",L"%E1%BB%9F",
			L"%E1%BB%A1",L"%E1%BB%A3",L"%C3%A8",L"%C3%A9",L"%E1%BA%BB",L"%E1%BA%BD",L"%E1%BA%B9",L"%C3%AA",L"%E1%BB%81",L"%E1%BA%BF",
			L"%E1%BB%83",L"%E1%BB%85",L"%E1%BB%87",L"%C3%A8",L"%C3%A9",L"%E1%BA%BB",L"%E1%BA%BD",L"%E1%BA%B9",L"%C3%AA",L"%E1%BB%81",
			L"%E1%BA%BF",L"%E1%BB%83",L"%E1%BB%85",L"%E1%BB%87",L"%C3%B9",L"%C3%BA",L"%E1%BB%A7",L"%C5%A9",L"%E1%BB%A5",L"%C6%B0",
			L"%E1%BB%AB",L"%E1%BB%A9",L"%E1%BB%AD",L"%E1%BB%AF",L"%E1%BB%B1",L"%C3%B9",L"%C3%BA",L"%E1%BB%A7",L"%C5%A9",L"%E1%BB%A5",
			L"%C6%B0",L"%E1%BB%AB",L"%E1%BB%A9",L"%E1%BB%AD",L"%E1%BB%AF",L"%E1%BB%B1",L"%C3%AC",L"%C3%AD",L"%E1%BB%89",L"%C4%A9",
			L"%E1%BB%8B",L"%C3%AC",L"%C3%AD",L"%E1%BB%89",L"%C4%A9",L"%E1%BB%8B",L"%E1%BB%B3",L"%C3%BD",L"%E1%BB%B7",L"%E1%BB%B9",
			L"%E1%BB%B5",L"%E1%BB%B3",L"%C3%BD",L"%E1%BB%B7",L"%E1%BB%B9",L"%E1%BB%B5",L"%C4%91",L"%C4%91",L"%C2%B1",L"%C3%B7",
			L"%C3%98",L"%C3%B8",L"%E2%80%B0",L"%26",L"%24",L"%23",L"%22",L"%5E",L"%C2%B2",L"%C2%B3",L"%C2%BD",
			L"%3D",L"%3F",L"%40",L"%3B",L"%2B",L"%3A",L"%2F",L"%C2%A9",L"%C2%AE",L"%5B",L"%5D",L"%7C" };

		for (int i = 0; i < 161; i++)
		{
			szChr.Replace(scharutf8[i], scharweb[i]);
		}

		return szChr;
	}
	catch (...) {}
	return L"";
}

CString CReadImageDlg::ReadingWebsite(CString sLinkW)
{
	try
	{
		WSADATA wsaData;
		if (WSAStartup(0x101, &wsaData) != 0) return L"";

		long fileSize = 0;
		char* memBuffer = ReadURL2(sLinkW, fileSize);
		if (strcmp(memBuffer, "") == 0) return L"";

		CString szval = ConvertCharsToCstring(memBuffer);
		if (fileSize != 0) delete(memBuffer);
		WSACleanup();

		szval = GetUKtoVietnam(szval);
		return szval;
	}
	catch (...) {}
	return L"";
}


CString CReadImageDlg::GetUKtoVietnam(CString szHeader)
{
	try
	{
		CString szFind1 = L"trans\":\"";
		CString szFind2 = L"\",\"orig";
		int pos = szHeader.Find(szFind1);
		if (pos >= 0)
		{
			CString szval = szHeader.Right(szHeader.GetLength() - pos);
			pos = szval.Find(szFind2);
			if (pos >= 0)
			{
				szval = szval.Left(pos);
				szval.Replace(szFind1, L"");
				szval.Trim();

				return szval;
			}
		}
		return L"";
	}
	catch (...){}
	return L"";
}

SOCKET CReadImageDlg::ConnectToServer(char *szServerName, WORD portNum)
{
	try
	{
		struct hostent *hp;
		unsigned int addr;
		struct sockaddr_in server;
		SOCKET conn;

		conn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (conn == INVALID_SOCKET) return NULL;

		if (inet_addr(szServerName) == INADDR_NONE)
		{
			hp = gethostbyname(szServerName);
		}
		else
		{
			addr = inet_addr(szServerName);
			hp = gethostbyaddr((char*)&addr, sizeof(addr), AF_INET);
		}

		if (hp == NULL)
		{
			closesocket(conn);
			return NULL;
		}

		server.sin_addr.s_addr = *((unsigned long*)hp->h_addr);
		server.sin_family = AF_INET;
		server.sin_port = htons(portNum);
		if (connect(conn, (struct sockaddr*)&server, sizeof(server)))
		{
			closesocket(conn);
			return NULL;
		}

		return conn;
	}
	catch (...) {}
}

char* CReadImageDlg::ReadURL2(CString szUrl, long &bytesReturnedOut)
{
	try
	{
		const int bufSize = 512;
		char readBuffer[bufSize];
		char *tmpResult = NULL, *result = NULL;
		long totalBytesRead, thisReadSize;

		CString szval = szUrl;
		szval.Replace(L"https://", L"");
		szval.Replace(L"http://", L"");

		CString sSever = szval, sGET = L"GET /", sHost = L"Host: ", sConnect = L"Connection: close";
		int pos = szval.Find(L"/");
		if (pos > 0)
		{
			sSever = szval.Left(pos);
			sGET += szval.Right(szval.GetLength() - pos - 1);
			sHost += sSever;
		}
		else
		{
			sGET += L" ";
			sHost += sSever;
		}

		sGET += L" HTTP/1.1";
		szval.Format(L"%s\r\n%s\r\n%s\r\n\r\n", sGET, sHost, sConnect);
		char *sendBuffer = ConvertCstringToChars(szval);
		SOCKET conn = ConnectToServer(ConvertCstringToChars(sSever), 80);
		send(conn, sendBuffer, strlen(sendBuffer), 0);

		totalBytesRead = 0;
		while (1)
		{
			memset(readBuffer, 0, bufSize);
			thisReadSize = recv(conn, readBuffer, bufSize, 0);

			if (thisReadSize <= 0) break;

			tmpResult = (char*)realloc(tmpResult, thisReadSize + totalBytesRead);

			memcpy(tmpResult + totalBytesRead, readBuffer, thisReadSize);
			totalBytesRead += thisReadSize;
		}

		result = new char[totalBytesRead + 1];
		memcpy(result, tmpResult, totalBytesRead);
		result[totalBytesRead] = 0x0;

		bytesReturnedOut = totalBytesRead;
		closesocket(conn);
		return(result);
	}
	catch (...) {}
	return "";
}


void CReadImageDlg::OnTimer(UINT_PTR nIDEvent)
{
	iMsClick = 0;
	iMouse_X = 0, iMouse_Y = 0;
	CDialogEx::OnTimer(nIDEvent);
}

void CReadImageDlg::CheckMouseClick()
{
	iMsClick++;
	if (iMsClick == 2) EventDoubleClick();
	else if (iMsClick == 3) EventTripleClick();
	else if (iMsClick >= 4) EventQuadrupleClick();
}

void CReadImageDlg::EventDoubleClick()
{
	try
	{

		int iStart = -1, iEnd = -1;
		CString szval = L"", szContent = L"";

		m_txt_result.GetSel(iStart, iEnd);
		m_txt_result.GetWindowTextW(szContent);
		if (szContent.Right(1) != L".") szContent += L".";

		int iLen = szContent.GetLength();
		int ivt1 = 0, ivt2 = iLen - 1;

		if (iStart >= 0)
		{
			if (iStart > 0)
			{
				for (int i = iStart - 1; i >= 0; i--)
				{
					szval = szContent.Mid(i, 1);
					if (szval == L" " || szval == L"." || szval == L"," || szval == L";")
					{
						ivt1 = i + 1;
						break;
					}
				}
			}

			if (iStart < ivt2)
			{
				for (int i = iStart + 1; i <= ivt2; i++)
				{
					szval = szContent.Mid(i, 1);
					if (szval == L" " || szval == L"." || szval == L"," || szval == L";")
					{
						ivt2 = i;
						break;
					}
				}
			}

			m_txt_result.SetSel(ivt1, ivt2);
		}
	}
	catch (...) {}
}

void CReadImageDlg::EventTripleClick()
{
	try
	{
		int iStart = -1, iEnd = -1;
		CString szval = L"", szContent = L"";

		m_txt_result.GetSel(iStart, iEnd);
		m_txt_result.GetWindowTextW(szContent);
		if (szContent.Right(1) != L".") szContent += L".";

		int iLen = szContent.GetLength();
		int ivt1 = 0, ivt2 = iLen - 1;

		if (iStart >= 0)
		{
			if (iStart > 0)
			{
				for (int i = iStart - 1; i >= 0; i--)
				{
					szval = szContent.Mid(i, 1);
					if (szval == L"." || szval == L"," || szval == L";")
					{
						ivt1 = i + 1;

						for (int j = ivt1; j < iLen; j++)
						{
							if (szContent.Mid(j, 1) != L" ")
							{
								ivt1 = j;
								break;
							}
						}

						break;
					}
				}
			}

			if (iStart < ivt2)
			{
				for (int i = iStart + 1; i <= ivt2; i++)
				{
					szval = szContent.Mid(i, 1);
					if (szval == L"." || szval == L"," || szval == L";")
					{
						ivt2 = i;

						if (ivt2 > 0)
						{
							for (int j = ivt2 - 1; j >= 0; j--)
							{
								if (szContent.Mid(j, 1) != L" ")
								{
									ivt2 = j + 1;
									break;
								}
							}
						}

						break;
					}
				}
			}

			m_txt_result.SetSel(ivt1, ivt2);
		}
	}
	catch (...) {}
}

void CReadImageDlg::EventQuadrupleClick()
{
	try
	{
		int iStart = -1, iEnd = -1;
		CString szval = L"", szContent = L"";

		m_txt_result.GetSel(iStart, iEnd);
		m_txt_result.GetWindowTextW(szContent);
		if (szContent.Right(1) != L".") szContent += L".";

		int iLen = szContent.GetLength();
		int ivt1 = 0, ivt2 = iLen - 1;

		if (iStart >= 0)
		{
			if (iStart > 0)
			{
				for (int i = iStart - 1; i >= 0; i--)
				{
					szval = szContent.Mid(i, 1);
					if (szval == L".")
					{
						ivt1 = i + 1;

						for (int j = ivt1; j < iLen; j++)
						{
							if (szContent.Mid(j, 1) != L" ")
							{
								ivt1 = j;
								break;
							}
						}

						break;
					}
				}
			}

			if (iStart < ivt2)
			{
				for (int i = iStart + 1; i <= ivt2; i++)
				{
					szval = szContent.Mid(i, 1);
					if (szval == L".")
					{
						ivt2 = i;

						if (ivt2 > 0)
						{
							for (int j = ivt2 - 1; j >= 0; j--)
							{
								if (szContent.Mid(j, 1) != L" ")
								{
									ivt2 = j + 1;
									break;
								}
							}
						}

						break;
					}
				}
			}

			m_txt_result.SetSel(ivt1, ivt2);
		}
	}
	catch (...) {}
}

CString CReadImageDlg::GetInfoFileInFolder(CString szPath)
{
	try
	{
		HMODULE hLib;
		GET_FILE_ATTRIBUTES_EX func;
		FILE_INFO fInfo;

		hLib = LoadLibrary(L"Kernel32.dll");
		if (hLib != NULL)
		{
			func = (GET_FILE_ATTRIBUTES_EX)GetProcAddress(hLib, "GetFileAttributesExW");
			if (func != NULL) func(szPath, 0, &fInfo);

			SYSTEMTIME times, stLocal;
			FileTimeToSystemTime(&fInfo.ftLastWriteTime, &times);
			SystemTimeToTzSpecificLocalTime(NULL, &times, &stLocal);

			CString szTimeModified = L"";
			szTimeModified.Format(L"%02d/%02d/%04d|%02d%02d%02d",
				stLocal.wDay, stLocal.wMonth, stLocal.wYear,
				stLocal.wHour, stLocal.wMinute, stLocal.wSecond);

			FreeLibrary(hLib);

			return szTimeModified;
		}
	}
	catch (...){}
	return _T("");
}

void CReadImageDlg::GetAllFileInFolder(CString szPath, vector<CString> &vecImg)
{
	try
	{
		vector<_tstring> vecFolder;		
		CString strWildCard[3] = { L".jpg", L".bmp", L".png" };		
		CUtil::GetFileList(_tstring(szPath.GetBuffer()), _T("*.*"), false, vecFolder);
		int isize = (int)vecFolder.size();

		int icheck = 0;
		CString szval = L"";
		for (int i = 0; i < isize; i++)
		{
			icheck = 0;
			szval = vecFolder[i].c_str();
			for (int j = 0; j < 3; j++)
			{
				if (strWildCard[j] == szval.Right(4))
				{
					icheck = 1;
					break;
				}
			}

			if (icheck == 1) vecImg.push_back(szval);
		}

		vecFolder.clear();
	}
	catch(...){}
}

bool CReadImageDlg::CheckConnection()
{
	if (InternetCheckConnection(L"https://www.google.com", FLAG_ICC_FORCE_CONNECTION, 0)) return true;
	return false;
}
