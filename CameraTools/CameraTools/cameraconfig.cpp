#include "cameraconfig.h"

CameraConfig::CameraConfig()
{
}


CameraConfig::~CameraConfig()
{
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
	if (guid == WMMEDIASUBTYPE_I420) {
		snprintf(buffer, 256, "I420");
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
		if (FAILED(pin->EnumMediaTypes(&mtEnum))) {
			break;
		}
		mtEnum->Reset();
		ULONG mt_fetched = 0;
		while (SUCCEEDED(mtEnum->Next(1, &mt, &mt_fetched)) && mt_fetched) {
			char subtypebuf[256];
			GetSubType(mt->subtype, subtypebuf);
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
				camera_info.data_resolution.insert(std::to_string(bmi->biWidth) + "x"
					+ std::to_string(bmi->biHeight));
				camera_info.data_bit.insert(std::to_string(bmi->biBitCount));
				camera_info.data_fps.insert(std::to_string((int)1e7 / avg_time));
			}
		}  // end second while
		pin->Release();
	}  // end first while
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
			WideCharToMultiByte(CP_ACP, 0, var_name.bstrVal, -1, 
				display_name, 1024, "", NULL);
			std::string str_temp(display_name);
			temp_camera_device_info = GetCameraDeviceInfo(pMoniker);
			temp_camera_device_info.friend_name = str_temp;
			device_list.push_back(temp_camera_device_info);
			VariantClear(&var_name);
		}
		pPropBag->Release();
		pMoniker->Release();
	}  // end while
	pDevEnum->Release();
	pEnum->Release();
	return device_list;
}

/*
void CameraConfig::ConfigCamera(IMoniker *pMoniker)
{
	ICaptureGraphBuilder2 *pCaptureGraphBuilder2;
	IBaseFilter *pDeviceFilter;
	HRESULT hRet = S_OK;
	AM_MEDIA_TYPE* pmt = NULL;
	LONGLONG avgTimePerFrame = 500000;
	
	IAMStreamConfig* pConfig;
	int icnt, isize;
	BYTE* pSCC = NULL;
	pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pDeviceFilter);
	CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC,
		IID_ICaptureGraphBuilder2, (LPVOID *)&pCaptureGraphBuilder2);
	pCaptureGraphBuilder2->FindInterface(&PIN_CATEGORY_CAPTURE, 0, pDeviceFilter, 
		IID_IAMStreamConfig, (void**)&pConfig);
	pConfig->GetNumberOfCapabilities(&icnt, &isize);
	pSCC = new BYTE[isize];
	pConfig->GetStreamCaps(0, &pmt, pSCC);

	VIDEOINFOHEADER* pvi = (VIDEOINFOHEADER*)pmt->pbFormat;
	pvi->AvgTimePerFrame = avgTimePerFrame;
	pvi->bmiHeader.biWidth = 176;
	pvi->bmiHeader.biHeight = 144;
	pvi->bmiHeader.biSizeImage = 176 * 144 * pvi->bmiHeader.biBitCount / 8;

	hRet = pConfig->SetFormat(pmt);

	int k = 0;
//	_DeleteMediaType(pmt);

}
*/