#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstdlib>

using namespace cv;
using namespace std;

int main() {
	// ������
	Mat image, image_canny, image_rect, image_plate;
	Rect rect, temp_rect;
	vector<vector<Point> > contours;

	int select, plate_width, plate_piece_x, count_check = 0, count_eat, count_eat_biggest = 0;
	double ratio, delta_x, delta_y, gradient;


	image = imread("old.jpg"); // ó���� �̹����� �ҷ��� image ������ ����
	imshow("original", image);
	waitKey(0);





	/* ��ó�� ���� */
	cvtColor(image, image_canny, COLOR_BGR2GRAY); // 1. �̹����� ������� ��ȯ�ϴ� opencv �޼ҵ� (��ó�� ��)
	imshow("gray", image_canny);
	waitKey(0);


	Canny(image_canny, image_canny, 100, 300, 3); // 2. Canny �˰������� ��� �̹����� ������ �����ϴ� opencv �޼ҵ� (��ó�� ��)
	imshow("canny", image_canny);
	waitKey(0);





	/* ������ �����ϰ�, ��ȣ�ǿ� ���� ������ �������� ã�Ƽ� �����Ѵ�.*/

	findContours(image_canny, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
	
	/*
	1. findContours �Լ�
	  (1). image_canny: ������ ã�� ��� �̹���
	  (2). contours: ������ �ش��ϴ� �������� ������ ���� ('����Ʈ�� ��ҷ� ���� ����'�� ��ҷ� ���� ����)
	  (3). RETR_LIST (mode): contour�� �θ���踦 ������ �ʰ� ó��
	  (4). CHAIN_APPROX_SIMPLE (method): contour�� ������ ��ȯ
	*/

	vector<vector<Point> > contours_rect(contours.size());
	vector<Rect> boundRect(contours.size());
	vector<Rect> boundRect_check(contours.size());


	for (int i = 0; i < contours.size(); ++i) {
		approxPolyDP(Mat(contours[i]), contours_rect[i], 1, true);	
		boundRect[i] = boundingRect(Mat(contours_rect[i]));
	}

	image_rect = Mat::zeros(image.size(), CV_8UC3); //initialize bitmap

	//������ ���̸� ����� ���� ��ȣ���� ��ȣ�� ������ �簢���鸸 �ٽ� ������.
	for (int i = 0; i < contours.size(); ++i) {
		ratio = (double)(boundRect[i].height / boundRect[i].width);

		// �����ϴ� ������ �������̱� ������, ������ ��� ȯ���� ������ �� �����ϴ�. (ex. �ڵ����� ī�޶��� �Ÿ��� ���� ���� ��)
		if ((ratio <= 2.5) && (ratio >= 0.5) && (boundRect[i].area() <= 700) && (boundRect[i].area() >= 100)) {
			drawContours(image_rect, contours, i, Scalar(0, 255, 255), 1, 8);
			rectangle(image_rect, boundRect[i].tl(), boundRect[i].br(), Scalar(255, 0, 0), 1, 8);

			boundRect_check[count_check] = boundRect[i];
			count_check += 1;
		}
	}

	boundRect_check.resize(count_check);

	imshow("selected contours", image_rect); // ����� ������ �Ķ��� �簢���� �׷��� �̹����� ����Ѵ�.
	waitKey(0);




	/*snake �˰����� ����, ������ �߿��� ���� ��ȣ�ǿ� ������ �������� ������ ã�Ƴ���.*/

	// ������ ���̸� �����ϴ� �簢������ ���� ����� x�� ��ǥ�� ũ�⸦ ���Ͽ� ������� �ٽ� �����Ѵ�. (�����Ʈ) 
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
	// ������ �Ÿ��� ��������, �ո����� ����� ���� ������ �簢������ ������ ��ȣ������ �����Ѵ�.
	for (int i = 0; i < (boundRect_check.size() - 1); ++i) {
		
		rectangle(image_plate, boundRect_check[i].tl(), boundRect_check[i].br(), Scalar(0, 255, 0), 1, 8, 0);

		count_eat = 0;

		for (int j = i + 1; j < boundRect_check.size(); ++j) {

			delta_x = abs(boundRect_check[j].tl().x - boundRect_check[i].tl().x);
			
			if (delta_x > 150) 
				break;    

			delta_y = abs(boundRect_check[j].tl().y - boundRect_check[i].tl().y);


			// 0�� �����ų�, 0���� ���� ������ �����Ѵ�.
			if (delta_x == 0) delta_x = 1;
			if (delta_y == 0) delta_y = 1;


			gradient = (double)(delta_y / delta_x);
			
			if (gradient < 0.25) // gradient�� ��ȿ�� ������ 0.25�� ��´�.
				count_eat += 1; // �������κ��� x������ ������ �Ÿ���, ��ȿ�� ����� �簢���� �����ϸ� 1�� ������Ų��.

		}

		if (count_eat > count_eat_biggest) { // ���� ���� �簢���� ������, �ش� ������ ������Ʈ�Ѵ�.
			select = i;
			count_eat_biggest = count_eat;
			rectangle(image_plate, boundRect_check[select].tl(), boundRect_check[select].br(), Scalar(255, 0, 0), 1, 8, 0);
			plate_width = (int)delta_x;
			plate_piece_x = boundRect_check[select].br().x - boundRect_check[select].tl().x;
		}
	}

	rectangle(image_plate, boundRect_check[select].tl(), boundRect_check[select].br(), Scalar(0, 0, 255), 2, 8, 0);
	line(image_plate, boundRect_check[select].tl(), Point(boundRect_check[select].tl().x + plate_width, boundRect_check[select].tl().y), Scalar(0, 0, 255), 1, 8, 0);
	
	// �˻��� ��� �簢���� �ʷϻ�, ������Ʈ�� ��ȣ�ǿ� �ش��ϴ� �簢���� �Ķ���, ���� �������� ������Ʈ �� �簢���� �׵θ��� ������ �簢���� ������ �׸� ��, ����Ѵ�.
	imshow("processing snake algorithm", image_plate); 
	waitKey(0);

	imshow("license plate", image(Rect(boundRect_check[select].tl().x - 20, boundRect_check[select].tl().y - 20, plate_width + (2 * plate_piece_x), 0.3 * plate_width)));
	waitKey(0);
}