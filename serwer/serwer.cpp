// serwer.cpp : Defines the entry point for the console application.
#pragma once
#define _WINSOCKAPI_    // stops windows.h including winsock.h
#include <windows.h>
#include "stdafx.h"
#include "../objects.h"
#include <map>

//#include "../quaternion.h"
#include "../net.h"
unicast_net *uni_reciv;						   // object (see net module) to recive messages from other applications
unicast_net *uni_send;                  // ...  to send messages ...
std::map<int, MovableObject*> movable_objects; 
struct Frame                                       // The main structure of net communication between aplications. Homogenous for simpicity.
{
	int type;                                      // frame type  
	int iID;                                       // object identifier 
	ObjectState state;                             // object state values (see object module)

	//long send_time;                                // time stamp
	//int iID_receiver;                              // not used
};
std::map<int, unsigned long> adresy;

DWORD WINAPI ReceiveThreadFun(void *ptr)
{
	unicast_net *pmt_net = (unicast_net*)ptr;  // the pointer to the object of multicast_net class (see net module)
	Frame frame;
	unsigned long adres_nadawcy =0;
	while (1)
	{
		int frame_size = pmt_net->reciv((char*)&frame,&adres_nadawcy, sizeof(Frame));   // waiting for frame 
		ObjectState state = frame.state;
			if (movable_objects[frame.iID] == NULL)           // object hasn't registered up till now 
				                        // == nullptr (C++ 11) it is not available in older versions than VC 2013
			{
				MovableObject *ob = new MovableObject();
				ob->iID = frame.iID;
				movable_objects[frame.iID] = ob;              // registration of new object 
		         
				//fprintf(f, "alien object ID = %d was registred\n", ob->iID);
			}
			movable_objects[frame.iID]->ChangeState(state);   // updating the state of the object
			if(adresy[frame.iID]== NULL){
				adresy[frame.iID] = adres_nadawcy;
			}
	}  // while(1)
	return 1;
}
void send(MovableObject * movable_object, unsigned long adres) {

	Frame frame;
	frame.state = movable_object->State();                // state of my own object
	frame.iID = movable_object->iID;                      // my object identifier

	uni_send->send((char*)&frame, adres , sizeof(Frame));
}
int main(int argc, _TCHAR* argv[])
{
	unicast_net *	uni_reciv = new unicast_net( 10002);      // object for receiving messages
	unicast_net	* uni_send = new unicast_net( 10001); 
	while(1)
	{

		std::map<int, MovableObject*>::iterator it = movable_objects.begin();

		ReceiveThreadFun(uni_reciv);
		while (it != movable_objects.end()) {
				std::map<int, unsigned long>::iterator iterator_adresy = adresy.begin();
				while (iterator_adresy != adresy.end()) {
						send((*it).second, (*iterator_adresy).second);
						iterator_adresy++;
				}
		it++;
		
		}
	}
 	
	return 0;
}

