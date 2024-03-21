/*
 * Copyright 2013-2020 Automatak, LLC
 *
 * Licensed to Green Energy Corp (www.greenenergycorp.com) and Automatak
 * LLC (www.automatak.com) under one or more contributor license agreements.
 * See the NOTICE file distributed with this work for additional information
 * regarding copyright ownership. Green Energy Corp and Automatak LLC license
 * this file to you under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License. You may obtain
 * a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <opendnp3/ConsoleLogger.h>
#include <opendnp3/DNP3Manager.h>
#include <opendnp3/channel/PrintingChannelListener.h>
#include <opendnp3/logging/LogLevels.h>
#include <opendnp3/master/DefaultMasterApplication.h>
#include <opendnp3/master/PrintingCommandResultCallback.h>
#include <opendnp3/master/PrintingSOEHandler.h>

using namespace std;
using namespace opendnp3;

class TestSOEHandler : public ISOEHandler
{
    virtual void BeginFragment(const ResponseInfo& info){
        std::cout << "BeginFragment unsolicited : " << info.unsolicited
        		<< " fir : " << info.fir
        	    << " fin : " << info.fin
				<< std::endl;
    };

    virtual void EndFragment(const ResponseInfo& info){
        std::cout << "EndFragment unsolicited : " << info.unsolicited
        		<< " fir : " << info.fir
        	    << " fin : " << info.fin
				<< std::endl;
    };

    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<Binary>>& values) {
        auto print = [](const Indexed<Binary>& pair) {
        	std::ostringstream oss;
        	oss << pair.value.value;
            std::cout << "Binary : "
            		<< "[" << pair.index << "] : "
					<< oss.str() << " : "
					<< static_cast<int>(pair.value.flags.value) << " : "
					<< pair.value.time.value
					<< std::endl;
        };
        values.ForeachItem(print);
    };

    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<DoubleBitBinary>>& values) {};

    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<Analog>>& values) {
        auto print = [](const Indexed<Analog>& pair) {
        	std::ostringstream oss;
        	oss << pair.value.value;
            std::cout << "Analog : "
            		<< "[" << pair.index << "] : "
					<< oss.str() << " : "
					<< static_cast<int>(pair.value.flags.value) << " : "
					<< pair.value.time.value
					<< std::endl;
        };
        values.ForeachItem(print);
    };

    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<Counter>>& values) {};

    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<FrozenCounter>>& values) {};

    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<BinaryOutputStatus>>& values) {
        auto print = [](const Indexed<BinaryOutputStatus>& pair) {
        	std::ostringstream oss;
        	oss << pair.value.value;
            std::cout << "BinaryOutputStatus : "
            		<< "[" << pair.index << "] : "
					<< oss.str() << " : "
					<< static_cast<int>(pair.value.flags.value) << " : "
					<< pair.value.time.value
					<< std::endl;
        };
        values.ForeachItem(print);

    };

    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<AnalogOutputStatus>>& values) {
        auto print = [](const Indexed<AnalogOutputStatus>& pair) {
        	std::ostringstream oss;
        	oss << pair.value.value;
            std::cout << "AnalogOutputStatus : "
            		<< "[" << pair.index << "] : "
					<< oss.str() << " : "
					<< static_cast<int>(pair.value.flags.value) << " : "
					<< pair.value.time.value
					<< std::endl;
        };
        values.ForeachItem(print);
    };

    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<OctetString>>& values) {};

    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<TimeAndInterval>>& values) {};

    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<BinaryCommandEvent>>& values) {
        auto print = [](const Indexed<BinaryCommandEvent>& pair) {
        	std::ostringstream oss;
        	oss << pair.value.value;
            std::cout << "BinaryCommandEvent : "
            		<< "[" << pair.index << "] : "
					<< oss.str() << " : "
					<< pair.value.time.value
					<< std::endl;
        };
        values.ForeachItem(print);

    };

    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<AnalogCommandEvent>>& values) {};

    virtual void Process(const HeaderInfo& info, const ICollection<DNPTime>& values) {};

};

int main(int argc, char* argv[])
{
	char cIp[32] = {0};
	int  iPort;	//20000
	
	if(argc != 3)
	{
		printf("Usage: ./rtx_master_ao_write [IP] [PORT]\n");
		return -1;
	}
	
	strcpy(cIp, argv[1]);
	iPort = stoi(argv[2]);
	
	printf("argc = %d\n", argc);
	printf("cIp = %s\n", cIp);
	printf("iPort = %d\n", iPort);
	
    // Specify what log levels to use. NORMAL is warning and above
    // You can add all the comms logging by uncommenting below
    //const auto logLevels = levels::ERR | levels::ALL_APP_COMMS;
    const auto logLevels = flags::ERR;

    // This is the main point of interaction with the stack
    DNP3Manager manager(1, ConsoleLogger::Create());

    // Connect via a TCPClient socket to a outstation
    auto channel = manager.AddTCPClient("tcpclient", logLevels, ChannelRetry::Default(), {IPEndpoint(cIp, iPort)},
                                        "0.0.0.0", PrintingChannelListener::Create());

    // The master config object for a master. The default are
    // useable, but understanding the options are important.
    MasterStackConfig stackConfig;

    // you can override application layer settings for the master here
    // in this example, we've change the application layer timeout to 2 seconds
    stackConfig.master.responseTimeout = TimeDuration::Seconds(5);
    stackConfig.master.disableUnsolOnStartup = false;

    // You can override the default link layer settings here
    // in this example we've changed the default link layer addressing
    stackConfig.link.LocalAddr = 3;
    stackConfig.link.RemoteAddr = 4;

    // Create a new master on a previously declared port, with a
    // name, log level, command acceptor, and config info. This
    // returns a thread-safe interface used for sending commands.
    auto master = channel->AddMaster("master",                           // id for logging
                                     PrintingSOEHandler::Create(),       // callback for data processing
                                     DefaultMasterApplication::Create(), // master application instance
                                     stackConfig                         // stack configuration
    );

    auto test_soe_handler = std::make_shared<TestSOEHandler>();

    // do an integrity poll (Class 3/2/1/0) once per minute
    auto integrityScan = master->AddClassScan(ClassField::AllClasses(), TimeDuration::Max(), test_soe_handler);
//    auto integrityScan = master->AddScan(TimeDuration::Max(), {Header::AllObjects(32, 6)}, test_soe_handler);
//    auto integrityScan = master->AddClassScan(ClassField(ClassField::CLASS_1), TimeDuration::Max(), test_soe_handler);

    // do a Class 1 exception poll every 5 seconds
    auto exceptionScan0 = master->AddClassScan(ClassField(ClassField::CLASS_0), TimeDuration::Max(), test_soe_handler);
    auto exceptionScan1 = master->AddClassScan(ClassField(ClassField::CLASS_1), TimeDuration::Max(), test_soe_handler);
    auto exceptionScan2 = master->AddClassScan(ClassField(ClassField::CLASS_2), TimeDuration::Max(), test_soe_handler);
    auto exceptionScan3 = master->AddClassScan(ClassField(ClassField::CLASS_3), TimeDuration::Max(), test_soe_handler);

    // Enable the master. This will start communications.
    master->Enable();

    bool channelCommsLoggingEnabled = true;
    bool masterCommsLoggingEnabled = true;

    auto cammandtest = [](const ICommandTaskResult& result){
         std::cout << "ICommandTaskResult : Received command result w/ summary: " << TaskCompletionSpec::to_string(result.summary) << std::endl;

          auto print = [](const CommandPointResult& res) {
              std::cout << "Header: " << res.headerIndex
            		    << " Index: " << res.index
                        << " State: " << CommandPointStateSpec::to_string(res.state)
                        << " Status: " << CommandStatusSpec::to_string(res.status) << std::endl;;
          };
          result.ForeachItem(print);
    };

    int16_t ival = 3;
    int iUserVal = 0;

	sleep(1);

    while (true)
    {
        //~ std::cout << "Enter a command" << std::endl;
        //~ std::cout << "x - exits program" << std::endl;
        //~ std::cout << "a - performs an ad-hoc range scan" << std::endl;
        //~ std::cout << "i - integrity demand scan" << std::endl;
        //~ std::cout << "e - exception demand scan" << std::endl;
        //~ std::cout << "d - disable unsolicited" << std::endl;
        //~ std::cout << "r - cold restart" << std::endl;
        //~ std::cout << "c - send crob" << std::endl;
        //~ std::cout << "t - toggle channel logging" << std::endl;
        //~ std::cout << "u - toggle master logging" << std::endl;

		cout << "\n===========================================\n";

        char cmd;
        cout << "Enter a AO index: (0~4)";
        std::cin >> cmd;
        ival++;
        
        
        
        cout << "Enter write to AO value: ";
        std::cin >> iUserVal;
        
        switch (cmd)
        {
        //~ case ('0'):
            	//~ exceptionScan0->Demand();
				//~ break;

        //~ case ('1'):
            	//~ exceptionScan1->Demand();
				//~ break;

        //~ case ('2'):
            	//~ exceptionScan2->Demand();
				//~ break;

        //~ case ('3'):
                //~ exceptionScan3->Demand();
				//~ break;

        case ('0'):
//				master->DirectOperate(AnalogOutputInt16(3,CommandStatus::NO_SELECT), 2, cammandtest);
				master->SelectAndOperate(AnalogOutputInt16(iUserVal), 0, cammandtest);
				break;
        case ('1'):
//				master->DirectOperate(AnalogOutputInt16(3,CommandStatus::NO_SELECT), 2, cammandtest);
				master->SelectAndOperate(AnalogOutputInt16(iUserVal), 1, cammandtest);
				break;
		case ('2'):
//				master->DirectOperate(AnalogOutputInt16(3,CommandStatus::NO_SELECT), 2, cammandtest);
				master->SelectAndOperate(AnalogOutputInt16(iUserVal), 2, cammandtest);
				break;
        case ('3'):
//				master->DirectOperate(AnalogOutputInt16(3,CommandStatus::NO_SELECT), 2, cammandtest);
				master->SelectAndOperate(AnalogOutputInt16(iUserVal), 3, cammandtest);
				break;
        case ('4'):
//				master->DirectOperate(AnalogOutputInt16(3,CommandStatus::NO_SELECT), 2, cammandtest);
				master->SelectAndOperate(AnalogOutputInt16(iUserVal), 4, cammandtest);
				break;

        //~ case ('a'):
            //~ master->ScanRange(GroupVariationID(42, 2), 0, 4, test_soe_handler);
            //~ break;
        //~ case ('d'):
         //~ master->PerformFunction("ASSIGN_CLASS", FunctionCode::ASSIGN_CLASS, {Header::Range16(60, 2, 0, 2)});
            //~ break;
        //~ case ('r'):
        //~ {
            //~ auto print = [](const RestartOperationResult& result) {
                //~ if (result.summary == TaskCompletion::SUCCESS)
                //~ {
                    //~ std::cout << "Success, Time: " << result.restartTime.ToString() << std::endl;
                //~ }
                //~ else
                //~ {
                    //~ std::cout << "Failure: " << TaskCompletionSpec::to_string(result.summary) << std::endl;
                //~ }
            //~ };
            //~ master->Restart(RestartType::COLD, print);
            //~ break;
        //~ }
        //~ case ('x'):
            //~ // C++ destructor on DNP3Manager cleans everything up for you
            //~ return 0;
        //~ case ('i'):
            //~ integrityScan->Demand();
            //~ break;
        //~ case ('e'):
            //~ exceptionScan1->Demand();
            //~ break;
        //~ case ('c'):
        //~ {
            //~ ControlRelayOutputBlock crob(OperationType::LATCH_ON);
            //~ master->SelectAndOperate(crob, 0, PrintingCommandResultCallback::Get());
            //~ break;
        //~ }
        //~ case ('t'):
        //~ {
            //~ channelCommsLoggingEnabled = !channelCommsLoggingEnabled;
            //~ auto levels = channelCommsLoggingEnabled ? levels::ALL_COMMS : levels::NORMAL;
            //~ channel->SetLogFilters(levels);
            //~ std::cout << "Channel logging set to: " << levels.get_value() << std::endl;
            //~ break;
        //~ }
        //~ case ('u'):
        //~ {
            //~ masterCommsLoggingEnabled = !masterCommsLoggingEnabled;
            //~ auto levels = masterCommsLoggingEnabled ? levels::ALL_COMMS : levels::NORMAL;
            //~ master->SetLogFilters(levels);
            //~ std::cout << "Master logging set to: " << levels.get_value() << std::endl;
            //~ break;
        //~ }
        default:
            std::cout << "Unknown action: " << cmd << std::endl;
            break;
        }
    }

    return 0;
}
