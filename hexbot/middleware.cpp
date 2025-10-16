#include <bits/stdc++.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "colors.h"
#include "coord.h"
#include "grid.h"
#include "node.h"

using namespace std;

/*      Middleware
 *      This class provices the interface to interact with the Hex server.
 *      It is responsible for handling the requests and responses.
 *
 *      On initialization, it will create a socket and connect to the server.
 *      It will then send a request to the server to join the game.
 *
 */

#define SERVER "127.0.0.1"
#define PORT 1234

// Socket and connection information
int status, valread, client_fd, turn;
struct sockaddr_in serv_addr;
HexGrid hexGrid;

// Board information
COLOUR ourColour;
int boardSize;

bool openSocket()
{
    // Create a socket
    // Connect to the server
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cerr << "Error: Socket creation error" << strerror(errno) << endl;
        return false;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, SERVER, &serv_addr.sin_addr) <= 0)
    {
        cerr << "Error: Invalid address/ Address not supported" << strerror(errno) << endl;
        return false;
    }

    if ((status = connect(client_fd, (struct sockaddr *)&serv_addr,
                          sizeof(serv_addr))) < 0)
    {
        cerr << "Error: Connection Failed " << strerror(errno) << endl;
        return false;
    }
    return true;
}

bool sendRequest(string request)
{
    // Send a request to the server
    // Return true if the request was sent successfully
    // Return false if the request was not sent successfully
    if (send(client_fd, request.c_str(), request.length(), 0) < 0)
    {
        cerr << "Error: Request not sent" << strerror(errno) << endl;
        return false;
    }
    return true;
}

string getResponse()
{
    // Get a response from the server
    // Return the response
    const int buffer_size = 4096;
    char buffer[buffer_size] = {0};
    valread = read(client_fd, buffer, buffer_size);
    return string(buffer);
}

bool makeMove(Coord move)
{
    // Send a move to the server
    // Return true if the move was sent successfully
    // Return false if the move was not sent successfully
    string request = to_string(move.q) + "," + to_string(move.r) + "\n";
    bool result = sendRequest(request);
    if(result){
        hexGrid.Update(move, ourColour);
    }
    return result;
}

vector<string> split(string str, char delimiter)
{
    // Split a string by a delimiter
    // Return a vector of strings
    vector<string> internal;
    stringstream ss(str); // Turn the string into a stream.
    string tok;

    while (getline(ss, tok, delimiter))
    {
        internal.push_back(tok);
    }

    return internal;
}

bool closeSocket()
{
    // Close the socket
    // Return true if the socket was closed successfully
    // Return false if the socket was not closed successfully
    if (close(client_fd) < 0)
    {
        cerr << "Error: Socket not closed" << strerror(errno) << endl;
        return false;
    }
    return true;
}

string getOurColour()
{
    if (ourColour == COLOUR::RED)
    {
        return "R";
    }
    else
    {
        return "B";
    }
}

COLOUR getColour(string colour)
{
    if (colour[0] == 'R')
    {
        return COLOUR::RED;
    }
    else if (colour[0] == 'B')
    {
        return COLOUR::BLUE;
    }
    else
    {
        return COLOUR::N;
    }
}

// Take two functions as parameters
// isSwap(x, y)
//      Returns true if the move at (x, y) will swap the board
//      Returns false if the move at (x, y) will not swap the board
// getMove(board, boardSize, (int)ourColour)
//      Returns a pair of integers (x, y) representing the move to make
void HexConnector(
    bool (*isSwap)(int, int),
    Coord (*getMove)(HexGrid *, COLOUR, Coord))
{
    // Open socket
    openSocket();

    // Enter game loop
    for (turn = 0;; turn++)
    {
        string response = getResponse();
        cout << "From server:" << response << endl;
        if (response.find("END") != string::npos)
        {
            // Game has ended
            printf("Game has ended\n");
            break;
        }
        // Parse response by splitting on ';'
        vector<string> message = split(response, ';');
        if (message[0] == "START")
        {
            // START;boardSize;colour
            boardSize = stoi(message[1]);
            ourColour = getColour(message[2]);
            if (ourColour == COLOUR::RED)
            {
                cout << "We are red" << endl;
                Coord move = getMove(&hexGrid, ourColour,Coord{-1,-1});
                makeMove(move);
            }
            else
            {
                cout << "We are blue" << endl;
            }
        }
        else if (message[0] == "CHANGE")
        {
            Coord pmove;
            if (message[1] == "SWAP")
            {
                pmove={-4,-2}; //secret code for telling the thing it swapped
                ourColour = flip(ourColour);
                printf("Colour changed to %s\n", getOurColour().c_str());
            }else{
                vector<string> move = split(message[1], ',');
                pmove = {stoi(move[0]), stoi(move[1])};
            }
            if (getColour(message[3]) == ourColour)
            {
                cout << "It is our turn" << endl;
                // Our time to move.
                if (turn == 1)
                {
                    // Opponent has made their first move
                    // We need to decide if we want to swap.
                    vector<string> move = split(message[1], ',');
                    if (isSwap(stoi(move[0]), stoi(move[1])))
                    {
                        // We want to swap
                        sendRequest("SWAP\n");
                        // No coulur change here - server will send us a CHANGE message
                        continue;
                    }
                }
                // Initialise board
                vector<vector<COLOUR>> board = vector<vector<COLOUR>>(boardSize, vector<COLOUR>(boardSize, COLOUR::N));
                // Parse board
                vector<string> boardString = split(message[2], ',');
                if (boardString.size() != boardSize)
                {
                    cout << "Error: Board size mismatch in rows." << endl;
                    cerr << "Error: Board size mismatch in rows." << endl;
                    break;
                }
                for (int i = 0; i < boardSize; i++)
                {
                    if (boardString[i].length() != boardSize)
                    {
                        cout << "Error: Board size mismatch on row " << to_string(i) << endl;
                        cerr << "Error: Board size mismatch on row " << to_string(i) << endl;
                        break;
                    }
                    for (int j = 0; j < boardSize; j++)
                    {
                        board[i][j] = getColour(boardString[i].substr(j, 1));
                    }
                }
                // Update to grid
                hexGrid.Update(board, boardSize);
                // Get move
                Coord move = getMove(&hexGrid, ourColour,pmove);
                // Make move
                makeMove(move);
            }
            else
            {
                cout << "It is not our turn" << endl;
            }
        }
        else
        {
            // Should not happen
            cout << "Error: Invalid response" << endl;
            cerr << "Error: Invalid response:" << message[0] << endl;
        }
    }

    // Game ends. Close socket.
    closeSocket();
}
