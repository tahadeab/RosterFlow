#include <iostream>
#include"Declareation.h"
using namespace std;
int main()
{
	Barca_Team player1 = Barca_Team("Messi",37,"Argentina","RW",10);
	Barca_Team player2 = Barca_Team("Inaki Pena", 22, "Spain", "GK",99);
	Barca_Team player3 = Barca_Team("Cubarsi", 17, "Spain", "CB",2);
	Barca_Team player4 = Barca_Team("L.Martinez", 30, "Spain", "CB",5);
	Barca_Team player5 = Barca_Team("Balde", 21, "Spain", "LB",3);
	Barca_Team player6 = Barca_Team("Kounde",24, "France", "RB",23);
	Barca_Team player7 = Barca_Team("M.Casado",19, "Spain", "LM",17);
	Barca_Team player8 = Barca_Team("Yamal",17, "Spain", "RW",19);
	Barca_Team player9 = Barca_Team("Raphiha",25, "Brazil", "LW",11);
	Barca_Team player10 = Barca_Team("Eric", 29, "Spain", "RM",24);
	Barca_Team player11 = Barca_Team("Lewandowski", 33, "Poland", "ST",9);
	Barca_Team player12 = Barca_Team("Pedri",22, "Spain", "LM",8);



	//D_LinkedList List = D_LinkedList();
	//List.Append(player1);
	//List.Append(player3);
	//List.InsertAt(player2,1);
	////List.Remove(2);
	//List.Search(4);
	//List.Desplay();
	
	
	//PQL priorityQueue1 = PQL();
	//priorityQueue1.Enqueue(player1, 1);
	//priorityQueue1.Enqueue(player2, 3);
	//priorityQueue1.Enqueue(player3, 5);
	//priorityQueue1.Enqueue(player6, 4);
	//priorityQueue1.Enqueue(player4, 2);
	////priorityQueue1.Dequeue(); 
	//priorityQueue1.Update(5, 3); // التعديل يكون برقم اللاعب والاولويه الجديده له

	////priorityQueue1.Desplay();
	//priorityQueue1.search(10);


	StackLinkedList Sta1 = StackLinkedList();
	Sta1.Push(player1);
	Sta1.Push(player3);
	Sta1.Push(player4);
	Sta1.Pop();
	//Sta1.Desplay();
	Sta1.Peek();
	
	
	
	
	return 0;
}
