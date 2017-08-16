#include "DirectShowTools.h"



DirectShowTools::DirectShowTools()
{
	m_pMoniker = NULL;
}


DirectShowTools::~DirectShowTools()
{
}

/*
 * 获得GUID对应的数据格式
 * 参数：
 *		guid：输入的GUID值
 * 返回值：guid对应的string数据类型
 */
std::string DirectShowTools::GetSubType(GUID guid)
{
	if (guid == MEDIASUBTYPE_YUY2) {
		return "YUY2";
	}
	if (guid == MEDIASUBTYPE_MJPG) {
		return "MJPG";
	}
	if (guid == MEDIASUBTYPE_RGB24) {
		return "RGB24";
	}
	if (guid == WMMEDIASUBTYPE_I420) {
		return "I420";
	}
	return std::string();
}

/*
 * 列举当前系统中可用的摄像头设备
 * 检测每个可用设备的详细参数，即分辨率，帧率，数据格式
 * 返回值：包含系统可用摄像头及其详细参数
 */
std::vector<CameraDeviceInfo> DirectShowTools::ListCameraDevice()
{
	std::vector<CameraDeviceInfo> deviceList;
	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pEnum = NULL;
	HRESULT hr = CoInitialize(0);

	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
		CLSCTX_INPROC_SERVER, IID_ICreateDevEnum,
		reinterpret_cast<void**>(&pDevEnum));

	if (FAILED(hr)) {
		return deviceList;
	}

	hr = pDevEnum->CreateClassEnumerator(
		CLSID_VideoInputDeviceCategory,
		&pEnum, 0);

	if (pEnum == NULL) {
		pDevEnum->Release();
		pEnum->Release();
		return deviceList;
	}

	while (pEnum->Next(1, &m_pMoniker, NULL) == S_OK) {
		IPropertyBag *pPropBag;
		hr = m_pMoniker->BindToStorage(0, 0, IID_IPropertyBag,
			(void**)(&pPropBag));

		if (FAILED(hr)) {
			m_pMoniker->Release();
			continue;
		}

		VARIANT varName;
		VariantInit(&varName);
		hr = pPropBag->Read(L"Description", &varName, 0);

		if (FAILED(hr)) {
			hr = pPropBag->Read(L"FriendlyName", &varName, 0);
		}

		if (SUCCEEDED(hr)) {
			CameraDeviceInfo temp_camera_device_info;
			char display_name[1024];
			WideCharToMultiByte(CP_ACP, 0, varName.bstrVal, -1,
				display_name, 1024, "", NULL);
			std::string str_temp(display_name);
			temp_camera_device_info = GetCameraDeviceInfo();
			temp_camera_device_info.friend_name = str_temp;
			deviceList.push_back(temp_camera_device_info);
			VariantClear(&varName);
		}

		pPropBag->Release();
		m_pMoniker->Release();
	}  // end while
	pDevEnum->Release();
	pEnum->Release();
	return deviceList;
	
}

/*
 * 得到当前设备的详细参数，即分辨率，帧率，数据格式
 * 返回值：摄像头的详细参数结构体
 */
CameraDeviceInfo DirectShowTools::GetCameraDeviceInfo()
{
	CameraDeviceInfo cameraInfo;
	HRESULT hr = NULL;
	IBaseFilter *pFilter;
	hr = m_pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&pFilter);

	if (!pFilter) {
		return cameraInfo;
	}

	IEnumPins *pinEnum = NULL;
	IPin *pin = NULL;

	if (FAILED(pFilter->EnumPins(&pinEnum))) {
		pinEnum->Release();
		return cameraInfo;
	}

	pinEnum->Reset();
	ULONG pinFetched = 0;

	while (SUCCEEDED(pinEnum->Next(1, &pin, &pinFetched)) && pinFetched) {
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
		AM_MEDIA_TYPE *mt = NULL;
		if (FAILED(pin->EnumMediaTypes(&mtEnum))) {
			break;
		}
		mtEnum->Reset();
		ULONG mt_fetched = 0;
		while (SUCCEEDED(mtEnum->Next(1, &mt, &mt_fetched)) && mt_fetched) {
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
				cameraInfo.data_type.insert(GetSubType(mt->subtype));
				cameraInfo.data_resolution.insert(std::to_string(bmi->biWidth) + "x"
					+ std::to_string(bmi->biHeight));
				cameraInfo.data_fps.insert(std::to_string((int)1e7 / avg_time));
			}
		}  // end second while
		pin->Release();
	}  // end first while
	return cameraInfo;
}

/*
 * 摄像头初始化函数
 * 参数：
 *		cInfo：需要设置的餐宿
 * 返回值：小于0，则初始化不成功；大于等于0，初始化成功
 */
int DirectShowTools::CameraInputInit(const PreviewCameraInfo& cInfo)
{
	avdevice_register_all();
	avcodec_register_all();
	av_register_all();

	AVCodec * pCodec;
	m_inputFormatCtx = avformat_alloc_context();
	AVInputFormat *iformat = av_find_input_format("dshow");
	AVDictionary *options = NULL;
	av_dict_set(&options, "video_size", cInfo.resolution.c_str(), 0);
	av_dict_set(&options, "framerate", cInfo.fps.c_str(), 0);
	av_dict_set(&options, "pix_fmt", cInfo.type.c_str(), 0);
	std::string cname = "video=" + cInfo.name;

	if (avformat_open_input(&m_inputFormatCtx,
		cname.c_str(), iformat, &options) != 0) {
		return -1;
	}

	if (avformat_find_stream_info(m_inputFormatCtx, NULL) < 0) {
		return -1;
	}

	for (int i = 0; i < m_inputFormatCtx->nb_streams; i++) {
		if (m_inputFormatCtx->streams[i]->codec->coder_type == AVMEDIA_TYPE_VIDEO) {
			m_videoStream = i;
			break;
		}
	}

	if (m_videoStream == -1) {
		return -1;
	}
	m_inputCodecCtx = m_inputFormatCtx->streams[m_videoStream]->codec;
	pCodec = avcodec_find_decoder(m_inputCodecCtx->codec_id);
	if (pCodec == NULL) {
		return -1;
	}

	if (avcodec_open2(m_inputCodecCtx, pCodec, NULL) < 0) {
		return -1;
	}
	AVPixelFormat  pFormat = AV_PIX_FMT_BGR24;
	uint8_t *buffer;
	int numBytes;

	m_inputFrame = av_frame_alloc();
	m_inputFrameRGB = av_frame_alloc();

	numBytes = avpicture_get_size(pFormat, m_inputCodecCtx->width, m_inputCodecCtx->height);
	buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
	avpicture_fill((AVPicture *)m_inputFrameRGB, buffer, pFormat,
		m_inputCodecCtx->width, m_inputCodecCtx->height);
	return 0;
}


/*
 * 解码从摄像头获取的数据包
 * 参数：
 *		flag: 得到数据是否成功的标志位
 * 返回值：小于0，则解码数据失败；大于等于0，则解码成功
 */
cv::Mat DirectShowTools::GetFrameData(int& flag)
{
	int ret = av_read_frame(m_inputFormatCtx, &m_packet);
	SwsContext* imgConvertCtx;

	if (ret < 0) {
		flag = -1;
		return cv::Mat();
	}

	if (m_packet.stream_index != m_videoStream) {
		flag = -1;
		return cv::Mat();
	}
	int frameFinished = 0;
	avcodec_decode_video2(m_inputCodecCtx, m_inputFrame, &frameFinished, &m_packet);
	if (!frameFinished) {
		flag = -1;
		return cv::Mat();
	}

	imgConvertCtx = sws_getCachedContext(NULL,
		m_inputCodecCtx->width,
		m_inputCodecCtx->height,
		m_inputCodecCtx->pix_fmt,
		m_inputCodecCtx->width,
		m_inputCodecCtx->height,
		AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
	sws_scale(imgConvertCtx, ((AVPicture*)m_inputFrame)->data,
		((AVPicture*)m_inputFrame)->linesize, 0, m_inputCodecCtx->height,
		((AVPicture *)m_inputFrameRGB)->data, ((AVPicture *)m_inputFrameRGB)->linesize);
	sws_freeContext(imgConvertCtx);

	cv::Mat img(m_inputFrame->height, m_inputFrame->width,
		CV_8UC3, m_inputFrameRGB->data[0]);

	av_free_packet(&m_packet);
	return img;
}

/*
 * 释放获取摄像头数据的相关参数的指针
 */
int DirectShowTools::DestoryInputParam()
{
	if (m_inputCodecCtx != NULL) {
		avcodec_close(m_inputCodecCtx);
	}
	if (m_inputFormatCtx != NULL) {
		avformat_close_input(&m_inputFormatCtx);
	}
	if (m_inputFrame != NULL) {
		av_free(m_inputFrame);
	}
	if (m_inputFrameRGB != NULL) {
		av_free(m_inputFrameRGB);
	}
	return 0;
}