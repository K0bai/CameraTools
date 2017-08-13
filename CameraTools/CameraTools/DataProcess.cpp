#include "DataProcess.h"
#include <qdatetime.h>
#include "DirectShowTools.h"
#include "MediaFoundationTools.h"

DataProcess::DataProcess()
{
	m_PictureBuff = NULL;
	m_VideoOutBuff = NULL;
	m_WaterMaskFlag = DP_WATERMASK_NONE;
	b_ThreadStop = false;
	m_ShowIndex = 0;
	m_Internal = 0;
	m_Transparency = 0;
	m_GrabImgFlag = DP_GRAB_READY;
	m_SaveVideoFlag = DP_SAVEVIDEO_NONE;
	m_ImageStyle = DP_IMAGESTYLE_NORMAL;
	m_NoiseOptimizationFlag = DP_NOISEOPTIMIZATION_NO;
	InitializeCriticalSection(&m_PreviewLock);
	InitializeCriticalSection(&m_SaveBufferLock);
	connect(this, SIGNAL(SendImageGrabMsg()), this,
		SLOT(SaveGrabImage()), Qt::QueuedConnection);
}


DataProcess::~DataProcess()
{
	DeleteCriticalSection(&m_PreviewLock);
	DeleteCriticalSection(&m_SaveBufferLock);
	//	DestoryInputParam();
	//	DestoryVideoParam();
	FreeImgBuffer();
}


void DataProcess::SetWaterMaskFlag(int flag)
{
	m_WaterMaskFlag = flag;
}

void DataProcess::SetTransparency(int value)
{
	m_Transparency = value;
}

void DataProcess::SetWaterMaskImg(cv::Mat img)
{
	m_WaterMaskImg = img;
}

void DataProcess::SetMaskImg(cv::Mat img)
{
	m_MaskImg = img;
}

void DataProcess::SetWaterMaskGifImg(std::vector<cv::Mat> img)
{
	m_WaterMaskGifImg = img;
}
void DataProcess::SetMaskGifImg(std::vector<cv::Mat> img)
{
	m_MaskGifImg = img;
}

void DataProcess::SetInternal(int num)
{
	m_Internal = num;
}

int DataProcess::GetInternal()
{
	return m_Internal;
}

void DataProcess::SetGrabImgFlag(int flag)
{
	m_GrabImgFlag = flag;
}

int DataProcess::GetGrabImgFlag()
{
	return m_GrabImgFlag;
}

void DataProcess::SetSaveVideoFlag(int flag)
{
	m_SaveVideoFlag = flag;
}
void DataProcess::SetNoiseOptimizationFlag(int flag)
{
	m_NoiseOptimizationFlag = flag;
}
int DataProcess::GetSaveVideoFlag()
{
	return m_SaveVideoFlag;
}

void DataProcess::SetCameraInfo(PreviewCameraInfo cInfo)
{
	m_CameraInfo = cInfo;
}
PreviewCameraInfo DataProcess::GetCameraInfo()
{
	return m_CameraInfo;
}

void DataProcess::SetSaveVideoPath(std::string str)
{
	m_SaveVideoPath = str;
}
std::string DataProcess::GetSaveVideoPath()
{
	return m_SaveVideoPath;
}

AVFormatContext * DataProcess::GetOutputFormatCtx()
{
	return m_OutputFormatCtx;
}

AVFrame * DataProcess::GetOutputFrame()
{
	return m_OutputFrame;
}

void DataProcess::SetThreadStop(bool flag)
{
	b_ThreadStop = flag;
}
bool DataProcess::GetThreadStop()
{
	return b_ThreadStop;
}
void DataProcess::SetImageStyle(int flag)
{
	m_ImageStyle = flag;
}

void DataProcess::SetCameraToolsInterface(CameraDetectToolsInterface * pCDTI)
{
	m_pCDTI = pCDTI;
}

int DataProcess::GetImageStyle()
{
	return m_ImageStyle;
}

void DataProcess::DestoryVideoParam()
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

void DataProcess::FreeImgBuffer()
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
	if (!m_ShowImageBuf.empty()) {
		m_ShowImageBuf.empty();
	}
}

void DataProcess::GetResolutionFromString(const std::string & res, int & width, int & height)
{
	int index = res.find('x');
	if (index == std::string::npos) {
		return;
	}
	width = std::atoi(res.substr(0, index).c_str());
	height = std::atoi(res.substr(index + 1).c_str());
}

// 录像时的初始化配置函数
// 参数1 录像文件存储名
// 返回值小于0，则初始化失败
// 返回值大于等于0，则初始化成功
int DataProcess::MP4OutputConfig(const PreviewCameraInfo& cInfo)
{
	avdevice_register_all();
	avcodec_register_all();
	av_register_all();
	QDateTime time = QDateTime::currentDateTime();
	QString str = time.toString("yyyyMMddhhmmsszzz") + ".mp4";
	std::string output_name = m_SaveVideoPath + "/" + str.toStdString();

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
		m_OutputVideoStream = AddVideoStream(m_OutputFormat->video_codec,cInfo);
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
AVStream* DataProcess::AddVideoStream(AVCodecID codec_id, const PreviewCameraInfo& cInfo)
//	const int& width, const int& height, const int& fps)
{
	int width = 0, height = 0, fps = 0;
	GetResolutionFromString(cInfo.resolution, width, height);
	fps = atoi(cInfo.fps.c_str());
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
	c->width = /*352*/width;
	c->height = /*288*/height;
	/* time base: this is the fundamental unit of time (in seconds) in terms
	of which frame timestamps are represented. for fixed-fps content,
	timebase should be 1/framerate and timestamp increments should be
	identically 1. */
	c->time_base.den = fps;
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
void DataProcess::OpenOutputVideo()
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
void DataProcess::WriteVideoFrame(const int &pts_num)
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
			m_VideoOutBuff, m_VideoOutBuffSize, m_OutputFrame);

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

int DataProcess::FlushEncoder(const int& stream_index)
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
void DataProcess::AddWaterMask(cv::Mat& img)
{
	if (m_WaterMaskFlag <= DP_WATERMASK_NONE) {
		return;
	}

	float tmp_tpy = m_Transparency / 100.0;

	switch (m_WaterMaskFlag)
	{
	case DP_WATERMASK_RGB:
		cv::add(tmp_tpy*img(cv::Rect(10, 10,
			m_WaterMaskImg.cols, m_WaterMaskImg.rows)),
			(1 - tmp_tpy)*m_WaterMaskImg,
			img(cv::Rect(10, 10, m_WaterMaskImg.cols, m_WaterMaskImg.rows)),
			m_MaskImg, -1);
		break;
	case DP_WATERMASK_GIF:
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
void DataProcess::GrabImage(cv::Mat& img)
{
	if (m_GrabImgFlag == DP_GRAB_PROCESSING) {
		m_GrabImgFlag = DP_GRAB_SAVING;
		m_GrabImgMat = img.clone();
		emit SendImageGrabMsg();
	}
}

// 存储抓拍图像
void DataProcess::SaveGrabImage()
{
	QDateTime time = QDateTime::currentDateTime();
	QString str = time.toString("yyyyMMddhhmmsszzz") + ".jpg";
	cv::imwrite(str.toStdString(), m_GrabImgMat);
	m_GrabImgMat.release();
	m_GrabImgFlag = DP_GRAB_READY;
}


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
}


// 预览功能图像处理函数
// 参数1 输入解码后的图像数据
// 根据当前显示风格进行不同的处理（目前还不支持油画）
// 处理完成将图像数据放入缓冲队列
void DataProcess::PreviewImage(cv::Mat& img)
{
	QImage Qimg;
	switch (m_ImageStyle)
	{
	case DP_IMAGESTYLE_NORMAL:
	{
		cv::cvtColor(img, img, CV_BGR2RGB);			// opencv默认是BGR排序，转到QT中显示需要RGB排序
		break;
	}
	case DP_IMAGESTYLE_OIL:
	{
		OilPaint(img, 2, 177);
		cv::cvtColor(img, img, CV_BGR2RGB);
		break;
	}
	case DP_IMAGESTYLE_GRAY:
	{
		cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
		break;
	}
	default:
		break;
	}

	EnterCriticalSection(&m_PreviewLock);
	while (m_ShowImageBuf.size() > 30) {
		m_ShowImageBuf.pop();
	}
	m_ShowImageBuf.push(img);
	LeaveCriticalSection(&m_PreviewLock);

	emit SendUpdataImgMsg();
}

void DataProcess::OptimizeImage(cv::Mat & img)
{
	if (m_NoiseOptimizationFlag) {
		cv::medianBlur(img, img, 3);
	}
}

// 得到待显示的摄像头图像数据
// 参数1 需要显示的QImage对象
// 从缓冲队列头部拿出图像数据
void DataProcess::GetShowImageBuffer(cv::Mat& img)
{
	EnterCriticalSection(&m_PreviewLock);
	if (!m_ShowImageBuf.empty()) {
		img = m_ShowImageBuf.front();
		m_ShowImageBuf.pop();
	}
	LeaveCriticalSection(&m_PreviewLock);
}

void DataProcess::GetSaveImageBuffer(cv::Mat& img)
{
	EnterCriticalSection(&m_SaveBufferLock);
	if (!m_SaveImageBuf.empty()) {
		img = m_SaveImageBuf.front();
		m_SaveImageBuf.pop();
	}
	LeaveCriticalSection(&m_SaveBufferLock);
}

DWORD WINAPI SaveVideoThread(LPVOID pm)
{
	DataProcess *pDP = static_cast<DataProcess*>(pm);
	int pts_num = 0;
	while (1)
	{
		if (pDP->GetSaveVideoFlag() == DP_SAVEVIDEO_SAVING) {
			cv::Mat img;
			pDP->GetSaveImageBuffer(img);
			if (img.empty()) {
				Sleep(500);
				continue;
			}
			uint8_t *rgb_src[3] = { img.data, NULL, NULL };
			SwsContext* pSwsCxt;
			if (img.channels() == 1/*pDP->GetImageStyle() == DP_IMAGESTYLE_GRAY*/) {
				pSwsCxt = sws_getContext(img.cols,
					img.rows, PIX_FMT_GRAY8, img.cols,
					img.rows, PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);
				int rgb_stride[3] = { img.cols, 0, 0 };
				sws_scale(pSwsCxt, rgb_src, rgb_stride, 0, img.rows,
					((AVPicture *)pDP->GetOutputFrame())->data, ((AVPicture *)pDP->GetOutputFrame())->linesize);
			}
			else {
				pSwsCxt = sws_getContext(img.cols,
					img.rows, PIX_FMT_RGB24, img.cols,
					img.rows, PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);
				int rgb_stride[3] = { 3 * img.cols, 0, 0 };
				sws_scale(pSwsCxt, rgb_src, rgb_stride, 0, img.rows,
					((AVPicture *)pDP->GetOutputFrame())->data, ((AVPicture *)pDP->GetOutputFrame())->linesize);
			}
			pDP->WriteVideoFrame(pts_num);
			pts_num++;
			sws_freeContext(pSwsCxt);
			img.release();
		}
		else if (pDP->GetSaveVideoFlag() == DP_SAVEVIDEO_END) {
			pDP->SetSaveVideoFlag(DP_SAVEVIDEO_NONE);
			//		FlushEncoder(0);
			av_write_trailer(pDP->GetOutputFormatCtx());
			pDP->DestoryVideoParam();
			break;
		}
		Sleep(1);
	}
	return 0;
}

// 录像功能主函数
// 参数1 待编码存储的图像数据
// 根据当前录像状态参数进行初始化，编码存储，
// 直接返回或者将数据写入文件尾，
void DataProcess::SaveVideo(cv::Mat& img, const PreviewCameraInfo& cInfo)
{
	if (m_SaveVideoFlag == DP_SAVEVIDEO_NONE) {
		return;
	}
	else if (m_SaveVideoFlag == DP_SAVEVIDEO_INIT) {
		if (MP4OutputConfig(cInfo) != DP_SAVEVIDEO_INIT_OK) {
			m_SaveVideoFlag = DP_SAVEVIDEO_NONE;
			return;
		}
		CreateThread(NULL, 0, SaveVideoThread, this, 0, NULL);
		m_PtsNum = 0;
		m_SaveVideoFlag = DP_SAVEVIDEO_SAVING;
	}
	else if (m_SaveVideoFlag == DP_SAVEVIDEO_SAVING) {
		EnterCriticalSection(&m_SaveBufferLock);
		m_SaveImgMat = img.clone();
		m_SaveImageBuf.push(m_SaveImgMat);
		LeaveCriticalSection(&m_SaveBufferLock);
	}
}


// 主要线程运行函数
// while循环一直不断的从摄像头读取数据包
// 在即将退出该函数时向主UI线程发送结束信号
void DataProcess::run()
{
	m_pCDTI->CameraInit(m_CameraInfo);
	int ret = 0;
	while (1) {
		if (b_ThreadStop) {
			b_ThreadStop = false;
			break;
		}
		cv::Mat img = m_pCDTI->GetFrameData();
		if (img.empty()) {
//			Sleep(5);
			continue;
		}
		OptimizeImage(img);
		AddWaterMask(img);		// 添加水印
		GrabImage(img);			// 抓拍图像
		PreviewImage(img);		// 预览图像

		SaveVideo(img, m_CameraInfo);			// 保存视频
		img.release();
//		Sleep(5);
		cvWaitKey(1);
	}
	m_pCDTI->DestoryInputParam();
	SendAbort(ret);
}