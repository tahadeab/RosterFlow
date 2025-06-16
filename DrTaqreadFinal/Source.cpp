#include<iostream>
#include"Declareation.h"
using namespace std;

Barca_Team::Barca_Team(string Name, int Age,string Nationality,string Place,int PlayerNumber)
{
	name = Name;
	age = Age;
	nationality = Nationality;
	place = Place;
	playerNumber = PlayerNumber;
}


D_LinkedList::D_LinkedList()
{
	Head = nullptr;
}
D_LinkedList::Node::Node(){
	next = nullptr;
	prev = nullptr;
}
void D_LinkedList :: Append(Barca_Team player)
{
	Node* NewNode = new Node;
	NewNode->playerData = player;
	NewNode->next = nullptr;
	NewNode->prev = nullptr;
	if (Head == nullptr)
	{
		Head = NewNode;
	}
	else
	{
		Node* temp = Head;
		while (temp->next!= nullptr)
		{
			temp = temp->next;
		}
		temp->next = NewNode;
		NewNode->prev = temp;

	}
}

void D_LinkedList::InsertAt(Barca_Team player, int pos)
{
	Node* NewNode = new Node;
	NewNode->playerData = player;
	if (Head == nullptr)
	{
		cout << "The list is empty" << endl;
	}
    else if (pos == 0)
	{
		Node* temp = Head;
		NewNode->next = Head->next;
		Head->next->prev = NewNode;
		Head = NewNode;
		delete temp;
	}
	else
	{
		Node* temp = Head;
		for (int i=0; i< pos &&temp != nullptr; i++)
		{
			temp = temp->next;
		}
		if (temp == nullptr)
		{
			cout << "the player has not found " << endl;
		}
		else
		{
			NewNode->prev = temp->prev;
			temp->prev->next = NewNode;
			if(temp->next != nullptr)
			temp->next->prev = NewNode;
			NewNode->next = temp->next;
			delete temp;
		}
	}
}

void D_LinkedList::Desplay()
{
	Node* Item = Head;
	while (Item!= NULL)
	{
		cout <<"Name: " << Item->playerData.name << endl;
		cout <<"Age: " << Item->playerData.age << endl;
		cout <<"Nationality" << Item->playerData.nationality << endl;
		cout <<"Place" << Item->playerData.place << endl;
		cout << "Number: " << Item->playerData.playerNumber << endl;
		Item = Item->next;
		cout << "===============================" << endl;

	}
}

void D_LinkedList::Remove(int pos)
{
	if (Head==nullptr)
	{
		cout << "The list is empty" << endl;
	}
	else if (pos == 0)
	{
		Node* temp = Head;
		Head = Head->next;
		if (Head->prev != NULL)
			Head->prev = NULL;
		delete temp;
	}
	else
	{
		Node* temp = Head;
		for (int i=0; i < pos && temp != nullptr ; i++)
		{
			temp = temp->next;
		}
		if (temp == nullptr)
		{
			cout << "You Entered Unvalied number " << endl;
		}
		else
		{
			temp->prev->next = temp->next;
			if (temp->next != NULL)
			{
				temp->next->prev = temp->prev;
			}
			delete temp;
		}
	}
}

void D_LinkedList::Search(int pos)
{
	Node* Item = Head;
	while (Item != nullptr)
	{
		if (Item->playerData.playerNumber == pos)
		{
			cout << "The player number "<<pos <<" is: " << endl;
			cout << "Name: " << Item->playerData.name << endl;
			cout << "Age: " << Item->playerData.age << endl;
			cout << "Nationality: " << Item->playerData.nationality << endl;
			cout << "Place: " << Item->playerData.place << endl;
			cout << "Number: " << Item->playerData.playerNumber << endl;
			cout << "===============================" << endl;
			Item = nullptr;
		}
		else
		{
			Item = Item->next;
		}
	}
	if (Item == nullptr)
	{
		cout << "There is no player with the number " << pos << " in the list" << endl;
		cout << "===============================" << endl;
	}
}












