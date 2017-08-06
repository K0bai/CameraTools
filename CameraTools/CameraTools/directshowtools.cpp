#include "directshowtools.h"
#include "cameraconfig.h"
#include <qdatetime.h>

DirectShowTools::DirectShowTools()
{
	m_PictureBuff = NULL;
	m_VideoOutBuff = NULL;
	m_MaskFlag = DST_WATERMASK_NONE;
	m_ThreadStop = false;
	m_ShowIndex = 0;
	m_Internal = 0;
	m_Transparency = 0;
	m_GrabImgFlag = 0;
	m_SaveVideoFlag = DST_SAVEVIDEO_NONE;

	connect(this, SIGNAL(SendImageGrabMsg()), this,
		SLOT(SaveGrabImage()), Qt::QueuedConnection);
}

void DirectShowTools::DestoryInputParam()
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
}

void DirectShowTools::DestoryVideoParam()
{
	if (m_OutputCodecCtx != NULL) {
	//	avcodec_close(m_OutputCodecCtx);
	}
	if (m_OutputFormatCtx != NULL) {
		avio_close(m_OutputFormatCtx->pb);
		avformat_free_context(m_OutputFormatCtx);
	}
	if (m_OutputFrame != NULL) {
		av_free(m_OutputFrame);
	}
	if (m_OutputVideoStream != NULL) {
		if (m_OutputVideoStream) {
			av_free(m_PictureBuff);
			av_free(m_VideoOutBuff);
		}
		avcodec_close(m_OutputVideoStream->codec);
	}
}

int DirectShowTools::FlushEncoder(unsigned int stream_index) 
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
		if (ret < 0) {
			break;
		}
        if (!got_frame) {
            ret = 0;
            break;
        }
        ret = av_write_frame(m_OutputFormatCtx, &enc_pkt);
		if (ret < 0) {
			break;
		}
    }
    return ret;
}

int DirectShowTools::CameraInputInit(const std::string& camera_name, int &video_stream)
{
	avdevice_register_all();
	avcodec_register_all();
	av_register_all();

    AVCodec * pCodec;
    m_InputFormatCtx = avformat_alloc_context();
    AVInputFormat *iformat = av_find_input_format("dshow");
	AVDictionary *options = NULL; 
//	av_dict_set(&options, "video_size", "1280x720", 0);
//	av_dict_set(&options, "framerate", "5", 0);
//	av_dict_set(&options, "frame_bits", "12", 0);
//	av_dict_set(&options, "pix_fmt", "I420", 0);
	std::string cname = "video=" + camera_name;
	if (avformat_open_input(&m_InputFormatCtx, 
		cname.c_str(), iformat, &options) != 0) {
		return -1;
	}
	if (avformat_find_stream_info(m_InputFormatCtx, NULL) < 0) {
		return -2;
	}
    av_dump_format(m_InputFormatCtx, 0, camera_name.c_str(), 0);

    for (int i = 0; i < m_InputFormatCtx->nb_streams; i++) {
        if (m_InputFormatCtx->streams[i]->codec->coder_type == AVMEDIA_TYPE_VIDEO) {
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
    avpicture_fill((AVPicture *)m_InputFrameRGB, buffer, pFormat, 
		m_InputCodecCtx->width, m_InputCodecCtx->height);
	return 0;
}

int DirectShowTools::MP4OutputConfig(const std::string& output_name)
{
	avformat_alloc_output_context2(&m_OutputFormatCtx, NULL,
		NULL, output_name.c_str());
	if (!m_OutputFormatCtx) {
		avformat_alloc_output_context2(&m_OutputFormatCtx, 
			NULL, "mpeg", output_name.c_str());
	}
	if (!m_OutputFormatCtx) {
		return 0;
	}

	m_OutputFormatCtx->oformat->video_codec = CODEC_ID_H264;
	m_OutputFormat = m_OutputFormatCtx->oformat;

	m_OutputVideoStream = NULL;
	if (m_OutputFormat->video_codec != CODEC_ID_NONE) {
		m_OutputVideoStream = AddVideoStream(m_OutputFormat->video_codec);
	}

	if (m_OutputVideoStream) {
		OpenOutputVideo();
	}

	if (!(m_OutputFormat->flags & AVFMT_NOFILE)) {
		if (avio_open(&m_OutputFormatCtx->pb, output_name.c_str(), AVIO_FLAG_WRITE) < 0) {
			return 0;
		}
	}
	avformat_write_header(m_OutputFormatCtx, NULL);

	m_OutputCodecCtx = m_OutputVideoStream->codec;
	m_OutputFrame = av_frame_alloc();
	m_VideoOutBuffSize = avpicture_get_size(m_OutputCodecCtx->pix_fmt, 
		m_OutputCodecCtx->width, m_OutputCodecCtx->height);

	m_PictureBuff = (uint8_t *)av_malloc(m_VideoOutBuffSize);

	avpicture_fill((AVPicture *)m_OutputFrame, m_PictureBuff, 
		m_OutputCodecCtx->pix_fmt, m_OutputCodecCtx->width, 
		m_OutputCodecCtx->height);

	m_VideoOutBuff = (uint8_t *)av_malloc(m_VideoOutBuffSize);

	return 0;
}


AVStream* DirectShowTools::AddVideoStream(enum AVCodecID codec_id)
{
	AVCodecContext *c;
	AVStream *st;
	AVCodec *codec;

	st = avformat_new_stream(m_OutputFormatCtx, NULL);
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
		c->max_b_frames = 3;
	}
	if (c->codec_id == CODEC_ID_MPEG1VIDEO) {
		/* Needed to avoid using macroblocks in which some coeffs overflow.
		This does not happen with normal video, it just happens here as
		the motion of the chroma plane does not match the luma plane. */
		c->mb_decision = 2;
	}
	// some formats want stream headers to be separate
	if (m_OutputFormatCtx->oformat->flags & AVFMT_GLOBALHEADER)
		c->flags |= CODEC_FLAG_GLOBAL_HEADER;

	return st;
}

void DirectShowTools::OpenOutputVideo()
{
	AVCodec *codec;
	AVCodecContext *c;

	c = m_OutputVideoStream->codec;

	/* find the video encoder */
	codec = avcodec_find_encoder(c->codec_id);
	if (!codec) {
		fprintf(stderr, "codec not found\n");
	}

	AVDictionary *param = 0;
	av_dict_set(&param, "preset", "superfast", 0);
	av_dict_set(&param, "tune", "zerolatency", 0);

	/* open the codec */
	if (avcodec_open2(c, codec, &param) < 0) {
		fprintf(stderr, "could not open codec\n");
	}
}

void DirectShowTools::WriteVideoFrame(int &pts_num)
{
	if (m_OutputFormatCtx->oformat->flags & AVFMT_RAWPICTURE) {
		AVPacket pkt;
		av_init_packet(&pkt);

		pkt.flags |= AV_PKT_FLAG_KEY;
		pkt.stream_index = m_OutputVideoStream->index;
		pkt.data = (uint8_t *)m_OutputFrame;
		pkt.size = sizeof(AVPicture);

		av_interleaved_write_frame(m_OutputFormatCtx, &pkt);
	}
	else {
		int out_size = 0;
		m_OutputFrame->pts = pts_num;
		out_size = avcodec_encode_video(m_OutputVideoStream->codec, 
			m_VideoOutBuff,m_VideoOutBuffSize, m_OutputFrame);

		if (out_size > 0) {
			AVPacket pkt;
			av_init_packet(&pkt);

			if (m_OutputVideoStream->codec->coded_frame->pts != AV_NOPTS_VALUE) {
				pkt.pts = av_rescale_q(m_OutputVideoStream->codec->coded_frame->pts, 
					m_OutputVideoStream->codec->time_base, m_OutputVideoStream->time_base);
			}
			if (m_OutputVideoStream->codec->coded_frame->key_frame) {
				pkt.flags |= AV_PKT_FLAG_KEY;
			}
			pkt.stream_index = m_OutputVideoStream->index;
			pkt.data = m_VideoOutBuff;
			pkt.size = out_size;
			av_interleaved_write_frame(m_OutputFormatCtx, &pkt);
		}
	}
}

void DirectShowTools::AddWaterMask(cv::Mat& img)
{
	if (m_MaskFlag <= DST_WATERMASK_NONE) {
		return;
	}

	float tmp_tpy = m_Transparency / 100.0;

	switch (m_MaskFlag)
	{
	case DST_WATERMASK_RGB:
		cv::add(tmp_tpy*img(cv::Rect(10, 10, m_WaterMaskImg.cols, m_WaterMaskImg.rows)),
			(1 - tmp_tpy)*m_WaterMaskImg, 
			img(cv::Rect(10, 10, m_WaterMaskImg.cols, m_WaterMaskImg.rows)),
			m_MaskImg, -1);
		break;
	case DST_WATERMASK_GIF:
	{
		if (m_WaterMaskGifImg.size() == 0) {
			break;
		}
		if (m_ShowIndex >= m_WaterMaskGifImg.size()) {
			m_ShowIndex = 0;
		}
		int i = m_ShowIndex;
		cv::cvtColor(m_WaterMaskGifImg[i], m_MaskImg, CV_BGR2GRAY);

		cv::add(tmp_tpy*img(cv::Rect(10, 10, m_WaterMaskGifImg[i].cols, m_WaterMaskGifImg[i].rows)),
			(1 - tmp_tpy)*m_WaterMaskGifImg[i],
			img(cv::Rect(10, 10, m_WaterMaskGifImg[i].cols, m_WaterMaskGifImg[i].rows)),
			m_MaskImg, -1);

		if (m_Internal >= 4) {
			m_ShowIndex++;
			m_Internal = 0;
		}
		m_Internal++;
		break;
	}
	default:
		break;
	}
}

void DirectShowTools::GrabImage(cv::Mat& img)
{
	if (m_GrabImgFlag == DST_GRAB_PROCESSING) {
		m_GrabImgFlag = DST_GRAB_SAVING;
		m_GrabImgMat = img.clone();
		emit SendImageGrabMsg();
	}
}

void DirectShowTools::SaveGrabImage()
{
	QDateTime time = QDateTime::currentDateTime();
	QString str = time.toString("yyyyMMddhhmmsszzz") + ".jpg";
	cv::imwrite(str.toStdString(), m_GrabImgMat);
	m_GrabImgMat.release();
	m_GrabImgFlag = DST_GRAB_READY;
}

void DirectShowTools::PreviewImage(cv::Mat& img)
{
	cv::cvtColor(img, img, CV_BGR2RGB);			// opencvĬ����BGR����ת��QT����ʾ��ҪRGB����
	cv::Mat temp_img = img.clone();
	QImage Qimg = QImage((const unsigned char*)(img.data), img.cols,
		img.rows, img.cols*img.channels(), QImage::Format_RGB888);
	emit SendImageData(Qimg);
}

void DirectShowTools::SaveVideo(cv::Mat& img)
{
	if (m_SaveVideoFlag == DST_SAVEVIDEO_NONE) {
		return ;
	}
	else if (m_SaveVideoFlag == DST_SAVEVIDEO_INIT) {
		QDateTime time = QDateTime::currentDateTime();
		QString str = time.toString("yyyyMMddhhmmsszzz") + ".mp4";
		MP4OutputConfig(m_SaveVideoPath + "/" + str.toStdString());
		m_PtsNum = 0;
		m_SaveVideoFlag = DST_SAVEVIDEO_SAVING;
	}
	else if (m_SaveVideoFlag == DST_SAVEVIDEO_SAVING) {
		SwsContext* pSwsCxt = sws_getContext(m_InputCodecCtx->width,
			m_InputCodecCtx->height, PIX_FMT_RGB24,m_InputCodecCtx->width, 
			m_InputCodecCtx->height, PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);
		uint8_t *rgb_src[3] = { img.data, NULL, NULL };
		int rgb_stride[3] = { 3 * m_InputCodecCtx->width, 0, 0 };

		sws_scale(pSwsCxt, rgb_src, rgb_stride,0, m_InputCodecCtx->height, 
			((AVPicture *)m_OutputFrame)->data, ((AVPicture *)m_OutputFrame)->linesize);

		WriteVideoFrame(m_PtsNum);
		m_PtsNum++;
		sws_freeContext(pSwsCxt);
	}
	else {
		m_SaveVideoFlag = DST_SAVEVIDEO_NONE;
		FlushEncoder(0);
		av_write_trailer(m_OutputFormatCtx);
		DestoryVideoParam();
	}
	
}

int DirectShowTools::GetImageData(int& video_stream, AVPacket& packet, SwsContext* &img_convert_ctx)
{
	if (packet.stream_index != video_stream) {
		return -1;
	}
	int frameFinished = 0;
	avcodec_decode_video2(m_InputCodecCtx, m_InputFrame, &frameFinished, &packet);
	if (!frameFinished) {
		return -1;
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
	return 0;
}

void DirectShowTools::run()
{
//	CameraConfig cc;
//	std::vector<CameraDeviceInfo> m_CameraList = cc.ListCameraDevice();

//	std::string camera_name = "video=" + m_CameraList[0].friend_name;
	int video_stream = 1;
	CameraInputInit(m_CameraName, video_stream);

	AVPacket packet;
	while (av_read_frame(m_InputFormatCtx, &packet) >= 0)
	{
		SwsContext * img_convert_ctx;
		int ret = GetImageData(video_stream, packet, img_convert_ctx);	// �õ�ͼ������	

		if (ret < 0) {
			continue;
		}
		cv::Mat img(m_InputFrame->height, m_InputFrame->width, 
			CV_8UC3, m_InputFrameRGB->data[0]);
		AddWaterMask(img);		// ���ˮӡ
		GrabImage(img);			// ץ��ͼ��
		PreviewImage(img);		// Ԥ��ͼ��
		SaveVideo(img);			// ������Ƶ

		cvWaitKey(1);
		sws_freeContext(img_convert_ctx);
		av_free_packet(&packet);

		if (m_ThreadStop) {
			m_ThreadStop = false;
			break;
		}
	}
	DestoryInputParam();
}

void DirectShowTools::ThreadStopFunc()
{
	m_ThreadStop = true;
}