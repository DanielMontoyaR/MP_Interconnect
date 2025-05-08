For running/compiling the Main code, you must be inside of the project folder and go /src/: 
  cd /src/
Then you must run the following line to make the executable: 
  g++ -std=c++20 Main.cpp MemorySave.cpp -o main

And for executing the executable file you must run: 
  ./main
Inside the file Main.cpp, you have the option to run using stepping and select the Scheduler you want to use.
For enabling the Stepping, you change the line 33 to: 
  int stepping = 1;

For enabling the FIFO Scheduler, use_fifo_policy must be true, if you want to use QoS, then use_fifo_policy must be false (line 156):
  SchedulerWrapper scheduler(/*use_fifo_policy=*/false);

Finally, if you want to run the Stepping GUI, you must go to the folder named Graphics.
  cd /Graphics/
And run the following command:
  python3 SteppingGUI.py
Important: Inside the Main.cpp stepping must be 1. You must have two Terminals, one running the executable file main and other one running the SteppingGUI. 


