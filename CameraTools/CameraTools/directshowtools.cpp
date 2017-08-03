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

void DirectShowTools::ConfigCamera()
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

int DirectShowTools::flush_encoder(unsigned int stream_index) 
{
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
        ret = avcodec_encode_video2(m_OutputFormatCtx->streams[stream_index]->codec,
									&enc_pkt, NULL, &got_frame);
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

// 引用几个函数修改自muxing.c 文档
// add_video_stream(AVFormatContext *oc, enum AVCodecID codec_id)
// open_video(AVFormatContext *oc, AVStream *st)
// write_video_frame(AVFormatContext *oc_m, AVStream *video_st_m, int &pts_num,
//					 AVFrame* &m_OutputFrame, int &framecnt)
AVStream* DirectShowTools::add_video_stream(AVFormatContext *oc, enum AVCodecID codec_id)
{
	AVCodecContext *c;
	AVStream *st;
	AVCodec *codec;

	st = avformat_new_stream(oc, NULL);
	if (!st) {
		return NULL;
	}

	c = st->codec;

	/* find the video encoder */
	codec = avcodec_find_encoder(codec_id);
	if (!codec) {
		return NULL;
	}
	avcodec_get_context_defaults3(c, codec);

	c->codec_id = codec_id;

	/* put sample parameters */
	c->bit_rate = /*400000*/3000000;
	/* resolution must be a multiple of two */
	c->width = /*352*/640;
	c->height = /*288*/480;
	/* time base: this is the fundamental unit of time (in seconds) in terms
	of which frame timestamps are represented. for fixed-fps content,
	timebase should be 1/framerate and timestamp increments should be
	identically 1. */
	c->time_base.den = 25;
	c->time_base.num = 1;
	c->gop_size = 12; /* emit one intra frame every twelve frames at most */
	c->pix_fmt = PIX_FMT_YUV420P;
	if (c->codec_id == CODEC_ID_MPEG2VIDEO) {
		/* just for testing, we also add B frames */
		c->max_b_frames = 2;
	}
	if (c->codec_id == CODEC_ID_MPEG1VIDEO) {
		/* Needed to avoid using macroblocks in which some coeffs overflow.
		This does not happen with normal video, it just happens here as
		the motion of the chroma plane does not match the luma plane. */
		c->mb_decision = 2;
	}
	// some formats want stream headers to be separate
	if (oc->oformat->flags & AVFMT_GLOBALHEADER)
		c->flags |= CODEC_FLAG_GLOBAL_HEADER;

	return st;
}

void DirectShowTools::open_video(AVFormatContext *oc, AVStream *st)
{
	AVCodec *codec;
	AVCodecContext *c;

	c = st->codec;

	/* find the video encoder */
	codec = avcodec_find_encoder(c->codec_id);
	if (!codec) {
		fprintf(stderr, "codec not found\n");
	}

	/* open the codec */
	if (avcodec_open2(c, codec, NULL) < 0) {
		fprintf(stderr, "could not open codec\n");
	}
}

void DirectShowTools::write_video_frame(AVFormatContext *oc_m, AVStream *video_st_m, int &pts_num,
					   AVFrame* &m_OutputFrame, int &framecnt)
{
	if (oc_m->oformat->flags & AVFMT_RAWPICTURE) {
		AVPacket pkt;
		av_init_packet(&pkt);

		pkt.flags |= AV_PKT_FLAG_KEY;
		pkt.stream_index = video_st_m->index;
		pkt.data = (uint8_t *)m_OutputFrame;
		pkt.size = sizeof(AVPicture);

		av_interleaved_write_frame(oc_m, &pkt);
	}
	else {
		int video_outbuf_size = 2000000;
		uint8_t* video_outbuf = (uint8_t *)av_malloc(video_outbuf_size);
		memset(video_outbuf, 0, video_outbuf_size);

		int out_size = 0;
		m_OutputFrame->pts = pts_num;
		out_size = avcodec_encode_video(video_st_m->codec, video_outbuf, video_outbuf_size, m_OutputFrame);

		if (out_size > 0) {
			AVPacket pkt;
			av_init_packet(&pkt);

			if (video_st_m->codec->coded_frame->pts != AV_NOPTS_VALUE)
				pkt.pts = av_rescale_q(video_st_m->codec->coded_frame->pts, video_st_m->codec->time_base, 
									   video_st_m->time_base);
			if (video_st_m->codec->coded_frame->key_frame)
				pkt.flags |= AV_PKT_FLAG_KEY;
			pkt.stream_index = video_st_m->index;
			pkt.data = video_outbuf;
			pkt.size = out_size;
			framecnt++;
			av_interleaved_write_frame(oc_m, &pkt);
		}
		av_free(video_outbuf);
	}
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

	std::string output_file_ = "t1.mp4";

	double video_pts_m;
	avformat_alloc_output_context2(&m_OutputFormatCtx, NULL, NULL, output_file_.c_str());
	if (!m_OutputFormatCtx) {
		avformat_alloc_output_context2(&m_OutputFormatCtx, NULL, "mpeg", output_file_.c_str());
	}
	if (!m_OutputFormatCtx) {
		return ;
	}

	m_OutputFormatCtx->oformat->video_codec = CODEC_ID_H264;
	m_OutputFormat = m_OutputFormatCtx->oformat;

	m_OutputVideoStream = NULL;
	if (m_OutputFormat->video_codec != CODEC_ID_NONE) {
		m_OutputVideoStream = add_video_stream(m_OutputFormatCtx, m_OutputFormat->video_codec);
	}

	if (m_OutputVideoStream)
		open_video(m_OutputFormatCtx, m_OutputVideoStream);

	if (!(m_OutputFormat->flags & AVFMT_NOFILE)) {
		if (avio_open(&m_OutputFormatCtx->pb, output_file_.c_str(), AVIO_FLAG_WRITE) < 0) {
			return ;
		}
	}
	avformat_write_header(m_OutputFormatCtx, NULL);

	m_OutputCodecCtx = m_OutputVideoStream->codec;
	m_OutputFrame = av_frame_alloc();
	int picture_size = avpicture_get_size(m_OutputCodecCtx->pix_fmt, m_OutputCodecCtx->width, m_OutputCodecCtx->height);
	uint8_t *picture_buf = (uint8_t *)av_malloc(picture_size);
	avpicture_fill((AVPicture *)m_OutputFrame, picture_buf, m_OutputCodecCtx->pix_fmt, m_OutputCodecCtx->width, m_OutputCodecCtx->height);

	int res, frameFinished, pts_num = 0, framecnt = 0;
	AVPacket packet;
	
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
							AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
		sws_scale(img_convert_ctx, ((AVPicture*)m_InputFrame)->data, 
			     ((AVPicture*)m_InputFrame)->linesize, 0, m_InputCodecCtx->height, 
			     ((AVPicture *)m_InputFrameRGB)->data, ((AVPicture *)m_InputFrameRGB)->linesize);

		cv::Mat img(m_InputFrame->height, m_InputFrame->width, CV_8UC3, m_InputFrameRGB->data[0]);
		QImage Qimg = QImage((const unsigned char*)(img.data), img.cols,
			img.rows, img.cols*img.channels(), QImage::Format_RGB888);
		emit send_image_data(Qimg);
		cvWaitKey(10);

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

		if (m_OutputVideoStream) {
			video_pts_m = (double)m_OutputVideoStream->pts.val * m_OutputVideoStream->time_base.num 
						  / m_OutputVideoStream->time_base.den;
		}
		else {
			video_pts_m = 0.0;
		}

		if (!m_OutputVideoStream || video_pts_m >= 5) {
			break;
		}
		write_video_frame(m_OutputFormatCtx, m_OutputVideoStream, pts_num, m_OutputFrame, framecnt);
		pts_num++;

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
}