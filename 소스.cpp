#include <iostream>
#include <winsock2.h>
#include <thread>
#include <ws2tcpip.h>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <string>
#include <map>

#pragma comment(lib, "ws2_32.lib") //비주얼에서 소켓프로그래밍 하기 위한 것
using namespace std;

#define PACKET_SIZE 1024
SOCKET skt, client_sock;

// 유저 정보 집어 넣을 map(나중에 빠르게 찾기 위해 unordered_map 사용해봄(1차)) (회원가입이나 로그인 할때 확인용으로만 사용)(db연결 전 까지만 사용)
// id,password, 나중에 로그인 상태도 여기에 추가
unordered_map<string, string> users;

// 그 유저별 친구들 map (int는 일단 상태로 했는데 나중에 다른것으로 수정하자. 친구 추가할때 저거 상태 설정이 애매하다.)
unordered_map<string, unordered_map<string, int>> friends;

// 어떤 아이디(쿼카)가 받은 친구요청
unordered_map<string,vector<string>> quokka_friend_rcv;

// 어떤 아이디(쿼카)가 요청한 친구요청
unordered_map<string, vector<string>> quokka_friend_req;

struct new_users {
	string id;
	SOCKET client_soc = 0;
	/*vector<string> friends;*/
};

//현재 접속중인 유저 수
unordered_map<string,new_users> current_user;

// 채팅방 당 구분할 수 있는 체크배열
unordered_map<int, int> check_chat;

//현재 유저가 참여중인 방
unordered_map<string, unordered_map<int, string>> chatroom;

//쿼카가 참여중인 방
unordered_map<int, string> quokka_in_chatroom;

void proc_recvs(int k) {

	char buffer[PACKET_SIZE] = { 0 };
	string temp;

	while (!WSAGetLastError()) {
		memset(&buffer, 0, sizeof(buffer));
		recv(client_sock, buffer, PACKET_SIZE, 0);
		temp = buffer;
		if (temp == "10101") {
			check_chat[k] = 0;
			cout << endl;
			cout << "채팅 연결을 종료하였습니다."<<endl;
			break;
		};
		cout << "받은 메세지: " << buffer << endl;
	}

}

void login(char* buffer, int client_sock) {
	//들어오는 문자열 나눠서 뿌려주기
	istringstream iss(buffer);
	int selectnum;
	string id;
	string password;
	iss >> selectnum >> id >> password;

	//있는 아이디 인지 찾기 위한 tempuser (나중에는 DB로 대체)
	auto tempuser = users.find(id);

	if (tempuser != users.end()) {

		/* 만약 운영자 아이가 입력되면 다른 기능 사용 할 수 있게 바꿔주기
		운영자 접속 (특정 아이디 접속)
		if (tempuser->first == "quokka") {

		전체공지(메시지 입력하면 접속해 있는 전부에게 메시지 뿌려줌)

			//운영자가 유저랑 채팅용
			//char buffer[PACKET_SIZE] = { 0 };
			//thread proc2(proc_recvs);

			//while (!WSAGetLastError()) {
			//	//띄어쓰기도 받기 위해서 cin말고 cin.getline 사용
			//	cin.getline(buffer, PACKET_SIZE, '\n');
			//	cout << "내가 전달한 메시지 : " << buffer << endl << endl;
			//	send(client_sock, buffer, strlen(buffer), 0);
			//}


			 접속 인원 확인 (현재 접속자 수랑 특정 유저 접속 해있는지 찾을 수 있게)
		}*/

		//일반 유저
		if (tempuser->second == password) {

			//id pass 전부 맞으면 1 보내기 (나중에는 뮤택스를 써서 cnt를 증가 시키거나 구분 할 수 있는 숫자를 보내주기)
			send(client_sock, "2", PACKET_SIZE, 0);
			new_users temp_user_struct;
			temp_user_struct.id = tempuser->first;
			temp_user_struct.client_soc = client_sock;
			current_user[tempuser->first] = temp_user_struct;

			//서버에 접속한 사람 뜨게 하기
			cout << "클라이언트 " << tempuser->first << " 연결완료" << endl << endl;

			//나중에는 유저 추가 될때마다 미리 변수 선언해서 ++ 하고 --해가면서 굳이 size계산에 시간 날리지 말기
			cout << "현재 접속인원 : " << current_user.size() << endl << endl;

			//친구추가 해둔 유저있으면 방금 들어온 유저 있다고 알려주는 메시지 뿌리기 (db에 저장된 친구 가져와서 뿌리기)
			//(클라이언트는 누가 들어왔는지 알 수 있게 친추 창 쓰레드 하나 계속 돌려놔야 겠다)
		}
		else {
			//틀리면 0 보내기
			send(client_sock, "1", PACKET_SIZE, 0);
		}
	}
	else send(client_sock, "0", PACKET_SIZE, 0);
}

void sign_up(char* buffer) {
	istringstream iss(buffer);
	int selectnum;
	string id;
	string password;
	iss >> selectnum >> id >> password;
	users[id] = password;
	cout << id << " 님이 회원가입 하였습니다" << endl << endl;
}

int main() {
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);


	//============== 더미 데이터 ================

	quokka_friend_rcv["quokka"].emplace_back("meetquack");
	quokka_friend_rcv["quokka"].emplace_back("raccoon");

	users["woobin"] = "123";
	users["yujin"] = "123";
	users["wallaby"] = "123";
	users["capybara"] = "123";

	new_users woobin;
	woobin.client_soc = 99999;
	woobin.id = "woobin";

	new_users yujin;
	yujin.client_soc = 99998;
	yujin.id = "yujin";

	current_user["woobin"] = woobin;
	current_user["yujin"] = yujin;

	//임시 쿼카 친구들
	friends["quokka"]["yujin"] = 1;
	friends["quokka"]["woobin"] = 1;
	friends["quokka"]["wallaby"] = 0;

	//============== 더미 데이터 ================


	skt = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	cout << "======================" << endl;
	cout << "======================" << endl;
	cout << "======================" << endl;
	cout << "=======서버시작=======" << endl;
	cout << "======================" << endl;
	cout << "======================" << endl;
	cout << "======================" << endl << endl << endl ;


	SOCKADDR_IN addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(9000);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(skt, (SOCKADDR*)&addr, sizeof(addr));
	listen(skt, SOMAXCONN);

	SOCKADDR_IN client = {};
	int client_size = sizeof(client);
	ZeroMemory(&client, client_size);

	client_sock = accept(skt, (SOCKADDR*)&client, &client_size);
	if (client_sock == -1) {
		cout << "accept 오류" << endl;
	}

	while (1) {

		char buffer[PACKET_SIZE] = { 0 };
		recv(client_sock,buffer,PACKET_SIZE,0);
		
		istringstream iss(buffer);
		int selectnum;
		iss >> selectnum;

		//로그인	
		if (selectnum == 1) {
			login(buffer, client_sock);
		}
		
		//회원가입(나중에 아이디 영문만 가능하게 하고, 패스워드도 특수문자 하나씩 넣어야 회원가입 되게 만들기)
		//DB연동
		else if (selectnum == 2) {
			/*if (!WSAGetLastError()) continue;*/
			sign_up(buffer);
			
		}

		//채팅
		else if (selectnum == 3) {

			string req_id;
			string rcv_id;
			istringstream iss(buffer);
			iss >> selectnum >> req_id >> rcv_id;

			// 처음 들어온 두 소켓을 합친 고유번호를 만들어서 그걸로 판단
			string temp_chat_terminate_check_s = to_string(current_user[req_id].client_soc) + to_string(current_user[rcv_id].client_soc);
			int temp_chat_terminate_check = stoi(temp_chat_terminate_check_s);

			check_chat[temp_chat_terminate_check] = 1;

			cout << "======================" << endl;
			cout << req_id << "님과 "<<rcv_id <<"님 채팅시작" << endl;
			cout << "======================" << endl;

			char buffer[PACKET_SIZE] = { 0 };
			thread proc2(proc_recvs, temp_chat_terminate_check);

			while (!WSAGetLastError()) {

				// 유저가 불편함 안느끼는 시간안에 while문 돌기
				Sleep(1000);
				if (!check_chat[temp_chat_terminate_check]) {
					send(client_sock, "10101", PACKET_SIZE, 0);
					break;
				}

				////띄어쓰기도 받기 위해서 cin말고 cin.getline 사용
				//cin.getline(buffer, PACKET_SIZE, '\n');
				//string finish = buffer;
				//if (finish == "10101") {
				//	send(client_sock, "10101", strlen(buffer), 0);
				//	cout << "유저와 채팅 연결을 종료 하였습니다." << endl;
				//	break;
				//}
				//cout << "서버 전달 : " << buffer << endl << endl;
				//send(client_sock, buffer, strlen(buffer), 0);

			}

			proc2.join();
		}

		//요청 받은 친구추가
		else if (selectnum == 94) {
			string req_id;
			int rcv_num_i;

			// 친구 벡터에 인덱스로 접근 해야해서 넘어오는 숫자에서 1빼주기
			istringstream iss(buffer);
			iss >> selectnum >> req_id >> rcv_num_i;
			rcv_num_i--;

			// 쿼카 친구 요청에 그 친구 추가 (상태는 나중에 수정하기)
			friends[req_id][quokka_friend_rcv[req_id][rcv_num_i]] = 1;
			// 그 친구 한테도 요청 보내기 (나중에 디비 연결 후 추가)
			//friends[quokka_friend_rcv[req_id][rcv_num_i]][req_id] = 1;
			
			quokka_friend_rcv[req_id].erase(quokka_friend_rcv[req_id].begin() + rcv_num_i);
			// 그 친구가 보낸 요청도 삭제 (나중에 디비 연결 후 추가)
			// 
		}

		//친구 새로운 요청 있는지 확인 + 요청이 몇건인지
		//요청한 그 아이디가 로그인 하면 친추 요청 모아둔거 쏴주거나, 온라인일때 실시간으로 받을 수 있게 해야함
		else if (selectnum==95) {
			string req_id;
			int friend_req_num;
			istringstream iss(buffer);
			iss >> selectnum >> friend_req_num >>req_id;

			// 새로운 요청 있는지 확인(없으면 0 보냄) + 새로운 요청 몇건인지 보내주기
			if (friend_req_num == 1) {
				string temp_friend_req;
				char buffer[PACKET_SIZE];
				memset(buffer, 0, PACKET_SIZE);
				temp_friend_req = to_string(quokka_friend_rcv[req_id].size());
				strcpy_s(buffer, temp_friend_req.c_str());
				send(client_sock, buffer, PACKET_SIZE, 0);
			}

			// 새로운 요청 누구누군지
			else if (friend_req_num == 2) {

				string temp_friend_req;

				for (int i = 0; i < quokka_friend_rcv["quokka"].size(); i++) {
					if (i == 0)
						temp_friend_req += quokka_friend_rcv["quokka"][i];
					else
						temp_friend_req += ("," + quokka_friend_rcv["quokka"][i]);
				}

				char buffer[PACKET_SIZE];
				memset(buffer, 0, PACKET_SIZE);
				strcpy_s(buffer, temp_friend_req.c_str());
				send(client_sock, buffer, PACKET_SIZE, 0);
			}

		}

		//친구 추가하기 위해서 전체 유저에서 그 유저 존재하는지 확인
		else if (selectnum==96) {
			string req_id;
			string rcv_id;
			int friend_req_num;
			istringstream iss(buffer);
			iss >> selectnum >> friend_req_num >>req_id >> rcv_id;

			//전체 유저 찾는 
			auto rcv_tempuser = users.find(rcv_id);

			//친구목록에 유저 찾는
			auto friends_tempuser = friends[req_id].find(rcv_id);

			// 전체 유저에서 그 유저 찾기 (친구 추가)
			if (friend_req_num == 1) {

				//요청한 아이디 친구 있음
				if (rcv_tempuser != users.end()) {
					send(client_sock, "1", PACKET_SIZE, 0);

				}
				//요청한 아이디 친구 없음
				else {
					send(client_sock, "0", PACKET_SIZE, 0);
				}

			}

			// 친구 목록에서 그 유저 찾기 (친구 삭제)
			else if (friend_req_num == 2) {

				//요청한 아이디 친구 있음
				if (friends_tempuser != friends[req_id].end()) {
					send(client_sock, "1", PACKET_SIZE, 0);

				}
				//요청한 아이디 친구 없음
				else {
					send(client_sock, "0", PACKET_SIZE, 0);
				}

			}

			// 친구신청 (요청자 친구 신청목록(req)에 추가 + 상대 받은 목록(rcv)에도 추가)
			else if (friend_req_num == 3) {
				quokka_friend_req[req_id].emplace_back(rcv_id);

				// 그 친구 받은 요청에 뜰 수 있게 (나중에 디비 연결 후 추가)
				//quokka_friend_rcv[rcv_id].emplace_back(req_id);
			}

			// 친구삭제 (요청자 친구 목록에서 삭제 + 상대 목록에서 삭제)
			else if (friend_req_num == 4) {

				// 내 목록에서 삭제
				friends[req_id].erase(rcv_id);

				// 나중에 그 친구 목록에서도 삭제
				friends[rcv_id].erase(req_id);
			}

		}

		//입력된 친구 있는지 확인
		else if (selectnum == 97) {
			string req_id;
			string rcv_id;
			istringstream iss(buffer);
			iss >> selectnum >> req_id >> rcv_id;

			auto rcv_tempuser = friends[req_id].find(rcv_id);

				if (rcv_tempuser != friends[req_id].end()) {
					send(client_sock, "1", PACKET_SIZE, 0);
				}

				else{
					send(client_sock, "0", PACKET_SIZE, 0);
				}

		}

		//현재 접속중인 친구 몇명인지 알려 주세요
		else if (selectnum == 98) {
			string in_id;
			istringstream iss(buffer);
			iss >> selectnum >> in_id;
			int temp_cnt = 0;
			string temp_friend;
			for (auto k : friends[in_id]) {
				if (k.second == 1)
					temp_cnt++;
			}
			temp_friend = to_string(temp_cnt);
			//마지막 공백 제거 후 전송
			char buffer[PACKET_SIZE];
			strcpy_s(buffer, temp_friend.c_str());
			send(client_sock, buffer, PACKET_SIZE, 0);
		}

		//총 친구 몇명인지 알려 주세요
		//else if (selectnum == 99) {
		//	string in_id;
		//	istringstream iss(buffer);
		//	iss >> selectnum >> in_id;
		//	char buffer[PACKET_SIZE];
		//	int friendsize = friends[in_id].size();
		//	sprintf_s(buffer, sizeof(buffer), "%d", friendsize);
		//	send(client_sock,buffer,PACKET_SIZE,0);
		//}
		
		//현재 접속중인 친구 목록 주세요
		else if (selectnum == 99) {
			string in_id;
			istringstream iss(buffer);
			iss >> selectnum >> in_id;

			//1. 하나씩 send 해주는것
			/*for (auto k : quokka_friends) {
				string temp_friend = k.second;
				char buffer[PACKET_SIZE];
				strcpy(buffer, temp_friend.c_str());
				send(client_sock, buffer,PACKET_SIZE,0 );
			}*/

			//2. 공백 구분자로 묶어서 string으로 보내기
			string temp_friend;
			for (auto k : friends[in_id]) {
				if (k.second == 1)
				temp_friend += (k.first +" ");
			}
			//마지막 공백 제거 후 전송
			temp_friend.pop_back();
			char buffer[PACKET_SIZE];
			strcpy_s(buffer, temp_friend.c_str());
			send(client_sock, buffer, PACKET_SIZE, 0);
		}


		//전체 친구목록 주세요
		else if (selectnum == 100) {

			string in_id;
			istringstream iss(buffer);
			iss >> selectnum >> in_id;

			//1. 하나씩 send 해주는것
			/*for (auto k : quokka_friends) {
				string temp_friend = k.second;
				char buffer[PACKET_SIZE];
				strcpy(buffer, temp_friend.c_str());
				send(client_sock, buffer,PACKET_SIZE,0 );
			}*/

			//2. 공백 구분자로 묶어서 string으로 보내기
			string temp_friend;
			for (auto k : friends[in_id]) {
				temp_friend += (k.first+to_string(k.second)+" ");
			}
			//마지막 공백 제거 후 전송
			temp_friend.pop_back();
			char buffer[PACKET_SIZE];
			strcpy_s(buffer, temp_friend.c_str());
			send(client_sock, buffer, PACKET_SIZE, 0);

		 }

		//로그아웃
		else if (selectnum == 101) {
			string req_id;
			istringstream iss(buffer);
			iss >> selectnum >> req_id;
			cout << req_id << "님 로그아웃" << endl;
			current_user.erase(req_id);
			cout << "현재 접속인원 : " << current_user.size() << endl << endl;
		}

	}

	closesocket(client_sock);
	closesocket(skt);
	WSACleanup();
	cout << "연결이 종료되었습니다.";
}