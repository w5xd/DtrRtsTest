
// DtrRtsDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CDtrRtsDlg dialog
class CDtrRtsDlg : public CDialogEx
{
// Construction
public:
	CDtrRtsDlg(CWnd* pParent = NULL);	// standard constructor
    ~CDtrRtsDlg();

// Dialog Data
	enum { IDD = IDD_DTRRTS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
    HANDLE m_CommChannel;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
    afx_msg void OnBnClickedApply();
    afx_msg void OnCbnSelchangeCommcombo();
    afx_msg void OnBnClickedRtsfirst();
    void ApplyRts(std::wstring &);
    void ApplyDtr(std::wstring &);
    CComboBox m_CommCombo;
    CButton m_ApplyBtn;
    CButton m_ApplyRtsFirst;
    bool m_PrevDtr;
    bool m_PrevRts;
};
