//
// Created by 박규열 on 2019-08-23.
//

//#define OPENCV_YOLO

#ifndef MISSIONTRIGGER2_MISSIONTRIGGER_H
#define MISSIONTRIGGER2_MISSIONTRIGGER_H

#ifdef OPENCV_YOLO
#include <opencv2/opencv.hpp>
using namespace cv;
#endif

#include <iostream>
#include <string>
#include <list>
#include <cmath>
#include <algorithm>

using namespace std;


// ############################## 사전 정의해야 할 값들 ########################################
// 미션 목록 (12개)
const string missions[] = { "intersectionState", "intersectionDistance", "crossWalk", "parking", "busLane", "staticObstacle", "dynamicObstacle", "kidSafe", "bust" };
// 전체 오브젝트 (27개) (아래에 종류별로 분류해 주어야 함)
const string objects[] = { "trafficGreen", "trafficLightGreenLeft", "trafficLightRed", "trafficLightRedLeft",
						  "trafficLightRedYellow", "trafficLightYellow", "straight", "construction", "crossWalk", "bicycle",
						  "bust", "parking", "busArrowDown", "left", "right", "kidSafe", "kidSafeExit", "roadDiamond",
						  "roadArrowStraight", "roadArrowStraightLeft", "roadArrowStraightRight", "roadArrowLeft",
						  "roadArrowRight", "roadArrowLeftRight", "roadArrowThree", "bustReal", "crossWalkReal" };
// 신호등 (6개)
const string lights[] = { "trafficGreen", "trafficLightGreenLeft", "trafficLightRed", "trafficLightRedLeft",
										   "trafficLightRedYellow", "trafficLightYellow" };
// 표지판 (11개)
const string signs[] = { "straight", "construction", "crossWalk", "bicycle", "bust", "parking", "busArrowDown", "left",
										  "right", "kidSafe", "kidSafeExit" };
// 노면표시 및 기타 (실제 오브젝트) (10개)
const string marks[] = { "roadDiamond", "roadArrowStraight", "roadArrowStraightLeft", "roadArrowStraightRight",
										  "roadArrowLeft", "roadArrowRight", "roadArrowLeftRight", "roadArrowThree", "bustReal", "crossWalkReal" };

const string arrows[] = { "roadArrowStraight", "roadArrowStraightLeft", "roadArrowStraightRight", "roadArrowLeft",
						 "roadArrowRight", "roadArrowLeftRight", "roadArrowThree" };
const string extras[] = { "straight", "construction", "crossWalk", "bicycle", "bust", "parking", "busArrowDown", "left",
						 "right", "kidSafe", "kidSafeExit", "bustReal", "crossWalkReal" };

// 오브젝트 및 미션 개수
//// 분모 뭐야 왜 나눠주는거야
const int len_objects = sizeof(objects) / sizeof(objects[0]);
const int len_missions = sizeof(missions) / sizeof(missions[0]);

//// 이건 뭐지??
int missionCount[3][len_objects] = { {}, {}, {} };

// ##########################################################################################



class Object {
private:
	string name;
	int distance;
	int recog; // 해당 오브젝트가 현재 프레임에서 인식이 되었는지 (인식되면 1, 인식 안되면 0)
	int count; // 인식된 프레임 개수 (최대 10, 5 이상일 경우에만 미션 업데이트에 사용)

	float x;
	float y;

public:
	Object(const string &_name, int _distance, float _x, float _y) {
		this->name = _name;
		this->distance = _distance; this->x = _x; this->y = _y;
		this->count = 1; this->recog = 1;
	}

	// 오브젝트 인식 (putbox)
	void recognized(int _distance, float _x, float _y) {
		this->distance = _distance;
		this->x = _x; this->y = _y;
		count = count < 10 ? count + 1 : 10;
		this->recog = 1;
	}

	// 오브젝트 인식 초기화 (update)
	void unrecognized() {
		count = recog == 1 ? count : (count > 0 ? count - 1 : 0);
		this->recog = 0;
	}

	string getName() { return name; }
	int getDistance() { return distance; }
	int getCount() { return count; }
	float getx() { return x; }
	float gety() { return y; }

};



class MissionContainer {
private:
	list<Object*> recognized_Objects;
	int* recognized_Missions;

	int direction = -1; // 교차로 진행방향. -1: 교차로 아님, 1: 교차로 좌회전, 2: 교차로 직진, 3: 교차로 우회전
	int distanceStopline = -1; // 교차로 정지선까지의 거리, -1: 인식안됨, 0 >=: 인식된 거리


	// 오브젝트까지의 거리를 반환
	static int checkDistance(const string &name, float area, float top, float bot) {

		int distance = -1;

		// 신호등의 높이를 신호등까지의 거리로 변환
		//// 내 생각엔  이상으로 바꿔줘야 될듯
		for (const auto &n : lights) {
			if (name == n) {
				if (top < 0.5) {
					distance = static_cast<int>(10 * ((0.9 - top) / 0.5)); // 높이를 거리로 변환
					distance = distance >= 1 ? distance : 0; // 거리가 1미터 이하면 거리 0으로 변환
				} return distance;
			}
		}

		// 로드마크의 높이를 로드마크까지의 거리로 변환
		//// 이것도  이하로 바꾸거나 해야될듯
		for (const auto &n : marks) {
			if (name == n) {
				if (bot > 0.5) {
					distance = static_cast<int>(10 * ((bot - 0.2) / 0.5)); // 높이를 거리로 변환
					distance = distance >= 1 ? distance : 0; // 거리가 1미터 이하면 거리 0으로 변환
				} return distance;
			}
		}

		// 표지판의 넓이를 표지판까지의 거리로 변환
		for (const auto &n : signs) {
			if (name == n) {
				if (n == "crossWalk" && area >= 0.0002) distance = static_cast<int>(10 * sqrt(0.0002) * (1 / sqrt(area)));
				return distance;
			}
			else if (name == n) {
				if (n == "bust" && area >= 0.0002) distance = static_cast<int>(10 * sqrt(0.0002) * (1 / sqrt(area)));
				return distance;
			}
			else if (name == n) {
				if (n == "construction" && area >= 0.0002) {
					distance = static_cast<int>(10 * sqrt(0.0002) * (1 / sqrt(area)));
					distance = distance >= 3 ? distance : 0; // 거리가 3미터 이하면 거리 0으로 변환
				} return distance;
			}
			else if (name == n) {
				if (n == "bicycle" && area >= 0.0002) {
					distance = static_cast<int>(10 * sqrt(0.0002) * (1 / sqrt(area)));
					distance = distance >= 3 ? distance : 0; // 거리가 3미터 이하면 거리 0으로 변환
				} return distance;
			}
			else if (name == n) {
				if (n == "parking" && area >= 0.0002) {
					distance = static_cast<int>(10 * sqrt(0.0002) * (1 / sqrt(area)));
					distance = distance >= 3 ? distance : 0; // 거리가 3미터 이하면 거리 0으로 변환
				} return distance;
			}
			else if (name == n) {
				if (n == "busArrowDown" && area >= 0.0002) {
					distance = static_cast<int>(10 * sqrt(0.0002) * (1 / sqrt(area)));
					distance = distance >= 3 ? distance : 0; // 거리가 3미터 이하면 거리 0으로 변환
				} return distance;
			}
			else if (name == n) {
				if (n == "kidSafe" && area >= 0.0002) {
					distance = static_cast<int>(10 * sqrt(0.0002) * (1 / sqrt(area)));
					distance = distance >= 3 ? distance : 0; // 거리가 3미터 이하면 거리 0으로 변환
				} return distance;
			}
			else if (name == n) {
				if (n == "kidSafeExit" && area >= 0.0002) {
					distance = static_cast<int>(10 * sqrt(0.0002) * (1 / sqrt(area)));
					distance = distance >= 10 ? distance : 0; // 거리가 3미터 이하면 거리 0으로 변환
				} return distance;
			}
			else {
				if (area >= 0.0002) {
					distance = static_cast<int>(10 * sqrt(0.0002) * (1 / sqrt(area)));
					distance = distance >= 3 ? distance : 0; // 거리가 3미터 이하면 거리 0으로 변환
				} return distance;
			}
		}

		// 해당 오브젝트 이름이 인식되지 않은 경우 -1 반환
		return -1;
	}



	// 유효한 미션을 업데이트한다
	void updateMissions() {

		list<Object *>::iterator iter;

		// @@ (일반주행) 현재 교차로 인식(또는 진입) 상태가 아니면, 교차로가 인식되었는지 검사한다.
		if (recognized_Missions[1] == -1) {
			// 현재 신호등이 인식되고 카운트가 5 이상이면, 해당 신호등 신호로 미션 업데이트 (가장 가까운 신호등으로 업데이트)
			for (const auto &n : lights) {
				for (iter = recognized_Objects.begin(); iter != recognized_Objects.end(); ++iter) {
					if ((*iter)->getCount() >= 5 && (*iter)->getName() == n && (*iter)->getDistance() <= recognized_Missions[1]) {

						// 교차로 상황이 아닐 때 처음 인식하는 신호등이면, 교차로 진행방향을 받아온다
						if (direction == -1) direction = getDirection();
						// 교차로 거리 업데이트
						recognized_Missions[1] = (*iter)->getDistance();

						// 좌회전인 경우 (1) (1번 주행상황)
						if (direction == 1) {
							if (n == "trafficLightRed" || n == "trafficLightRedYellow" || n == "trafficLightYellow")
								recognized_Missions[0] = 0; // 빨간불 정지
							else if (n == "trafficLightGreenLeft" || n == "trafficLightRedLeft")
								recognized_Missions[0] = direction; // 좌회전
							else
								recognized_Missions[0] = 0; // 초록불은 들어왔지만, 좌회전 신호 대기
						}
						// 직진인 경우 (2) (2번 주행상황)
						else if (direction == 2) {
							if (n == "trafficLightRed" || n == "trafficLightRedLeft" || n == "trafficLightRedYellow" || n == "trafficLightYellow")
								recognized_Missions[0] = 0; // 빨간불 정지
							else recognized_Missions[0] = direction; // 직진
						}
						// 우회전인 경우 (3) (3번 주행상황)
						else if (direction == 3) {
							if (n == "trafficLightRed" || n == "trafficLightRedLeft" || n == "trafficLightRedYellow" || n == "trafficLightYellow")
								recognized_Missions[0] = 0; // 빨간불 정지
							else recognized_Missions[0] = direction; // 우회전
						}
					}
				}
			}
			// 신호등 교차로가 인식되지 않았을 경우, 로드마크로 교차로 여부 확인
			if (recognized_Missions[1] == -1) {
				for (const auto &n : arrows) {
					for (iter = recognized_Objects.begin(); iter != recognized_Objects.end(); ++iter) {
						if ((*iter)->getCount() >= 5 && (*iter)->getName() == n && (*iter)->getDistance() <= recognized_Missions[1]) {
							if (n == "roadArrowThree" || n == "roadArrowStraightRight" || n == "roadArrowLeftRight") {
								// 교차로 상황이 아닐 때 처음 인식하는 신호등이면, 교차로 진행방향을 받아온다
								if (direction == -1) direction = getDirection();
								// 교차로 거리 업데이트
								recognized_Missions[1] = (*iter)->getDistance();
								// 교차로 상황 업데이트
								recognized_Missions[0] = direction;
							}
						}
					}
				}
			}
		}
		// @@ (교차로 진입 대기) 교차로가 인식되고 있는 상태인 경우, 교차로에 진입할 때까지 거리를 업데이트한다.
		else if (recognized_Missions[1] > 0) {
			// 정지선 인식이 되지 않고 있는 상태일 경우, 신호등과 로드마크로 교차로까지의 거리를 업데이트하고, 로드마크 인식 시 정지선을 검사한다
			if (distanceStopline == -1) {
				// 신호등으로 거리 업데이트
				for (const auto &n : lights) {
					for (iter = recognized_Objects.begin(); iter != recognized_Objects.end() && recognized_Missions[1] > 5; ++iter) {
						if ((*iter)->getCount() >= 5 && (*iter)->getName() == n && (*iter)->getDistance() <= recognized_Missions[1])
							recognized_Missions[1] = (*iter)->getDistance();
					}
				}
				// 로드마크로 거리 추가 업데이트, 정지선 검사 (신호등보다 로드마크에 거리 우선순위)
				for (const auto &n : marks) {
					for (iter = recognized_Objects.begin(); iter != recognized_Objects.end() && recognized_Missions[1] > 5; ++iter) {
						if ((*iter)->getCount() >= 5 && (*iter)->getName() == n && (*iter)->getDistance() <= recognized_Missions[1]) {
							if (n == "roadArrowThree" || n == "roadArrowStraightRight" || n == "roadArrowLeftRight" || n == "crossWalkReal")
								recognized_Missions[1] = (*iter)->getDistance();
							distanceStopline = findStopline(); // 정지선 검사 (인식 실패시 -1 반환)
							if (distanceStopline != -1) recognized_Missions[1] = distanceStopline; // 인식된 경우 거리 업데이트
						}
					}
				}
				// 교차로까지의 거리 5미터 이내 진입 시, 실시간으로 정지선 검사
				if (distanceStopline == -1 && recognized_Missions[1] <= 5) {
					distanceStopline = findStopline(); // 정지선 인식 (인식 실패시 -1 반환)
					if (distanceStopline != -1) recognized_Missions[1] = distanceStopline; // 인식된 경우 거리 업데이트
				}
			}
			// 정지선이 인식되고 있는 상태일 경우 정지선으로 교차로까지의 거리를 업데이트한다
			else if (distanceStopline != -1) {
				distanceStopline = findStopline(); // 정지선 인식 (인식 실패시 -1 반환)
				if (distanceStopline != -1) recognized_Missions[1] = distanceStopline; // 인식된 경우 거리 업데이트
				else if (distanceStopline == -1) distanceStopline = recognized_Missions[1]; // 인식 안된 경우 이전 거리를 사용
			}
			// 현재 인식되고 있는 신호등으로 미션 실시간 업데이트
			for (const auto &n : lights) {
				for (iter = recognized_Objects.begin(); iter != recognized_Objects.end(); ++iter) {
					if ((*iter)->getCount() >= 5 && (*iter)->getName() == n && (*iter)->getDistance() <= recognized_Missions[1]) {
						// 좌회전인 경우 (1) (1번 주행상황)
						if (direction == 1) {
							if (n == "trafficLightRed" || n == "trafficLightRedYellow" || n == "trafficLightYellow")
								recognized_Missions[0] = 0; // 빨간불 정지
							else if (n == "trafficLightGreenLeft" || n == "trafficLightRedLeft")
								recognized_Missions[0] = direction; // 좌회전
							else
								recognized_Missions[0] = 0; // 초록불은 들어왔지만, 좌회전 신호 대기
						}
						// 직진인 경우 (2) (2번 주행상황)
						else if (direction == 2) {
							if (n == "trafficLightRed" || n == "trafficLightRedLeft" || n == "trafficLightRedYellow" || n == "trafficLightYellow")
								recognized_Missions[0] = 0; // 빨간불 정지
							else recognized_Missions[0] = direction; // 직진
						}
						// 우회전인 경우 (3) (3번 주행상황)
						else if (direction == 3) {
							if (n == "trafficLightRed" || n == "trafficLightRedLeft" || n == "trafficLightRedYellow" || n == "trafficLightYellow")
								recognized_Missions[0] = 0; // 빨간불 정지
							else recognized_Missions[0] = direction; // 우회전
						}
					}
				}
			}
		}
		// @@ (교차로 주행) 교차로에 진입해 있는 상태인 경우, 로드마크 또는 횡단보도 진입 시 교차로 상태를 해제한다.
		else if (recognized_Missions[1] == 0) {
			// 로드마크가 인식되면 교차로 상황 해제
			for (const auto &n : marks) {
				for (iter = recognized_Objects.begin(); iter != recognized_Objects.end(); ++iter) {
					if ((*iter)->getCount() >= 5 && (*iter)->getName() == n && (*iter)->getDistance() <= 3) {
						recognized_Missions[0] = -1; recognized_Missions[1] = -1; // 교차로 상황 해제
						direction = -1; // 교차로 방향 제거
						distanceStopline = -1; // 정지선 거리 초기화
					}
				}
			}
		}

		// 기타 표지판 미션
		for (const auto &n : extras) {
			for (iter = recognized_Objects.begin(); iter != recognized_Objects.end(); ++iter) {
				if ((*iter)->getCount() >= 5 && (*iter)->getName() == n) {
					// 정적장애물 미션
					if (n == "construction") {
						for (int i = 0; i < len_missions; i++) {
							if (missions[i] == "staticObstacle") {
								recognized_Missions[i] = (*iter)->getDistance(); break;
							}
						}
					}
					// 동적장애물 미션
					else if (n == "bicycle") {
						for (int i = 0; i < len_missions; i++) {
							if (missions[i] == "dynamicObstacle") {
								recognized_Missions[i] = (*iter)->getDistance(); break;
							}
						}
					}
					// 횡단보도 인식
					else if (n == "crossWalk" || n == "crossWalkReal") {
						for (int i = 0; i < len_missions; i++) {
							if (missions[i] == "crossWalk") {
								recognized_Missions[i] = (*iter)->getDistance(); break;
							}
						}
					}
					// 과속방지턱 인식
					else if (n == "bust" || n == "bustReal") {
						for (int i = 0; i < len_missions; i++) {
							if (missions[i] == "bust") {
								recognized_Missions[i] = (*iter)->getDistance(); break;
							}
						}
					}
					// 주차장 인식
					else if (n == "parking") {
						for (int i = 0; i < len_missions; i++) {
							if (missions[i] == "parking") {
								recognized_Missions[i] = (*iter)->getDistance(); break;
							}
						}
					}
					// 버스전용차로 주의
					else if (n == "busArrowDown") {
						for (int i = 0; i < len_missions; i++) {
							if (missions[i] == "busLane") {
								recognized_Missions[i] = (*iter)->getDistance(); break;
							}
						}
					}
					// 어린이보호구역 진입
					else if (n == "kidSafe") {
						for (int i = 0; i < len_missions; i++) {
							if (missions[i] == "kidSafe") {
								recognized_Missions[i] = (*iter)->getDistance(); break;
							}
						}
					}
					// 어린이보호구역 탈출
					else if (n == "kidSafeExit") {
						for (int i = 0; i < len_missions; i++) {
							if (missions[i] == "kidSafe" && recognized_Missions[i] <= 5) {
								recognized_Missions[i] = -1; break;
							}
						}
					}
				}
			}
		}
	}



	int getDirection() {
		list


	}




public:
	MissionContainer() { recognized_Missions = new int[len_missions]; fill_n(recognized_Missions, len_missions, -1); }
	int* getMissions() { return recognized_Missions; } // 인식된 미션 리스트를 반환한다


	// 인식된 박스를 넣어 일정 크기 이상이면 인식해서 리스트에 넣는다.
	void putbox(const string &name, float x, float y, float width, float height) {

		// 인식된 객체의 거리와 넓이 계산
		float area = width * height;
		int distance = checkDistance(name, area, y, y);
		// 객체의 크기가 너무 작으면(너무 멀리 있으면) 해당 객체를 무시한다
		if (distance == -1) return;

		// 이미 인식되고 있는 해당 클래스 객체가 있으면, 위치를 비교해 동일 물체이면 해당 포인터 반환 (임계값 정해줘야 함)
		Object* findedObject = nullptr;
		list<Object*>::iterator iter;
		for (iter = recognized_Objects.begin(); iter != recognized_Objects.end(); ++iter) {
			if ((*iter)->getName() == name) {
				if (abs((*iter)->getx() - x) < 0.05 && abs((*iter)->gety() - y) < 0.05)
					findedObject = *iter;
			}
		}

		// 인식되고 있는 오브젝트가 없으면 리스트에 새로 넣는다 / 있으면 해당 객체 카운트 업데이트
		if (findedObject == nullptr)
			recognized_Objects.push_back(new Object(name, distance, x, y));
		else findedObject->recognized(distance, x, y);

	}


	// 업데이트
	void update() {
		cout << "update &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << "\n";
		list<Object*>::iterator iter;

		// 해당 프레임에서 인식 안된 물체들 카운트 줄이고, 카운트가 0이 되면 삭제한다
		for (iter = recognized_Objects.begin(); iter != recognized_Objects.end(); ++iter) {
			(*iter)->unrecognized(); // 해당 프레임 상태가 해제되고, 카운트를 업데이트한다
			cout << "******** update *********" << endl;
			cout << " name : " << (*iter)->getName() << " / count : " << (*iter)->getCount() << endl;
			// 카운트가 0이 되면 삭제
			if ((*iter)->getCount() == 0) {
				cout << "@Erased (low count) : " << (*iter)->getName() << "\n";
				delete *iter;
				iter = recognized_Objects.erase(iter); // 카운트가 0이면 삭제..
				iter--;
			}
		}

		// 인식된 물체들로부터 미션 업데이트
		updateMissions();

		cout << "\n\n\n\n\n";
	}

#ifdef OPENCV_YOLO
	// 현재 인식된 미션에 대해 missions 폴더에 있는 해당 사진을 띄운다
	void showMissions(int waitTime) {
		int missionSize = recognized_Missions.size();
		Mat image;
		map<string, int>::iterator mapiter;
		string filename;
		string missionName;
		int danger;
		string ls[] = { "intersectionReady", "intersectionStraight", "intersectionLeft", "intersectionRight", "intersectionStop",
					   "crossWalk", "parking", "busLane", "staticObstacle", "dynamicObstacle", "kidSafe", "bust" };

		if (missionSize != 0) {
			for (const auto & l : ls) {
				mapiter = recognized_Missions.find(l);
				if (mapiter != recognized_Missions.end()) {
					missionName = mapiter->first;
					danger = mapiter->second;
					filename = string("missions/") + missionName + ".png";
					image = imread(filename, IMREAD_COLOR);
					if (image.empty()) {
						cout << "Could not open folder(missions) image : " << filename << endl;
						continue;
					}
					namedWindow(missionName, WINDOW_AUTOSIZE);
					putText(image, string("danger = ") + to_string(danger), Point(10, 40), 2, 1.2, Scalar(255, 255, 255));
					imshow(missionName, image);
					waitKey(waitTime);
				}
				else { destroyWindow(l); }
			}
		}

	}


	// 현재 인식된 오브젝트에 대해 missions 폴더에 있는 해당 사진을 띄운다
	void showObjects(int waitTime) {
		int objectSize = activated_Objects.size();
		Mat image;
		map<string, int>::iterator mapiter;
		string filename;
		string objectName;
		int danger;

		string ls[] = { "straight", "construction", "crossWalk", "bicycle", "bust", "parking", "busArrowDown", "left",
					   "right", "trafficGreen", "trafficLightGreenLeft", "trafficLightRed", "trafficLightRedLeft",
					   "trafficLightRedYellow", "trafficLightYellow", "roadDiamond", "roadArrowStraight",
					   "roadArrowStraightLeft", "roadArrowStraightRight", "roadArrowLeft", "roadArrowRight",
					   "roadArrowLeftRight", "roadArrowThree", "bustReal", "crossWalkReal", "kidSafeSero", "kidSafeExit" };

		if (objectSize != 0) {
			for (const auto &l : ls) {
				mapiter = activated_Objects.find(l);
				if (mapiter != activated_Objects.end()) {
					objectName = mapiter->first;
					danger = mapiter->second;
					filename = string("missions/") + missionName + ".png";
					image = imread(filename, IMREAD_COLOR);
					if (image.empty()) {
						cout << "Could not open folder(objects) image : " << filename << endl;
						continue;
					}
					namedWindow(missionName, WINDOW_AUTOSIZE);
					putText(image, string("danger = ") + to_string(danger), Point(10, 40), 2, 1.2,
						Scalar(255, 255, 255));
					imshow(objectName, image);
					waitKey(waitTime);
				}
				else { destroyWindow(l); }
			}
		}

	}
#endif

};



#endif //MISSIONTRIGGER2_MISSIONTRIGGER_H