#include "DataProcess.h"
#include <qdatetime.h>
#include "DirectShowTools.h"
#include "MediaFoundationTools.h"

DataProcess::DataProcess()
{
	m_pictureBuff = NULL;
	m_videoOutBuff = NULL;
	m_waterMaskFlag = DP_WATERMASK_NONE;
	b_threadStop = false;
	m_showIndex = 0;
	m_internal = 0;
	m_transparency = 0;
	m_grabImgFlag = DP_GRAB_READY;
	m_saveVideoFlag = DP_SAVEVIDEO_NONE;
	m_imageStyle = DP_IMAGESTYLE_NORMAL;
	m_noiseOptimizationFlag = DP_NOISEOPTIMIZATION_NO;
	m_saveVideoPath = "";
	m_saveGrabImgPath = "";

	InitializeCriticalSection(&m_previewLock);
	InitializeCriticalSection(&m_saveBufferLock);

	connect(this, SIGNAL(SendImageGrabMsg()), this,
		SLOT(SaveGrabImage()), Qt::QueuedConnection);
}


DataProcess::~DataProcess()
{
	DeleteCriticalSection(&m_previewLock);
	DeleteCriticalSection(&m_saveBufferLock);
	//	DestoryInputParam();
	//	DestoryVideoParam();
	FreeImgBuffer();
}


void DataProcess::SetWaterMaskFlag(int flag)
{
	m_waterMaskFlag = flag;
}

void DataProcess::SetTransparency(int value)
{
	m_transparency = value;
}

void DataProcess::SetWaterMaskImg(cv::Mat img)
{
	m_waterMaskImg = img;
}

void DataProcess::SetMaskImg(cv::Mat img)
{
	m_maskImg = img;
}

void DataProcess::SetWaterMaskGifImg(std::vector<cv::Mat> img)
{
	m_waterMaskGifImg = img;
}
void DataProcess::SetMaskGifImg(std::vector<cv::Mat> img)
{
	m_maskGifImg = img;
}

void DataProcess::SetInternal(int num)
{
	m_internal = num;
}

int DataProcess::GetInternal()
{
	return m_internal;
}

void DataProcess::SetGrabImgFlag(int flag)
{
	m_grabImgFlag = flag;
}

int DataProcess::GetGrabImgFlag()
{
	return m_grabImgFlag;
}

void DataProcess::SetSaveVideoFlag(int flag)
{
	m_saveVideoFlag = flag;
}
void DataProcess::SetNoiseOptimizationFlag(int flag)
{
	m_noiseOptimizationFlag = flag;
}
int DataProcess::GetSaveVideoFlag()
{
	return m_saveVideoFlag;
}

void DataProcess::SetCameraInfo(PreviewCameraInfo cInfo)
{
	m_cameraInfo = cInfo;
}

PreviewCameraInfo DataProcess::GetCameraInfo()
{
	return m_cameraInfo;
}

void DataProcess::SetSaveVideoPath(std::string str)
{
	m_saveVideoPath = str;
}

void DataProcess::SetSaveGrabImgPath(std::string str)
{
	m_saveGrabImgPath = str;
}

void DataProcess::SetWaterMaskRect(QRect rect)
{
	m_waterMaskRect = rect;
}

std::string DataProcess::GetSaveVideoPath()
{
	return m_saveVideoPath;
}

std::string DataProcess::GetSaveGrabImgPath()
{
	return m_saveGrabImgPath;
}

AVFormatContext * DataProcess::GetOutputFormatCtx()
{
	return m_outputFormatCtx;
}

AVFrame * DataProcess::GetOutputFrame()
{
	return m_outputFrame;
}

void DataProcess::SetThreadStop(bool flag)
{
	b_threadStop = flag;
}

bool DataProcess::GetThreadStop()
{
	return b_threadStop;
}

QRect DataProcess::GetWaterMaskRect()
{
	return m_waterMaskRect;
}

void DataProcess::SetImageStyle(int flag)
{
	m_imageStyle = flag;
}

void DataProcess::SetDataProcessPtr(CameraDetectTools * pCDT)
{
	m_pCDT = pCDT;
}

int DataProcess::GetImageStyle()
{
	return m_imageStyle;
}

void DataProcess::DestoryVideoParam()
{
	if (m_outputCodecCtx != NULL) {
		avcodec_close(m_outputCodecCtx);
	}

	if (m_outputFormatCtx != NULL) {
		avio_close(m_outputFormatCtx->pb);
		avformat_free_context(m_outputFormatCtx);
	}

	if (m_outputFrame != NULL) {
		av_free(m_outputFrame);
	}

	if (m_outputVideoStream != NULL) {
		if (m_outputVideoStream) {
			av_free(m_pictureBuff);
			av_free(m_videoOutBuff);
		}
	}
}

void DataProcess::FreeImgBuffer()
{
	if (!m_waterMaskImg.empty()) {
		m_waterMaskImg.release();
	}

	if (!m_maskImg.empty()) {
		m_maskImg.release();
	}

	if (!m_grabImgMat.empty()) {
		m_grabImgMat.release();
	}

	if (!m_waterMaskGifImg.empty()) {
		m_waterMaskGifImg.clear();
	}

	if (!m_maskGifImg.empty()) {
		m_maskGifImg.empty();
	}

	if (!m_showImageBuf.empty()) {
		m_showImageBuf.empty();
	}
}

/*
 * 将string型表示的分辨率分解成int型长和宽
 * 参数：
 *		res: 输入的待分解的字符串分辨率
 *		width：得到的分辨率的宽度
 *		height：得到的分辨率的长度
 */
void DataProcess::GetResolutionFromString(
	const std::string & res, 
	int & width, 
	int & height)
{
	int index = res.find('x');
	if (index == std::string::npos) {
		return;
	}
	width = std::atoi(res.substr(0, index).c_str());
	height = std::atoi(res.substr(index + 1).c_str());
}

/*
 * 录像时的初始化配置函数
 * 参数：
 *		cInfo: 录像文件的参数包括分辨率，帧率
 * 返回值：小于0，则初始化失败；大于等于0，则初始化成功
 */
int DataProcess::MP4OutputConfig(const PreviewCameraInfo& cInfo)
{
	avdevice_register_all();
	avcodec_register_all();
	av_register_all();
	QDateTime time = QDateTime::currentDateTime();
	QString str = time.toString("yyyyMMddhhmmsszzz") + ".mp4";
	std::string output_name = m_saveVideoPath + "/" + str.toStdString();

	avformat_alloc_output_context2(&m_outputFormatCtx, NULL,
		NULL, output_name.c_str());

	if (!m_outputFormatCtx) {
		avformat_alloc_output_context2(&m_outputFormatCtx,
			NULL, "mpeg", output_name.c_str());
	}

	if (!m_outputFormatCtx) {
		return -1;
	}

	m_outputFormatCtx->oformat->video_codec = CODEC_ID_H264;
	m_outputFormat = m_outputFormatCtx->oformat;

	m_outputVideoStream = NULL;

	if (m_outputFormat->video_codec != CODEC_ID_NONE) {
		m_outputVideoStream = AddVideoStream(m_outputFormat->video_codec,cInfo);
	}

	if (m_outputVideoStream) {
		OpenOutputVideo();
	}

	if (!(m_outputFormat->flags & AVFMT_NOFILE)) {
		if (avio_open(&m_outputFormatCtx->pb, output_name.c_str(), AVIO_FLAG_WRITE) < 0) {
			return -1;
		}
	}
	avformat_write_header(m_outputFormatCtx, NULL);

	m_outputCodecCtx = m_outputVideoStream->codec;
	m_outputFrame = av_frame_alloc();
	m_videoOutBuffSize = avpicture_get_size(
							m_outputCodecCtx->pix_fmt,
							m_outputCodecCtx->width, 
							m_outputCodecCtx->height);

	m_pictureBuff = (uint8_t *)av_malloc(m_videoOutBuffSize);

	avpicture_fill(
		(AVPicture *)m_outputFrame,
		m_pictureBuff,
		m_outputCodecCtx->pix_fmt, 
		m_outputCodecCtx->width,
		m_outputCodecCtx->height
	);

	m_videoOutBuff = (uint8_t *)av_malloc(m_videoOutBuffSize);

	return 0;
}

/*
 * 修改自ffmpeg官方文档muxing.c，
 * 配置参数包括图像大小，帧率等常用视频编码参数
 * 参数：
 *		codec_id: 视频流编码类型
 *		cInfo: 输出视频流的参数，包括分辨率，帧率
 * 返回值：返回配置好的AVStream*， 若为NULL则配置失败
 */
AVStream* DataProcess::AddVideoStream(AVCodecID codec_id, const PreviewCameraInfo& cInfo)
{
	int width = 0, height = 0, fps = 0;
	GetResolutionFromString(cInfo.resolution, width, height);
	fps = atoi(cInfo.fps.c_str());
	AVCodecContext *c;
	AVStream *st;
	AVCodec *codec;

	st = avformat_new_stream(m_outputFormatCtx, NULL);
	if (!st) {
		return NULL;
	}
	c = st->codec;

	// find the video encoder 
	codec = avcodec_find_encoder(codec_id);
	if (!codec) {
		return NULL;
	}
	avcodec_get_context_defaults3(c, codec);

	c->codec_id = codec_id;

	// put sample parameters
	c->bit_rate = /*400000*/3000000;

	// resolution must be a multiple of two 
	c->width = width;
	c->height = height;

	/* time base: this is the fundamental unit of time (in seconds) in terms
	of which frame timestamps are represented. for fixed-fps content,
	timebase should be 1/framerate and timestamp increments should be
	identically 1. */
	c->time_base.den = fps;
	c->time_base.num = 1;
	c->gop_size = 12; // emit one intra frame every twelve frames at most 
	c->pix_fmt = PIX_FMT_YUV420P;
	if (c->codec_id == CODEC_ID_MPEG2VIDEO) {

		//just for testing, we also add B frames 
		c->max_b_frames = 3;
	}
	if (c->codec_id == CODEC_ID_MPEG1VIDEO) {

		/* Needed to avoid using macroblocks in which some coeffs overflow.
		This does not happen with normal video, it just happens here as
		the motion of the chroma plane does not match the luma plane. */
		c->mb_decision = 2;
	}

	// some formats want stream headers to be separate
	if (m_outputFormatCtx->oformat->flags & AVFMT_GLOBALHEADER)
		c->flags |= CODEC_FLAG_GLOBAL_HEADER;

	return st;
}

/*
 * 修改自ffmpeg官方文档muxing.c
 * 打开所配置的视频编码AVCodecContext
 */
void DataProcess::OpenOutputVideo()
{
	AVCodec *codec;
	AVCodecContext *c;

	c = m_outputVideoStream->codec;

	/* find the video encoder */
	codec = avcodec_find_encoder(c->codec_id);
	if (!codec) {
		return;
	}
	AVDictionary *param = 0;
	av_dict_set(&param, "preset", "superfast", 0);
	av_dict_set(&param, "tune", "zerolatency", 0);

	// open the codec
	if (avcodec_open2(c, codec, &param) < 0) {
		return;
	}
}

/*
 * 修改自ffmpeg官方文档muxing.c
 * 可实现将当前帧进行编码并写入到文件中
 * 参数：
 *		pts_num: 输入当前帧的显示时间戳
 */
void DataProcess::WriteVideoFrame(const int &pts_num)
{
	if (m_outputFormatCtx->oformat->flags & AVFMT_RAWPICTURE) {
		AVPacket pkt;
		av_init_packet(&pkt);

		pkt.flags |= AV_PKT_FLAG_KEY;
		pkt.stream_index = m_outputVideoStream->index;
		pkt.data = (uint8_t *)m_outputFrame;
		pkt.size = sizeof(AVPicture);

		av_interleaved_write_frame(m_outputFormatCtx, &pkt);
	}
	else {
		int out_size = 0;
		m_outputFrame->pts = pts_num;
		out_size = avcodec_encode_video(m_outputVideoStream->codec,
			m_videoOutBuff, m_videoOutBuffSize, m_outputFrame);

		if (out_size <= 0) {
			return;
		}
		AVPacket pkt;
		av_init_packet(&pkt);

		if (m_outputVideoStream->codec->coded_frame->pts != AV_NOPTS_VALUE) {
			pkt.pts = av_rescale_q(m_outputVideoStream->codec->coded_frame->pts,
				m_outputVideoStream->codec->time_base, m_outputVideoStream->time_base);
		}

		if (m_outputVideoStream->codec->coded_frame->key_frame) {
			pkt.flags |= AV_PKT_FLAG_KEY;
		}
		pkt.stream_index = m_outputVideoStream->index;
		pkt.data = m_videoOutBuff;
		pkt.size = out_size;
		av_interleaved_write_frame(m_outputFormatCtx, &pkt);
	}
}

int DataProcess::FlushEncoder(const int& stream_index)
{
	int ret;
	int gotFrame;
	AVPacket encPkt;
	if (!(m_outputFormatCtx->streams[stream_index]->codec->codec->capabilities &
		CODEC_CAP_DELAY))
		return 0;

	while (1) {
		encPkt.data = NULL;
		encPkt.size = 0;
		av_init_packet(&encPkt);
		ret = avcodec_encode_video2(
				m_outputFormatCtx->streams[stream_index]->codec,
				&encPkt,
				NULL, 
				&gotFrame);

		av_frame_free(NULL);
		if (ret < 0) {
			break;
		}
		if (!gotFrame) {
			ret = 0;
			break;
		}
		ret = av_write_frame(m_outputFormatCtx, &encPkt);
		if (ret < 0) {
			break;
		}
	}
	av_free_packet(&encPkt);
	return ret;
}

/*
 * 添加水印功能函数,可实现将水印与当前帧图像合并
 * 参数:
 *		img: 输入当前摄像头获取的图像数据
 */
void DataProcess::AddWaterMask(cv::Mat& img)
{
	if (m_waterMaskFlag <= DP_WATERMASK_NONE) {
		return;
	}

	float tmpTpy = m_transparency / 100.0;
	cv::Rect rectShow(m_waterMaskRect.left(), m_waterMaskRect.top(),
		m_waterMaskRect.width(), m_waterMaskRect.height());

	switch (m_waterMaskFlag)
	{
	case DP_WATERMASK_RGB:
		cv::add(tmpTpy*img(rectShow),
			(1 - tmpTpy)*m_waterMaskImg,
			img(rectShow),
			m_maskImg, -1);
		break;
	case DP_WATERMASK_GIF:
	{
		if (m_waterMaskGifImg.size() == 0) {
			break;
		}
		if (m_showIndex >= m_waterMaskGifImg.size()) {
			m_showIndex = 0;
		}
		int i = m_showIndex;
		cv::add(tmpTpy*img(rectShow),
			(1 - tmpTpy)*m_waterMaskGifImg[i],
			img(rectShow),
			m_maskGifImg[i], -1);

		if (m_internal >= 4 /*该值可以之后设置*/) {
			m_showIndex++;
			m_internal = 0;
		}
		m_internal++;
		break;
	}
	default:
		break;
	}
}

/*
 * 抓拍图像,赋值当前帧数据，并发送信号进行存储
 * 参数：
 *		img: 输入当前显示帧图像
 */
void DataProcess::GrabImage(const cv::Mat& img)
{
	if (m_grabImgFlag == DP_GRAB_PROCESSING) {
		m_grabImgFlag = DP_GRAB_SAVING;
		m_grabImgMat = img.clone();
		emit SendImageGrabMsg();
	}
}

/*
 * 存储抓拍图像
 */
void DataProcess::SaveGrabImage()
{
	QDateTime time = QDateTime::currentDateTime();
	QString str = time.toString("yyyyMMddhhmmsszzz") + ".jpg";
	cv::imwrite(m_saveGrabImgPath + "/" + str.toStdString(), m_grabImgMat);
	m_grabImgMat.release();
	m_grabImgFlag = DP_GRAB_READY;
}

void OilPaint(cv::Mat& img, int brushSize, int coarseness)
{
	assert(!img.empty());
	if (brushSize < 1) brushSize = 1;
	if (brushSize > 8) brushSize = 8;

	if (coarseness < 1) coarseness = 1;
	if (coarseness > 255) coarseness = 255;

	int width = img.cols;
	int height = img.rows;

	int lenArray = coarseness + 1;
	int* CountIntensity = new int[lenArray];
	uint* RedAverage = new uint[lenArray];
	uint* GreenAverage = new uint[lenArray];
	uint* BlueAverage = new uint[lenArray];

	cv::Mat gray;
	cvtColor(img, gray, cv::COLOR_BGR2GRAY);

	cv::Mat dst = cv::Mat::zeros(img.size(), img.type());

	for (int nY = 0; nY <height; nY++)
	{
		int top = nY - brushSize;
		int bottom = nY + brushSize + 1;

		if (top<0) top = 0;
		if (bottom >= height) bottom = height - 1;

		for (int nX = 0; nX<width; nX++)
		{
			int left = nX - brushSize;
			int right = nX + brushSize + 1;

			if (left<0) left = 0;
			if (right >= width) right = width - 1;

			for (int i = 0; i <lenArray; i++)
			{
				CountIntensity[i] = 0;
				RedAverage[i] = 0;
				GreenAverage[i] = 0;
				BlueAverage[i] = 0;
			}

			for (int j = top; j<bottom; j++)
			{
				for (int i = left; i < right; i++)
				{
					uchar intensity = static_cast<uchar>(coarseness*gray.at<uchar>(j, i) / 255.0);
					CountIntensity[intensity]++;

					RedAverage[intensity] += img.at<cv::Vec3b>(j, i)[2];
					GreenAverage[intensity] += img.at<cv::Vec3b>(j, i)[1];
					BlueAverage[intensity] += img.at<cv::Vec3b>(j, i)[0];
				}
			}

			uchar chosenIntensity = 0;
			int maxInstance = CountIntensity[0];
			for (int i = 1; i<lenArray; i++)
			{
				if (CountIntensity[i] > maxInstance)
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
	dst.copyTo(img);
}

/*
 * 预览功能图像处理函数，
 * 根据当前显示风格进行不同的处理
 * 处理完成将图像数据放入缓冲队列
 * 参数：
 *		img: 待预处理的图像
 */
void DataProcess::PreviewImage(cv::Mat& img)
{

	switch (m_imageStyle)
	{
	case DP_IMAGESTYLE_NORMAL:
	{
		cv::cvtColor(img, img, CV_BGR2RGB);			// opencv默认是BGR排序，转到QT中显示需要RGB排序
		break;
	}
	case DP_IMAGESTYLE_OIL:
	{
		OilPaint(img, 2, 100);
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

	EnterCriticalSection(&m_previewLock);

	while (m_showImageBuf.size() > 1) {
		m_showImageBuf.pop();
	}
	m_showImageBuf.push(img);

	LeaveCriticalSection(&m_previewLock);

	emit SendUpdataImgMsg();
}

/*
 * 对图像进行噪点优化
 * 参数：
 *		img: 待优化的图像
 */
void DataProcess::OptimizeImage(cv::Mat & img)
{
	if (m_noiseOptimizationFlag) {
		cv::medianBlur(img, img, 3);
	}
}

/*
 * 得到待显示的摄像头图像数据
 * 参数：
 *		img：从缓冲队列得到的图像数据
 */
void DataProcess::GetShowImageBuffer(cv::Mat& img)
{
	EnterCriticalSection(&m_previewLock);

	if (!m_showImageBuf.empty()) {
		img = m_showImageBuf.front();
		m_showImageBuf.pop();
	}

	LeaveCriticalSection(&m_previewLock);
}

/*
 * 得到待编码存储的图像数据
 * 参数：
 *		img：从缓冲队列得到的图像数据
 */
void DataProcess::GetSaveImageBuffer(cv::Mat& img)
{
	EnterCriticalSection(&m_saveBufferLock);

	if (!m_saveImageBuf.empty()) {
		img = m_saveImageBuf.front();
		m_saveImageBuf.pop();
	}

	LeaveCriticalSection(&m_saveBufferLock);
}

/*
 * 图像编码存储线程
 * 参数：
 *		pm：另一个线程传入的对象指针
 */
DWORD WINAPI SaveVideoThread(LPVOID pm)
{
	DataProcess *pDP = static_cast<DataProcess*>(pm);
	int ptsNum = 0;
	while (1)
	{
		if (pDP->GetSaveVideoFlag() == DP_SAVEVIDEO_SAVING) {
			cv::Mat img;
			pDP->GetSaveImageBuffer(img);
			if (img.empty()) {
				Sleep(500);
				continue;
			}
			uint8_t *rgbSrc[3] = { img.data, NULL, NULL };
			SwsContext* pSwsCxt;

			if (img.channels() == 1/*pDP->GetImageStyle() == DP_IMAGESTYLE_GRAY*/) {
				pSwsCxt = sws_getContext(img.cols,
					img.rows, PIX_FMT_GRAY8, img.cols,
					img.rows, PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);
				int rgbStride[3] = { img.cols, 0, 0 };
				sws_scale(pSwsCxt, rgbSrc, rgbStride, 0, img.rows,
					((AVPicture *)pDP->GetOutputFrame())->data, ((AVPicture *)pDP->GetOutputFrame())->linesize);
			}
			else {
				pSwsCxt = sws_getContext(img.cols,
					img.rows, PIX_FMT_RGB24, img.cols,
					img.rows, PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);
				int rgbStride[3] = { 3 * img.cols, 0, 0 };
				sws_scale(pSwsCxt, rgbSrc, rgbStride, 0, img.rows,
					((AVPicture *)pDP->GetOutputFrame())->data, ((AVPicture *)pDP->GetOutputFrame())->linesize);
			}

			pDP->WriteVideoFrame(ptsNum);
			ptsNum++;
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

/*
 * 录像功能主函数
 * 根据当前录像状态参数进行初始化，编码存储
 * 直接返回或者将数据写入文件尾
 * 参数：
 *		img：待编码存储的图像数据
 *		cInfo：编码的参数，包括分辨率，帧率
 */
void DataProcess::SaveVideo(const cv::Mat& img, const PreviewCameraInfo& cInfo)
{
	if (m_saveVideoFlag == DP_SAVEVIDEO_NONE) {
		return;
	}
	else if (m_saveVideoFlag == DP_SAVEVIDEO_INIT) {
		if (MP4OutputConfig(cInfo) != DP_SAVEVIDEO_INIT_OK) {
			m_saveVideoFlag = DP_SAVEVIDEO_NONE;
			return;
		}
		CreateThread(NULL, 0, SaveVideoThread, this, 0, NULL);
		m_ptsNum = 0;
		m_saveVideoFlag = DP_SAVEVIDEO_SAVING;
	}
	else if (m_saveVideoFlag == DP_SAVEVIDEO_SAVING) {
		EnterCriticalSection(&m_saveBufferLock);
		m_saveImgMat = img.clone();
		m_saveImageBuf.push(m_saveImgMat);
		LeaveCriticalSection(&m_saveBufferLock);
	}
}

/*
 * 获取图像数据线程
 * 对图像根据需求做对应的处理
 * 在即将结束该线程时时向主UI线程发送结束信号
 */
void DataProcess::run()
{
	LARGE_INTEGER nFreq;
	LARGE_INTEGER nFirstTime;
	LARGE_INTEGER nEndTime;

	QueryPerformanceFrequency(&nFreq);

	m_pCDT->CameraInputInit(m_cameraInfo);

	int fps = atoi(m_cameraInfo.fps.c_str());
	int perTime = 1000 / fps;
	int ret = 0;
	while (1) {
		if (b_threadStop) {
			b_threadStop = false;
			break;
		}

		QueryPerformanceCounter(&nFirstTime);

		cv::Mat img = m_pCDT->GetFrameData();
		if (img.empty()) {
			Sleep(100);
			continue;
		}
		OptimizeImage(img);				// 噪点优化
		AddWaterMask(img);				// 添加水印
		GrabImage(img);					// 抓拍图像
		PreviewImage(img);				// 预览图像
		
		SaveVideo(img, m_cameraInfo);	// 保存视频
		img.release();

		QueryPerformanceCounter(&nEndTime);
		int m_interTime = (double)(nEndTime.QuadPart - nFirstTime.QuadPart) 
								* 1000 / (double)nFreq.QuadPart;

		if (m_interTime < perTime) {
			Sleep(perTime - m_interTime);
		}
	}

	m_pCDT->DestoryInputParam();
	SendAbort(ret);
}