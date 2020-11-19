gcc agent.c map.c agent_control.c agent_data.c -o mininet/agent -lpthread -std=gnu99
gcc manager.c network_manager.c topology_manager.c net_util.c map.c -o mininet/manager -lpthread -std=gnu99
gcc client.c net_util.c -o mininet/client -lpthread -std=gnu99
