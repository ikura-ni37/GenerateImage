
//実行時引数
// 1 背景画像入ったフォルダの絶対パス


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

//ファイルダイアログを開く関数

bool OpenFile(cv::String& filestring);

#define GENERATE_NUMBER 1000				//生成画像の枚数指定

int main(int argc, const char* argv[])
{

#define MAX_FOREIMAGE_SIZE 10000

#ifdef _DEBUG

	//デバック版

	const cv::String foreFilename = "doraemon.jpg";
	fs::path imagefolderAbsPass = " "; //init Folder name
#else
	//引数の個数が一致しなかった場合は終了
	if (argc != 2) {
		cout << "実行時引数でフォルダパスを指定してください" << endl;
		return -1;
	}

	//リリース版
	cv::String foreFilename;
	if (!::OpenFile(foreFilename)) return 1;
	fs::path imagefolderAbsPass{ argv[1] };

#endif

		//ファイルパスを相対パスに変換
		fs::path rel_p = fs::relative(imagefolderAbsPass);

		//例外処理
		if (!fs::exists(rel_p)||!fs::is_directory(rel_p)) {
			std::cout << "背景画像フォルダのパスが無効です" << std::endl;
			return -1;
		}



	//ウィンドウ作成
	cv::Mat frame = cv::Mat(cv::Size(1300, 600), CV_8UC3);

	cvui::init(WINDOW_NAME, 20);

	/*ボタンが押されたかの判別用*/
	bool generate = false;					//openボタン
	bool checkTrapezoidalization = false;	//台形化選択
	bool checkInversion = false;			//反転選択
	bool checkRotation = false;				//回転選択

	int ImageX, ImageY;						//マウス位置用変数


	int generateImgNum = GENERATE_NUMBER;	//生成画像数
	int backImgNum;							//背景画像数
	int ImageNum = 0;						//選択画像を表すパラメータ

	cv::Mat foreImage,						//前景画像用Mat
			resizeforeImage,				//リサイズした前景画像用Mat
			showImage;						//画像表示用Mat

	vector<cv::Mat> backgroundImage;		//背景画像格納用Mat型vector
	vector<cv::Mat> generateImage;			//生成画像格納用Mat型vector

	vector<vector<double>> coordinate;		//前景画像の座標格納用二次元vector

	/*前景画像の読み込み*/
	foreImage = cv::imread(foreFilename, 1);
	
	/*前景画像のリサイズ（縦と横の長いほうが100になるように縦横比を保持したままリサイズ）*/
	double resizePersent;//倍率

	if (foreImage.cols > foreImage.rows)
		resizePersent =(double) MAX_FOREIMAGE_SIZE / foreImage.cols;
	else 
		resizePersent = (double)MAX_FOREIMAGE_SIZE / foreImage.rows;

	cv::resize(foreImage, resizeforeImage, cv::Size(foreImage.cols * resizePersent, foreImage.rows * resizePersent),cv::INTER_CUBIC);//リサイズ


	/*背景画像の読み込み*/
	cv::Mat defaultImage;		//読み込み背景画像格納用Mat
	cv::Mat resizeImage;		//リサイズした背景画像格納用Mat

	/*ファイルディレクトリ走査*/
	for (const fs::directory_entry& entry : fs::directory_iterator(rel_p)) {
		fs::path p = entry;
		if (p.extension() == ".jpg") {
		//画像ファイルであったら読み込む
			const std::string backPath = p.string();

			/*画像読み込み*/
			defaultImage = cv::imread(backPath);
			/*画像のリサイズ*/
			cv::resize(defaultImage, resizeImage, cv::Size(600, 400));
			/*背景画像格納用vectorに保存*/
			backgroundImage.push_back(resizeImage.clone());
		}

		if (backgroundImage.empty()) {
		//背景画像がなかった場合
			cout << "フォルダ内に画像ファイルがありません" << endl;
			return -1;
		}
		
	}
	/*背景画像数の取得*/
	backImgNum = backgroundImage.size();


	while (true) {
		//フレーム着色
		frame = cv::Scalar(49, 52, 49);

		//マウス位置の決定
		cv::Rect rectangle(500, 150, 600, 400);
		int status = cvui::iarea(rectangle.x, rectangle.y, rectangle.width, rectangle.height);

		ImageX = cvui::mouse().x - rectangle.x;
		ImageY = cvui::mouse().y - rectangle.y;

		cvui::beginColumn(frame, 100, 50, 100, 200, 20);

		/* generateボタン
		*　　画像合成を行う
		*/
		if (cvui::button(100, 30, "Genarate") && !generate) {

			//乱数作成用
			std::random_device rnd;

			std::mt19937 mt(rnd());     //  メルセンヌ・ツイスタの32ビット版、引数は初期シード値


			std::uniform_int_distribution<int> back(0, backImgNum-1);			//背景画像用乱数
			std::uniform_real_distribution<double> magnificant(0.5, 3.0);		//拡大収縮用乱数
			std::uniform_int_distribution<int> degree(0, 360);					//回転角度用乱数
			std::uniform_int_distribution<int> mode(0, 10);						//透視変換用乱数
			std::uniform_real_distribution<double>xMove(0.0, 500.0);			//ｘ軸方向移動用乱数
			std::uniform_real_distribution<double>yMove(0.0, 300.0);			//ｙ軸方向移動用乱数

			cv::Point2f dstPoint[4];				//台形変換後の座標格納用

			int xMergin, yMergin, x0, x1, y0, y1;	//台形変換用変数

			double sizePercent, colSize, rowSize;	//拡大収縮処理用変数

			cv::Mat targetImage,					//拡大収縮後の画像格納用
					selectImage,					//生成された画像格納用
					perspectiveMat,					//透視変換用の行列格納用
					rotationMat;					//回転行列格納用

			cv::Mat unitMatrix = (cv::Mat_<double>(3, 3) << 1, 0, 0, 0, 1, 0, 0, 0, 1);		//単位行列の作成


			for (int i = 0; i < generateImgNum; i++)
			{
				/*背景画像の決定*/
				selectImage = backgroundImage[back(mt)].clone();
				
				/*拡大・収縮処理*/
				sizePercent = magnificant(mt);						//倍率の設定
				colSize = resizeforeImage.cols * sizePercent;		//処理後の横幅
				rowSize = resizeforeImage.rows * sizePercent;		//処理後の縦幅

				cv::resize(resizeforeImage, targetImage, cv::Size(colSize, rowSize));
				

				//台形化処理を行う場合
				if (checkTrapezoidalization) {
					/*台形化処理（透視変換行列）*/
					x0 = targetImage.cols / 4;			//前景画像の台形変換前の４点の座標を格納
					x1 = (targetImage.cols / 4) * 3;
					y0 = targetImage.rows / 4;
					y1 = (targetImage.rows / 4) * 3;

					cv::Point2f srcPoint[] = {
					cv::Point(x0,y0),
					cv::Point(x0,y1),
					cv::Point(x1,y1),
					cv::Point(x1,y0)
					};

					//乱数によりモードを決めて台形化処理を行う
					switch (mode(mt)) {

					case 0://上短下長の台形
						xMergin = targetImage.cols / 10;
						yMergin = targetImage.rows / 10;
						dstPoint[0] = cv::Point(x0 + xMergin, y0 + yMergin);
						dstPoint[1] = srcPoint[1];
						dstPoint[2] = srcPoint[2];
						dstPoint[3] = cv::Point(x1 - xMergin, y0 + yMergin);

						break;

					case 1://右短左長の台形
						xMergin = targetImage.cols / 10;
						yMergin = targetImage.rows / 10;
						dstPoint[0] = srcPoint[0];
						dstPoint[1] = srcPoint[1];
						dstPoint[2] = cv::Point(x1 - xMergin, y1 - yMergin);
						dstPoint[3] = cv::Point(x1 - xMergin, y0 + yMergin);

						break;

					case 2://平行四辺形
						xMergin = targetImage.cols / 10;
						yMergin = targetImage.rows / 10;
						dstPoint[0] = cv::Point(x0 + xMergin, y0 + yMergin);
						dstPoint[1] = srcPoint[1];
						dstPoint[2] = cv::Point(x1 - xMergin, y1 - yMergin);
						dstPoint[3] = srcPoint[3];
						break;

					case 3://上長下短の台形
						xMergin = targetImage.cols / 10;
						yMergin = targetImage.rows / 10;
						dstPoint[0] = srcPoint[0];
						dstPoint[1] = cv::Point(x0 + xMergin, y1 - yMergin);
						dstPoint[2] = cv::Point(x1 - xMergin, y1 - yMergin);
						dstPoint[3] = srcPoint[3];
						break;

					case 4://右短左長の台形
						xMergin = targetImage.cols / 10;
						yMergin = targetImage.rows / 10;
						dstPoint[0] = cv::Point(x0 + xMergin, y0 + yMergin);
						dstPoint[1] = cv::Point(x0 + xMergin, y1 - yMergin);
						dstPoint[2] = srcPoint[2];
						dstPoint[3] = srcPoint[3];
						break;

					case 5://平行四辺形
						xMergin = targetImage.cols / 10;
						yMergin = targetImage.rows / 10;
						dstPoint[0] = srcPoint[0];
						dstPoint[1] = cv::Point(x0 + xMergin, y1 - yMergin);
						dstPoint[2] = srcPoint[2];
						dstPoint[3] = cv::Point(x1 - xMergin, y0 + yMergin);
						break;

					default://台形変換を行わない場合
						dstPoint[0] = srcPoint[0];
						dstPoint[1] = srcPoint[1];
						dstPoint[2] = srcPoint[2];
						dstPoint[3] = srcPoint[3];
					}

					perspectiveMat = getPerspectiveTransform(srcPoint, dstPoint);//台形化処理透視変換行列の作成
				}
				else {
					perspectiveMat = unitMatrix.clone();
				}
				

				//回転処理を行う場合
				if (checkRotation) {
					/*回転処理（アフィン変換行列）*/
					cv::Point2d ctr(targetImage.cols / 2, targetImage.rows / 2);//前景画像の回転中心の設定
					rotationMat = cv::getRotationMatrix2D(ctr, degree(mt), 1.0);//回転行列の作成

					cv::Mat calcMat = (cv::Mat_<double>(1, 3) << 0.0, 0.0, 1.0);	//合成用
					rotationMat.push_back(calcMat);	//回転行列を３×３に拡張
				}
				else {
					rotationMat = unitMatrix.clone();
				}

								
				/*平行移動処理（アフィン変換行列）*/
				std::uniform_real_distribution<double>xMove(0.0, 600 - colSize);	//横方向移動用乱数
				std::uniform_real_distribution<double>yMove(0.0, 400 - rowSize);	//縦方向移動用乱数

				cv::Mat shiftMat = (cv::Mat_<double>(3, 3) << 1, 0, xMove(mt), 0, 1, yMove(mt), 0, 0, 1);	//平行移動行列の作成
				
				
				/*アフィン変換行列と透視変換行列の合成（掛け算）*/
				cv::Mat convertionMat =  shiftMat * rotationMat * perspectiveMat;

				/*前景画像の４点の出力*/
				double m_coordinate[4][3] = {	//拡大後の前景画像の初期座標
						{0.0 ,0.0,1},
						{0.0, targetImage.rows,1},
						{targetImage.cols ,targetImage.rows,1},
						{targetImage.cols ,0.0,1}
				};

				/*各座標における回転の実行*/
				vector <double>m_code;				//座標をいれるベクトル
				double xCoordinate, yCoordinate;	//座標をいれる変数
				bool boundary = TRUE;				//境界判断用変数
				for (int j = 0; j < 4; j++) {
					cv::Mat m_coordinateMat = (cv::Mat_<double>(3, 1) << m_coordinate[j][0], m_coordinate[j][1], m_coordinate[j][2]);
					cv::Mat rotation = convertionMat * m_coordinateMat;

					xCoordinate = rotation.at<double>(0, 0) / rotation.at<double>(2, 0);
					yCoordinate = rotation.at<double>(1, 0) / rotation.at<double>(2, 0);

					if (xCoordinate < 0 || xCoordinate>600 || yCoordinate < 0 || yCoordinate>400) {
						//境界値判断
						boundary = FALSE;
					}

					m_code.push_back(xCoordinate);
					m_code.push_back(yCoordinate);

				}

				/*境界値判断*/
				if (!boundary) {
					//一つでも頂点が背景画像内になかった場合
					//画像と画像情報を保存せずもう一度生成を行う
					i--;
					continue;
				}
				coordinate.push_back(m_code);

				/* 前景画像背景画像の合成*/
				cv::warpPerspective(targetImage, selectImage, convertionMat, cv::Size(600, 400), cv::INTER_LINEAR, cv::BORDER_TRANSPARENT);
				generateImage.push_back(selectImage.clone());
			}

			generate = true;
		}


		/*　台形化チェックボックス*/
		cvui::checkbox("Trapezoidalization", &checkTrapezoidalization);

		/*　回転処理化チェックボックス　*/
		cvui::checkbox("Rotation", &checkRotation);


		/*saveボタン
		*	 画像と画像情報の入ったファイルを保存する．
		*/
		if (cvui::button(100, 30, "save")&&generate) {
			int num;
			string str_buf;
			string str_conma_buf;
			string output_csv_file_path = "outImage.csv";

			// 書き込むcsvファイルを開く(std::ofstreamのコンストラクタで開く)
			std::ofstream ofs_csv_file(output_csv_file_path);
			if (!ofs_csv_file) {
					std::cout << "ファイルが開けませんでした。" << std::endl;
					return 0;
			}

			try {
			//例外処理
				do {
					std::cout << "先頭ファイル番号を入力してください" << std::endl;
					std::cin >> num;
				} while (num > 9999 || num < 0);
			}
			catch (...) {
				std::cout << "数字で入力してください" << std::endl;

			}
			//画像と画像情報の入ったファイルの保存
			for (int i = 0; i < generateImgNum; i++) {
				//画像
				std::ostringstream oss;
				oss << std::setfill('0') << std::setw(4) << num;
				cv::imwrite("output" + oss.str() + ".jpg", generateImage[i]);
				//画像情報
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
			//終了の告知
			cout << "end";
		}


		/*endボタン
		*	終了する
		*/
		if (cvui::button(100, 30, "end")) {
			break;
		}

		cvui::endColumn();

		cvui::beginColumn(frame,500, 50, 50);

		/*deleteボタン（生成終了後のみ押せる）
		*	データを削除する
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

		/*画像指定用トラックバー*/
		cvui::trackbar(500, &ImageNum, 0, generateImgNum - 1);

		if (cvui::button(50, 30, "+")&&ImageNum!=GENERATE_NUMBER) {
			ImageNum ++;
		}

		cvui::endRow();

		/*画像領域のマウス位置表示*/
		if (status == cvui::OUT || generate == false) {
			cvui::printf("x:-- y:-- value:--", frame);
		}
		else
		{
			cvui::printf("x:%d y%d value:%d", ImageX, ImageY, generateImage[ImageNum].at<uchar>(ImageY, ImageX), frame);
		}

		cvui::endColumn();

		/*生成画像の表示*/
		if (generate == true) {
			//生成が終わった場合
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
	/*OpenFile関数
		説明：ファイルを開く
		引数： filestring　ファイル名を格納する文字列
		戻り値：true:ファイルが指定された false:キャンセル
	*/


	wchar_t wpath[MAX_PATH], wfilename[MAX_PATH];
	wpath[0] = L'\0';
		
	OPENFILENAME ofn;

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = wpath;					
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = wfilename;			//選択されたファイル名
	ofn.nMaxFileTitle = MAX_PATH;
	
	ofn.lpstrFilter = L"JPEG(*.jpg)\0*.jpg\0\0"; //ファイル種類フィルタ
	ofn.lpstrTitle = L"画像ファイルを選択してください"; //ダイアログボックスのタイトル
	
	if (::GetOpenFileName(&ofn) == FALSE) return false;
	char filename[MAX_PATH * 2];
	size_t len;
	wcstombs_s(&len, filename, sizeof(filename), wfilename, _TRUNCATE);
	filestring = filename;

	return true;
}
