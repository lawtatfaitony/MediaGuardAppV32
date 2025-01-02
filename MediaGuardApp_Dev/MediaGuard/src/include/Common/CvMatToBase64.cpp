#include "CvMatToBase64.h"

using namespace Basic;

//ref https://blog.csdn.net/qikaihuting/article/details/119419302
//imgType 包括png bmp jpg jpeg等opencv能够进行编码解码的文件
// usage:std::string img_data = Mat2Base64(image, ".jpg");

CvMatToBase64::CvMatToBase64()
{
}

CvMatToBase64::~CvMatToBase64()
{
}

std::string CvMatToBase64::Mat2Base64(const cv::Mat& image, std::string imgType = ".jpg") {
	//Mat转base64
	std::vector<uchar> buf;
	cv::imencode(imgType, image, buf);
	//uchar *enc_msg = reinterpret_cast<unsigned char*>(buf.data());
	std::string img_data = Base64::Encode(buf.data(), buf.size());
	return img_data;
}

cv::Mat CvMatToBase64::Base2Mat(std::string& base64_data) {
	cv::Mat img;
	std::string s_mat;
	s_mat = Base64::Decode(base64_data.data());
	std::vector<char> base64_img(s_mat.begin(), s_mat.end());
	img = cv::imdecode(base64_img, cv::IMREAD_COLOR); //CV::IMREAD_UNCHANGED
	return img;
}