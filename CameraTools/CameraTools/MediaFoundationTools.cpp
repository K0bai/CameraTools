#include "MediaFoundationTools.h"


MediaFoundationTools::MediaFoundationTools()
{
	ppDevices = NULL;
	pAttributes = NULL;
	reader = nullptr;
	m_DataTypeFlag = MFT_DATATYPE_RGB24;
}


MediaFoundationTools::~MediaFoundationTools()
{
}

std::string MediaFoundationTools::WChar2String(LPCWSTR pwszSrc)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, NULL, 0, NULL, NULL);

	if (nLen <= 0) return std::string("");

	char* pszDst = new char[nLen];
	if (NULL == pszDst) return std::string("");

	WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, pszDst, nLen, NULL, NULL);
	pszDst[nLen - 1] = 0;

	std::string strTemp(pszDst);
	delete[] pszDst;
	return strTemp;
}

std::string MediaFoundationTools::ToFpsString(int framerate, int den)
{
	std::stringstream ss;
	ss << (float)framerate / den;
	std::string fps;
	ss >> fps;
	ss.clear();
	return fps;
}

std::string MediaFoundationTools::GetSubType(GUID guid)
{
	if (guid == MFVideoFormat_YUY2) {
		return "YUY2";
	}
	if (guid == MFVideoFormat_MJPG) {
		return "MJPG";
	}
	if (guid == MFVideoFormat_RGB24) {
		return "RGB24";
	}
	if (guid == MFVideoFormat_I420) {
		return "I420";
	}
	return std::string();
}

GUID MediaFoundationTools::SetSubType(std::string type)
{
	if (type == "YUY2") {
		m_DataTypeFlag = MFT_DATATYPE_YUV2;
		return MFVideoFormat_YUY2;
	}
	if (type == "MJPG") {
		m_DataTypeFlag = MFT_DATATYPE_MJPG;
		return MFVideoFormat_MJPG;
	}
	if (type == "RGB24") {
		m_DataTypeFlag = MFT_DATATYPE_RGB24;
		return MFVideoFormat_RGB24;
	}
	if (type == "I420") {
		m_DataTypeFlag = MFT_DATATYPE_I420;
		return MFVideoFormat_I420;
	}
	return GUID();
}

int MediaFoundationTools::SetDevice(std::string name)
{
	HRESULT hr;
	hr = MFStartup(MF_VERSION, MFSTARTUP_LITE);
	if (FAILED(hr)) {
		return -1;
	}

	hr = MFCreateAttributes(&pAttributes, 1);
	if (FAILED(hr)) {
		return -1;
	}
	hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
		MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
	if (FAILED(hr)) {
		return -1;
	}

	UINT32 dwCount = 0;
	hr = MFEnumDeviceSources(pAttributes, &ppDevices, &dwCount);
	if (FAILED(hr) || dwCount == 0) {
		return -1;
	}

	WCHAR FriendlyName[256];
	for (DWORD i = 0; i < dwCount; i++) {
		ppDevices[i]->GetString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
			FriendlyName, 256, NULL);
		m_DeviceIndex = i;
		if (WChar2String(FriendlyName) == name) {
			return 0;
		}
		ppDevices[i]->Release();
	}
	return -1;
}

void MediaFoundationTools::SetDataInfo(const PreviewCameraInfo& cInfo)
{
	GUID subtype = SetSubType(cInfo.type);

	IMFMediaType *pNativeType = NULL;
	IMFMediaType *pType = NULL;

	HRESULT hr = reader->GetNativeMediaType(
		(DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 
		0, 
		&pNativeType);

	if (FAILED(hr)) {
		goto done;
	}

	GUID majorType;
	hr = pNativeType->GetGUID(MF_MT_MAJOR_TYPE, &majorType);
	if (FAILED(hr)) {
		goto done;
	}

	hr = MFCreateMediaType(&pType);
	if (FAILED(hr)) {
		goto done;
	}

	hr = pType->SetGUID(MF_MT_MAJOR_TYPE, majorType);
	if (FAILED(hr)) {
		goto done;
	}

	hr = pType->SetGUID(MF_MT_SUBTYPE, subtype);
	if (FAILED(hr)) {
		goto done;
	}
	int width = 0, height = 0;
	GetResolutionFromString(cInfo.resolution, width, height);
	m_Width = width;
	m_Height = height;
	MFSetAttributeSize(pType, MF_MT_FRAME_SIZE, width, height);
	MFSetAttributeRatio(pType, MF_MT_FRAME_RATE, atoi(cInfo.fps.c_str()), 1);

	hr = reader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, pType);
	if (FAILED(hr)) {
		goto done;
	}

done:
	pNativeType->Release();
	pNativeType = NULL;
	pType->Release();
	pType = NULL;
}

void MediaFoundationTools::GetResolutionFromString(const std::string & res, int & width, int & height)
{
	int index = res.find('x');
	if (index == std::string::npos) {
		return;
	}
	width = std::atoi(res.substr(0, index).c_str());
	height = std::atoi(res.substr(index+1).c_str());
}


std::vector<CameraDeviceInfo> MediaFoundationTools::ListCameraDevice()
{
	std::vector<CameraDeviceInfo> cInfo;
	CameraDeviceInfo cDeviceInfo;
	HRESULT hr;
	hr = MFStartup(MF_VERSION, MFSTARTUP_LITE);
	if (FAILED(hr)) {
		return cInfo;
	}
	
	hr = MFCreateAttributes(&pAttributes, 1);
	if (FAILED(hr)) {
		return cInfo;
	}
	hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
		MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
	if (FAILED(hr)) {
		return cInfo;
	}

	UINT32 dwCount = 0;
	
	hr = MFEnumDeviceSources(pAttributes, &ppDevices, &dwCount);
	if (FAILED(hr)) {
		return cInfo;
	}
	if (dwCount == 0) {
		hr = E_FAIL;
	}
	WCHAR FriendlyName[256];
	for (DWORD i = 0; i < dwCount; i++) {
		ppDevices[i]->GetString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
			FriendlyName, 256, NULL);
		m_DeviceIndex = i;
		cDeviceInfo = GetCameraDeviceInfo();
		cDeviceInfo.friend_name = WChar2String(FriendlyName);
		ppDevices[i]->Release();
		cInfo.push_back(cDeviceInfo);
	}
	CoTaskMemFree(ppDevices);
	pAttributes->Release();
	MFShutdown();
	return cInfo;
}

CameraDeviceInfo MediaFoundationTools::GetCameraDeviceInfo()
{
	CameraDeviceInfo cDeviceInfo;
	IMFMediaSource *pSource = NULL;
	ppDevices[m_DeviceIndex]->ActivateObject(
		__uuidof(IMFMediaSource),
		(void**)&pSource
	);
	HRESULT hr = MFCreateSourceReaderFromMediaSource(
		pSource,
		pAttributes,
		&reader
	);

	IMFMediaType* mediaType = nullptr;
	GUID  subtype{ 0 };

	DWORD index = 0;
	hr = S_OK;
	UINT32 width, height, frameRate, denominator;
	while (hr == S_OK)
	{
		hr = reader->GetNativeMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 
			 index, &mediaType);
		if (hr != S_OK)
			break;
		
		hr = mediaType->GetGUID(MF_MT_SUBTYPE, &subtype);
		
		hr = MFGetAttributeSize(mediaType, MF_MT_FRAME_SIZE, &width, &height);
		hr = MFGetAttributeRatio(mediaType, MF_MT_FRAME_RATE, &frameRate, &denominator);

		cDeviceInfo.data_type.insert(GetSubType(subtype));
		cDeviceInfo.data_fps.insert(ToFpsString(frameRate,denominator));
		cDeviceInfo.data_resolution.insert(std::to_string(width) + "x"
			+ std::to_string(height));
		++index;
	}
	return cDeviceInfo;
}

cv::Mat MediaFoundationTools::TransformRawDataToRGB(
	IMFSample *pSample,
	uint8_t * pRGBData,
	const int & width,
	const int & height)
{
	IMFMediaBuffer* buffer;
	pSample->GetBufferByIndex(0, &buffer);
	cv::Mat frameImg;
	BYTE* pRawData;
	DWORD buffer_size;
	buffer->Lock(&pRawData, NULL, &buffer_size);

	switch (m_DataTypeFlag)
	{
	case MFT_DATATYPE_I420:
		I420_To_BGR24(pRawData, pRGBData, width, height);
		frameImg = cv::Mat(m_Height, m_Width, CV_8UC3, pRGBData);
		break;

	case MFT_DATATYPE_MJPG:
		MJPG_To_BGR24(pRawData, buffer_size, pRGBData);
		frameImg = cv::Mat(m_Height, m_Width, CV_8UC3, pRGBData);
		cv::cvtColor(frameImg, frameImg, CV_RGB2BGR);
		break;

	case MFT_DATATYPE_RGB24:
		memcpy(pRGBData, pRawData, buffer_size);
		frameImg = cv::Mat(m_Height, m_Width, CV_8UC3, pRGBData);
		cv::flip(frameImg, frameImg, 0);
		break;

	case MFT_DATATYPE_YUV2:
		YUV2_To_RGB24(pRawData, pRGBData, width, height);
		frameImg = cv::Mat(m_Height, m_Width, CV_8UC3, pRGBData);
		cv::flip(frameImg, frameImg, 0);
		break;

	default:
		break;
	}
	
	buffer->Unlock();
	buffer->Release();
	return frameImg;
}

bool MediaFoundationTools::YUV2_To_RGB24(
	uint8_t* pYUVData,
	uint8_t *pRGBData, 
	const int& width, 
	const int& height)
{
	if (NULL == pRGBData) {
		return false;
	}
	int Y1, U1, V1, Y2, R1, G1, B1, R2, G2, B2;
	int C1, D1, E1, C2;

	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width / 2; ++j) {
			Y1 = *(pYUVData + i*width * 2 + j * 4);
			U1 = *(pYUVData + i*width * 2 + j * 4 + 1);
			Y2 = *(pYUVData + i*width * 2 + j * 4 + 2);
			V1 = *(pYUVData + i*width * 2 + j * 4 + 3);
			C1 = Y1 - 16;
			C2 = Y2 - 16;
			D1 = U1 - 128;
			E1 = V1 - 128;
			R1 = ((298 * C1 + 409 * E1 + 128) >> 8 > 255 ? 255 : (298 * C1 + 409 * E1 + 128) >> 8);
			G1 = ((298 * C1 - 100 * D1 - 208 * E1 + 128) >> 8 > 255 ? 255 : (298 * C1 - 100 * D1 - 208 * E1 + 128) >> 8);
			B1 = ((298 * C1 + 516 * D1 + 128) >> 8 > 255 ? 255 : (298 * C1 + 516 * D1 + 128) >> 8);
			R2 = ((298 * C2 + 409 * E1 + 128) >> 8 > 255 ? 255 : (298 * C2 + 409 * E1 + 128) >> 8);
			G2 = ((298 * C2 - 100 * D1 - 208 * E1 + 128) >> 8 > 255 ? 255 : (298 * C2 - 100 * D1 - 208 * E1 + 128) >> 8);
			B2 = ((298 * C2 + 516 * D1 + 128) >> 8 > 255 ? 255 : (298 * C2 + 516 * D1 + 128) >> 8);
			*(pRGBData + (height - i - 1)*width * 3 + j * 6 + 2) = R1 < 0 ? 0 : R1;
			*(pRGBData + (height - i - 1)*width * 3 + j * 6 + 1) = G1 < 0 ? 0 : G1;
			*(pRGBData + (height - i - 1)*width * 3 + j * 6) = B1 < 0 ? 0 : B1;
			*(pRGBData + (height - i - 1)*width * 3 + j * 6 + 5) = R2 < 0 ? 0 : R2;
			*(pRGBData + (height - i - 1)*width * 3 + j * 6 + 4) = G2 < 0 ? 0 : G2;
			*(pRGBData + (height - i - 1)*width * 3 + j * 6 + 3) = B2 < 0 ? 0 : B2;
		}
	}
	return true;
}

bool MediaFoundationTools::I420_To_BGR24(
	uint8_t* pYUV420,
	uint8_t *pRGBData,
	const int& width,
	const int& height)
{
	int baseSize = width * height;
	int rgbSize = baseSize * 3;

	BYTE* rgbData = new BYTE[rgbSize];
	memset(rgbData, 0, rgbSize);

	int temp = 0;

	BYTE* puc_y = pYUV420;            
	BYTE* puc_u = pYUV420 + baseSize;    
	BYTE* puc_v = puc_u + (baseSize >> 2);   

	BYTE* rData = rgbData;   
	BYTE* gData = rgbData + baseSize;   
	BYTE* bData = gData + baseSize; 

	int uvIndex = 0, yIndex = 0;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			uvIndex = (y >> 1) * (width >> 1) + (x >> 1);
			yIndex = y * width + x;

			temp = (int)(puc_y[yIndex] + (puc_v[uvIndex] - 128) * 1.4022);
			rData[yIndex] = temp < 0 ? 0 : (temp > 255 ? 255 : temp);

			temp = (int)(puc_y[yIndex] + (puc_u[uvIndex] - 128) * (-0.3456) +
				(puc_v[uvIndex] - 128) * (-0.7145));
			gData[yIndex] = temp < 0 ? 0 : (temp > 255 ? 255 : temp);

			temp = (int)(puc_y[yIndex] + (puc_u[uvIndex] - 128) * 1.771);
			bData[yIndex] = temp < 0 ? 0 : (temp > 255 ? 255 : temp);
		}
	}

	int widthStep = width * 3;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			pRGBData[y * widthStep + x * 3 + 2] = rData[y * width + x];
			pRGBData[y * widthStep + x * 3 + 1] = gData[y * width + x]; 
			pRGBData[y * widthStep + x * 3 + 0] = bData[y * width + x]; 
		}
	}

	if (!pRGBData) {
		return false;
	}
	delete[] rgbData;
	return true;
}

bool MediaFoundationTools::MJPG_To_BGR24(
	uint8_t* pMjpg, 
	const int& srcSize,
	uint8_t* prgb)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	jpeg_mem_src(&cinfo, pMjpg, srcSize);
	jpeg_read_header(&cinfo, TRUE);
	int m_height = cinfo.image_height;
	int m_width = cinfo.image_width;
	jpeg_start_decompress(&cinfo);

	JSAMPROW row_pointer[1];

	while (cinfo.output_scanline < cinfo.output_height)
	{
		row_pointer[0] = &prgb[cinfo.output_scanline*cinfo.image_width*cinfo.num_components];
		jpeg_read_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	return true;
}

cv::Mat MediaFoundationTools::GetFrameData()
{
	HRESULT hr;
	IMFSample *pSample;
	BYTE *pBuffer = NULL;
	DWORD dataSize = 0;
	DWORD streamIndex, flags;
	LONGLONG llTimeStamp;
	cv::Mat tempImg;

	hr = reader->ReadSample(
		MF_SOURCE_READER_FIRST_VIDEO_STREAM,
		0,
		&streamIndex,
		&flags,
		&llTimeStamp,
		&pSample
	);
	if (FAILED(hr)) {
		return m_FrameImg;
	}
	
	if (pSample != NULL) {
		int widthLine = (m_Width * 24 + 31) / 8;
		uint8_t* pRGB = new uint8_t[widthLine*m_Height];
		tempImg = TransformRawDataToRGB(pSample, pRGB, m_Width, m_Height);
		m_FrameImg = tempImg.clone();
		pSample->Release();
		delete [] pRGB;
		tempImg.release();
	}	
	return m_FrameImg;
}

int MediaFoundationTools::CameraInputInit(const PreviewCameraInfo& cInfo)
{
	if (SetDevice(cInfo.name) < 0) {
		return -1;
	}
	IMFMediaSource *pSource = NULL;
	ppDevices[m_DeviceIndex]->ActivateObject(
		__uuidof(IMFMediaSource),
		(void**)&pSource
	);
	HRESULT hr = MFCreateSourceReaderFromMediaSource(
		pSource,
		pAttributes,
		&reader
	);

	SetDataInfo(cInfo);
	CoTaskMemFree(ppDevices);
	pAttributes->Release();
	MFShutdown();
	return 0;
}

int MediaFoundationTools::DestoryInputParam()
{
//	ppDevices[m_DeviceIndex]->Release();
	reader->Release();
	return 0;
}
