#include "cameraconfig.h"

CameraConfig::CameraConfig()
{
}


CameraConfig::~CameraConfig()
{
}

int CameraConfig::GuidToString(const GUID &guid, char* buffer) {
	int buf_len = 64;
	snprintf(
		buffer,
		buf_len,
		"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1],
		guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);
	return 0;
}

int CameraConfig::GetMajorType(GUID guid, char* buffer) {
	memset(buffer, 0, 256);
	if (guid == MEDIATYPE_Video) {
		snprintf(buffer, 256, "MEDIATYPE_Video");
		return 0;
	}
	if (guid == MEDIATYPE_Audio) {
		snprintf(buffer, 256, "MEDIATYPE_Audio");
		return 0;
	}
	if (guid == MEDIASUBTYPE_RGB24) {
		snprintf(buffer, 256, "MEDIATYPE_Stream");
		return 0;
	}
	return -1;
}

int CameraConfig::GetSubType(GUID guid, char* buffer) {
	memset(buffer, 0, 256);
	if (guid == MEDIASUBTYPE_YUY2) {
		snprintf(buffer, 256, "YUY2");
		return 0;
	}
	if (guid == MEDIASUBTYPE_MJPG) {
		snprintf(buffer, 256, "MJPG");
		return 0;
	}
	if (guid == MEDIASUBTYPE_RGB24) {
		snprintf(buffer, 256, "RGB24");
		return 0;
	}
	return -1;
}

int CameraConfig::GetFormatType(GUID guid, char* buffer) {
	memset(buffer, 0, 256);
	if (guid == FORMAT_VideoInfo) {
		snprintf(buffer, 256, "FORMAT_VideoInfo");
		return 0;
	}
	if (guid == FORMAT_VideoInfo2) {
		snprintf(buffer, 256, "FORMAT_VideoInfo2");
		return 0;
	}
	return -1;
}
/*
void AddCameraInfo(CameraDeviceInfo &camera_info, const std::string &data_type,
	const std::string &data_info)
{
	int index = 0;
	if (camera_info.data.size() == 0) {
		std::vector<std::string> temp_vec;
		temp_vec.push_back(data_type);
		temp_vec.push_back(data_info);
		camera_info.data.push_back(temp_vec);
	}
	else {
		for (; index < camera_info.data.size(); ++index) {
			if (camera_info.data[index][0] == data_type) {
				break;
			}
		}
		if (index == camera_info.data.size()) {
			std::vector<std::string> temp_vec;
			temp_vec.push_back(data_type);
			temp_vec.push_back(data_info);
			camera_info.data.push_back(temp_vec);
		}
		else {
			std::vector<std::string>::iterator it;
			it = std::find(camera_info.data[index].begin(), camera_info.data[index].end(),
				data_info);
			if (it == camera_info.data[index].end()) {
				camera_info.data[index].push_back(data_info);
			}
		}
	}
}
*/
CameraDeviceInfo CameraConfig::GetCameraDeviceInfo(IMoniker* pMoniker)
{
	CameraDeviceInfo camera_info;
	HRESULT hr = NULL;
	IBaseFilter *pFilter;
	hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&pFilter);
	if (!pFilter) {
		return camera_info;
	}
	IEnumPins *pinEnum = NULL;
	IPin *pin = NULL;
	if (FAILED(pFilter->EnumPins(&pinEnum))) {
		pinEnum->Release();
		return camera_info;
	}
	pinEnum->Reset();
	ULONG pin_fetched = 0;
	while (SUCCEEDED(pinEnum->Next(1, &pin, &pin_fetched)) && pin_fetched) {
		if (!pin) {
			continue;
		}
		PIN_INFO pin_info;
		if (FAILED(pin->QueryPinInfo(&pin_info))) {
			continue;
		}
		if (pin_info.dir != PINDIR_OUTPUT) {
			continue;
		}

		IEnumMediaTypes *mtEnum = NULL;
		AM_MEDIA_TYPE   *mt = NULL;
		if (FAILED(pin->EnumMediaTypes(&mtEnum)))
			break;
		mtEnum->Reset();
		ULONG mt_fetched = 0;
		while (SUCCEEDED(mtEnum->Next(1, &mt, &mt_fetched)) && mt_fetched) {
			char majorbuf[256];
			if (GetMajorType(mt->majortype, majorbuf) != 0) {
				GuidToString(mt->majortype, majorbuf);
			}
			char subtypebuf[256];
			if (GetSubType(mt->subtype, subtypebuf) != 0) {
				GuidToString(mt->subtype, subtypebuf);
			}
			char formatbuf[256];
			if (GetFormatType(mt->formattype, formatbuf) != 0) {
				GuidToString(mt->formattype, formatbuf);
			}

			BITMAPINFOHEADER* bmi = NULL;
			int avg_time;
			if (mt->formattype == FORMAT_VideoInfo) {
				if (mt->cbFormat >= sizeof(VIDEOINFOHEADER)) {
					VIDEOINFOHEADER *pVih = reinterpret_cast<VIDEOINFOHEADER*>(mt->pbFormat);
					bmi = &(pVih->bmiHeader);
					avg_time = pVih->AvgTimePerFrame;
				}
			}
			else if (mt->formattype == FORMAT_VideoInfo2) {
				if (mt->cbFormat >= sizeof(VIDEOINFOHEADER2)) {
					VIDEOINFOHEADER2* pVih = reinterpret_cast<VIDEOINFOHEADER2*>(mt->pbFormat);
					bmi = &(pVih->bmiHeader);
					avg_time = pVih->AvgTimePerFrame;
				}
			}
			if (bmi) {
				std::string data_type_str(subtypebuf);
				camera_info.data_type.insert(data_type_str);
				camera_info.data_resolution.insert(std::to_string(bmi->biWidth) + " * "
					+ std::to_string(bmi->biHeight));
				camera_info.data_bit.insert(std::to_string(bmi->biBitCount));
				camera_info.data_fps.insert(std::to_string((int)1e7 / avg_time));

			}
		}
		pin->Release();
	}
	return camera_info;
}

std::vector<CameraDeviceInfo> CameraConfig::ListCameraDevice()
{
	std::vector<CameraDeviceInfo> device_list;
	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pEnum = NULL;
	HRESULT hr = CoInitialize(0);
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
		CLSCTX_INPROC_SERVER, IID_ICreateDevEnum,
		reinterpret_cast<void**>(&pDevEnum));

	if (SUCCEEDED(hr)) {
		hr = pDevEnum->CreateClassEnumerator(
			CLSID_VideoInputDeviceCategory,
			&pEnum, 0);
	}

	IMoniker *pMoniker = NULL;
	if (pEnum == NULL) {
		pDevEnum->Release();
		pEnum->Release();
		return device_list;
	}
	while (pEnum->Next(1, &pMoniker, NULL) == S_OK) {
		IPropertyBag *pPropBag;
		hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag,
			(void**)(&pPropBag));
		if (FAILED(hr)) {
			pMoniker->Release();
			continue;
		}
		VARIANT var_name;
		VariantInit(&var_name);
		hr = pPropBag->Read(L"Description", &var_name, 0);
		if (FAILED(hr)) {
			hr = pPropBag->Read(L"FriendlyName", &var_name, 0);
		}
		if (SUCCEEDED(hr)) {
			CameraDeviceInfo temp_camera_device_info;
			char display_name[1024];
			WideCharToMultiByte(CP_ACP, 0, var_name.bstrVal, -1, display_name, 1024, "", NULL);
			std::string str_temp(display_name);
			temp_camera_device_info = GetCameraDeviceInfo(pMoniker);
			temp_camera_device_info.friend_name = str_temp;
			device_list.push_back(temp_camera_device_info);
			VariantClear(&var_name);
		}
		pPropBag->Release();
		pMoniker->Release();
	}
	pDevEnum->Release();
	pEnum->Release();
	return device_list;
}

void CameraConfig::ConfigCamera()
{
	IMediaControl *pMediaCtrl;
	ICaptureGraphBuilder2 *pCaptureGB;
	IBaseFilter *pBaseFilter;

	//	pMediaCtrl->Stop();
	HRESULT hr;
	IAMStreamConfig *pIAMStreamConfig;
	ISpecifyPropertyPages *pISecifyPP;

	hr = pCaptureGB->FindInterface(&PIN_CATEGORY_CAPTURE,
		&MEDIATYPE_Video, pBaseFilter, IID_IAMStreamConfig,
		(void **)&pIAMStreamConfig);

	if (SUCCEEDED(hr)) {
		CAUUID cauuid;
		hr = pIAMStreamConfig->QueryInterface(IID_ISpecifyPropertyPages, (void**)&pISecifyPP);
		if (hr == S_OK)
		{
			HWND p;
			hr = pISecifyPP->GetPages(&cauuid);
			hr = OleCreatePropertyFrame(p, 30, 30, NULL, 1, (IUnknown**)&pIAMStreamConfig, cauuid.cElems, (GUID*)cauuid.pElems, 0, 0, NULL);
			CoTaskMemFree(cauuid.pElems);

		}
	}
	pIAMStreamConfig->Release();
	pISecifyPP->Release();
	pMediaCtrl->Run();
}