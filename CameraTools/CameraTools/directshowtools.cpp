#include "directshowtools.h"
#include <qdatetime.h>

DirectShowTools::DirectShowTools()
{
	m_PictureBuff = NULL;
	m_VideoOutBuff = NULL;
	m_WaterMaskFlag = DST_WATERMASK_NONE;
	b_ThreadStop = false;
	m_ShowIndex = 0;
	m_Internal = 0;
	m_Transparency = 0;
	m_GrabImgFlag = DST_GRAB_READY;
	m_SaveVideoFlag = DST_SAVEVIDEO_NONE;
	m_ImageStyle = DST_IMAGESTYLE_NORMAL;
	InitializeCriticalSection(&m_PreviewLock);
	connect(this, SIGNAL(SendImageGrabMsg()), this,
		SLOT(SaveGrabImage()), Qt::QueuedConnection);
}

DirectShowTools::~DirectShowTools()
{
	DeleteCriticalSection(&m_PreviewLock);
//	DestoryInputParam();
//	DestoryVideoParam();
	FreeImgBuffer();
}

void DirectShowTools::SetWaterMaskFlag(int flag)
{
	m_WaterMaskFlag = flag;
}

void DirectShowTools::SetTransparency(int value)
{
	m_Transparency = value;
}

void DirectShowTools::SetWaterMaskImg(cv::Mat img)
{
	m_WaterMaskImg = img;
}

void DirectShowTools::SetMaskImg(cv::Mat img)
{
	m_MaskImg = img;
}

void DirectShowTools::SetWaterMaskGifImg(std::vector<cv::Mat> img)
{
	m_WaterMaskGifImg = img;
}
void DirectShowTools::SetMaskGifImg(std::vector<cv::Mat> img)
{
	m_MaskGifImg = img;
}

void DirectShowTools::SetInternal(int num)
{
	m_Internal = num;
}

int DirectShowTools::GetInternal()
{
	return m_Internal;
}

void DirectShowTools::SetGrabImgFlag(int flag)
{
	m_GrabImgFlag = flag;
}

int DirectShowTools::GetGrabImgFlag()
{
	return m_GrabImgFlag;
}

void DirectShowTools::SetSaveVideoFlag(int flag)
{
	m_SaveVideoFlag = flag;
}
int DirectShowTools::GetSaveVideoFlag()
{
	return m_SaveVideoFlag;
}

void DirectShowTools::SetCameraInfo(PreviewCameraInfo cInfo)
{
	m_CameraInfo = cInfo;
}
PreviewCameraInfo DirectShowTools::GetCameraInfo()
{
	return m_CameraInfo;
}

void DirectShowTools::SetSaveVideoPath(std::string str)
{
	m_SaveVideoPath = str;
}
std::string DirectShowTools::GetSaveVideoPath()
{
	return m_SaveVideoPath;
}

void DirectShowTools::SetThreadStop(bool flag)
{
	b_ThreadStop = flag;
}
bool DirectShowTools::GetThreadStop()
{
	return b_ThreadStop;
}
void DirectShowTools::SetImageStyle(int flag)
{
	m_ImageStyle = flag;
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
		if (m_OutputVideoStream) {
			av_free(m_PictureBuff);
			av_free(m_VideoOutBuff);
		}
	}
}

void DirectShowTools::FreeImgBuffer()
{
	if (!m_WaterMaskImg.empty()) {
		m_WaterMaskImg.release();
	}
	if (!m_MaskImg.empty()) {
		m_MaskImg.release();
	}
	if (!m_GrabImgMat.empty()) {
		m_GrabImgMat.release();
	}
	if (!m_WaterMaskGifImg.empty()) {
		m_WaterMaskGifImg.clear();
	}
	if (!m_MaskGifImg.empty()) {
		m_MaskGifImg.empty();
	}
	if (!m_ImageBuf.empty()) {
		m_ImageBuf.empty();
	}
}

// 摄像头初始化函数
// 参数1 输入的需要设置的摄像头参数
// 参数2 摄像头输入数据中视频数据的索引
// 返回值小于0，则初始化不成功
// 返回值大于等于0，初始化成功
int DirectShowTools::CameraInputInit(PreviewCameraInfo cInfo, int &video_stream)
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

// 录像时的初始化配置函数
// 参数1 录像文件存储名
// 返回值小于0，则初始化失败
// 返回值大于等于0，则初始化成功
int DirectShowTools::MP4OutputConfig(const std::string& output_name)
{
	avformat_alloc_output_context2(&m_OutputFormatCtx, NULL,
		NULL, output_name.c_str());
	if (!m_OutputFormatCtx) {
		avformat_alloc_output_context2(&m_OutputFormatCtx, 
			NULL, "mpeg", output_name.c_str());
	}
	if (!m_OutputFormatCtx) {
		return -1;
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
			return -1;
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

// 修改自ffmpeg官方文档muxing.c
// 输入参数即视频流编码方式
// 返回配置好的AVStream*
// 配置参数包括图像大小，帧率等常用视频编码参数
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
	c->width = /*352*/m_InputFrame->width;
	c->height = /*288*/m_InputFrame->height;
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

// 修改自ffmpeg官方文档muxing.c
// 打开所配置的视频编码AVCodecContext
void DirectShowTools::OpenOutputVideo()
{
	AVCodec *codec;
	AVCodecContext *c;

	c = m_OutputVideoStream->codec;

	/* find the video encoder */
	codec = avcodec_find_encoder(c->codec_id);
	if (!codec) {
		return;
	}
	AVDictionary *param = 0;
	av_dict_set(&param, "preset", "superfast", 0);
	av_dict_set(&param, "tune", "zerolatency", 0);

	/* open the codec */
	if (avcodec_open2(c, codec, &param) < 0) {
		return;
	}
}

// 修改自ffmpeg官方文档muxing.c
// 参数1 输入当前帧的显示时间戳
// 可实现将当前帧进行编码并写入到文件中
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

		if (out_size <= 0) {
			return;
		}
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
		ret = avcodec_encode_video2(
			m_OutputFormatCtx->streams[stream_index]->codec,
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
	av_free_packet(&enc_pkt);
	return ret;
}

// 添加水印功能函数
// 参数1 输入当前摄像头获取的图像数据
// 可实现将水印与当前帧图像合并
void DirectShowTools::AddWaterMask(cv::Mat& img)
{
	if (m_WaterMaskFlag <= DST_WATERMASK_NONE) {
		return;
	}

	float tmp_tpy = m_Transparency / 100.0;

	switch (m_WaterMaskFlag)
	{
	case DST_WATERMASK_RGB:
		cv::add(tmp_tpy*img(cv::Rect(10, 10, 
			m_WaterMaskImg.cols, m_WaterMaskImg.rows)),
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
		cv::add(tmp_tpy*img(cv::Rect(10, 10, m_WaterMaskGifImg[i].cols, m_WaterMaskGifImg[i].rows)),
			(1 - tmp_tpy)*m_WaterMaskGifImg[i],
			img(cv::Rect(10, 10, m_WaterMaskGifImg[i].cols, m_WaterMaskGifImg[i].rows)),
			m_MaskGifImg[i], -1);

		if (m_Internal >= 4 /*该值可以之后设置*/) {
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

// 抓拍图像
// 参数1 输入当前显示帧图像
// 将当前帧图像copy，并发送信号进行存储
void DirectShowTools::GrabImage(cv::Mat& img)
{
	if (m_GrabImgFlag == DST_GRAB_PROCESSING) {
		m_GrabImgFlag = DST_GRAB_SAVING;
		m_GrabImgMat = img.clone();
		emit SendImageGrabMsg();
	}
}

// 存储抓拍图像
void DirectShowTools::SaveGrabImage()
{
	QDateTime time = QDateTime::currentDateTime();
	QString str = time.toString("yyyyMMddhhmmsszzz") + ".jpg";
	cv::imwrite(str.toStdString(), m_GrabImgMat);
	m_GrabImgMat.release();
	m_GrabImgFlag = DST_GRAB_READY;
}

/* 油画处理部分，还需修改
void OilPaint(cv::Mat& I, int brushSize, int coarseness)
{
	assert(!I.empty());
	if (brushSize < 1) brushSize = 1;
	if (brushSize > 8) brushSize = 8;

	if (coarseness < 1) coarseness = 1;
	if (coarseness > 255) coarseness = 255;

	int width = I.cols;
	int height = I.rows;

	int lenArray = coarseness + 1;
	int* CountIntensity = new int[lenArray];
	uint* RedAverage = new uint[lenArray];
	uint* GreenAverage = new uint[lenArray];
	uint* BlueAverage = new uint[lenArray];

	/// 图像灰度化
	cv::Mat gray;
	cvtColor(I, gray, cv::COLOR_BGR2GRAY);


	/// 目标图像
	cv::Mat dst = cv::Mat::zeros(I.size(), I.type());

	for (int nY = 0; nY <height; nY++)
	{
		// 油画渲染范围上下边界
		int top = nY - brushSize;
		int bottom = nY + brushSize + 1;

		if (top<0) top = 0;
		if (bottom >= height) bottom = height - 1;

		for (int nX = 0; nX<width; nX++)
		{
			// 油画渲染范围左右边界
			int left = nX - brushSize;
			int right = nX + brushSize + 1;

			if (left<0) left = 0;
			if (right >= width) right = width - 1;

			//初始化数组
			for (int i = 0; i <lenArray; i++)
			{
				CountIntensity[i] = 0;
				RedAverage[i] = 0;
				GreenAverage[i] = 0;
				BlueAverage[i] = 0;
			}


			// 下面这个内循环类似于外面的大循环
			// 也是油画特效处理的关键部分
			for (int j = top; j<bottom; j++)
			{
				for (int i = left; i<right; i++)
				{
					uchar intensity = static_cast<uchar>(coarseness*gray.at<uchar>(j, i) / 255.0);
					CountIntensity[intensity]++;

					RedAverage[intensity] += I.at<cv::Vec3b>(j, i)[2];
					GreenAverage[intensity] += I.at<cv::Vec3b>(j, i)[1];
					BlueAverage[intensity] += I.at<cv::Vec3b>(j, i)[0];
				}
			}

			// 求最大值，并记录下数组索引
			uchar chosenIntensity = 0;
			int maxInstance = CountIntensity[0];
			for (int i = 1; i<lenArray; i++)
			{
				if (CountIntensity[i]>maxInstance)
				{
					chosenIntensity = (uchar)i;
					maxInstance = CountIntensity[i];
				}
			}

			dst.at<cv::Vec3b>(nY, nX)[2] = static_cast<uchar>(RedAverage[chosenIntensity] / static_cast<float>(maxInstance));
			dst.at<cv::Vec3b>(nY, nX)[1] = static_cast<uchar>(GreenAverage[chosenIntensity] / static_cast<float>(maxInstance));
			dst.at<cv::Vec3b>(nY, nX)[0] = static_cast<uchar>(BlueAverage[chosenIntensity] / static_cast<float>(maxInstance));
		}

	}

	delete[] CountIntensity;
	delete[] RedAverage;
	delete[] GreenAverage;
	delete[] BlueAverage;
	dst.copyTo(I);
//	I = dst.clone();
}
*/

// 预览功能图像处理函数
// 参数1 输入解码后的图像数据
// 根据当前显示风格进行不同的处理（目前还不支持油画）
// 处理完成将图像数据放入缓冲队列
void DirectShowTools::PreviewImage(cv::Mat& img)
{
	QImage Qimg;
	switch (m_ImageStyle)
	{
	case DST_IMAGESTYLE_NORMAL:
	{
		cv::cvtColor(img, img, CV_BGR2RGB);			// opencv默认是BGR排序，转到QT中显示需要RGB排序
		Qimg = QImage((const unsigned char*)(img.data), img.cols,
			img.rows, img.cols*img.channels(), QImage::Format_RGB888);
		break;
	}
	case DST_IMAGESTYLE_OIL:
	{
//		OilPaint(img, 3, 50);
		cv::cvtColor(img, img, CV_BGR2RGB);	 
		Qimg = QImage((const unsigned char*)(img.data), img.cols,
			img.rows, img.cols*img.channels(), QImage::Format_RGB888);
		break;
	}
	case DST_IMAGESTYLE_GRAY:
	{
		cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
		Qimg = QImage(img.cols, img.rows, QImage::Format_Indexed8);
		Qimg.setColorCount(256);
		for (int i = 0; i < 256; i++)
		{
			Qimg.setColor(i, qRgb(i, i, i));
		}
		uchar *pSrc = img.data;
		for (int row = 0; row < img.rows; row++)
		{
			uchar *pDest = Qimg.scanLine(row);
			memcpy(pDest, pSrc, img.cols);
			pSrc += img.step;
		}
		break;
	}
	default:
		break;
	}

	EnterCriticalSection(&m_PreviewLock);
	while (m_ImageBuf.size() > 30) {
		m_ImageBuf.pop();
	}
	m_ImageBuf.push(Qimg);
	LeaveCriticalSection(&m_PreviewLock);
	emit SendUpdataImgMsg();
}

// 得到待显示的摄像头图像数据
// 参数1 需要显示的QImage对象
// 从缓冲队列头部拿出图像数据
void DirectShowTools::GetImageBuffer(QImage& Qimg)
{
	EnterCriticalSection(&m_PreviewLock);
	Qimg = m_ImageBuf.front();
	m_ImageBuf.pop();
	LeaveCriticalSection(&m_PreviewLock);
}

// 录像功能主函数
// 参数1 待编码存储的图像数据
// 根据当前录像状态参数进行初始化，编码存储，
// 直接返回或者将数据写入文件尾，
void DirectShowTools::SaveVideo(cv::Mat& img)
{
	if (m_SaveVideoFlag == DST_SAVEVIDEO_NONE) {
		return ;
	}
	else if (m_SaveVideoFlag == DST_SAVEVIDEO_INIT) {
		QDateTime time = QDateTime::currentDateTime();
		QString str = time.toString("yyyyMMddhhmmsszzz") + ".mp4";
		if (MP4OutputConfig(m_SaveVideoPath + "/" + str.toStdString()) 
			!= DST_SAVEVIDEO_INIT_OK) {
			m_SaveVideoFlag = DST_SAVEVIDEO_NONE;
			return;
		}
		m_PtsNum = 0;
		m_SaveVideoFlag = DST_SAVEVIDEO_SAVING;
	}
	else if (m_SaveVideoFlag == DST_SAVEVIDEO_SAVING) {
		uint8_t *rgb_src[3] = { img.data, NULL, NULL };
		SwsContext* pSwsCxt;
		if (m_ImageStyle == DST_IMAGESTYLE_GRAY) {
			pSwsCxt = sws_getContext(m_InputCodecCtx->width,
				m_InputCodecCtx->height, PIX_FMT_GRAY8, m_InputCodecCtx->width,
				m_InputCodecCtx->height, PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);
			int rgb_stride[3] = { m_InputCodecCtx->width, 0, 0 };
			sws_scale(pSwsCxt, rgb_src, rgb_stride, 0, m_InputCodecCtx->height,
				((AVPicture *)m_OutputFrame)->data, ((AVPicture *)m_OutputFrame)->linesize);
		}
		else {
			pSwsCxt = sws_getContext(m_InputCodecCtx->width,
				m_InputCodecCtx->height, PIX_FMT_RGB24, m_InputCodecCtx->width,
				m_InputCodecCtx->height, PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);
			int rgb_stride[3] = { 3 * m_InputCodecCtx->width, 0, 0 };
			sws_scale(pSwsCxt, rgb_src, rgb_stride, 0, m_InputCodecCtx->height,
				((AVPicture *)m_OutputFrame)->data, ((AVPicture *)m_OutputFrame)->linesize);
		}
		WriteVideoFrame(m_PtsNum);
		m_PtsNum++;
		sws_freeContext(pSwsCxt);
	}
	else {
		m_SaveVideoFlag = DST_SAVEVIDEO_NONE;
//		FlushEncoder(0);
		av_write_trailer(m_OutputFormatCtx);
		DestoryVideoParam();
	}
	
}

// 主要线程运行函数
// while循环一直不断的从摄像头读取数据包
// 在即将退出该函数时向主UI线程发送结束信号
void DirectShowTools::run()
{
	int video_stream = 1;
	if (CameraInputInit(m_CameraInfo, video_stream) != DST_CAMERA_INIT_OK) {
		return ;
	}
	AVPacket packet;
	int ret = 0;
	while ((ret = av_read_frame(m_InputFormatCtx, &packet)) >= 0) {
		if (b_ThreadStop) {
			b_ThreadStop = false;
			break;
		}
		SwsContext * img_convert_ctx;
		ret = GetImageData(video_stream, packet, img_convert_ctx);	// 解码图像数据	
		if (ret < 0) {
			continue;
		}
		cv::Mat img(m_InputFrame->height, m_InputFrame->width, 
			CV_8UC3, m_InputFrameRGB->data[0]);

		AddWaterMask(img);		// 添加水印
		GrabImage(img);			// 抓拍图像
		PreviewImage(img);		// 预览图像
		SaveVideo(img);			// 保存视频

		cvWaitKey(1);
		sws_freeContext(img_convert_ctx);
		av_free_packet(&packet);
	}
	SendAbort(ret);
	DestoryInputParam();
}