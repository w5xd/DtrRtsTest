
// DtrRtsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DtrRts.h"
#include "DtrRtsDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDtrRtsDlg dialog



CDtrRtsDlg::CDtrRtsDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDtrRtsDlg::IDD, pParent)
    , m_CommChannel(INVALID_HANDLE_VALUE)
    , m_PrevDtr(false)
    , m_PrevRts(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CDtrRtsDlg::~CDtrRtsDlg()
{
    if (m_CommChannel != INVALID_HANDLE_VALUE)
        ::CloseHandle(m_CommChannel);
}

void CDtrRtsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMMCOMBO, m_CommCombo);
    DDX_Control(pDX, IDC_APPLY, m_ApplyBtn);
    DDX_Control(pDX, IDC_RTSFIRST, m_ApplyRtsFirst);
}

BEGIN_MESSAGE_MAP(CDtrRtsDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_APPLY, &CDtrRtsDlg::OnBnClickedApply)
    ON_CBN_SELCHANGE(IDC_COMMCOMBO, &CDtrRtsDlg::OnCbnSelchangeCommcombo)
    ON_BN_CLICKED(IDC_RTSFIRST, &CDtrRtsDlg::OnBnClickedRtsfirst)
END_MESSAGE_MAP()


// CDtrRtsDlg message handlers

BOOL CDtrRtsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

    m_CommCombo.AddString(L"None");
    for (int i = 1; i <= 16; i++)
    {
        std::wostringstream oss;
        oss << "COM" << i;
        m_CommCombo.AddString(oss.str().c_str());
    }
    m_CommCombo.SetCurSel(0);
    m_ApplyBtn.EnableWindow(FALSE);
    m_ApplyRtsFirst.EnableWindow(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDtrRtsDlg::OnPaint()
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
HCURSOR CDtrRtsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDtrRtsDlg::ApplyRts(std::wstring &msg)
{
    bool rts = IsDlgButtonChecked(IDC_CHECKRTS) != FALSE;
    if (rts != m_PrevRts)
    {
        if (rts)
            msg += L"Set RTS.";
        else
            msg += L"Clr RTS. ";
        EscapeCommFunction(m_CommChannel, rts ? SETRTS : CLRRTS);
    }
    else
        msg += L"NC RTS. ";
    m_PrevRts = rts;
}

void CDtrRtsDlg::ApplyDtr(std::wstring &msg)
{
    bool dtr = IsDlgButtonChecked(IDC_CHECKDTR) != FALSE;
    if (dtr != m_PrevDtr)
    {
        if (dtr)
            msg += L"Set DTR.";
        else
            msg += L"Clr DTR. ";
        EscapeCommFunction(m_CommChannel, dtr ? SETDTR : CLRDTR);
    }
    else
        msg += L"NC DTR. ";
    m_PrevDtr = dtr;
}

void CDtrRtsDlg::OnBnClickedApply()
{
    if (m_CommChannel == INVALID_HANDLE_VALUE)
        return;

    std::wstring msg;
    ApplyDtr(msg);
    ApplyRts(msg);
    GetDlgItem(IDC_STATUS)->SetWindowTextW(msg.c_str());
}

void CDtrRtsDlg::OnBnClickedRtsfirst()
{
    if (m_CommChannel == INVALID_HANDLE_VALUE)
        return;

    std::wstring msg;
    ApplyRts(msg);
    ApplyDtr(msg);
    GetDlgItem(IDC_STATUS)->SetWindowTextW(msg.c_str());
}


void CDtrRtsDlg::OnCbnSelchangeCommcombo()
{
    if (m_CommChannel != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(m_CommChannel);
        m_CommChannel = INVALID_HANDLE_VALUE;
    }
    m_ApplyBtn.EnableWindow(FALSE);
    m_ApplyRtsFirst.EnableWindow(FALSE);
    m_PrevDtr = false;
    m_PrevRts = false;
    CheckDlgButton(IDC_CHECKDTR, FALSE);
    CheckDlgButton(IDC_CHECKRTS, FALSE);
    GetDlgItem(IDC_STATUS)->SetWindowText(L"");

    int i = m_CommCombo.GetCurSel();
    if (i <= 0)
        return;

    std::wostringstream oss;
    oss << L"\\\\.\\COM" << i;

    m_CommChannel = CreateFile(oss.str().c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        0,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0);
    if (m_CommChannel != INVALID_HANDLE_VALUE)
    {
        SetupComm(m_CommChannel, 100, 100);
        COMMTIMEOUTS CommTimeouts = { INFINITE, 0, 0, 0, 0 };
        SetCommTimeouts(m_CommChannel, &CommTimeouts);

        DCB dcb;
        DWORD    CommErrorBits(0);
        ClearCommError(m_CommChannel, &CommErrorBits, NULL);
        GetCommState(m_CommChannel, &dcb);

        dcb.BaudRate = 45;
        dcb.ByteSize = 5;
        dcb.Parity = NOPARITY;
        dcb.StopBits = TWOSTOPBITS;		//KLUDGE NT WILL NOT ACCEPT THIS
        dcb.fBinary = 1;
        dcb.fParity = 0;
        dcb.fOutxCtsFlow = 0;
        dcb.fOutxDsrFlow = 0;
        dcb.fRtsControl = RTS_CONTROL_DISABLE;
        dcb.fDtrControl = DTR_CONTROL_DISABLE;
        dcb.fOutX = 0;
        dcb.fInX = 0;
        dcb.fNull = 0;

        bool ret = SetCommState(m_CommChannel, &dcb) != 0;
        if (!ret)
        {
            dcb.StopBits = ONE5STOPBITS;	//NT takes this path. WIN98 sends 1/2 if you set 2!!!
            ret = SetCommState(m_CommChannel, &dcb) != 0;
        }
        EscapeCommFunction(m_CommChannel, CLRDTR);
        EscapeCommFunction(m_CommChannel, CLRRTS);
        GetDlgItem(IDC_STATUS)->SetWindowText(L"DTR and RTS initialized CLEARED");
        m_ApplyBtn.EnableWindow(TRUE);
        m_ApplyRtsFirst.EnableWindow(TRUE);
    }
    else
    {
        MessageBox(L"Error opening COM port", L"DtrRts");
        m_CommCombo.SetCurSel(0);
    }
}

