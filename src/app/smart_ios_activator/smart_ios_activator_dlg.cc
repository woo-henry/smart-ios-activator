#include "pch.h"
#include "smart_ios_activator_app.h"
#include "smart_ios_activator_dlg.h"

BEGIN_MESSAGE_MAP(SmartiOSActivatorDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_IOS_DEVICE_OBJECT_ADD, OnMessageDeviceObjectAdd)
	ON_MESSAGE(WM_IOS_DEVICE_OBJECT_REMOVE, OnMessageDeviceObjectRemove)
	ON_MESSAGE(WM_IOS_DEVICE_OBJECT_UPDATE, OnMessageDeviceObjectUpdate)
	ON_MESSAGE(WM_IOS_DEVICE_STATUS_UPDATE, OnMessageDeviceStatusUpdate)
	ON_MESSAGE(WM_IOS_DEVICE_MESSAGE_SUCCESS, OnMessageDeviceMessageSuccess)
	ON_MESSAGE(WM_IOS_DEVICE_MESSAGE_ERROR, OnMessageDeviceMessageError)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_DEVICE, &SmartiOSActivatorDlg::OnRightClickListDevice)
	ON_COMMAND(IDC_ACTIVATE, &SmartiOSActivatorDlg::OnCommandDeviceActivate)
	ON_COMMAND(IDC_DEACTIVATE, &SmartiOSActivatorDlg::OnCommandDeviceDeactivate)
	ON_BN_CLICKED(IDC_ACTIVATE_ALL, &SmartiOSActivatorDlg::OnCommandDeviceActivateAll)
END_MESSAGE_MAP()

SmartiOSActivatorDlg::SmartiOSActivatorDlg(CWnd* pParent)
	: CDialogEx(IDD_DEVICE_DIALOG, pParent)
	, _device_notify(nullptr)
	, _device_callbacks({0})
{
	_icon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

SmartiOSActivatorDlg::~SmartiOSActivatorDlg()
{
	ShutdowniOSDeviceEnviroment();
}

BOOL SmartiOSActivatorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	SetIcon(_icon, TRUE);
	SetIcon(_icon, FALSE);

	InitListCtrl();

	_device_callbacks.callback_connected = &SmartiOSActivatorDlg::OniOSDeviceConnected;
	_device_callbacks.callback_disconnected = &SmartiOSActivatorDlg::OniOSDeviceDisconnected;
	_device_callbacks.callback_query = &SmartiOSActivatorDlg::OniOSDeviceQuery;
	_device_callbacks.callback_error = &SmartiOSActivatorDlg::OniOSDeviceError;
	int result = InitiOSDeviceEnviroment(this, &_device_callbacks);
	if (result != ERROR_SUCCESS)
	{
		AfxMessageBox(TEXT("iOS设备环境初始化失败"));
		return FALSE;
	}

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
		
		result = QueryiOSDevice(device->device_id);

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
	wchar_t* serial_number_w = nullptr;
	wchar_t* product_name_w = nullptr;
	wchar_t* product_type_w = nullptr;
	wchar_t* product_version_w = nullptr;
	wchar_t* phone_number_w = nullptr;
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

		std::string serial_number = device->serial_number;
#ifdef UNICODE
		serial_number_w = (wchar_t*)SmartMemAlloc((serial_number.length() + 1) * sizeof(wchar_t));
		if (serial_number_w == nullptr)
			break;

		int serial_number_length = 0;
		if (!SmartStrA2W(serial_number_w, &serial_number_length, serial_number.c_str()))
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

		std::string product_type = device->product_type;
#ifdef UNICODE
		product_type_w = (wchar_t*)SmartMemAlloc((product_type.length() + 1) * sizeof(wchar_t));
		if (product_type_w == nullptr)
			break;

		int product_type_length = 0;
		if (!SmartStrA2W(product_type_w, &product_type_length, product_type.c_str()))
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

		const bool activated = device->activated;

#ifdef UNICODE
		UpdateListItem(device_id_w, device_name_w, serial_number_w, product_name_w, product_type_w, product_version_w, phone_number_w, activated);
#else
		UpdateListItem(device_id.c_str(), device_name.c_str(), serial_number.c_str(), product_name.c_str(), product_type.c_str(), product_version.c_str(), phone_number.c_str(), activated);
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

	} while (false);

	return result;
}

LRESULT SmartiOSActivatorDlg::OnMessageDeviceMessageSuccess(WPARAM wparam, LPARAM lparam)
{
	LRESULT result = ERROR_SUCCESS;
	char* device_id = nullptr;

	do
	{
		device_id = (char*)wparam;
		if (device_id == nullptr)
			break;

		CStringA message;
		message.Format("设备[%s]激活成功\r\n请断开连接，并在屏幕上点击继续，以便设备完成自动激活过程！", device_id);
		MessageBoxA(GetSafeHwnd(), message, "设备激活", MB_ICONINFORMATION);

	} while (false);

	if (device_id)
	{
		SmartMemFree(device_id);
		device_id = nullptr;
	}

	return result;
}

LRESULT SmartiOSActivatorDlg::OnMessageDeviceMessageError(WPARAM wparam, LPARAM lparam)
{
	LRESULT result = ERROR_SUCCESS;
	char* device_id = nullptr;

	do
	{
		device_id = (char*)wparam;
		if (device_id == nullptr)
			break;

		if (lparam == -2)
		{
			CStringA message;
			message.Format("设备[%s]激活成功\r\n请断开连接，并在屏幕上点击继续，以便设备完成自动激活过程！", device_id);
			MessageBoxA(GetSafeHwnd(), message, "设备激活", MB_ICONINFORMATION);
		}
		else
		{
			CStringA message;
			message.Format("设备[%s]激活失败\r\n错误代码：%d", device_id, (int)lparam);
			MessageBoxA(GetSafeHwnd(), message, "设备激活", MB_ICONERROR);
		}
	} while (false);

	if (device_id)
	{
		SmartMemFree(device_id);
		device_id = nullptr;
	}

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
	parent_menu.LoadMenu(IDR_DEVICE);

	CMenu* popup_menu = parent_menu.GetSubMenu(0);
	if (popup_menu)
	{
		CPoint point;
		GetCursorPos(&point);

		popup_menu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}
	
	*pResult = 0;
}

void SmartiOSActivatorDlg::OnCommandDeviceActivate()
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
		HANDLE thread_handle = CreateThread(nullptr, 0, ActivateDeviceThreadRoutine, device_id_a, 0, &thread_id);
		if (thread_handle)
		{
			CloseHandle(thread_handle);
			thread_handle = nullptr;
		}
	}
	else
	{
		if (device_id_a)
		{
			SmartMemFree(device_id_a);
			device_id_a = nullptr;
		}
	}
}

void SmartiOSActivatorDlg::OnCommandDeviceDeactivate()
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
		int result = DeactivateiOSDevice(device_id_a);
		if (result != ERROR_SUCCESS)
		{
			AfxMessageBox(TEXT("设备反激活失败！"));
		}
		else
		{
			AfxMessageBox(TEXT("设备反激活成功！"));
			_device_listctrl.SetItemText(index, 7, TEXT("未激活"));
		}
	}

	if (device_id_a)
	{
		SmartMemFree(device_id_a);
		device_id_a = nullptr;
	}
}

void SmartiOSActivatorDlg::OnCommandDeviceActivateAll()
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
	HANDLE thread_handle = CreateThread(nullptr, 0, ActivateAllDeviceThreadRoutine, device_ids, 0, &thread_id);
	if (thread_handle)
	{
		CloseHandle(thread_handle);
		thread_handle = nullptr;
	}
	else
	{
		delete device_ids;
		device_ids = nullptr;
	}
}

void SmartiOSActivatorDlg::InitListCtrl()
{
	int index = _device_listctrl.InsertColumn(0, TEXT("设备ID"), 0, 360);
	_device_listctrl.SetItemData(index, FALSE);

	_device_listctrl.InsertColumn(1, TEXT("设备名称"), 0, 120);
	_device_listctrl.InsertColumn(2, TEXT("序列号"), 0, 150);
	_device_listctrl.InsertColumn(3, TEXT("系统名称"), 0, 100);
	_device_listctrl.InsertColumn(4, TEXT("系统类型"), 0, 100);
	_device_listctrl.InsertColumn(5, TEXT("系统版本"), 0, 100);
	_device_listctrl.InsertColumn(6, TEXT("电话号码"), 0, 150);
	_device_listctrl.InsertColumn(7, TEXT("激活状态"), 0, 100);
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

void SmartiOSActivatorDlg::UpdateListItem(const CString& device_id, const CString& device_name, const CString& serial_number,
	const CString& product_name, const CString& product_type, const CString& product_version, const CString& phone_number,
	const bool activated)
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
				_device_listctrl.SetItemText(position, 2, serial_number.GetString());
				_device_listctrl.SetItemText(position, 3, product_name.GetString());
				_device_listctrl.SetItemText(position, 4, product_type.GetString());
				_device_listctrl.SetItemText(position, 5, product_version.GetString());
				_device_listctrl.SetItemText(position, 6, phone_number.GetString());
				_device_listctrl.SetItemText(position, 7, activated ? TEXT("已激活") : TEXT("未激活"));
				break;
			}
		}
	}
	_device_listctrl.SetRedraw(TRUE);
}

void SmartiOSActivatorDlg::OniOSDeviceConnected(void* context, const char* device_id)
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

void SmartiOSActivatorDlg::OniOSDeviceDisconnected(void* context, const char* device_id)
{
	SmartiOSActivatorDlg* dlg = (SmartiOSActivatorDlg*)context;
	if (dlg == nullptr)
		return;

	IosDevice* ios_device = (IosDevice*)SmartMemAlloc(sizeof(IosDevice));
	if (ios_device == nullptr)
		return;

	RtlZeroMemory(ios_device, sizeof(IosDevice));
	lstrcpyA(ios_device->device_id, device_id);

	::PostMessage(dlg->GetSafeHwnd(), WM_IOS_DEVICE_OBJECT_REMOVE, (WPARAM)ios_device, (LPARAM)context);
}

void SmartiOSActivatorDlg::OniOSDeviceQuery(void* context,
	const char* device_id,
	const char* device_name,
	const char* serial_number,
	const char* product_name,
	const char* product_type,
	const char* product_version,
	const char* phone_number)
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
	lstrcpyA(ios_device->serial_number, serial_number);
	lstrcpyA(ios_device->product_name, product_name);
	lstrcpyA(ios_device->product_type, product_type);
	lstrcpyA(ios_device->product_version, product_version);
	lstrcpyA(ios_device->phone_number, phone_number);

	bool activated = false;
	QueryiOSDeviceState(device_id, &ios_device->activated);

	::PostMessage(dlg->GetSafeHwnd(), WM_IOS_DEVICE_OBJECT_UPDATE, (WPARAM)ios_device, (LPARAM)context);
}

void SmartiOSActivatorDlg::OniOSDeviceError(void* context, const char* device_id, int error_code)
{
	SmartiOSActivatorDlg* dlg = (SmartiOSActivatorDlg*)context;
	if (dlg == nullptr)
		return;

	::PostMessageA(dlg->GetSafeHwnd(), WM_IOS_DEVICE_STATUS_UPDATE, (WPARAM)device_id, (LPARAM)error_code);
}

DWORD WINAPI SmartiOSActivatorDlg::ActivateDeviceThreadRoutine(PVOID parameter)
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

		result = ActivateiOSDevice(device_id);
		if (result != ERROR_SUCCESS)
		{
			this_app.GetMainWnd()->PostMessageW(WM_IOS_DEVICE_MESSAGE_ERROR, (WPARAM)device_id, (LPARAM)result);
			SmartLogError("Activate iOS Device[%s] Error : %d", device_id, result);
		}
		else
		{
			this_app.GetMainWnd()->PostMessageW(WM_IOS_DEVICE_MESSAGE_SUCCESS, (WPARAM)device_id, (LPARAM)result);
		}
	} while (false);

	return result;
}

DWORD WINAPI SmartiOSActivatorDlg::ActivateAllDeviceThreadRoutine(PVOID parameter)
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
			std::string device_id_string = *it;
			char* device_id = (char*)SmartMemAlloc(device_id_string.length() + 1);
			if (device_id)
			{
				lstrcpyA(device_id, device_id_string.c_str());
				result = ActivateiOSDevice(device_id);
				if (result != ERROR_SUCCESS)
				{
					AfxGetMainWnd()->PostMessageW(WM_IOS_DEVICE_MESSAGE_ERROR, (WPARAM)device_id, (LPARAM)result);
					SmartLogError("Activate iOS Device[%s] Error : %d", device_id, result);
				}
				else
				{
					AfxGetMainWnd()->PostMessageW(WM_IOS_DEVICE_MESSAGE_SUCCESS, (WPARAM)device_id);
				}
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