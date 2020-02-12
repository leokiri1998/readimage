#pragma once

#include "Util.h"
#include "FileFilter.h"
#include "BrowseForFolder.h"

// CReadImageDlg dialog
class CReadImageDlg : public CDialogEx
{
// Construction
public:
	CReadImageDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_READIMAGE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	afx_msg BOOL PreTranslateMessage(MSG* pMsg);

private:
	CEdit m_txt_browse, m_txt_result, m_txt_translate;

	int iMsClick;
	int iMouse_X, iMouse_Y;
	CString szpathDefault;

private:
	int FileExists(CString szPath);
	char* ConvertCstringToChars(CString szvalue);
	CString ConvertCharsToCstring(char* chr);	
	CString GetPathFolder(CString szPath);	
	CString TesseractPreprocess(CString szPathImage);
	CString TesseractOCR(CString szPreprocessedFile);
	CString ReplateUTF8Web(CString szChr);
	CString ReadingWebsite(CString sLinkW);
	CString GetUKtoVietnam(CString szHeader);
	SOCKET ConnectToServer(char *szServerName, WORD portNum);
	char* ReadURL2(CString szUrl, long &bytesReturnedOut);
	CString GetInfoFileInFolder(CString szPath);
	void GetAllFileInFolder(CString szPath, vector<CString> &vecImg);	
	CString LoadFileDefault();
	CString GetDesktopDir();
	void CheckMouseClick();
	void EventDoubleClick();
	void EventTripleClick();
	void EventQuadrupleClick();
	bool CheckConnection();

public:
	afx_msg void OnBnClickedBtnBrowse();
	afx_msg void OnBnClickedBtnOpen();
	afx_msg void OnBnClickedBtnConvert();
	afx_msg void OnBnClickedBtnTranslate();
	afx_msg void OnBnClickedBtnGoogleTranslate();
	afx_msg void OnBnClickedBtnGoogleSearch();
	afx_msg void OnBnClickedBtnClose();		
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
