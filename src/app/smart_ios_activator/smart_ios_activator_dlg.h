#pragma once

#include <string>
#include <vector>
#include <map>

#define IOS_DEVICE_STATUS_UNKNOWN	0
#define IOS_DEVICE_STATUS_STARTED	1
#define IOS_DEVICE_STATUS_STOPPED	2
#define IOS_DEVICE_STATUS_ERRORED	9

class DevicePlayDlg;

typedef struct tagMirrorClient
{
	IosDevice		device;
	void*			context;
	DevicePlayDlg*  player;
} MirrorClient;

class SmartiOSActivatorDlg 
	: public CDialogEx
{
public:
	SmartiOSActivatorDlg(CWnd* pParent = nullptr);
	virtual ~SmartiOSActivatorDlg();
	enum { IDD = IDD_DEVICE_MIRROR_DIALOG};
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
	afx_msg void OnCommandDeviceMirrorStart();
	afx_msg void OnCommandDeviceMirrorStop();
	afx_msg void OnCommandDeviceMirrorCapture();
	afx_msg void OnBnClickedDeviceMirrorStopAll();
	DECLARE_MESSAGE_MAP()
public:
	void SetDeviceMirrorStatus(const char* device_id, int status);
private:
	void InitListCtrl();
	void InsertListItem(const CString& device_id);
	void DeleteListItem(const CString& device_id);
	void UpdateListItem(const CString& device_id,
		const CString& device_name,
		const CString& product_name,
		const CString& product_version,
		const CString& phone_number,
		const CString& serial_number,
		const bool device_paired,
		int mirror_status);
	void UpdateListItemStatus(int position, int index, int mirror_status);
private:
	int InitMirrorEnviroment();
	void ShutdownMirrorEnviroment();
private:
	int CreateMirrorClient(IosDevice* ios_device, void* context);
	void DestroyMirrorClient(IosDevice* ios_device, void* context);
	void DestroyMirrorClients();
	void DestroyMirrorClient(MirrorClient** mirror_client);
	MirrorClient* GetMirrorClient(const char* device_id);
	void StartMirrorClient(const char* device_id);
	void StopMirrorClient(const char* device_id);
protected:
	static DWORD WINAPI StopDeviceMirrorThreadRoutine(PVOID parameter);
	static DWORD WINAPI StopAllDeviceMirrorThreadRoutine(PVOID parameter);
	static void OnIosDeviceConnected(void* context, const char* device_id);
	static void OnIosDeviceDisconnected(void* context, const char* device_id);
	static void OnIosDeviceQuery(void* context,
		const char* device_id, 
		const char* device_name,
		const char* product_name,
		const char* product_version,
		const char* phone_number,
		const char* serial_number);
	static void OnIosDeviceMirrorStarted(void* context, const char* device_id);
	static void OnIosDeviceMirrorStopped(void* context, const char* device_id);
	static void OnIosDeviceMirrorOutputAudio(void* context, const char* device_id, PCMData* pcm_data);
	static void OnIosDeviceMirrorOutputVideo(void* context, const char* device_id, H264Data* h264_data);
	static void OnIosDeviceMirrorError(void* context, const char* device_id, int error_code);
protected:
	HICON										_icon;
	CListCtrl									_device_listctrl;
	HDEVNOTIFY									_device_notify;
	IosDeviceCallbacks							_device_callbacks;
	std::map<std::string, MirrorClient*>		_mirror_clients;
	CRITICAL_SECTION							_mirror_clients_lock;
	RECT										_mirror_rect;
};