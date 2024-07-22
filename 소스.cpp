#include <iostream>
#include <winsock2.h>
#include <thread>
#include <ws2tcpip.h>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <string>
#include <map>
#include <mysql.h>
#pragma comment (lib, "libmysql.lib") // mysql ����

#pragma comment(lib, "ws2_32.lib") //���־󿡼� �������α׷��� �ϱ� ���� ��
using namespace std;

MYSQL Conn;
MYSQL* ConnPtr = NULL;
MYSQL_RES* Result;
MYSQL_ROW Row;
int MysqlResult;

#define PACKET_SIZE 1024
SOCKET skt, client_sock;

// ���� ���� ���� ���� map(���߿� ������ ã�� ���� unordered_map ����غ�(1��)) (ȸ�������̳� �α��� �Ҷ� Ȯ�ο����θ� ���)(db���� �� ������ ���)
// id,password, ���߿� �α��� ���µ� ���⿡ �߰�
unordered_map<string, string> users;

// �� ������ ģ���� map (int�� �ϴ� ���·� �ߴµ� ���߿� �ٸ������� ��������. ģ�� �߰��Ҷ� ���� ���� ������ �ָ��ϴ�.)
unordered_map<string, unordered_map<string, int>> friends;

// � ���̵�(��ī)�� ���� ģ����û
unordered_map<string,vector<string>> quokka_friend_rcv;

// � ���̵�(��ī)�� ��û�� ģ����û
unordered_map<string, vector<string>> quokka_friend_req;

struct new_users {
	string id;
	SOCKET client_soc = 0;
	/*vector<string> friends;*/
};

//���� �������� ���� ��
unordered_map<string,new_users> current_user;

// ä�ù� �� ������ �� �ִ� üũ�迭
unordered_map<int, int> check_chat;

// �θ� �̻��� ������ ���� ������ �������� ��  (�� ���̵� �������� ���� ��Ƽ��)
// ó�� �ʴ��� ���� ���� �� ���� ���� ��ȣ�� �״�� �����ϵ�, ��Ƽ�� ���Ѹ� �Ѱ��� �� �ִ� bool��(1) ����
unordered_map<int, vector<pair<string,bool>>> partymember;

// ��ī�� ���� ��Ƽ �ʴ��û (� ����, �ʴ뺸�� ��Ƽ�� ��Ƽ�� ���Ϲ�ȣ ���)
unordered_map<int, vector<int>> party_join_req;

vector<int> party_king;
vector<vector<pair<string,bool>>> party_member;

void proc_recvs(int k) {

	char buffer[PACKET_SIZE] = { 0 };
	string temp;

	while (!WSAGetLastError()) {
		memset(&buffer, 0, sizeof(buffer));
		recv(client_sock, buffer, PACKET_SIZE, 0);
		temp = buffer;
		if (temp == "10101") {
			check_chat[k] = 0;
			std::cout << endl;
			std::cout << "ä�� ������ �����Ͽ����ϴ�." << endl;
			break;
		};
		std::cout << "���� �޼���: " << buffer << endl;
	}

}

void login(char* buffer, int client_sock) {
	//������ ���ڿ� ������ �ѷ��ֱ�
	istringstream iss(buffer);
	int selectnum;
	string id;
	string password;
	iss >> selectnum >> id >> password;

	//�ִ� ���̵� ���� ã�� ���� tempuser (���߿��� DB�� ��ü)
	auto tempuser = users.find(id);

	if (tempuser != users.end()) {

		/* ���� ��� ���̰� �ԷµǸ� �ٸ� ��� ��� �� �� �ְ� �ٲ��ֱ�
		��� ���� (Ư�� ���̵� ����)
		if (tempuser->first == "quokka") {

		��ü����(�޽��� �Է��ϸ� ������ �ִ� ���ο��� �޽��� �ѷ���)

			//��ڰ� ������ ä�ÿ�
			//char buffer[PACKET_SIZE] = { 0 };
			//thread proc2(proc_recvs);

			//while (!WSAGetLastError()) {
			//	//���⵵ �ޱ� ���ؼ� cin���� cin.getline ���
			//	cin.getline(buffer, PACKET_SIZE, '\n');
			//	cout << "���� ������ �޽��� : " << buffer << endl << endl;
			//	send(client_sock, buffer, strlen(buffer), 0);
			//}


			 ���� �ο� Ȯ�� (���� ������ ���� Ư�� ���� ���� ���ִ��� ã�� �� �ְ�)
		}*/

		//�Ϲ� ����
		if (tempuser->second == password) {

			//id pass ���� ������ 1 ������ (���߿��� ���ý��� �Ἥ cnt�� ���� ��Ű�ų� ���� �� �� �ִ� ���ڸ� �����ֱ�)
			send(client_sock, "2", PACKET_SIZE, 0);
			new_users temp_user_struct;
			temp_user_struct.id = tempuser->first;
			temp_user_struct.client_soc = client_sock;
			current_user[tempuser->first] = temp_user_struct;

			//������ ������ ��� �߰� �ϱ�
			std::cout << "Ŭ���̾�Ʈ " << tempuser->first << " ����Ϸ�" << endl << endl;

			//���߿��� ���� �߰� �ɶ����� �̸� ���� �����ؼ� ++ �ϰ� --�ذ��鼭 ���� size��꿡 �ð� ������ ����
			std::cout << "���� �����ο� : " << current_user.size() << endl << endl;

			partymember[client_sock].emplace_back(pair<string, bool>(tempuser->first,true)) ;
		}
		else {
			//Ʋ���� 0 ������
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
	string k = "INSERT INTO user VALUES(NULL,'" +id+ "','" +password+ "')";
	const char* Query = &*k.begin();
	MysqlResult = mysql_query(ConnPtr, Query);
	if (MysqlResult != 0) std::cout << "����ó��"<< endl;
	else {
		std::cout << id << " ���� ȸ������ �Ͽ����ϴ�" << endl << endl;
		mysql_free_result(Result);
	}
}

int main() {
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	mysql_init(&Conn);
	ConnPtr = mysql_real_connect(&Conn, "127.0.0.1", "root", "1234", "quokka_server", 3306, (char*)NULL, 0);
	/*if (ConnPtr == NULL) {}*/
	//std::cout << "ClientInfo : " << mysql_get_client_info() << std::endl; // mysql ���� �߰� �ϴ°�

	//============== ���� ������ ================

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

	//�ӽ� ��ī ģ����
	friends["quokka"]["yujin"] = 1;
	friends["quokka"]["woobin"] = 1;
	friends["quokka"]["wallaby"] = 0;

	//============== ���� ������ ================


	skt = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	std::cout << "======================" << endl;
	std::cout << "======================" << endl;
	std::cout << "======================" << endl;
	std::cout << "=======��������=======" << endl;
	std::cout << "======================" << endl;
	std::cout << "======================" << endl;
	std::cout << "======================" << endl << endl << endl ;


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
		std::cout << "accept ����" << endl;
	}

	while (1) {

		char buffer[PACKET_SIZE] = { 0 };
		recv(client_sock,buffer,PACKET_SIZE,0);
		
		istringstream iss(buffer);
		int selectnum;
		iss >> selectnum;

		//�α���	
		if (selectnum == 1) {
			login(buffer, client_sock);
		}
		
		//ȸ������(���߿� ���̵� ������ �����ϰ� �ϰ�, �н����嵵 Ư������ �ϳ��� �־�� ȸ������ �ǰ� �����)
		//DB����
		else if (selectnum == 2) {
			/*if (!WSAGetLastError()) continue;*/
			sign_up(buffer);
			
		}

		//ä��
		else if (selectnum == 3) {

			string req_id;
			string rcv_id;
			int selectnum_party_chat_i;
			istringstream iss(buffer);
			iss >> selectnum >> selectnum_party_chat_i >>req_id >> rcv_id;

			
			// ���� ��Ƽ ä�ÿ� �����մϴ�
			if (selectnum_party_chat_i == 1) {

				send(client_sock, "1", PACKET_SIZE, 0);

				// ó�� ���� �� ������ ��ģ ������ȣ�� ���� �װɷ� �Ǵ�
				string temp_chat_terminate_check_s = to_string(current_user[req_id].client_soc) + to_string(current_user[rcv_id].client_soc);
				int temp_chat_terminate_check = stoi(temp_chat_terminate_check_s);

				check_chat[temp_chat_terminate_check] = 1;

				char buffer[PACKET_SIZE] = { 0 };

				while (!WSAGetLastError()) {

					// ������ ������ �ȴ����� �ð��ȿ� while�� ����
					Sleep(1000);
					if (!check_chat[temp_chat_terminate_check]) {
						send(client_sock, "10101", PACKET_SIZE, 0);
						break;
					}
					Sleep(1000);
					send(client_sock, "��üä������", PACKET_SIZE, 0);

					//	////���⵵ �ޱ� ���ؼ� cin���� cin.getline ���
					//	//cin.getline(buffer, PACKET_SIZE, '\n');
					//	//string finish = buffer;
					//	//if (finish == "10101") {
					//	//	send(client_sock, "10101", strlen(buffer), 0);
					//	//	cout << "������ ä�� ������ ���� �Ͽ����ϴ�." << endl;
					//	//	break;
					//	//}
					//	//cout << "���� ���� : " << buffer << endl << endl;
					//	//send(client_sock, buffer, strlen(buffer), 0);

					//}

					//proc2.join();

				}
			}

			//�ӼӸ� ����
			else if (selectnum_party_chat_i == 2) {
				char buffer[PACKET_SIZE] = { 0 };
				recv(client_sock,buffer,PACKET_SIZE,0);
				string temp_whisper = buffer;		
				send(current_user[rcv_id].client_soc,"�ӼӸ� ����",PACKET_SIZE,0);
			}

		} //ä�� ������ else if��

		//��ƼȮ��
		else if (selectnum == 4) {
			int partycheck_num;
			string req_id;

			istringstream iss(buffer);
			iss >> selectnum >> partycheck_num >> req_id;

			int partymember_i = current_user[req_id].client_soc;

			// ���� ��Ƽ�� �������
			if (partycheck_num == 1) {
				string temp_partymember_num_s = to_string(partymember[partymember_i].size());
				memset(buffer,0,PACKET_SIZE);
				strcpy_s(buffer, temp_partymember_num_s.c_str());
				send(client_sock, buffer, PACKET_SIZE, 0);
			}

			//���� ��Ƽ�� ����������
			/*for (auto& k : partymember[partymember_i]) {
				
			}*/

			// ��Ƽ �ʴ� ��û �ִ���
			else if (partycheck_num == 2) {

	
			}
			// partycheck_num == 2

			}

		//��Ƽ �ʴ��ϱ� (��û ������)
		else if (selectnum == 84) {
			// ���� ��Ƽ ��ȣ (���Ϲ�ȣ)
			int party_num;
			string req_id;

		}

		//��û ���� ģ���߰�
		else if (selectnum == 94) {
			string req_id;
			int rcv_num_i;

			// ģ�� ���Ϳ� �ε����� ���� �ؾ��ؼ� �Ѿ���� ���ڿ��� 1���ֱ�
			istringstream iss(buffer);
			iss >> selectnum >> req_id >> rcv_num_i;
			rcv_num_i--;

			// ��ī ģ�� ��û�� �� ģ�� �߰� (���´� ���߿� �����ϱ�)
			friends[req_id][quokka_friend_rcv[req_id][rcv_num_i]] = 1;

			// �� ģ�� ���׵� ��û ������ (���߿� ��� ���� �� �߰�)
			//friends[quokka_friend_rcv[req_id][rcv_num_i]][req_id] = 1;
			
			quokka_friend_rcv[req_id].erase(quokka_friend_rcv[req_id].begin() + rcv_num_i);
			// �� ģ���� ���� ��û�� ���� (���߿� ��� ���� �� �߰�)
			// 
		}

		//ģ�� ���ο� ��û �ִ��� Ȯ�� + ��û�� �������
		//��û�� �� ���̵� �α��� �ϸ� ģ�� ��û ��Ƶа� ���ְų�, �¶����϶� �ǽð����� ���� �� �ְ� �ؾ���
		else if (selectnum==95) {
			string req_id;
			int friend_req_num;
			istringstream iss(buffer);
			iss >> selectnum >> friend_req_num >>req_id;

			// ���ο� ��û �ִ��� Ȯ��(������ 0 ����) + ���ο� ��û ������� �����ֱ�
			if (friend_req_num == 1) {
				string temp_friend_req;
				char buffer[PACKET_SIZE];
				memset(buffer, 0, PACKET_SIZE);
				temp_friend_req = to_string(quokka_friend_rcv[req_id].size());
				strcpy_s(buffer, temp_friend_req.c_str());
				send(client_sock, buffer, PACKET_SIZE, 0);
			}

			// ���ο� ��û ����������
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

		//ģ�� �߰��ϱ� ���ؼ� ��ü �������� �� ���� �����ϴ��� Ȯ��
		else if (selectnum==96) {
			string req_id;
			string rcv_id;
			int friend_req_num;
			istringstream iss(buffer);
			iss >> selectnum >> friend_req_num >>req_id >> rcv_id;

			//��ü ���� ã�� 
			auto rcv_tempuser = users.find(rcv_id);

			//ģ����Ͽ� ���� ã��
			auto friends_tempuser = friends[req_id].find(rcv_id);

			// ��ü �������� �� ���� ã�� (ģ�� �߰�)
			if (friend_req_num == 1) {

				//��û�� ���̵� ģ�� ����
				if (rcv_tempuser != users.end()) {
					send(client_sock, "1", PACKET_SIZE, 0);

				}
				//��û�� ���̵� ģ�� ����
				else {
					send(client_sock, "0", PACKET_SIZE, 0);
				}

			}

			// ģ�� ��Ͽ��� �� ���� ã�� (ģ�� ����)
			else if (friend_req_num == 2) {

				//��û�� ���̵� ģ�� ����
				if (friends_tempuser != friends[req_id].end()) {
					send(client_sock, "1", PACKET_SIZE, 0);

				}
				//��û�� ���̵� ģ�� ����
				else {
					send(client_sock, "0", PACKET_SIZE, 0);
				}

			}

			// ģ����û (��û�� ģ�� ��û���(req)�� �߰� + ��� ���� ���(rcv)���� �߰�)
			else if (friend_req_num == 3) {
				quokka_friend_req[req_id].emplace_back(rcv_id);

				// �� ģ�� ���� ��û�� �� �� �ְ� (���߿� ��� ���� �� �߰�)
				//quokka_friend_rcv[rcv_id].emplace_back(req_id);
			}

			// ģ������ (��û�� ģ�� ��Ͽ��� ���� + ��� ��Ͽ��� ����)
			else if (friend_req_num == 4) {

				// �� ��Ͽ��� ����
				friends[req_id].erase(rcv_id);

				// ���߿� �� ģ�� ��Ͽ����� ����
				friends[rcv_id].erase(req_id);
			}

		}

		//�Էµ� ģ�� �ִ��� Ȯ��
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

		//���� �������� ģ�� ������� �˷� �ּ���
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
			//������ ���� ���� �� ����
			char buffer[PACKET_SIZE];
			strcpy_s(buffer, temp_friend.c_str());
			send(client_sock, buffer, PACKET_SIZE, 0);
		}

		//�� ģ�� ������� �˷� �ּ���
		//else if (selectnum == 99) {
		//	string in_id;
		//	istringstream iss(buffer);
		//	iss >> selectnum >> in_id;
		//	char buffer[PACKET_SIZE];
		//	int friendsize = friends[in_id].size();
		//	sprintf_s(buffer, sizeof(buffer), "%d", friendsize);
		//	send(client_sock,buffer,PACKET_SIZE,0);
		//}
		
		//���� �������� ģ�� ��� �ּ���
		else if (selectnum == 99) {
			string in_id;
			istringstream iss(buffer);
			iss >> selectnum >> in_id;

			//1. �ϳ��� send ���ִ°�
			/*for (auto k : quokka_friends) {
				string temp_friend = k.second;
				char buffer[PACKET_SIZE];
				strcpy(buffer, temp_friend.c_str());
				send(client_sock, buffer,PACKET_SIZE,0 );
			}*/

			//2. ���� �����ڷ� ��� string���� ������
			string temp_friend;
			for (auto k : friends[in_id]) {
				if (k.second == 1)
				temp_friend += (k.first +" ");
			}
			//������ ���� ���� �� ����
			temp_friend.pop_back();
			char buffer[PACKET_SIZE];
			strcpy_s(buffer, temp_friend.c_str());
			send(client_sock, buffer, PACKET_SIZE, 0);
		}


		//��ü ģ����� �ּ���
		else if (selectnum == 100) {

			string req_id;
			istringstream iss(buffer);
			iss >> selectnum >> req_id;

			//1. �ϳ��� send ���ִ°�
			/*for (auto k : quokka_friends) {
				string temp_friend = k.second;
				char buffer[PACKET_SIZE];
				strcpy(buffer, temp_friend.c_str());
				send(client_sock, buffer,PACKET_SIZE,0 );
			}*/

			//2. ���� �����ڷ� ��� string���� ������
			string temp_friend;
			for (auto k : friends[req_id]) {
				temp_friend += (k.first+to_string(k.second)+" ");
			}
			//������ ���� ���� �� ����
			temp_friend.pop_back();
			char buffer[PACKET_SIZE];
			strcpy_s(buffer, temp_friend.c_str());
			send(client_sock, buffer, PACKET_SIZE, 0);

		 }

		//�α׾ƿ�
		else if (selectnum == 101) {
			string req_id;
			istringstream iss(buffer);
			iss >> selectnum >> req_id;
			std::cout << req_id << "�� �α׾ƿ�" << endl;
			current_user.erase(req_id);
			std::cout << "���� �����ο� : " << current_user.size() << endl << endl;
		}

	}

	closesocket(client_sock);
	closesocket(skt);
	mysql_close(ConnPtr);
	WSACleanup();
	std::cout << "������ ����Ǿ����ϴ�.";
}