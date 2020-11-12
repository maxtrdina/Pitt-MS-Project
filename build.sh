gcc agent.c map.c control.c -o agent -lpthread -std=gnu99
gcc manager.c network_manager.c topology_manager.c net_util.c map.c -o manager -lpthread -std=gnu99
gcc client.c net_util.c -o client -lpthread -std=gnu99
