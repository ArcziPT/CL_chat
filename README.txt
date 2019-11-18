Comand Line Chat based on ncurses.

Server automaticly starts main channel, which is used by new users to register.

Evry channel can be configured to:
- require authorization or not
- run on specified port

When connection is established, client gets its own session_id, which he sends with every request to authenticate.

Beacuse Server and Client app are multithreaded in order to achieve simultaneous message(from server/client)/error printing and input reading it was necessary to use thread synchronization in access to ncurses( which provided access to terminal).	