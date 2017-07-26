#ifndef LR_WPAN_NODE_HELPER_H
#define LR_WPAN_NODE_HELPER_H

#include <ns3/log.h>
#include <ns3/core-module.h>
#include <ns3/lr-wpan-module.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/propagation-delay-model.h>
#include <ns3/simulator.h>
#include <ns3/single-model-spectrum-channel.h>
#include <ns3/multi-model-spectrum-channel.h>
#include <ns3/constant-position-mobility-model.h>
#include <ns3/packet.h>

#include <iostream>


namespace ns3{

const int numNodes = 200;

double totalTime;

bool received[numNodes];

	class NodeHelperClass{

		Node node;
		LrWpanNetDevice device;

		uint32_t keys[50]; //Keys array
		Mac16Address neighbour_table_address[numNodes];
		uint32_t neighbour_table_keys[numNodes];


		static void DataConfirm (McpsDataConfirmParams params){

		  NS_LOG_UNCOND ("LrWpanMcpsDataConfirmStatus = " << params.m_status);
		}

		static void StateChangeNotification (std::string context, Time now, LrWpanPhyEnumeration oldState, LrWpanPhyEnumeration newState){
		}


		public:
		ConstantPositionMobilityModel mobility;
		uint32_t neighbour_table_size;
		void memsetTable();
		uint32_t size;  //number of keys selected
		uint32_t poolSize; //pool size as described

		Mac16Address getNeighbourTableAddress(int n);
		uint32_t getNeighbourTableKeys(int n);
		uint32_t getNeighbourTableSize();

		void setNeighbourTableAddress(int n, Mac16Address address);
		void setNeighbourTableKeys(int n, uint32_t key);
		void setNeighbourTableSize();

		uint8_t cache[1000];
		bool state; // true for relay, false for initial
		bool checkCache(uint8_t* in);
		void addToCache(uint8_t* in);
		bool compare(uint8_t* in1, uint8_t* in2);
		NodeHelperClass(char address[], Ptr<MultiModelSpectrumChannel> channel, Vector position, uint16_t size, uint16_t poolSize);
		~NodeHelperClass();
		void BroadcastKeys(double);
		LrWpanNetDevice* GetNetDevice();
		void populateKeyArray();
		static uint32_t RandAB (uint32_t a, uint32_t b);
		void Broadcast(double seconds, uint8_t data[]);
		bool checkNeighbourhoodTable(Mac16Address address); //check if address already present
		void displayTable();
		bool checkMyKeys(uint32_t ikey);
		void SendEncrypted(double seconds, uint8_t data[], Mac16Address address);

	};


	NodeHelperClass *n[numNodes];   ////////////////////

}
#endif /* LR_WPAN_NODE_HELPER_H */
