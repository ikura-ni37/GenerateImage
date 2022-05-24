
//���s������
// 1 �w�i�摜�������t�H���_�̐�΃p�X


#include "nkcOpenCV.h"

#define CVUI_IMPLEMENTATION
#include "cvui.h"

#define WINDOW_NAME	"Image Data Generation"
#define _USE_MATH_DEFINES

#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <fstream>
#include <sstream>
#include <random>
#include <stdlib.h>
#include <filesystem>
#include <math.h>

using namespace std;
namespace fs = std::filesystem;

//�t�@�C���_�C�A���O���J���֐�

bool OpenFile(cv::String& filestring);

#define GENERATE_NUMBER 1000				//�����摜�̖����w��

int main(int argc, const char* argv[])
{

#define MAX_FOREIMAGE_SIZE 10000

#ifdef _DEBUG

	//�f�o�b�N��

	const cv::String foreFilename = "doraemon.jpg";
	fs::path imagefolderAbsPass = " "; //init Folder name
#else
	//�����̌�����v���Ȃ������ꍇ�͏I��
	if (argc != 2) {
		cout << "���s�������Ńt�H���_�p�X���w�肵�Ă�������" << endl;
		return -1;
	}

	//�����[�X��
	cv::String foreFilename;
	if (!::OpenFile(foreFilename)) return 1;
	fs::path imagefolderAbsPass{ argv[1] };

#endif

		//�t�@�C���p�X�𑊑΃p�X�ɕϊ�
		fs::path rel_p = fs::relative(imagefolderAbsPass);

		//��O����
		if (!fs::exists(rel_p)||!fs::is_directory(rel_p)) {
			std::cout << "�w�i�摜�t�H���_�̃p�X�������ł�" << std::endl;
			return -1;
		}



	//�E�B���h�E�쐬
	cv::Mat frame = cv::Mat(cv::Size(1300, 600), CV_8UC3);

	cvui::init(WINDOW_NAME, 20);

	/*�{�^���������ꂽ���̔��ʗp*/
	bool generate = false;					//open�{�^��
	bool checkTrapezoidalization = false;	//��`���I��
	bool checkInversion = false;			//���]�I��
	bool checkRotation = false;				//��]�I��

	int ImageX, ImageY;						//�}�E�X�ʒu�p�ϐ�


	int generateImgNum = GENERATE_NUMBER;	//�����摜��
	int backImgNum;							//�w�i�摜��
	int ImageNum = 0;						//�I���摜��\���p�����[�^

	cv::Mat foreImage,						//�O�i�摜�pMat
			resizeforeImage,				//���T�C�Y�����O�i�摜�pMat
			showImage;						//�摜�\���pMat

	vector<cv::Mat> backgroundImage;		//�w�i�摜�i�[�pMat�^vector
	vector<cv::Mat> generateImage;			//�����摜�i�[�pMat�^vector

	vector<vector<double>> coordinate;		//�O�i�摜�̍��W�i�[�p�񎟌�vector

	/*�O�i�摜�̓ǂݍ���*/
	foreImage = cv::imread(foreFilename, 1);
	
	/*�O�i�摜�̃��T�C�Y�i�c�Ɖ��̒����ق���100�ɂȂ�悤�ɏc�����ێ������܂܃��T�C�Y�j*/
	double resizePersent;//�{��

	if (foreImage.cols > foreImage.rows)
		resizePersent =(double) MAX_FOREIMAGE_SIZE / foreImage.cols;
	else 
		resizePersent = (double)MAX_FOREIMAGE_SIZE / foreImage.rows;

	cv::resize(foreImage, resizeforeImage, cv::Size(foreImage.cols * resizePersent, foreImage.rows * resizePersent),cv::INTER_CUBIC);//���T�C�Y


	/*�w�i�摜�̓ǂݍ���*/
	cv::Mat defaultImage;		//�ǂݍ��ݔw�i�摜�i�[�pMat
	cv::Mat resizeImage;		//���T�C�Y�����w�i�摜�i�[�pMat

	/*�t�@�C���f�B���N�g������*/
	for (const fs::directory_entry& entry : fs::directory_iterator(rel_p)) {
		fs::path p = entry;
		if (p.extension() == ".jpg") {
		//�摜�t�@�C���ł�������ǂݍ���
			const std::string backPath = p.string();

			/*�摜�ǂݍ���*/
			defaultImage = cv::imread(backPath);
			/*�摜�̃��T�C�Y*/
			cv::resize(defaultImage, resizeImage, cv::Size(600, 400));
			/*�w�i�摜�i�[�pvector�ɕۑ�*/
			backgroundImage.push_back(resizeImage.clone());
		}

		if (backgroundImage.empty()) {
		//�w�i�摜���Ȃ������ꍇ
			cout << "�t�H���_���ɉ摜�t�@�C��������܂���" << endl;
			return -1;
		}
		
	}
	/*�w�i�摜���̎擾*/
	backImgNum = backgroundImage.size();


	while (true) {
		//�t���[�����F
		frame = cv::Scalar(49, 52, 49);

		//�}�E�X�ʒu�̌���
		cv::Rect rectangle(500, 150, 600, 400);
		int status = cvui::iarea(rectangle.x, rectangle.y, rectangle.width, rectangle.height);

		ImageX = cvui::mouse().x - rectangle.x;
		ImageY = cvui::mouse().y - rectangle.y;

		cvui::beginColumn(frame, 100, 50, 100, 200, 20);

		/* generate�{�^��
		*�@�@�摜�������s��
		*/
		if (cvui::button(100, 30, "Genarate") && !generate) {

			//�����쐬�p
			std::random_device rnd;

			std::mt19937 mt(rnd());     //  �����Z���k�E�c�C�X�^��32�r�b�g�ŁA�����͏����V�[�h�l


			std::uniform_int_distribution<int> back(0, backImgNum-1);			//�w�i�摜�p����
			std::uniform_real_distribution<double> magnificant(0.5, 3.0);		//�g����k�p����
			std::uniform_int_distribution<int> degree(0, 360);					//��]�p�x�p����
			std::uniform_int_distribution<int> mode(0, 10);						//�����ϊ��p����
			std::uniform_real_distribution<double>xMove(0.0, 500.0);			//���������ړ��p����
			std::uniform_real_distribution<double>yMove(0.0, 300.0);			//���������ړ��p����

			cv::Point2f dstPoint[4];				//��`�ϊ���̍��W�i�[�p

			int xMergin, yMergin, x0, x1, y0, y1;	//��`�ϊ��p�ϐ�

			double sizePercent, colSize, rowSize;	//�g����k�����p�ϐ�

			cv::Mat targetImage,					//�g����k��̉摜�i�[�p
					selectImage,					//�������ꂽ�摜�i�[�p
					perspectiveMat,					//�����ϊ��p�̍s��i�[�p
					rotationMat;					//��]�s��i�[�p

			cv::Mat unitMatrix = (cv::Mat_<double>(3, 3) << 1, 0, 0, 0, 1, 0, 0, 0, 1);		//�P�ʍs��̍쐬


			for (int i = 0; i < generateImgNum; i++)
			{
				/*�w�i�摜�̌���*/
				selectImage = backgroundImage[back(mt)].clone();
				
				/*�g��E���k����*/
				sizePercent = magnificant(mt);						//�{���̐ݒ�
				colSize = resizeforeImage.cols * sizePercent;		//������̉���
				rowSize = resizeforeImage.rows * sizePercent;		//������̏c��

				cv::resize(resizeforeImage, targetImage, cv::Size(colSize, rowSize));
				

				//��`���������s���ꍇ
				if (checkTrapezoidalization) {
					/*��`�������i�����ϊ��s��j*/
					x0 = targetImage.cols / 4;			//�O�i�摜�̑�`�ϊ��O�̂S�_�̍��W���i�[
					x1 = (targetImage.cols / 4) * 3;
					y0 = targetImage.rows / 4;
					y1 = (targetImage.rows / 4) * 3;

					cv::Point2f srcPoint[] = {
					cv::Point(x0,y0),
					cv::Point(x0,y1),
					cv::Point(x1,y1),
					cv::Point(x1,y0)
					};

					//�����ɂ�胂�[�h�����߂đ�`���������s��
					switch (mode(mt)) {

					case 0://��Z�����̑�`
						xMergin = targetImage.cols / 10;
						yMergin = targetImage.rows / 10;
						dstPoint[0] = cv::Point(x0 + xMergin, y0 + yMergin);
						dstPoint[1] = srcPoint[1];
						dstPoint[2] = srcPoint[2];
						dstPoint[3] = cv::Point(x1 - xMergin, y0 + yMergin);

						break;

					case 1://�E�Z�����̑�`
						xMergin = targetImage.cols / 10;
						yMergin = targetImage.rows / 10;
						dstPoint[0] = srcPoint[0];
						dstPoint[1] = srcPoint[1];
						dstPoint[2] = cv::Point(x1 - xMergin, y1 - yMergin);
						dstPoint[3] = cv::Point(x1 - xMergin, y0 + yMergin);

						break;

					case 2://���s�l�ӌ`
						xMergin = targetImage.cols / 10;
						yMergin = targetImage.rows / 10;
						dstPoint[0] = cv::Point(x0 + xMergin, y0 + yMergin);
						dstPoint[1] = srcPoint[1];
						dstPoint[2] = cv::Point(x1 - xMergin, y1 - yMergin);
						dstPoint[3] = srcPoint[3];
						break;

					case 3://�㒷���Z�̑�`
						xMergin = targetImage.cols / 10;
						yMergin = targetImage.rows / 10;
						dstPoint[0] = srcPoint[0];
						dstPoint[1] = cv::Point(x0 + xMergin, y1 - yMergin);
						dstPoint[2] = cv::Point(x1 - xMergin, y1 - yMergin);
						dstPoint[3] = srcPoint[3];
						break;

					case 4://�E�Z�����̑�`
						xMergin = targetImage.cols / 10;
						yMergin = targetImage.rows / 10;
						dstPoint[0] = cv::Point(x0 + xMergin, y0 + yMergin);
						dstPoint[1] = cv::Point(x0 + xMergin, y1 - yMergin);
						dstPoint[2] = srcPoint[2];
						dstPoint[3] = srcPoint[3];
						break;

					case 5://���s�l�ӌ`
						xMergin = targetImage.cols / 10;
						yMergin = targetImage.rows / 10;
						dstPoint[0] = srcPoint[0];
						dstPoint[1] = cv::Point(x0 + xMergin, y1 - yMergin);
						dstPoint[2] = srcPoint[2];
						dstPoint[3] = cv::Point(x1 - xMergin, y0 + yMergin);
						break;

					default://��`�ϊ����s��Ȃ��ꍇ
						dstPoint[0] = srcPoint[0];
						dstPoint[1] = srcPoint[1];
						dstPoint[2] = srcPoint[2];
						dstPoint[3] = srcPoint[3];
					}

					perspectiveMat = getPerspectiveTransform(srcPoint, dstPoint);//��`�����������ϊ��s��̍쐬
				}
				else {
					perspectiveMat = unitMatrix.clone();
				}
				

				//��]�������s���ꍇ
				if (checkRotation) {
					/*��]�����i�A�t�B���ϊ��s��j*/
					cv::Point2d ctr(targetImage.cols / 2, targetImage.rows / 2);//�O�i�摜�̉�]���S�̐ݒ�
					rotationMat = cv::getRotationMatrix2D(ctr, degree(mt), 1.0);//��]�s��̍쐬

					cv::Mat calcMat = (cv::Mat_<double>(1, 3) << 0.0, 0.0, 1.0);	//�����p
					rotationMat.push_back(calcMat);	//��]�s����R�~�R�Ɋg��
				}
				else {
					rotationMat = unitMatrix.clone();
				}

								
				/*���s�ړ������i�A�t�B���ϊ��s��j*/
				std::uniform_real_distribution<double>xMove(0.0, 600 - colSize);	//�������ړ��p����
				std::uniform_real_distribution<double>yMove(0.0, 400 - rowSize);	//�c�����ړ��p����

				cv::Mat shiftMat = (cv::Mat_<double>(3, 3) << 1, 0, xMove(mt), 0, 1, yMove(mt), 0, 0, 1);	//���s�ړ��s��̍쐬
				
				
				/*�A�t�B���ϊ��s��Ɠ����ϊ��s��̍����i�|���Z�j*/
				cv::Mat convertionMat =  shiftMat * rotationMat * perspectiveMat;

				/*�O�i�摜�̂S�_�̏o��*/
				double m_coordinate[4][3] = {	//�g���̑O�i�摜�̏������W
						{0.0 ,0.0,1},
						{0.0, targetImage.rows,1},
						{targetImage.cols ,targetImage.rows,1},
						{targetImage.cols ,0.0,1}
				};

				/*�e���W�ɂ������]�̎��s*/
				vector <double>m_code;				//���W�������x�N�g��
				double xCoordinate, yCoordinate;	//���W�������ϐ�
				bool boundary = TRUE;				//���E���f�p�ϐ�
				for (int j = 0; j < 4; j++) {
					cv::Mat m_coordinateMat = (cv::Mat_<double>(3, 1) << m_coordinate[j][0], m_coordinate[j][1], m_coordinate[j][2]);
					cv::Mat rotation = convertionMat * m_coordinateMat;

					xCoordinate = rotation.at<double>(0, 0) / rotation.at<double>(2, 0);
					yCoordinate = rotation.at<double>(1, 0) / rotation.at<double>(2, 0);

					if (xCoordinate < 0 || xCoordinate>600 || yCoordinate < 0 || yCoordinate>400) {
						//���E�l���f
						boundary = FALSE;
					}

					m_code.push_back(xCoordinate);
					m_code.push_back(yCoordinate);

				}

				/*���E�l���f*/
				if (!boundary) {
					//��ł����_���w�i�摜���ɂȂ������ꍇ
					//�摜�Ɖ摜����ۑ�����������x�������s��
					i--;
					continue;
				}
				coordinate.push_back(m_code);

				/* �O�i�摜�w�i�摜�̍���*/
				cv::warpPerspective(targetImage, selectImage, convertionMat, cv::Size(600, 400), cv::INTER_LINEAR, cv::BORDER_TRANSPARENT);
				generateImage.push_back(selectImage.clone());
			}

			generate = true;
		}


		/*�@��`���`�F�b�N�{�b�N�X*/
		cvui::checkbox("Trapezoidalization", &checkTrapezoidalization);

		/*�@��]�������`�F�b�N�{�b�N�X�@*/
		cvui::checkbox("Rotation", &checkRotation);


		/*save�{�^��
		*	 �摜�Ɖ摜���̓������t�@�C����ۑ�����D
		*/
		if (cvui::button(100, 30, "save")&&generate) {
			int num;
			string str_buf;
			string str_conma_buf;
			string output_csv_file_path = "outImage.csv";

			// ��������csv�t�@�C�����J��(std::ofstream�̃R���X�g���N�^�ŊJ��)
			std::ofstream ofs_csv_file(output_csv_file_path);
			if (!ofs_csv_file) {
					std::cout << "�t�@�C�����J���܂���ł����B" << std::endl;
					return 0;
			}

			try {
			//��O����
				do {
					std::cout << "�擪�t�@�C���ԍ�����͂��Ă�������" << std::endl;
					std::cin >> num;
				} while (num > 9999 || num < 0);
			}
			catch (...) {
				std::cout << "�����œ��͂��Ă�������" << std::endl;

			}
			//�摜�Ɖ摜���̓������t�@�C���̕ۑ�
			for (int i = 0; i < generateImgNum; i++) {
				//�摜
				std::ostringstream oss;
				oss << std::setfill('0') << std::setw(4) << num;
				cv::imwrite("output" + oss.str() + ".jpg", generateImage[i]);
				//�摜���
				ofs_csv_file << "output" + oss.str() + ".jpg" << ",";
				for (int j = 0; j < 8; j++) {
					ofs_csv_file << coordinate[i][j];
					if (j != 7) {
						ofs_csv_file << ",";
					}	
				}
				ofs_csv_file << endl;

				num++;
			}
			//�I���̍��m
			cout << "end";
		}


		/*end�{�^��
		*	�I������
		*/
		if (cvui::button(100, 30, "end")) {
			break;
		}

		cvui::endColumn();

		cvui::beginColumn(frame,500, 50, 50);

		/*delete�{�^���i�����I����̂݉�����j
		*	�f�[�^���폜����
		*/
		if (cvui::button(100, 30, "delete")&&generate) {
			generateImage.erase(generateImage.begin() + ImageNum - 1);
			coordinate.erase(coordinate.begin() + ImageNum - 1);
			generateImgNum--;
		}
		cvui::beginRow(10);
		if (cvui::button(50, 30, "-")&&ImageNum!=0) {
			ImageNum--;
		}

		/*�摜�w��p�g���b�N�o�[*/
		cvui::trackbar(500, &ImageNum, 0, generateImgNum - 1);

		if (cvui::button(50, 30, "+")&&ImageNum!=GENERATE_NUMBER) {
			ImageNum ++;
		}

		cvui::endRow();

		/*�摜�̈�̃}�E�X�ʒu�\��*/
		if (status == cvui::OUT || generate == false) {
			cvui::printf("x:-- y:-- value:--", frame);
		}
		else
		{
			cvui::printf("x:%d y%d value:%d", ImageX, ImageY, generateImage[ImageNum].at<uchar>(ImageY, ImageX), frame);
		}

		cvui::endColumn();

		/*�����摜�̕\��*/
		if (generate == true) {
			//�������I������ꍇ
			cvui::image(frame, 500, 150 ,generateImage[ImageNum]);
		}

		cvui::update();	

		cvui::imshow(WINDOW_NAME, frame);

		if (cv::waitKey(20) == 27) {
			break;
		}

	}

	return 0;
}


bool OpenFile(cv::String& filestring) {
	/*OpenFile�֐�
		�����F�t�@�C�����J��
		�����F filestring�@�t�@�C�������i�[���镶����
		�߂�l�Ftrue:�t�@�C�����w�肳�ꂽ false:�L�����Z��
	*/


	wchar_t wpath[MAX_PATH], wfilename[MAX_PATH];
	wpath[0] = L'\0';
		
	OPENFILENAME ofn;

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = wpath;					
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = wfilename;			//�I�����ꂽ�t�@�C����
	ofn.nMaxFileTitle = MAX_PATH;
	
	ofn.lpstrFilter = L"JPEG(*.jpg)\0*.jpg\0\0"; //�t�@�C����ރt�B���^
	ofn.lpstrTitle = L"�摜�t�@�C����I�����Ă�������"; //�_�C�A���O�{�b�N�X�̃^�C�g��
	
	if (::GetOpenFileName(&ofn) == FALSE) return false;
	char filename[MAX_PATH * 2];
	size_t len;
	wcstombs_s(&len, filename, sizeof(filename), wfilename, _TRUNCATE);
	filestring = filename;

	return true;
}
