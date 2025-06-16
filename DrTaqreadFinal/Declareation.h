#pragma once
using namespace std;
class Barca_Team
{
public :
	string name;
	int age;
	string nationality;
	string place;
	int playerNumber;
	Barca_Team() {}
	Barca_Team(string Name, int Age,string Nationality,string Place,int PlayerNumber);
};

class D_LinkedList
{
public:
	D_LinkedList();
	struct Node
	{
		Barca_Team playerData;
		Node* next;
		Node* prev;	
		Node();
	};
	Node* Head = nullptr;
	void Append(Barca_Team player);
	void InsertAt(Barca_Team player, int pos);
	void Desplay();
	void Remove(int pos);
	void Search(int pos);
};

class PQL
{
private :
	struct Node
	{
		Barca_Team data;
		int priority;
		Node* next;
	};
	Node* front;
public :
	PQL();
	void Enqueue(Barca_Team player, int priority);//تم
	void Dequeue();//تم
	bool IsEmpty();//تم
	void Desplay();//تم
	void search(int playerNumber);//تم
	void Update(int playerNumber, int NewPriority);//تم

};

class StackLinkedList
{
	struct Node
	{
		Barca_Team playerdata;
		Node* next;
	};
	Node* Top;
public:
	StackLinkedList();
	void Push(Barca_Team player);
	void Pop();
	bool IsEmpty();
	void Peek();
	void Desplay();

};








