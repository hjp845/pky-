//
// Created by �ڱԿ� on 2019-08-23.
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


// ############################## ���� �����ؾ� �� ���� ########################################
// �̼� ��� (12��)
const string missions[] = { "intersectionState", "intersectionDistance", "crossWalk", "parking", "busLane", "staticObstacle", "dynamicObstacle", "kidSafe", "bust" };
// ��ü ������Ʈ (27��) (�Ʒ��� �������� �з��� �־�� ��)
const string objects[] = { "trafficGreen", "trafficLightGreenLeft", "trafficLightRed", "trafficLightRedLeft",
						  "trafficLightRedYellow", "trafficLightYellow", "straight", "construction", "crossWalk", "bicycle",
						  "bust", "parking", "busArrowDown", "left", "right", "kidSafe", "kidSafeExit", "roadDiamond",
						  "roadArrowStraight", "roadArrowStraightLeft", "roadArrowStraightRight", "roadArrowLeft",
						  "roadArrowRight", "roadArrowLeftRight", "roadArrowThree", "bustReal", "crossWalkReal" };
// ��ȣ�� (6��)
const string lights[] = { "trafficGreen", "trafficLightGreenLeft", "trafficLightRed", "trafficLightRedLeft",
										   "trafficLightRedYellow", "trafficLightYellow" };
// ǥ���� (11��)
const string signs[] = { "straight", "construction", "crossWalk", "bicycle", "bust", "parking", "busArrowDown", "left",
										  "right", "kidSafe", "kidSafeExit" };
// ���ǥ�� �� ��Ÿ (���� ������Ʈ) (10��)
const string marks[] = { "roadDiamond", "roadArrowStraight", "roadArrowStraightLeft", "roadArrowStraightRight",
										  "roadArrowLeft", "roadArrowRight", "roadArrowLeftRight", "roadArrowThree", "bustReal", "crossWalkReal" };

const string arrows[] = { "roadArrowStraight", "roadArrowStraightLeft", "roadArrowStraightRight", "roadArrowLeft",
						 "roadArrowRight", "roadArrowLeftRight", "roadArrowThree" };
const string extras[] = { "straight", "construction", "crossWalk", "bicycle", "bust", "parking", "busArrowDown", "left",
						 "right", "kidSafe", "kidSafeExit", "bustReal", "crossWalkReal" };

// ������Ʈ �� �̼� ����
//// �и� ���� �� �����ִ°ž�
const int len_objects = sizeof(objects) / sizeof(objects[0]);
const int len_missions = sizeof(missions) / sizeof(missions[0]);

//// �̰� ����??
int missionCount[3][len_objects] = { {}, {}, {} };

// ##########################################################################################



class Object {
private:
	string name;
	int distance;
	int recog; // �ش� ������Ʈ�� ���� �����ӿ��� �ν��� �Ǿ����� (�νĵǸ� 1, �ν� �ȵǸ� 0)
	int count; // �νĵ� ������ ���� (�ִ� 10, 5 �̻��� ��쿡�� �̼� ������Ʈ�� ���)

	float x;
	float y;

public:
	Object(const string &_name, int _distance, float _x, float _y) {
		this->name = _name;
		this->distance = _distance; this->x = _x; this->y = _y;
		this->count = 1; this->recog = 1;
	}

	// ������Ʈ �ν� (putbox)
	void recognized(int _distance, float _x, float _y) {
		this->distance = _distance;
		this->x = _x; this->y = _y;
		count = count < 10 ? count + 1 : 10;
		this->recog = 1;
	}

	// ������Ʈ �ν� �ʱ�ȭ (update)
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

	int direction = -1; // ������ �������. -1: ������ �ƴ�, 1: ������ ��ȸ��, 2: ������ ����, 3: ������ ��ȸ��
	int distanceStopline = -1; // ������ ������������ �Ÿ�, -1: �νľȵ�, 0 >=: �νĵ� �Ÿ�


	// ������Ʈ������ �Ÿ��� ��ȯ
	static int checkDistance(const string &name, float area, float top, float bot) {

		int distance = -1;

		// ��ȣ���� ���̸� ��ȣ������� �Ÿ��� ��ȯ
		//// �� ������  �̻����� �ٲ���� �ɵ�
		for (const auto &n : lights) {
			if (name == n) {
				if (top < 0.5) {
					distance = static_cast<int>(10 * ((0.9 - top) / 0.5)); // ���̸� �Ÿ��� ��ȯ
					distance = distance >= 1 ? distance : 0; // �Ÿ��� 1���� ���ϸ� �Ÿ� 0���� ��ȯ
				} return distance;
			}
		}

		// �ε帶ũ�� ���̸� �ε帶ũ������ �Ÿ��� ��ȯ
		//// �̰͵�  ���Ϸ� �ٲٰų� �ؾߵɵ�
		for (const auto &n : marks) {
			if (name == n) {
				if (bot > 0.5) {
					distance = static_cast<int>(10 * ((bot - 0.2) / 0.5)); // ���̸� �Ÿ��� ��ȯ
					distance = distance >= 1 ? distance : 0; // �Ÿ��� 1���� ���ϸ� �Ÿ� 0���� ��ȯ
				} return distance;
			}
		}

		// ǥ������ ���̸� ǥ���Ǳ����� �Ÿ��� ��ȯ
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
					distance = distance >= 3 ? distance : 0; // �Ÿ��� 3���� ���ϸ� �Ÿ� 0���� ��ȯ
				} return distance;
			}
			else if (name == n) {
				if (n == "bicycle" && area >= 0.0002) {
					distance = static_cast<int>(10 * sqrt(0.0002) * (1 / sqrt(area)));
					distance = distance >= 3 ? distance : 0; // �Ÿ��� 3���� ���ϸ� �Ÿ� 0���� ��ȯ
				} return distance;
			}
			else if (name == n) {
				if (n == "parking" && area >= 0.0002) {
					distance = static_cast<int>(10 * sqrt(0.0002) * (1 / sqrt(area)));
					distance = distance >= 3 ? distance : 0; // �Ÿ��� 3���� ���ϸ� �Ÿ� 0���� ��ȯ
				} return distance;
			}
			else if (name == n) {
				if (n == "busArrowDown" && area >= 0.0002) {
					distance = static_cast<int>(10 * sqrt(0.0002) * (1 / sqrt(area)));
					distance = distance >= 3 ? distance : 0; // �Ÿ��� 3���� ���ϸ� �Ÿ� 0���� ��ȯ
				} return distance;
			}
			else if (name == n) {
				if (n == "kidSafe" && area >= 0.0002) {
					distance = static_cast<int>(10 * sqrt(0.0002) * (1 / sqrt(area)));
					distance = distance >= 3 ? distance : 0; // �Ÿ��� 3���� ���ϸ� �Ÿ� 0���� ��ȯ
				} return distance;
			}
			else if (name == n) {
				if (n == "kidSafeExit" && area >= 0.0002) {
					distance = static_cast<int>(10 * sqrt(0.0002) * (1 / sqrt(area)));
					distance = distance >= 10 ? distance : 0; // �Ÿ��� 3���� ���ϸ� �Ÿ� 0���� ��ȯ
				} return distance;
			}
			else {
				if (area >= 0.0002) {
					distance = static_cast<int>(10 * sqrt(0.0002) * (1 / sqrt(area)));
					distance = distance >= 3 ? distance : 0; // �Ÿ��� 3���� ���ϸ� �Ÿ� 0���� ��ȯ
				} return distance;
			}
		}

		// �ش� ������Ʈ �̸��� �νĵ��� ���� ��� -1 ��ȯ
		return -1;
	}



	// ��ȿ�� �̼��� ������Ʈ�Ѵ�
	void updateMissions() {

		list<Object *>::iterator iter;

		// @@ (�Ϲ�����) ���� ������ �ν�(�Ǵ� ����) ���°� �ƴϸ�, �����ΰ� �νĵǾ����� �˻��Ѵ�.
		if (recognized_Missions[1] == -1) {
			// ���� ��ȣ���� �νĵǰ� ī��Ʈ�� 5 �̻��̸�, �ش� ��ȣ�� ��ȣ�� �̼� ������Ʈ (���� ����� ��ȣ������ ������Ʈ)
			for (const auto &n : lights) {
				for (iter = recognized_Objects.begin(); iter != recognized_Objects.end(); ++iter) {
					if ((*iter)->getCount() >= 5 && (*iter)->getName() == n && (*iter)->getDistance() <= recognized_Missions[1]) {

						// ������ ��Ȳ�� �ƴ� �� ó�� �ν��ϴ� ��ȣ���̸�, ������ ��������� �޾ƿ´�
						if (direction == -1) direction = getDirection();
						// ������ �Ÿ� ������Ʈ
						recognized_Missions[1] = (*iter)->getDistance();

						// ��ȸ���� ��� (1) (1�� �����Ȳ)
						if (direction == 1) {
							if (n == "trafficLightRed" || n == "trafficLightRedYellow" || n == "trafficLightYellow")
								recognized_Missions[0] = 0; // ������ ����
							else if (n == "trafficLightGreenLeft" || n == "trafficLightRedLeft")
								recognized_Missions[0] = direction; // ��ȸ��
							else
								recognized_Missions[0] = 0; // �ʷϺ��� ��������, ��ȸ�� ��ȣ ���
						}
						// ������ ��� (2) (2�� �����Ȳ)
						else if (direction == 2) {
							if (n == "trafficLightRed" || n == "trafficLightRedLeft" || n == "trafficLightRedYellow" || n == "trafficLightYellow")
								recognized_Missions[0] = 0; // ������ ����
							else recognized_Missions[0] = direction; // ����
						}
						// ��ȸ���� ��� (3) (3�� �����Ȳ)
						else if (direction == 3) {
							if (n == "trafficLightRed" || n == "trafficLightRedLeft" || n == "trafficLightRedYellow" || n == "trafficLightYellow")
								recognized_Missions[0] = 0; // ������ ����
							else recognized_Missions[0] = direction; // ��ȸ��
						}
					}
				}
			}
			// ��ȣ�� �����ΰ� �νĵ��� �ʾ��� ���, �ε帶ũ�� ������ ���� Ȯ��
			if (recognized_Missions[1] == -1) {
				for (const auto &n : arrows) {
					for (iter = recognized_Objects.begin(); iter != recognized_Objects.end(); ++iter) {
						if ((*iter)->getCount() >= 5 && (*iter)->getName() == n && (*iter)->getDistance() <= recognized_Missions[1]) {
							if (n == "roadArrowThree" || n == "roadArrowStraightRight" || n == "roadArrowLeftRight") {
								// ������ ��Ȳ�� �ƴ� �� ó�� �ν��ϴ� ��ȣ���̸�, ������ ��������� �޾ƿ´�
								if (direction == -1) direction = getDirection();
								// ������ �Ÿ� ������Ʈ
								recognized_Missions[1] = (*iter)->getDistance();
								// ������ ��Ȳ ������Ʈ
								recognized_Missions[0] = direction;
							}
						}
					}
				}
			}
		}
		// @@ (������ ���� ���) �����ΰ� �νĵǰ� �ִ� ������ ���, �����ο� ������ ������ �Ÿ��� ������Ʈ�Ѵ�.
		else if (recognized_Missions[1] > 0) {
			// ������ �ν��� ���� �ʰ� �ִ� ������ ���, ��ȣ��� �ε帶ũ�� �����α����� �Ÿ��� ������Ʈ�ϰ�, �ε帶ũ �ν� �� �������� �˻��Ѵ�
			if (distanceStopline == -1) {
				// ��ȣ������ �Ÿ� ������Ʈ
				for (const auto &n : lights) {
					for (iter = recognized_Objects.begin(); iter != recognized_Objects.end() && recognized_Missions[1] > 5; ++iter) {
						if ((*iter)->getCount() >= 5 && (*iter)->getName() == n && (*iter)->getDistance() <= recognized_Missions[1])
							recognized_Missions[1] = (*iter)->getDistance();
					}
				}
				// �ε帶ũ�� �Ÿ� �߰� ������Ʈ, ������ �˻� (��ȣ��� �ε帶ũ�� �Ÿ� �켱����)
				for (const auto &n : marks) {
					for (iter = recognized_Objects.begin(); iter != recognized_Objects.end() && recognized_Missions[1] > 5; ++iter) {
						if ((*iter)->getCount() >= 5 && (*iter)->getName() == n && (*iter)->getDistance() <= recognized_Missions[1]) {
							if (n == "roadArrowThree" || n == "roadArrowStraightRight" || n == "roadArrowLeftRight" || n == "crossWalkReal")
								recognized_Missions[1] = (*iter)->getDistance();
							distanceStopline = findStopline(); // ������ �˻� (�ν� ���н� -1 ��ȯ)
							if (distanceStopline != -1) recognized_Missions[1] = distanceStopline; // �νĵ� ��� �Ÿ� ������Ʈ
						}
					}
				}
				// �����α����� �Ÿ� 5���� �̳� ���� ��, �ǽð����� ������ �˻�
				if (distanceStopline == -1 && recognized_Missions[1] <= 5) {
					distanceStopline = findStopline(); // ������ �ν� (�ν� ���н� -1 ��ȯ)
					if (distanceStopline != -1) recognized_Missions[1] = distanceStopline; // �νĵ� ��� �Ÿ� ������Ʈ
				}
			}
			// �������� �νĵǰ� �ִ� ������ ��� ���������� �����α����� �Ÿ��� ������Ʈ�Ѵ�
			else if (distanceStopline != -1) {
				distanceStopline = findStopline(); // ������ �ν� (�ν� ���н� -1 ��ȯ)
				if (distanceStopline != -1) recognized_Missions[1] = distanceStopline; // �νĵ� ��� �Ÿ� ������Ʈ
				else if (distanceStopline == -1) distanceStopline = recognized_Missions[1]; // �ν� �ȵ� ��� ���� �Ÿ��� ���
			}
			// ���� �νĵǰ� �ִ� ��ȣ������ �̼� �ǽð� ������Ʈ
			for (const auto &n : lights) {
				for (iter = recognized_Objects.begin(); iter != recognized_Objects.end(); ++iter) {
					if ((*iter)->getCount() >= 5 && (*iter)->getName() == n && (*iter)->getDistance() <= recognized_Missions[1]) {
						// ��ȸ���� ��� (1) (1�� �����Ȳ)
						if (direction == 1) {
							if (n == "trafficLightRed" || n == "trafficLightRedYellow" || n == "trafficLightYellow")
								recognized_Missions[0] = 0; // ������ ����
							else if (n == "trafficLightGreenLeft" || n == "trafficLightRedLeft")
								recognized_Missions[0] = direction; // ��ȸ��
							else
								recognized_Missions[0] = 0; // �ʷϺ��� ��������, ��ȸ�� ��ȣ ���
						}
						// ������ ��� (2) (2�� �����Ȳ)
						else if (direction == 2) {
							if (n == "trafficLightRed" || n == "trafficLightRedLeft" || n == "trafficLightRedYellow" || n == "trafficLightYellow")
								recognized_Missions[0] = 0; // ������ ����
							else recognized_Missions[0] = direction; // ����
						}
						// ��ȸ���� ��� (3) (3�� �����Ȳ)
						else if (direction == 3) {
							if (n == "trafficLightRed" || n == "trafficLightRedLeft" || n == "trafficLightRedYellow" || n == "trafficLightYellow")
								recognized_Missions[0] = 0; // ������ ����
							else recognized_Missions[0] = direction; // ��ȸ��
						}
					}
				}
			}
		}
		// @@ (������ ����) �����ο� ������ �ִ� ������ ���, �ε帶ũ �Ǵ� Ⱦ�ܺ��� ���� �� ������ ���¸� �����Ѵ�.
		else if (recognized_Missions[1] == 0) {
			// �ε帶ũ�� �νĵǸ� ������ ��Ȳ ����
			for (const auto &n : marks) {
				for (iter = recognized_Objects.begin(); iter != recognized_Objects.end(); ++iter) {
					if ((*iter)->getCount() >= 5 && (*iter)->getName() == n && (*iter)->getDistance() <= 3) {
						recognized_Missions[0] = -1; recognized_Missions[1] = -1; // ������ ��Ȳ ����
						direction = -1; // ������ ���� ����
						distanceStopline = -1; // ������ �Ÿ� �ʱ�ȭ
					}
				}
			}
		}

		// ��Ÿ ǥ���� �̼�
		for (const auto &n : extras) {
			for (iter = recognized_Objects.begin(); iter != recognized_Objects.end(); ++iter) {
				if ((*iter)->getCount() >= 5 && (*iter)->getName() == n) {
					// ������ֹ� �̼�
					if (n == "construction") {
						for (int i = 0; i < len_missions; i++) {
							if (missions[i] == "staticObstacle") {
								recognized_Missions[i] = (*iter)->getDistance(); break;
							}
						}
					}
					// ������ֹ� �̼�
					else if (n == "bicycle") {
						for (int i = 0; i < len_missions; i++) {
							if (missions[i] == "dynamicObstacle") {
								recognized_Missions[i] = (*iter)->getDistance(); break;
							}
						}
					}
					// Ⱦ�ܺ��� �ν�
					else if (n == "crossWalk" || n == "crossWalkReal") {
						for (int i = 0; i < len_missions; i++) {
							if (missions[i] == "crossWalk") {
								recognized_Missions[i] = (*iter)->getDistance(); break;
							}
						}
					}
					// ���ӹ����� �ν�
					else if (n == "bust" || n == "bustReal") {
						for (int i = 0; i < len_missions; i++) {
							if (missions[i] == "bust") {
								recognized_Missions[i] = (*iter)->getDistance(); break;
							}
						}
					}
					// ������ �ν�
					else if (n == "parking") {
						for (int i = 0; i < len_missions; i++) {
							if (missions[i] == "parking") {
								recognized_Missions[i] = (*iter)->getDistance(); break;
							}
						}
					}
					// ������������ ����
					else if (n == "busArrowDown") {
						for (int i = 0; i < len_missions; i++) {
							if (missions[i] == "busLane") {
								recognized_Missions[i] = (*iter)->getDistance(); break;
							}
						}
					}
					// ��̺�ȣ���� ����
					else if (n == "kidSafe") {
						for (int i = 0; i < len_missions; i++) {
							if (missions[i] == "kidSafe") {
								recognized_Missions[i] = (*iter)->getDistance(); break;
							}
						}
					}
					// ��̺�ȣ���� Ż��
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
	int* getMissions() { return recognized_Missions; } // �νĵ� �̼� ����Ʈ�� ��ȯ�Ѵ�


	// �νĵ� �ڽ��� �־� ���� ũ�� �̻��̸� �ν��ؼ� ����Ʈ�� �ִ´�.
	void putbox(const string &name, float x, float y, float width, float height) {

		// �νĵ� ��ü�� �Ÿ��� ���� ���
		float area = width * height;
		int distance = checkDistance(name, area, y, y);
		// ��ü�� ũ�Ⱑ �ʹ� ������(�ʹ� �ָ� ������) �ش� ��ü�� �����Ѵ�
		if (distance == -1) return;

		// �̹� �νĵǰ� �ִ� �ش� Ŭ���� ��ü�� ������, ��ġ�� ���� ���� ��ü�̸� �ش� ������ ��ȯ (�Ӱ谪 ������� ��)
		Object* findedObject = nullptr;
		list<Object*>::iterator iter;
		for (iter = recognized_Objects.begin(); iter != recognized_Objects.end(); ++iter) {
			if ((*iter)->getName() == name) {
				if (abs((*iter)->getx() - x) < 0.05 && abs((*iter)->gety() - y) < 0.05)
					findedObject = *iter;
			}
		}

		// �νĵǰ� �ִ� ������Ʈ�� ������ ����Ʈ�� ���� �ִ´� / ������ �ش� ��ü ī��Ʈ ������Ʈ
		if (findedObject == nullptr)
			recognized_Objects.push_back(new Object(name, distance, x, y));
		else findedObject->recognized(distance, x, y);

	}


	// ������Ʈ
	void update() {
		cout << "update &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << "\n";
		list<Object*>::iterator iter;

		// �ش� �����ӿ��� �ν� �ȵ� ��ü�� ī��Ʈ ���̰�, ī��Ʈ�� 0�� �Ǹ� �����Ѵ�
		for (iter = recognized_Objects.begin(); iter != recognized_Objects.end(); ++iter) {
			(*iter)->unrecognized(); // �ش� ������ ���°� �����ǰ�, ī��Ʈ�� ������Ʈ�Ѵ�
			cout << "******** update *********" << endl;
			cout << " name : " << (*iter)->getName() << " / count : " << (*iter)->getCount() << endl;
			// ī��Ʈ�� 0�� �Ǹ� ����
			if ((*iter)->getCount() == 0) {
				cout << "@Erased (low count) : " << (*iter)->getName() << "\n";
				delete *iter;
				iter = recognized_Objects.erase(iter); // ī��Ʈ�� 0�̸� ����..
				iter--;
			}
		}

		// �νĵ� ��ü��κ��� �̼� ������Ʈ
		updateMissions();

		cout << "\n\n\n\n\n";
	}

#ifdef OPENCV_YOLO
	// ���� �νĵ� �̼ǿ� ���� missions ������ �ִ� �ش� ������ ����
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


	// ���� �νĵ� ������Ʈ�� ���� missions ������ �ִ� �ش� ������ ����
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