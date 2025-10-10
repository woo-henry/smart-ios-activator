#pragma once

#include <string>
#include <vector>
#include <map>

class SmartiOSActivatorDlg 
	: public CDialogEx
{
public:
	SmartiOSActivatorDlg(CWnd* pParent = nullptr);
	virtual ~SmartiOSActivatorDlg();
	enum { IDD = IDD_DEVICE_DIALOG};
protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);
protected:
	afx_msg void OnClose();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnMessageDeviceObjectAdd(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT OnMessageDeviceObjectRemove(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT OnMessageDeviceObjectUpdate(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT OnMessageDeviceStatusUpdate(WPARAM wparam, LPARAM lparam);
	afx_msg void OnRightClickListDevice(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCommandDeviceActivate();
	afx_msg void OnCommandDeviceDeactivate();
	afx_msg void OnCommandDeviceActivateAll();
	DECLARE_MESSAGE_MAP()
private:
	void InitListCtrl();
	void InsertListItem(const CString& device_id);
	void DeleteListItem(const CString& device_id);
	void UpdateListItem(const CString& device_id, 
		const CString& device_name, 
		const CString& serial_number,
		const CString& product_name, 
		const CString& product_type, 
		const CString& product_version, 
		const CString& phone_number,
		const bool activated);
protected:
	static void OniOSDeviceConnected(void* context, const char* device_id);
	static void OniOSDeviceDisconnected(void* context, const char* device_id);
	static void OniOSDeviceQuery(void* context,
		const char* device_id,
		const char* device_name,
		const char* serial_number,
		const char* product_name,
		const char* product_type,
		const char* product_version,
		const char* phone_number);
	static void OniOSDeviceError(void* context, const char* device_id, int error_code);
protected:
	static DWORD WINAPI ActivateDeviceThreadRoutine(PVOID parameter);
	static DWORD WINAPI ActivateAllDeviceThreadRoutine(PVOID parameter);
protected:
	HICON										_icon;
	CListCtrl									_device_listctrl;
	HDEVNOTIFY									_device_notify;
	iOSDeviceCallbacks							_device_callbacks;
};