#include "directshowtools.h"

DirectShowTools::DirectShowTools()
{

}

void DirectShowTools::Destory()
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
    if (m_OutputCodecCtx != NULL) {
        avcodec_close(m_OutputCodecCtx);
    }
    if (m_OutputFormatCtx != NULL) {
        avio_close(m_OutputFormatCtx->pb);
        avformat_free_context(m_OutputFormatCtx);
    }
    if (m_OutputFrame != NULL) {
        av_free(m_OutputFrame);
    }
    if (m_OutputVideoStream != NULL) {
        avcodec_close(m_OutputVideoStream->codec);
    }
}

int DirectShowTools::GuidToString(const GUID &guid, char* buffer) {
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

int DirectShowTools::GetMajorType(GUID guid, char* buffer) {
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

int DirectShowTools::GetSubType(GUID guid, char* buffer) {
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

int DirectShowTools::GetFormatType(GUID guid, char* buffer) {
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

void AddCameraInfo(CameraDeviceInfo &camera_info,const std::string &data_type,
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
            it = std::find(camera_info.data[index].begin(), camera_info.data[index].end(), data_info);
            if (it == camera_info.data[index].end()) {
                camera_info.data[index].push_back(data_info);
            }
        }
    }
}

CameraDeviceInfo DirectShowTools::GetCameraDeviceInfo(IMoniker* pMoniker)
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

std::vector<CameraDeviceInfo> DirectShowTools::ListCameraDevice()
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

int DirectShowTools::flush_encoder(unsigned int stream_index) {
    int ret;
    int got_frame;
    AVPacket enc_pkt;
    if (!(m_OutputFormatCtx->streams[stream_index]->codec->codec->capabilities &
        CODEC_CAP_DELAY))
        return 0;
    while (1) {
        enc_pkt.data = NULL;
        enc_pkt.size = 0;
        av_init_packet(&enc_pkt);
        ret = avcodec_encode_video2(m_OutputFormatCtx->streams[stream_index]->codec, &enc_pkt,
            NULL, &got_frame);
        av_frame_free(NULL);
        if (ret < 0)
            break;
        if (!got_frame) {
            ret = 0;
            break;
        }
        printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", enc_pkt.size);
        ret = av_write_frame(m_OutputFormatCtx, &enc_pkt);
        if (ret < 0)
            break;
    }
    return ret;
}

int DirectShowTools::CameraInputInit(const std::string& camera_name, int &video_stream)
{
    AVCodec * pCodec;
    m_InputFormatCtx = avformat_alloc_context();
    AVInputFormat *iformat = av_find_input_format("dshow");
    if (avformat_open_input(&m_InputFormatCtx, camera_name.c_str(), iformat, NULL) != 0) return -12;
    if (avformat_find_stream_info(m_InputFormatCtx, NULL) < 0) return -13;
    av_dump_format(m_InputFormatCtx, 0, camera_name.c_str(), 0);

    for (int i = 0; i < m_InputFormatCtx->nb_streams; i++)
    {
        if (m_InputFormatCtx->streams[i]->codec->coder_type == AVMEDIA_TYPE_VIDEO)
        {
            video_stream = i;
            break;
        }
    }

    if (video_stream == -1) return -14;
    m_InputCodecCtx = m_InputFormatCtx->streams[video_stream]->codec;

    pCodec = avcodec_find_decoder(m_InputCodecCtx->codec_id);
    if (pCodec == NULL) return -15; //codec not found

    if (avcodec_open2(m_InputCodecCtx, pCodec, NULL) < 0) return -16;
    AVPixelFormat  pFormat = AV_PIX_FMT_BGR24;

    uint8_t *buffer;
    int numBytes;

    m_InputFrame = av_frame_alloc();
    m_InputFrameRGB = av_frame_alloc();

    numBytes = avpicture_get_size(pFormat, m_InputCodecCtx->width, m_InputCodecCtx->height);
    buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
    avpicture_fill((AVPicture *)m_InputFrameRGB, buffer, pFormat, m_InputCodecCtx->width, m_InputCodecCtx->height);
}


int DirectShowTools::H264OutputConfig(const std::string& output_name, int& picture_size, uint8_t* picture_buf)
{
    AVOutputFormat* fmt;
    AVCodec* pCodec;
    m_OutputFormatCtx = avformat_alloc_context();
    fmt = av_guess_format(NULL, output_name.c_str(), NULL);
    if (NULL == fmt) {
        return -1;
    }
    m_OutputFormatCtx->oformat = fmt;
    if (avio_open(&m_OutputFormatCtx->pb, output_name.c_str(), AVIO_FLAG_READ_WRITE) < 0) {
        return -1;
    }
    m_OutputVideoStream = avformat_new_stream(m_OutputFormatCtx, 0);
    if (m_OutputVideoStream == NULL) {
        return -1;
    }
    m_OutputCodecCtx = m_OutputVideoStream->codec;
    m_OutputCodecCtx->codec_id = fmt->video_codec;
    m_OutputCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    m_OutputCodecCtx->pix_fmt = PIX_FMT_YUV420P;
    m_OutputCodecCtx->width = 640;
    m_OutputCodecCtx->height = 480;
    m_OutputCodecCtx->time_base.num = 1;
    m_OutputCodecCtx->time_base.den = 25;
    m_OutputCodecCtx->bit_rate = 400000;
    m_OutputCodecCtx->gop_size = 10;
    m_OutputCodecCtx->qmin = 10;
    m_OutputCodecCtx->qmax = 51;
    m_OutputCodecCtx->max_b_frames = 3;

    AVDictionary *param_out = 0;
    if (m_OutputCodecCtx->codec_id == AV_CODEC_ID_H264) {
        av_dict_set(&param_out, "preset", "slow", 0);
        av_dict_set(&param_out, "tune", "zerolatency", 0);
    }
    av_dump_format(m_OutputFormatCtx, 0, output_name.c_str(), 1);
    pCodec = avcodec_find_encoder(m_OutputCodecCtx->codec_id);

    if (!pCodec) {
        return -1;
    }
    if (avcodec_open2(m_OutputCodecCtx, pCodec, &param_out) < 0) {
        return -1;
    }
    m_OutputFrame = av_frame_alloc();
    picture_size = avpicture_get_size(m_OutputCodecCtx->pix_fmt, m_OutputCodecCtx->width, m_OutputCodecCtx->height);
    picture_buf = (uint8_t *)av_malloc(picture_size);
    avpicture_fill((AVPicture *)m_OutputFrame, picture_buf, m_OutputCodecCtx->pix_fmt, m_OutputCodecCtx->width, m_OutputCodecCtx->height);
    avformat_write_header(m_OutputFormatCtx, NULL);

}

void DirectShowTools::run()
{
	std::vector<CameraDeviceInfo> camera_device_list;
	camera_device_list = ListCameraDevice();

	avdevice_register_all();
	avcodec_register_all();
	av_register_all();

	std::string camera_name = "video=" + camera_device_list[1].friend_name;
	int video_stream = 1;
	CameraInputInit(camera_name, video_stream);

	uint8_t* picture_buf = NULL;
	int picture_size = 0;
	std::string output_file_name = "test.h264";
	H264OutputConfig(output_file_name, picture_size, picture_buf);

	AVPacket pkt_out;
	av_new_packet(&pkt_out, picture_size);

	int res;
	int frameFinished;
	AVPacket packet;
	int pts_num = 0;
	int framecnt = 0;
	while (res = av_read_frame(m_InputFormatCtx, &packet) >= 0)
	{
		if (packet.stream_index != video_stream) {
			continue;
		}
		avcodec_decode_video2(m_InputCodecCtx, m_InputFrame, &frameFinished, &packet);
		if (!frameFinished) {
			continue;
		}
		struct SwsContext * img_convert_ctx;
		img_convert_ctx = sws_getCachedContext(NULL, m_InputCodecCtx->width, 
							m_InputCodecCtx->height, m_InputCodecCtx->pix_fmt, 
							m_InputCodecCtx->width, m_InputCodecCtx->height, 
							AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
		sws_scale(img_convert_ctx, ((AVPicture*)m_InputFrame)->data, 
			     ((AVPicture*)m_InputFrame)->linesize, 0, m_InputCodecCtx->height, 
			     ((AVPicture *)m_InputFrameRGB)->data, ((AVPicture *)m_InputFrameRGB)->linesize);

		cv::Mat img(m_InputFrame->height, m_InputFrame->width, CV_8UC3, m_InputFrameRGB->data[0]);
		cv::imshow("display", img);
		cvWaitKey(1);
		sws_freeContext(img_convert_ctx);

		if (framecnt > 200) break;

		struct SwsContext * img_convert_ctx_out;
		img_convert_ctx_out = sws_getCachedContext(NULL, m_InputCodecCtx->width, 
							  m_InputCodecCtx->height, m_InputCodecCtx->pix_fmt, 
							  m_InputCodecCtx->width, m_InputCodecCtx->height, 
							  AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, NULL, NULL, NULL);

		sws_scale(img_convert_ctx_out, ((AVPicture*)m_InputFrame)->data, 
				 ((AVPicture*)m_InputFrame)->linesize, 0, m_InputCodecCtx->height, 
				 ((AVPicture *)m_OutputFrame)->data, ((AVPicture *)m_OutputFrame)->linesize);

		m_OutputFrame->pkt_pts = pts_num * (m_OutputVideoStream->time_base.den)
								 /((m_OutputVideoStream->time_base.num) * 25);
		pts_num++;
		int got_picture = 0;
		int ret = avcodec_encode_video2(m_OutputCodecCtx, &pkt_out, m_OutputFrame, &got_picture);
		if (ret < 0) {
			return ;
		}
		if (got_picture == 1) {
			framecnt++;
			pkt_out.stream_index = m_OutputVideoStream->index;
			ret = av_write_frame(m_OutputFormatCtx, &pkt_out);
			av_free_packet(&pkt_out);
		}
		av_free_packet(&packet);
		sws_freeContext(img_convert_ctx_out);
	}
	flush_encoder(0);
	av_write_trailer(m_OutputFormatCtx);
	if (m_OutputVideoStream) {
		av_free(picture_buf);
	}
	Destory();
	av_free_packet(&packet);
	av_free_packet(&pkt_out);
}