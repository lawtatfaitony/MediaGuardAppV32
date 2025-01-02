#include <string.h>
#include "opencv2/opencv.hpp"
#include "Basic/Base64.h"

NAMESPACE_BASIC_BEGIN


class CvMatToBase64
{
public:
	CvMatToBase64();
	virtual ~CvMatToBase64();

public:

	static std::string Mat2Base64(const cv::Mat& image, std::string imgType);

	static cv::Mat Base2Mat(std::string& base64_data);

};
NAMESPACE_BASIC_END