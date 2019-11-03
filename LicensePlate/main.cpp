#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstdlib>

using namespace cv;
using namespace std;

int main() {
	// 데이터
	Mat image, image_canny, image_rect, image_plate;
	Rect rect, temp_rect;
	vector<vector<Point> > contours;

	int select, plate_width, plate_piece_x, count_check = 0, count_eat, count_eat_biggest = 0;
	double ratio, delta_x, delta_y, gradient;


	image = imread("old.jpg"); // 처리할 이미지를 불러와 image 변수에 대입
	imshow("original", image);
	waitKey(0);





	/* 전처리 과정 */
	cvtColor(image, image_canny, COLOR_BGR2GRAY); // 1. 이미지를 흑백으로 변환하는 opencv 메소드 (전처리 중)
	imshow("gray", image_canny);
	waitKey(0);


	Canny(image_canny, image_canny, 100, 300, 3); // 2. Canny 알고리즘으로 흑백 이미지의 엣지를 검출하는 opencv 메소드 (전처리 중)
	imshow("canny", image_canny);
	waitKey(0);





	/* 윤곽을 추출하고, 번호판에 가장 적합한 윤곽들을 찾아서 저장한다.*/

	findContours(image_canny, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
	
	/*
	1. findContours 함수
	  (1). image_canny: 윤곽을 찾을 대상 이미지
	  (2). contours: 윤곽에 해당하는 끝점들을 저장할 벡터 ('포인트를 요소로 갖는 벡터'를 요소로 갖는 벡터)
	  (3). RETR_LIST (mode): contour의 부모관계를 따지지 않고 처리
	  (4). CHAIN_APPROX_SIMPLE (method): contour의 끝점만 반환
	*/

	vector<vector<Point> > contours_rect(contours.size());
	vector<Rect> boundRect(contours.size());
	vector<Rect> boundRect_check(contours.size());


	for (int i = 0; i < contours.size(); ++i) {
		approxPolyDP(Mat(contours[i]), contours_rect[i], 1, true);	
		boundRect[i] = boundingRect(Mat(contours_rect[i]));
	}

	image_rect = Mat::zeros(image.size(), CV_8UC3); //initialize bitmap

	//비율과 넓이를 고려해 가장 번호판의 번호에 적합한 사각형들만 다시 모은다.
	for (int i = 0; i < contours.size(); ++i) {
		ratio = (double)(boundRect[i].height / boundRect[i].width);

		// 예측하는 범위는 고정적이기 때문에, 사진을 찍는 환경이 일정한 게 유리하다. (ex. 자동차와 카메라의 거리나 각도 같은 것)
		if ((ratio <= 2.5) && (ratio >= 0.5) && (boundRect[i].area() <= 700) && (boundRect[i].area() >= 100)) {
			drawContours(image_rect, contours, i, Scalar(0, 255, 255), 1, 8);
			rectangle(image_rect, boundRect[i].tl(), boundRect[i].br(), Scalar(255, 0, 0), 1, 8);

			boundRect_check[count_check] = boundRect[i];
			count_check += 1;
		}
	}

	boundRect_check.resize(count_check);

	imshow("selected contours", image_rect); // 노란색 윤곽과 파란색 사각형이 그려진 이미지를 출력한다.
	waitKey(0);




	/*snake 알고리즘을 통해, 윤곽들 중에서 가장 번호판에 근접한 윤곽들의 집합을 찾아낸다.*/

	// 비율과 넓이를 만족하는 사각형들의 좌측 상단의 x축 좌표의 크기를 비교하여 순서대로 다시 정렬한다. (버블소트) 
	for (int i = 0; i < boundRect_check.size(); ++i) {
		for (int j = 1; j < (boundRect_check.size() - i); ++j) {
			if (boundRect_check[j - 1].tl().x > boundRect_check[j].tl().x) {
				temp_rect = boundRect_check[j - 1];
				boundRect_check[j - 1] = boundRect_check[j];
				boundRect_check[j] = temp_rect;
			}
		}
	}

	image.copyTo(image_plate);
	// 일정한 거리를 기준으로, 합리적인 기울기로 가장 밀집된 사각형들의 집합을 번호판으로 추측한다.
	for (int i = 0; i < (boundRect_check.size() - 1); ++i) {
		
		rectangle(image_plate, boundRect_check[i].tl(), boundRect_check[i].br(), Scalar(0, 255, 0), 1, 8, 0);

		count_eat = 0;

		for (int j = i + 1; j < boundRect_check.size(); ++j) {

			delta_x = abs(boundRect_check[j].tl().x - boundRect_check[i].tl().x);
			
			if (delta_x > 150) 
				break;    

			delta_y = abs(boundRect_check[j].tl().y - boundRect_check[i].tl().y);


			// 0을 나누거나, 0으로 나눌 오류를 예방한다.
			if (delta_x == 0) delta_x = 1;
			if (delta_y == 0) delta_y = 1;


			gradient = (double)(delta_y / delta_x);
			
			if (gradient < 0.25) // gradient가 유효할 기준을 0.25로 삼는다.
				count_eat += 1; // 기준으로부터 x축으로 적당한 거리에, 유효한 기울기로 사각형이 존재하면 1을 증가시킨다.

		}

		if (count_eat > count_eat_biggest) { // 가장 많은 사각형을 먹으면, 해당 변수를 업데이트한다.
			select = i;
			count_eat_biggest = count_eat;
			rectangle(image_plate, boundRect_check[select].tl(), boundRect_check[select].br(), Scalar(255, 0, 0), 1, 8, 0);
			plate_width = (int)delta_x;
			plate_piece_x = boundRect_check[select].br().x - boundRect_check[select].tl().x;
		}
	}

	rectangle(image_plate, boundRect_check[select].tl(), boundRect_check[select].br(), Scalar(0, 0, 255), 2, 8, 0);
	line(image_plate, boundRect_check[select].tl(), Point(boundRect_check[select].tl().x + plate_width, boundRect_check[select].tl().y), Scalar(0, 0, 255), 1, 8, 0);
	
	// 검사한 모든 사각형에 초록색, 업데이트한 번호판에 해당하는 사각형은 파란색, 가장 마지막에 업데이트 된 사각형의 테두리에 빨간색 사각형과 라인을 그린 후, 출력한다.
	imshow("processing snake algorithm", image_plate); 
	waitKey(0);

	imshow("license plate", image(Rect(boundRect_check[select].tl().x - 20, boundRect_check[select].tl().y - 20, plate_width + (2 * plate_piece_x), 0.3 * plate_width)));
	waitKey(0);
}