#include <iostream>
#include <winsock2.h>
#include <thread>
#include <ws2tcpip.h>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <string>

#pragma comment(lib, "ws2_32.lib") //���־󿡼� �������α׷��� �ϱ� ���� ��
using namespace std;

#define PACKET_SIZE 1024
SOCKET skt;

//���� ���� ���� ���� �迭(���߿� ������ ã�� ���� unordered_map ����غ�(1��)) (ȸ�������̳� �α��� �Ҷ� Ȯ�ο����θ� ���)(db���� �� ������ ���)
//id,password
unordered_map<string, string> users;

struct new_users {
	string id;
	SOCKET client_soc = 0;
	/*vector<string> friends;*/
};

//���� �������� ���� ��
unordered_map<string,new_users> current_user;

//�̰� ���߿� ��ڶ� ������ ä�� �ϱ� ���� �ڵ� 
void proc_recvs(SOCKET client_sock) {
	char buffer[PACKET_SIZE] = { 0 };

	while (!WSAGetLastError()) {
		ZeroMemory(&buffer, PACKET_SIZE);
		recv(client_sock, buffer, PACKET_SIZE, 0);
		cout << "���� �޼���: " << buffer << endl << endl;
	}
}

void newuser() {

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
			send(client_sock, "1", PACKET_SIZE, 0);
			new_users temp_user_struct;
			temp_user_struct.id = tempuser->first;
			temp_user_struct.client_soc = client_sock;
			current_user[tempuser->first] = temp_user_struct;

			//������ ������ ��� �߰� �ϱ�
			cout << "Ŭ���̾�Ʈ " << client_sock << " ����Ϸ�" << endl << endl;

			//���߿��� ���� �߰� �ɶ����� �̸� ���� �����ؼ� ++ �ϰ� --�ذ��鼭 ���� size��꿡 �ð� ������ ����
			cout << "���� �����ο� : " << current_user.size();

			//ģ���߰� �ص� ���������� ��� ���� ���� �ִٰ� �˷��ִ� �޽��� �Ѹ��� (db�� ����� ģ�� �����ͼ� �Ѹ���)
			//(Ŭ���̾�Ʈ�� ���� ���Դ��� �� �� �ְ� ģ�� â ������ �ϳ� ��� �������� �ڴ�)

		}
		else {
			//Ʋ���� 0 ������
			send(client_sock, "0", PACKET_SIZE, 0);
		}

	}
}

void sign_up(char* buffer) {
	istringstream iss(buffer);
	int selectnum;
	string id;
	string password;
	iss >> selectnum >> id >> password;
	users[id] = password;
}

int main() {
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	skt = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	cout << "======================" << endl;
	cout << "======================" << endl;
	cout << "======================" << endl;
	cout << "=======��������=======" << endl;
	cout << "======================" << endl;
	cout << "======================" << endl;
	cout << "======================" << endl << endl << endl << endl << endl;

	SOCKADDR_IN addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(9000);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(skt, (SOCKADDR*)&addr, sizeof(addr));
	listen(skt, SOMAXCONN);

	SOCKET client_sock;
	SOCKADDR_IN client = {};
	int client_size = sizeof(client);
	ZeroMemory(&client, client_size);

	while (1) {

		client_sock = accept(skt, (SOCKADDR*)&client, &client_size);
		if (client_sock == -1) {
			cout << "accept ����" << endl;
			continue;
		}

		char buffer[PACKET_SIZE] = { 0 };
		recv(client_sock,buffer,PACKET_SIZE,0);
		
		istringstream iss(buffer);
		int selectnum;
		iss >> selectnum;

		//ȸ������(���߿� ���̵� ������ �����ϰ� �ϰ�, �н����嵵 Ư������ �ϳ��� �־�� ȸ������ �ǰ� �����)
		//DB����	
		if (selectnum == 1) {
			sign_up(buffer);
		}

		//�α���
		else if (selectnum == 2) {
			login(buffer, client_sock);
		}

		//ä��
		else if (selectnum == 3) {

		}

		//�α׾ƿ�
		else if (selectnum == 101) {
			string id;
			iss >> selectnum >> id;
			current_user.erase(id);
		}


	}

	//proc2.join();

	closesocket(client_sock);
	closesocket(skt);
	WSACleanup();
	cout << "������ ����Ǿ����ϴ�.";
}