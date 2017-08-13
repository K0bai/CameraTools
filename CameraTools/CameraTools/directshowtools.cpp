#include "DirectShowTools.h"



DirectShowTools::DirectShowTools()
{
	pMoniker = NULL;
}


DirectShowTools::~DirectShowTools()
{
}

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

std::vector<CameraDeviceInfo> DirectShowTools::ListCameraDevice()
{
	std::vector<CameraDeviceInfo> device_list;
	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pEnum = NULL;
	HRESULT hr = CoInitialize(0);
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
		CLSCTX_INPROC_SERVER, IID_ICreateDevEnum,
		reinterpret_cast<void**>(&pDevEnum));

	if (FAILED(hr)) {
		return device_list;
	}
	hr = pDevEnum->CreateClassEnumerator(
		CLSID_VideoInputDeviceCategory,
		&pEnum, 0);

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
			temp_camera_device_info = GetCameraDeviceInfo();
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

CameraDeviceInfo DirectShowTools::GetCameraDeviceInfo()
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
				camera_info.data_type.insert(GetSubType(mt->subtype));
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

// 摄像头初始化函数
// 参数1 输入的需要设置的摄像头参数
// 参数2 摄像头输入数据中视频数据的索引
// 返回值小于0，则初始化不成功
// 返回值大于等于0，初始化成功
int DirectShowTools::CameraInputInit(const PreviewCameraInfo& cInfo)
{
	avdevice_register_all();
	avcodec_register_all();
	av_register_all();

	AVCodec * pCodec;
	m_InputFormatCtx = avformat_alloc_context();
	AVInputFormat *iformat = av_find_input_format("dshow");
	AVDictionary *options = NULL;
	av_dict_set(&options, "video_size", cInfo.resolution.c_str(), 0);
	av_dict_set(&options, "framerate", cInfo.fps.c_str(), 0);
	av_dict_set(&options, "frame_bits", cInfo.bit.c_str(), 0);
	av_dict_set(&options, "pix_fmt", cInfo.type.c_str(), 0);
	std::string cname = "video=" + cInfo.name;
	if (avformat_open_input(&m_InputFormatCtx,
		cname.c_str(), iformat, &options) != 0) {
		return -1;
	}
	if (avformat_find_stream_info(m_InputFormatCtx, NULL) < 0) {
		return -1;
	}
	for (int i = 0; i < m_InputFormatCtx->nb_streams; i++) {
		if (m_InputFormatCtx->streams[i]->codec->coder_type == AVMEDIA_TYPE_VIDEO) {
			video_stream = i;
			break;
		}
	}

	if (video_stream == -1) {
		return -1;
	}
	m_InputCodecCtx = m_InputFormatCtx->streams[video_stream]->codec;
	pCodec = avcodec_find_decoder(m_InputCodecCtx->codec_id);
	if (pCodec == NULL) {
		return -1;
	}

	if (avcodec_open2(m_InputCodecCtx, pCodec, NULL) < 0) {
		return -1;
	}
	AVPixelFormat  pFormat = AV_PIX_FMT_BGR24;
	uint8_t *buffer;
	int numBytes;

	m_InputFrame = av_frame_alloc();
	m_InputFrameRGB = av_frame_alloc();

	numBytes = avpicture_get_size(pFormat, m_InputCodecCtx->width, m_InputCodecCtx->height);
	buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
	avpicture_fill((AVPicture *)m_InputFrameRGB, buffer, pFormat,
		m_InputCodecCtx->width, m_InputCodecCtx->height);
	return 0;
}


// 解码从摄像头获取的数据包
// 参数1 摄像头数据中视频流的索引
// 参数2 从摄像头获取的数据包
// 参数3 SwsContext*对象，用于数据的格式转换
// 若返回小于0，则解码数据失败
// 若返回大于等于0，则解码成功
cv::Mat DirectShowTools::GetFrameData()
{
	int ret = av_read_frame(m_InputFormatCtx, &packet);
	SwsContext* img_convert_ctx;
	if (packet.stream_index != video_stream) {
		return cv::Mat();
	}
	int frameFinished = 0;
	avcodec_decode_video2(m_InputCodecCtx, m_InputFrame, &frameFinished, &packet);
	if (!frameFinished) {
		return cv::Mat();
	}

	img_convert_ctx = sws_getCachedContext(NULL,
		m_InputCodecCtx->width,
		m_InputCodecCtx->height,
		m_InputCodecCtx->pix_fmt,
		m_InputCodecCtx->width,
		m_InputCodecCtx->height,
		AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
	sws_scale(img_convert_ctx, ((AVPicture*)m_InputFrame)->data,
		((AVPicture*)m_InputFrame)->linesize, 0, m_InputCodecCtx->height,
		((AVPicture *)m_InputFrameRGB)->data, ((AVPicture *)m_InputFrameRGB)->linesize);
	sws_freeContext(img_convert_ctx);

	cv::Mat img(m_InputFrame->height, m_InputFrame->width,
		CV_8UC3, m_InputFrameRGB->data[0]);

	av_free_packet(&packet);
	return img;
}

int DirectShowTools::DestoryInputParam()
{
	if (m_InputCodecCtx != NULL) {
		avcodec_close(m_InputCodecCtx);
	}
	if (m_InputFormatCtx != NULL) {
		avformat_close_input(&m_InputFormatCtx);
	}
	if (m_InputFrame != NULL) {
		av_free(m_InputFrame);
	}
	if (m_InputFrameRGB != NULL) {
		av_free(m_InputFrameRGB);
	}
	return 0;
}