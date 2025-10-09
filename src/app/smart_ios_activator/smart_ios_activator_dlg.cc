#include "pch.h"
#include "smart_ios_activator_app.h"
#include "smart_ios_activator_dlg.h"

typedef struct tagMirrorClientThreadContext
{
	SOCKET			 socket;
	SmartiOSActivatorDlg* context;
} MirrorClientThreadContext;

BEGIN_MESSAGE_MAP(SmartiOSActivatorDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_IOS_DEVICE_OBJECT_ADD, OnMessageDeviceObjectAdd)
	ON_MESSAGE(WM_IOS_DEVICE_OBJECT_REMOVE, OnMessageDeviceObjectRemove)
	ON_MESSAGE(WM_IOS_DEVICE_OBJECT_UPDATE, OnMessageDeviceObjectUpdate)
	ON_MESSAGE(WM_IOS_DEVICE_STATUS_UPDATE, OnMessageDeviceStatusUpdate)
	ON_COMMAND(IDC_MIRROR_START, OnCommandDeviceMirrorStart)
	ON_COMMAND(IDC_MIRROR_STOP, OnCommandDeviceMirrorStop)
	ON_COMMAND(IDC_MIRROR_CAPTURE, OnCommandDeviceMirrorCapture)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_DEVICE, &SmartiOSActivatorDlg::OnRightClickListDevice)
	ON_BN_CLICKED(IDC_BTN_STOP_ALL, &SmartiOSActivatorDlg::OnBnClickedDeviceMirrorStopAll)
END_MESSAGE_MAP()

SmartiOSActivatorDlg::SmartiOSActivatorDlg(CWnd* pParent)
	: CDialogEx(IDD_DEVICE_MIRROR_DIALOG, pParent)
	, _device_notify(nullptr)
	, _device_callbacks({0})
	, _mirror_rect({0})
{
	_icon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	InitializeCriticalSection(&_mirror_clients_lock);
}

SmartiOSActivatorDlg::~SmartiOSActivatorDlg()
{
	ShutdownMirrorEnviroment();

	ShutdownIosDeviceMirror();

	DestroyMirrorClients();

	DeleteCriticalSection(&_mirror_clients_lock);
}

BOOL SmartiOSActivatorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	SetIcon(_icon, TRUE);
	SetIcon(_icon, FALSE);

	InitListCtrl();

	_device_callbacks.callback_connected = &SmartiOSActivatorDlg::OnIosDeviceConnected;
	_device_callbacks.callback_disconnected = &SmartiOSActivatorDlg::OnIosDeviceDisconnected;
	_device_callbacks.callback_query = &SmartiOSActivatorDlg::OnIosDeviceQuery;
	_device_callbacks.callback_started = &SmartiOSActivatorDlg::OnIosDeviceMirrorStarted;
	_device_callbacks.callback_stopped = &SmartiOSActivatorDlg::OnIosDeviceMirrorStopped;
	_device_callbacks.callback_output_audio = &SmartiOSActivatorDlg::OnIosDeviceMirrorOutputAudio;
	_device_callbacks.callback_output_video = &SmartiOSActivatorDlg::OnIosDeviceMirrorOutputVideo;
	_device_callbacks.callback_error = &SmartiOSActivatorDlg::OnIosDeviceMirrorError;
	int result = InitIosDeviceMirror(this, &_device_callbacks);
	if (result != ERROR_SUCCESS)
	{
		AfxMessageBox(TEXT("iOS设备投屏环境初始化失败"));
		return FALSE;
	}

	result = InitMirrorEnviroment();
	if (result != ERROR_SUCCESS)
	{
		AfxMessageBox(TEXT("iOS投屏环境初始化失败"));
		return FALSE;
	}

	GetWindowRect(&_mirror_rect);
	ClientToScreen(&_mirror_rect);

	return TRUE;
}

void SmartiOSActivatorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DEVICE, _device_listctrl);
}

void SmartiOSActivatorDlg::OnClose()
{	
	CDialogEx::OnClose();
}

void SmartiOSActivatorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);

		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, _icon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR SmartiOSActivatorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(_icon);
}

LRESULT SmartiOSActivatorDlg::OnMessageDeviceObjectAdd(WPARAM wparam, LPARAM lparam)
{
	LRESULT result = ERROR_SUCCESS;
	IosDevice* device = nullptr;
#ifdef UNICODE
	wchar_t* device_id_w = nullptr;
#endif // UNICODE

	do
	{
		device = (IosDevice*)wparam;
		if (device == nullptr)
			break;

#ifdef UNICODE
		int device_id_length = lstrlenA(device->device_id) + 1;
		device_id_w = (wchar_t*)SmartMemAlloc(device_id_length * sizeof(wchar_t));
		if (device_id_w == nullptr)
			break;

		if (!SmartStrA2W(device_id_w, &device_id_length, device->device_id))
			break;

		InsertListItem(device_id_w);
#else
		InsertListItem(device->device_id);
#endif // UNICODE
		
		result = QueryIosDevice(device->device_id);

	} while (false);

#ifdef UNICODE
	if (device_id_w)
	{
		SmartMemFree(device_id_w);
		device_id_w = nullptr;
	}
#endif // UNICODE

	if (device)
	{
		SmartMemFree(device);
		device = nullptr;
	}

	return result;
}

LRESULT SmartiOSActivatorDlg::OnMessageDeviceObjectRemove(WPARAM wparam, LPARAM lparam)
{
	LRESULT result = ERROR_SUCCESS;
	IosDevice* device = nullptr;
#ifdef UNICODE
	wchar_t* device_id_w = nullptr;
#endif // UNICODE

	do
	{
		device = (IosDevice*)wparam;
		if (device == nullptr)
			break;

#ifdef UNICODE
		int device_id_length = lstrlenA(device->device_id) + 1;
		device_id_w = (wchar_t*)SmartMemAlloc(device_id_length * sizeof(wchar_t));
		if (device_id_w == nullptr)
			break;

		if (!SmartStrA2W(device_id_w, &device_id_length, device->device_id))
			break;

		DeleteListItem(device_id_w);
#else
		DeleteListItem(device->device_id);
#endif // UNICODE

	} while (false);

#ifdef UNICODE
	if (device_id_w)
	{
		SmartMemFree(device_id_w);
		device_id_w = nullptr;
	}
#endif // UNICODE

	if (device)
	{
		SmartMemFree(device);
		device = nullptr;
	}

	return result;
}

LRESULT SmartiOSActivatorDlg::OnMessageDeviceObjectUpdate(WPARAM wparam, LPARAM lparam)
{
	LRESULT result = ERROR_SUCCESS;
	IosDevice* device = nullptr;
#ifdef UNICODE
	wchar_t* device_id_w = nullptr;
	wchar_t* device_name_w = nullptr;
	wchar_t* product_name_w = nullptr;
	wchar_t* product_version_w = nullptr;
	wchar_t* phone_number_w = nullptr;
	wchar_t* serial_number_w = nullptr;
#endif // UNICODE

	do
	{
		device = (IosDevice*)wparam;
		if (device == nullptr)
			break;

		std::string device_id = device->device_id;
#ifdef UNICODE
		device_id_w = (wchar_t*)SmartMemAlloc((device_id.length() + 1) * sizeof(wchar_t));
		if (device_id_w == nullptr)
			break;

		int device_id_length = 0;
		if (!SmartStrA2W(device_id_w, &device_id_length, device_id.c_str()))
			break;
#endif // UNICODE

		std::string device_name = device->device_name;
#ifdef UNICODE
		device_name_w = (wchar_t*)SmartMemAlloc((device_name.length() + 1) * sizeof(wchar_t));
		if (device_name_w == nullptr)
			break;

		int device_name_length = 0;
		if (!SmartStrA2W(device_name_w, &device_name_length, device_name.c_str()))
			break;
#endif // UNICODE

		std::string product_name = device->product_name;
#ifdef UNICODE
		product_name_w = (wchar_t*)SmartMemAlloc((product_name.length() + 1) * sizeof(wchar_t));
		if (product_name_w == nullptr)
			break;

		int product_name_length = 0;
		if (!SmartStrA2W(product_name_w, &product_name_length, product_name.c_str()))
			break;
#endif // UNICODE

		std::string product_version = device->product_version;
#ifdef UNICODE
		product_version_w = (wchar_t*)SmartMemAlloc((product_version.length() + 1) * sizeof(wchar_t));
		if (product_version_w == nullptr)
			break;

		int product_version_length = 0;
		if (!SmartStrA2W(product_version_w, &product_version_length, product_version.c_str()))
			break;
#endif // UNICODE

		std::string phone_number = device->phone_number;
#ifdef UNICODE
		phone_number_w = (wchar_t*)SmartMemAlloc((phone_number.length() + 1) * sizeof(wchar_t));
		if (phone_number_w == nullptr)
			break;

		int phone_number_length = 0;
		if (!SmartStrA2W(phone_number_w, &phone_number_length, phone_number.c_str()))
			break;
#endif // UNICODE

		std::string serial_number = device->serial_number;
#ifdef UNICODE
		serial_number_w = (wchar_t*)SmartMemAlloc((serial_number.length() + 1) * sizeof(wchar_t));
		if (serial_number_w == nullptr)
			break;

		int serial_number_length = 0;
		if (!SmartStrA2W(serial_number_w, &serial_number_length, serial_number.c_str()))
			break;
#endif // UNICODE

		const bool device_paired = device->device_paired;

#ifdef UNICODE
		UpdateListItem(device_id_w, device_name_w, product_name_w, product_version_w, phone_number_w, serial_number_w, device_paired, IOS_DEVICE_STATUS_UNKNOWN);
#else
		UpdateListItem(device_id.c_str(), device_name.c_str(), product_name.c_str(), product_version.c_str(), phone_number.c_str(), serial_number.c_str(), device_paired, IOS_DEVICE_MIRROR_UNKNOWN);
#endif // UNICODE

	} while (false);

#ifdef UNICODE
	if (serial_number_w)
	{
		SmartMemFree(serial_number_w);
		serial_number_w = nullptr;
	}

	if (phone_number_w)
	{
		SmartMemFree(phone_number_w);
		phone_number_w = nullptr;
	}

	if (product_version_w)
	{
		SmartMemFree(product_version_w);
		product_version_w = nullptr;
	}

	if (product_name_w)
	{
		SmartMemFree(product_name_w);
		product_name_w = nullptr;
	}

	if (device_name_w)
	{
		SmartMemFree(device_name_w);
		device_name_w = nullptr;
	}

	if (device_id_w)
	{
		SmartMemFree(device_id_w);
		device_id_w = nullptr;
	}
#endif // UNICODE

	if (device)
	{
		SmartMemFree(device);
		device = nullptr;
	}

	return result;
}

LRESULT SmartiOSActivatorDlg::OnMessageDeviceStatusUpdate(WPARAM wparam, LPARAM lparam)
{
	LRESULT result = ERROR_SUCCESS;

	do
	{
		char* device_id = (char*)wparam;
		if (device_id == nullptr)
			break;

		SetDeviceMirrorStatus(device_id, (int)lparam);

	} while (false);

	return result;
}

void SmartiOSActivatorDlg::OnRightClickListDevice(NMHDR* pNMHDR, LRESULT* pResult)
{
	int selected_count = _device_listctrl.GetSelectedCount();
	if (selected_count == 0)
		return;

	int index = _device_listctrl.GetSelectionMark();
	if (index == -1)
		return;

	int state = _device_listctrl.GetItemData(index);
	CMenu parent_menu;
	if (state == IOS_DEVICE_STATUS_STARTED)
	{
		parent_menu.LoadMenu(IDR_DEVICE_STOP);
	}
	else
	{
		parent_menu.LoadMenu(IDR_DEVICE_START);
	}

	CMenu* popup_menu = parent_menu.GetSubMenu(0);
	if (popup_menu)
	{
		CPoint point;
		GetCursorPos(&point);

		popup_menu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}
	
	*pResult = 0;
}

void SmartiOSActivatorDlg::OnCommandDeviceMirrorStart()
{
	IosDeviceMirrorOptions options;
	RtlZeroMemory(&options, sizeof(IosDeviceMirrorOptions));
	options.display_custom = true;
	// 1920*1080, 1280*720， 1200*688, 1024*576
	options.display_width = 1024;
	options.display_height = 576;
#ifdef _DEBUG
	options.enable_file = false;
#else
	options.enable_file = false;
#endif // _DEBUG

	int selected_count = _device_listctrl.GetSelectedCount();
	if (selected_count == 0)
		return;

	int index = _device_listctrl.GetSelectionMark();
	if (index == -1)
		return;

	CString device_id = _device_listctrl.GetItemText(index, 0);
	if (device_id.IsEmpty())
		return;

	char* device_id_a = (char*)SmartMemAlloc(MAX_PATH);
	if (device_id_a == nullptr)
		return;

	int device_id_length = 0;
	if (SmartStrW2A(device_id_a, &device_id_length, device_id.GetString()))
	{
		int result = StartIosDeviceMirror(device_id_a, &options);
		if (result != ERROR_SUCCESS)
		{
			SmartLogError("SmartiOSActivatorDlg::OnCommandDeviceStart StartIosDeviceMirror Error : %d", result);
		}
	}

	if (device_id_a)
	{
		SmartMemFree(device_id_a);
		device_id_a = nullptr;
	}	
}

void SmartiOSActivatorDlg::OnCommandDeviceMirrorStop()
{
	int selected_count = _device_listctrl.GetSelectedCount();
	if (selected_count == 0)
		return;

	int index = _device_listctrl.GetSelectionMark();
	if (index == -1)
		return;

	CString device_id = _device_listctrl.GetItemText(index, 0);
	if (device_id.IsEmpty())
		return;

	char* device_id_a = (char*)SmartMemAlloc(MAX_PATH);
	if (device_id_a == nullptr)
		return;

	int device_id_length = 0;
	if (SmartStrW2A(device_id_a, &device_id_length, device_id.GetString()))
	{
		DWORD thread_id = 0;
		HANDLE thread_handle = CreateThread(nullptr, 0, StopDeviceMirrorThreadRoutine, device_id_a, 0, &thread_id);
		if (thread_handle)
		{
			CloseHandle(thread_handle);
			thread_handle = nullptr;
		}
	}
}

void SmartiOSActivatorDlg::OnCommandDeviceMirrorCapture()
{
	int selected_count = _device_listctrl.GetSelectedCount();
	if (selected_count == 0)
		return;

	int index = _device_listctrl.GetSelectionMark();
	if (index == -1)
		return;

	CString device_id = _device_listctrl.GetItemText(index, 0);
	if (device_id.IsEmpty())
		return;

	char* device_id_a = (char*)SmartMemAlloc(MAX_PATH);
	if (device_id_a == nullptr)
		return;

	int device_id_length = 0;
	if (SmartStrW2A(device_id_a, &device_id_length, device_id.GetString()))
	{
		MirrorClient* mirror_client = GetMirrorClient(device_id_a);
		if (mirror_client && mirror_client->player)
		{
			mirror_client->player->Captrue();
		}
	}

	if (device_id_a)
	{
		SmartMemFree(device_id_a);
		device_id_a = nullptr;
	}
}

void SmartiOSActivatorDlg::OnBnClickedDeviceMirrorStopAll()
{
	int item_count = _device_listctrl.GetItemCount();
	if (item_count == 0)
		return;

	std::vector<std::string>* device_ids = new std::vector<std::string>;
	for (int i = 0; i < item_count; i++)
	{
		CString device_id = _device_listctrl.GetItemText(i, 0);
		if (device_id.IsEmpty())
			continue;

		char* device_id_a = (char*)SmartMemAlloc(MAX_PATH);
		if (device_id_a == nullptr)
			continue;

		int device_id_length = 0;
		if (SmartStrW2A(device_id_a, &device_id_length, device_id.GetString()))
		{
			device_ids->push_back(device_id_a);
		}

		if (device_id_a)
		{
			SmartMemFree(device_id_a);
			device_id_a = nullptr;
		}
	}

	DWORD thread_id = 0;
	HANDLE thread_handle = CreateThread(nullptr, 0, StopAllDeviceMirrorThreadRoutine, device_ids, 0, &thread_id);
	if (thread_handle)
	{
		CloseHandle(thread_handle);
		thread_handle = nullptr;
	}
}

void SmartiOSActivatorDlg::SetDeviceMirrorStatus(const char* device_id, int status)
{
	wchar_t* device_id_w = nullptr;

	do
	{
		int device_id_length = lstrlenA(device_id);
		device_id_w = (wchar_t*)SmartMemAlloc((device_id_length + 1) * sizeof(wchar_t));
		if (device_id_w == nullptr)
			break;

		if (!SmartStrA2W(device_id_w, &device_id_length, device_id))
			break;

		int item_count = _device_listctrl.GetItemCount();
		if (item_count == 0)
			return;

		for (int i = 0; i < item_count; i++)
		{
			CString device_id_string = _device_listctrl.GetItemText(i, 0);
			if (lstrcmpiW(device_id_string, device_id_w) == 0)
			{
				_device_listctrl.SetItemData(i, status);
				switch (status)
				{
				case IOS_DEVICE_STATUS_UNKNOWN:
					_device_listctrl.SetItemText(i, 7, TEXT("未投屏"));
					break;
				case IOS_DEVICE_STATUS_STARTED:
					_device_listctrl.SetItemText(i, 7, TEXT("正在投屏"));
					StartMirrorClient(device_id);
					break;
				case IOS_DEVICE_STATUS_STOPPED:
					_device_listctrl.SetItemText(i, 7, TEXT("结束投屏"));
					StopMirrorClient(device_id);
					break;
				case IOS_DEVICE_STATUS_ERRORED:
					_device_listctrl.SetItemText(i, 7, TEXT("投屏失败"));
					StopMirrorClient(device_id);
					break;
				default:
					break;
				}
				break;
			}
		}
	} while (false);

	if (device_id_w)
	{
		SmartMemFree(device_id_w);
		device_id_w = nullptr;
	}
}

void SmartiOSActivatorDlg::InitListCtrl()
{
	int index = _device_listctrl.InsertColumn(0, TEXT("设备ID"), 0, 300);
	_device_listctrl.SetItemData(index, FALSE);

	_device_listctrl.InsertColumn(1, TEXT("设备名称"), 0, 100);
	_device_listctrl.InsertColumn(2, TEXT("系统名称"), 0, 100);
	_device_listctrl.InsertColumn(3, TEXT("系统版本"), 0, 100);
	_device_listctrl.InsertColumn(4, TEXT("电话号码"), 0, 120);
	_device_listctrl.InsertColumn(5, TEXT("序列号"), 0, 120);
	_device_listctrl.InsertColumn(6, TEXT("信任状态"), 0, 100);
	_device_listctrl.InsertColumn(7, TEXT("投屏状态"), 0, 100);
}

void SmartiOSActivatorDlg::InsertListItem(const CString& device_id)
{
	_device_listctrl.SetRedraw(FALSE);
	{
		int index = _device_listctrl.GetItemCount();
		_device_listctrl.InsertItem(index, device_id);
	}
	_device_listctrl.SetRedraw(TRUE);
}

void SmartiOSActivatorDlg::DeleteListItem(const CString& device_id)
{
	_device_listctrl.SetRedraw(FALSE);
	{
		int item_count = _device_listctrl.GetItemCount();
		for (int i = 0; i < item_count; i++)
		{
			CString text = _device_listctrl.GetItemText(i, 0);
			if (lstrcmp(text.GetString(), device_id.GetString()) == 0)
			{
				_device_listctrl.DeleteItem(i);
				break;
			}
		}
	}
	_device_listctrl.SetRedraw(TRUE);
}

void SmartiOSActivatorDlg::UpdateListItem(const CString& device_id, const CString& device_name,
	const CString& product_name, const CString& product_version, const CString& phone_number, 
	const CString& serial_number, const bool device_paired, int mirror_status)
{
	_device_listctrl.SetRedraw(FALSE);
	{
		int count = _device_listctrl.GetItemCount();
		for (int position = 0; position < count; position++)
		{
			CString exist_device_id = _device_listctrl.GetItemText(position, 0);
			if (exist_device_id == device_id)
			{
				_device_listctrl.SetItemText(position, 1, device_name.GetString());
				_device_listctrl.SetItemText(position, 2, product_name.GetString());
				_device_listctrl.SetItemText(position, 3, product_version.GetString());
				_device_listctrl.SetItemText(position, 4, phone_number.GetString());
				_device_listctrl.SetItemText(position, 5, serial_number.GetString());
				_device_listctrl.SetItemText(position, 6, device_paired ? TEXT("已信任") : TEXT("未信任"));
				UpdateListItemStatus(position, 7, mirror_status);
				break;
			}
		}
	}
	_device_listctrl.SetRedraw(TRUE);
}

void SmartiOSActivatorDlg::UpdateListItemStatus(int position, int index, int mirror_status)
{
	_device_listctrl.SetRedraw(FALSE);

	switch (mirror_status)
	{
	case IOS_DEVICE_STATUS_UNKNOWN:
		_device_listctrl.SetItemText(position, index, TEXT("未投屏"));
		break;
	case IOS_DEVICE_STATUS_STARTED:
		_device_listctrl.SetItemText(position, index, TEXT("正在投屏"));
		break;
	case IOS_DEVICE_STATUS_STOPPED:
		_device_listctrl.SetItemText(position, index, TEXT("结束投屏"));
		break;
	case IOS_DEVICE_STATUS_ERRORED:
		_device_listctrl.SetItemText(position, index, TEXT("投屏失败"));
		break;
	default:
		break;
	}

	_device_listctrl.SetRedraw(TRUE);
}

int SmartiOSActivatorDlg::InitMirrorEnviroment()
{
	int result = ERROR_SUCCESS;

	do
	{
		result = avformat_network_init();
		if (result != ERROR_SUCCESS)
			break;

	} while (false);

	return result;
}

void SmartiOSActivatorDlg::ShutdownMirrorEnviroment()
{
	avformat_network_deinit();
}

int SmartiOSActivatorDlg::CreateMirrorClient(IosDevice* ios_device, void* context)
{
	int result = ERROR_SUCCESS;

	do
	{
		MirrorClient* mirror_client = new MirrorClient;
		if (mirror_client == nullptr)
		{
			result = ERROR_NOT_ENOUGH_MEMORY;
			break;
		}

		RtlCopyMemory(&mirror_client->device, ios_device, sizeof(IosDevice));
		mirror_client->context = context;
		mirror_client->player = new DevicePlayDlg(ios_device, _mirror_rect);
		
		EnterCriticalSection(&_mirror_clients_lock);
		_mirror_clients.insert(std::pair<std::string, MirrorClient*>(ios_device->device_id, mirror_client));
		LeaveCriticalSection(&_mirror_clients_lock);

	} while (false);

	return result;
}

void SmartiOSActivatorDlg::DestroyMirrorClient(IosDevice* ios_device, void* context)
{
	MirrorClient* mirror_client = nullptr;

	EnterCriticalSection(&_mirror_clients_lock);

	do
	{
		std::map<std::string, MirrorClient*>::const_iterator it = _mirror_clients.find(ios_device->device_id);
		if (it == _mirror_clients.end())
			break;

		mirror_client = it->second;

		_mirror_clients.erase(it);

	} while (false);
	
	LeaveCriticalSection(&_mirror_clients_lock);

	DestroyMirrorClient(&mirror_client);
}

void SmartiOSActivatorDlg::DestroyMirrorClients()
{
	for (std::map<std::string, MirrorClient*>::const_iterator it = _mirror_clients.begin(); it != _mirror_clients.end(); it++)
	{
		MirrorClient* mirror_client = it->second;
		DestroyMirrorClient(&mirror_client);
	}

	EnterCriticalSection(&_mirror_clients_lock);
	_mirror_clients.erase(_mirror_clients.begin(), _mirror_clients.end());
	LeaveCriticalSection(&_mirror_clients_lock);
}

void SmartiOSActivatorDlg::DestroyMirrorClient(MirrorClient** mirror_client)
{
	if (mirror_client == nullptr)
		return;

	MirrorClient* client = *mirror_client;
	if (client == nullptr)
		return;

	DevicePlayDlg* player = client->player;
	if (player)
	{
		player->Close();
	}

	delete client;
	client = nullptr;
}

MirrorClient* SmartiOSActivatorDlg::GetMirrorClient(const char* device_id)
{
	MirrorClient* result = nullptr;

	EnterCriticalSection(&_mirror_clients_lock);

	do
	{
		std::map<std::string, MirrorClient*>::const_iterator it = _mirror_clients.find(device_id);
		if (it == _mirror_clients.end())
			break;

		result = it->second;

	} while (false);
	
	LeaveCriticalSection(&_mirror_clients_lock);

	return result;
}

void SmartiOSActivatorDlg::StartMirrorClient(const char* device_id)
{
	MirrorClient* mirror_client = GetMirrorClient(device_id);
	if (mirror_client == nullptr)
		return;

	DevicePlayDlg* player = mirror_client->player;
	if (player == nullptr)
		return;

	if (!player->IsInitialized())
	{
		player->Create(IDD_DEVICE_PLAY_DIALOG);
	}

	player->Start();
}

void SmartiOSActivatorDlg::StopMirrorClient(const char* device_id)
{
	MirrorClient* mirror_client = GetMirrorClient(device_id);
	if (mirror_client == nullptr)
		return;

	DevicePlayDlg* player = mirror_client->player;
	if (player == nullptr)
		return;

	player->Stop();
}

DWORD SmartiOSActivatorDlg::StopDeviceMirrorThreadRoutine(PVOID parameter)
{
	 int result = ERROR_SUCCESS;
	 char* device_id = nullptr;

	 do
	 {
		 device_id = (char*)parameter;
		 if (device_id == nullptr)
		 {
			 result = ERROR_INVALID_PARAMETER;
			 break;
		 }

		 result = StopIosDeviceMirror(device_id);
		 if (result != ERROR_SUCCESS)
		 {
			 SmartLogError("SmartiOSActivatorDlg::StopDeviceMirrorThreadRoutine StopIosDeviceMirror Error : %d", result);
		 }

	 } while (false);

	 if (device_id)
	 {
		 SmartMemFree(device_id);
		 device_id = nullptr;
	 }
	 
	 return result;
}

 DWORD SmartiOSActivatorDlg::StopAllDeviceMirrorThreadRoutine(PVOID parameter)
 {
	 int result = ERROR_SUCCESS;
	 std::vector<std::string>* device_ids = nullptr;

	 do
	 {
		 device_ids = (std::vector<std::string>*)parameter;
		 if (device_ids == nullptr)
		 {
			 result = ERROR_INVALID_PARAMETER;
			 break;
		 }

		 for (std::vector<std::string>::const_iterator it = device_ids->begin(); it != device_ids->end(); it++)
		 {
			 std::string device_id = *it;
			 result = StopIosDeviceMirror(device_id.c_str());
			 if (result != ERROR_SUCCESS)
			 {
				 SmartLogError("SmartiOSActivatorDlg::StopAllDeviceMirrorThreadRoutine StopIosDeviceMirror %s Error : %d", device_id.c_str(), result);
			 }
		 }
	 } while (false);

	 if (device_ids)
	 {
		 delete device_ids;
		 device_ids = nullptr;
	 }

	 return result;
 }

void SmartiOSActivatorDlg::OnIosDeviceConnected(void* context, const char* device_id)
{
	SmartiOSActivatorDlg* dlg = (SmartiOSActivatorDlg*)context;
	if (dlg == nullptr)
		return;

	IosDevice* ios_device = (IosDevice*)SmartMemAlloc(sizeof(IosDevice));
	if (ios_device == nullptr)
		return;

	RtlZeroMemory(ios_device, sizeof(IosDevice));
	lstrcpyA(ios_device->device_id, device_id);

	::PostMessage(dlg->GetSafeHwnd(), WM_IOS_DEVICE_OBJECT_ADD, (WPARAM)ios_device, (LPARAM)context);
}

void SmartiOSActivatorDlg::OnIosDeviceDisconnected(void* context, const char* device_id)
{
	SmartiOSActivatorDlg* dlg = (SmartiOSActivatorDlg*)context;
	if (dlg == nullptr)
		return;

	IosDevice* ios_device = (IosDevice*)SmartMemAlloc(sizeof(IosDevice));
	if (ios_device == nullptr)
		return;

	RtlZeroMemory(ios_device, sizeof(IosDevice));
	lstrcpyA(ios_device->device_id, device_id);

	dlg->DestroyMirrorClient(ios_device, context);

	::PostMessage(dlg->GetSafeHwnd(), WM_IOS_DEVICE_OBJECT_REMOVE, (WPARAM)ios_device, (LPARAM)context);
}

void SmartiOSActivatorDlg::OnIosDeviceQuery(void* context,
	const char* device_id,
	const char* device_name,
	const char* product_name,
	const char* product_version,
	const char* phone_number,
	const char* serial_number)
{
	SmartiOSActivatorDlg* dlg = (SmartiOSActivatorDlg*)context;
	if (dlg == nullptr)
		return;

	IosDevice* ios_device = (IosDevice*)SmartMemAlloc(sizeof(IosDevice));
	if (ios_device == nullptr)
		return;

	RtlZeroMemory(ios_device, sizeof(IosDevice));
	lstrcpyA(ios_device->device_id, device_id);
	lstrcpyA(ios_device->device_name, device_name);
	lstrcpyA(ios_device->product_name, product_name);
	lstrcpyA(ios_device->product_version, product_version);
	lstrcpyA(ios_device->phone_number, phone_number);
	lstrcpyA(ios_device->serial_number, serial_number);
	ios_device->device_paired = (ValidateIosDevice(device_id) == ERROR_SUCCESS);

	int result = dlg->CreateMirrorClient(ios_device, context);
	if (result != ERROR_SUCCESS)
	{
		if (ios_device)
		{
			SmartMemFree(ios_device);
			ios_device = nullptr;
		}
		return;
	}

	::PostMessage(dlg->GetSafeHwnd(), WM_IOS_DEVICE_OBJECT_UPDATE, (WPARAM)ios_device, (LPARAM)context);
}

void SmartiOSActivatorDlg::OnIosDeviceMirrorStarted(void* context, const char* device_id)
{
	SmartiOSActivatorDlg* dlg = (SmartiOSActivatorDlg*)context;
	if (dlg == nullptr)
		return;

	::PostMessageA(dlg->GetSafeHwnd(), WM_IOS_DEVICE_STATUS_UPDATE, (WPARAM)device_id, (LPARAM)IOS_DEVICE_STATUS_STARTED);
}

void SmartiOSActivatorDlg::OnIosDeviceMirrorStopped(void* context, const char* device_id)
{
	SmartiOSActivatorDlg* dlg = (SmartiOSActivatorDlg*)context;
	if (dlg == nullptr)
		return;

	::PostMessageA(dlg->GetSafeHwnd(), WM_IOS_DEVICE_STATUS_UPDATE, (WPARAM)device_id, (LPARAM)IOS_DEVICE_STATUS_STOPPED);
}

void SmartiOSActivatorDlg::OnIosDeviceMirrorOutputAudio(void* context, const char* device_id, PCMData* pcm_data)
{
	SmartiOSActivatorDlg* dlg = (SmartiOSActivatorDlg*)context;
	if (dlg == nullptr)
		return;

	MirrorClient* mirror_client = dlg->GetMirrorClient(device_id);
	if (mirror_client == nullptr)
		return;

	DevicePlayDlg* player = mirror_client->player;
	if (player == nullptr)
		return;

	player->PlayAudio(pcm_data);
}

void SmartiOSActivatorDlg::OnIosDeviceMirrorOutputVideo(void* context, const char* device_id, H264Data* h264_data)
{
	SmartiOSActivatorDlg* dlg = (SmartiOSActivatorDlg*)context;
	if (dlg == nullptr)
		return;

	MirrorClient* mirror_client = dlg->GetMirrorClient(device_id);
	if (mirror_client == nullptr)
		return;

	DevicePlayDlg* player = mirror_client->player;
	if (player == nullptr)
		return;

	player->PlayVideo(h264_data);
}

void SmartiOSActivatorDlg::OnIosDeviceMirrorError(void* context, const char* device_id, int error_code)
{
	SmartiOSActivatorDlg* dlg = (SmartiOSActivatorDlg*)context;
	if (dlg == nullptr)
		return;

	::PostMessageA(dlg->GetSafeHwnd(), WM_IOS_DEVICE_STATUS_UPDATE, (WPARAM)device_id, (LPARAM)IOS_DEVICE_STATUS_ERRORED);
}