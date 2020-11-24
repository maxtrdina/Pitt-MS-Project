# gcc agent.c map.c agent_control.c agent_data.c -o mininet/agent -lpthread -std=gnu99
gcc manager.c network_manager.c topology_manager.c net_util.c map.c -o mininet/manager -lpthread -std=gnu99
gcc client.c net_util.c -o mininet/client -lpthread -std=gnu99

gcc -g -O2 -Wall -D_REENTRANT -DNDEBUG -fPIC -c -o tmp/map.o map.c
gcc -g -O2 -Wall -D_REENTRANT -DNDEBUG -fPIC -c -o tmp/agent_control.o agent_control.c
gcc -g -O2 -Wall -D_REENTRANT -DNDEBUG -fPIC -c -o tmp/agent_data.o agent_data.c

gcc -g -O2 -Wall -D_REENTRANT -DNDEBUG -fPIC  -Ispines/stdutil/include -Ispines/libspread-util/include -Ispines/libspines -Ispines/daemon -Ispines/stdutil/include -Ispines/libspread-util/include -Ispines/libspines -Ispines/daemon -c -o tmp/agent.o agent.c
gcc -Lspines/stdutil/lib -Lspines/libspread-util/lib -Lspines/libspines -Lspines/stdutil/lib -Lspines/libspread-util/lib -Lspines/libspines -o mininet/agent tmp/agent.o tmp/map.o tmp/agent_control.o tmp/agent_data.o spines/stdutil/lib/libstdutil.a spines/libspread-util/lib/libspread-util.a spines/libspines/libspines.a -ldl -lrt -lm -lnsl -lpthread -lcrypto

gcc -g -O2 -Wall -D_REENTRANT -DNDEBUG -fPIC  -Ispines/stdutil/include -Ispines/libspread-util/include -Ispines/libspines -Ispines/daemon -Ispines/stdutil/include -Ispines/libspread-util/include -Ispines/libspines -Ispines/daemon -c -o tmp/in_test.o in_test.c
gcc -Lspines/stdutil/lib -Lspines/libspread-util/lib -Lspines/libspines -Lspines/stdutil/lib -Lspines/libspread-util/lib -Lspines/libspines -o mininet/in_test tmp/in_test.o tmp/map.o tmp/agent_control.o tmp/agent_data.o spines/stdutil/lib/libstdutil.a spines/libspread-util/lib/libspread-util.a spines/libspines/libspines.a -ldl -lrt -lm -lnsl -lpthread -lcrypto

gcc -g -O2 -Wall -D_REENTRANT -DNDEBUG -fPIC  -Ispines/stdutil/include -Ispines/libspread-util/include -Ispines/libspines -Ispines/daemon -Ispines/stdutil/include -Ispines/libspread-util/include -Ispines/libspines -Ispines/daemon -c -o tmp/out_test.o out_test.c
gcc -Lspines/stdutil/lib -Lspines/libspread-util/lib -Lspines/libspines -Lspines/stdutil/lib -Lspines/libspread-util/lib -Lspines/libspines -o mininet/out_test tmp/out_test.o tmp/map.o tmp/agent_control.o tmp/agent_data.o spines/stdutil/lib/libstdutil.a spines/libspread-util/lib/libspread-util.a spines/libspines/libspines.a -ldl -lrt -lm -lnsl -lpthread -lcrypto
