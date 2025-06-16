#include<iostream>
#include"Declareation.h"

PQL::PQL()
{
	front = nullptr;
}
void PQL::Enqueue(Barca_Team player, int priority)
{
	Node* newNode = new Node;
	newNode->data = player;
	newNode->priority = priority;
	newNode->next = nullptr;
	if (front == nullptr || front->priority > priority)
	{
		newNode->next = front;
		front = newNode;
	}
	else
	{
		Node* temp = front;
		while (temp->next != nullptr && temp->next->priority <= priority)
		{
			temp = temp->next;
		}
		newNode->next = temp->next;
		temp->next = newNode;
	}
}
void PQL::Dequeue()
{
	if (IsEmpty())
	{
		cout << "the queue is empty" << endl;
	}
	else
	{
		Node* temp = front;
		front = front->next;
		delete temp;
	}
}
bool PQL::IsEmpty()
{
	return front == nullptr;
}
void PQL::Update(int playerNumber, int NewPriority)
{
	if (front == nullptr)
	{
		cout << "the queue is empty" << endl;
	}
	else
	{
		Node* temp = front;
		Node* prev = nullptr;
		while (temp != nullptr && temp->data.playerNumber != playerNumber)
		{
			prev = temp;
			temp = temp->next;
		}
		if (temp == nullptr)
		{
			cout << "The player is not in the queue" << endl;
		}
		else
		{
			if (prev == nullptr)
			{
				front = front->next;
			}
			else
			{
				prev->next = temp->next;
			}
			Enqueue(temp->data, NewPriority);
			delete temp;
		}
		
	  
	}





}
void PQL::Desplay()
{
	Node* Item = front;
	while (Item != nullptr)
	{
		cout << "Name: " << Item->data.name << endl;
		cout << "Age: " << Item->data.age << endl;
		cout << "Nationality: " << Item->data.nationality << endl;
		cout << "Place: " << Item->data.place << endl;
		cout << "Number: " << Item->data.playerNumber << endl;
		Item = Item->next;
		cout << "===============================" << endl;
	}
}
void PQL::search(int playerNumber)
{
	if (front == nullptr)
	{
		cout << "the queue is empty " << endl;
	}
	else
	{
		Node* temp = front;
		while (temp != nullptr && temp->data.playerNumber != playerNumber)
		{
			temp = temp->next;
		}
		if (temp == nullptr)
		{
			cout << "the player is not in the queue" << endl;
		}
		else
		{
			cout << "Name: " << temp->data.name << endl;
			cout << "Age: " << temp->data.age << endl;
			cout << "Nationality: " << temp->data.nationality << endl;
			cout << "Place: " << temp->data.place << endl;
			cout << "Number: " << temp->data.playerNumber << endl;
			cout << "The priority in the queue: " << temp->priority << endl;

		}
	
	}
}

