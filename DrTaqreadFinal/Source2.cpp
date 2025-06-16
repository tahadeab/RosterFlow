#include<iostream>
#include"Declareation.h"
StackLinkedList::StackLinkedList()
{
	Top = nullptr;
}
void StackLinkedList::Push(Barca_Team player)
{
	Node* NewNode = new Node;
	NewNode->playerdata = player;
	if (IsEmpty())
	{
	    NewNode->next = nullptr;
		Top = NewNode;
	}
	else
	{
		NewNode->next = Top;
		Top = NewNode;
	}
}

bool StackLinkedList::IsEmpty()
{
	return Top == nullptr;
}

void StackLinkedList::Pop()
{
	if (IsEmpty())
	{
		cout << "The list is empty" << endl;
	}
	else
	{
		Node* curent = Top;
		Top = Top->next;
		delete curent;
	}
}

void StackLinkedList::Peek()
{
	cout << "Name: " << Top->playerdata.name << endl;
	cout << "Age: " << Top->playerdata.age << endl;
	cout << "Nationality: " << Top->playerdata.nationality << endl;
	cout << "Place: " << Top->playerdata.place << endl;
	cout << "T-Number: " << Top->playerdata.playerNumber << endl;
}

void StackLinkedList::Desplay()
{ 
	Node* temp = Top;
	while (temp != nullptr)
	{
		cout << "Name: " << temp->playerdata.name << endl;
		cout << "Age: " << temp->playerdata.age << endl;
		cout << "Nationality: " << temp->playerdata.nationality << endl;
		cout << "Place: " << temp->playerdata.place << endl;
		cout << "T-Number: " << temp->playerdata.playerNumber << endl;
		cout << "===============================" << endl;
		temp = temp->next;
	}

}











