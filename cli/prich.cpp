#include "../cli/prich.hpp"
#include "../head/define.hpp"

bool isIllegalSequence(const std::string& sequence)
{
    static const std::set<std::string> illegalSequences = {
        "^[OA", "^[OB", "^[OD", "^[OC", "^[[3~", "^[OF", "^[[6~", "^[[OH", "^[[5~", "^[[OM"
    };
    return illegalSequences.count(sequence) > 0;
}

void Prich::run()
{
    try
    {
        std::cout << "Start Chat !!!!" << std::endl;
        std::vector<std::string> OfflineMsg = GetOfflineMsg();
        int terminalWidth = getTerminalWidth();

        // Process each offline message in reverse order
        for (int i = OfflineMsg.size() - 1; i >= 0; i--)
        {
            try
            {
                nlohmann::json js = nlohmann::json::parse(OfflineMsg[i]);
                printRightAligned(js["send"], terminalWidth);
            }
            catch (const nlohmann::json::parse_error& e)
            {
                std::cerr << "Failed to parse JSON message: " << e.what() << std::endl;
            }
        }

        std::cout << "\033[31mChat interface (type your message and press Enter to send, press Esc to exit)\033[0m" << std::endl;
        startMessageReceiver();

        // Initialize chat interface for user input
        while (true)
        {
            std::string userInput;
            bool escPressed = false;

            while (true)
            {
                userInput.clear();  // Clear the buffer

                std::cin.clear();   // Clear any error flags on std::cin
                std::cin.sync();    // Synchronize the buffer

                char c;
                while (std::cin.get(c))
                {
                    if (c == '\n')
                    {
                        break; // End of message
                    }
                    else if (c == ' ')
                    {
                        userInput += '+'; // Convert space to '+'
                    }
                    else if (c == '\x1B') // 退出按键
                    {
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        try
                        {
                            Sen s;
                            nlohmann::json js = {
                                {REQUEST, EXITCHAT},
                                {"id", m_id}
                            };
                            exitRequested = true;
                            s.send_cil(m_fd, js.dump());
                        }
                        catch (const std::exception& e)
                        {
                            std::cerr << "Failed to send exit message: " << e.what() << std::endl;
                        }

                        escPressed = true;
                        break;
                    }
                    else
                    {
                        userInput += c;
                    }
                }
                
                // Check if Esc was pressed
                if (escPressed)
                {
                    break;
                }

                
                // If input is not empty, process it
                if (!userInput.empty())
                {
                    if(userInput.size()>4096)
                    {
                        std::cout << "\033[31mYour message is too long!\033[0m" << std::endl;
                        continue;
                    }
                    std::cout << "Your : " << getCurrentTime() << " :" << userInput << std::endl;
                    bool allPlus = std::all_of(userInput.begin(), userInput.end(), [](char c) { return c == '+'; });
                    if (allPlus)
                    {
                        std::cout << "\033[31mDon't allow input all spaces!\033[0m" << std::endl;
                    }
                    else
                    {
                        try
                        {
                            Sen s;
                            nlohmann::json js = {
                                {"time", getCurrentTime()},
                                {"msg", userInput},
                                {REQUEST, CHAT},
                                {"friend_id", m_friend},
                                {"id", m_id},
                                
                            };
                            s.send_cil(m_fd, js.dump());
                        }
                        catch (const std::exception& e)
                        {
                            std::cerr << "Failed to send message: " << e.what() << std::endl;
                        }
                    }
                }
            }

            // Exit the outer loop if Esc was pressed
            if (escPressed)
            {
                exitRequested = true;
                messageReceiverThread.join();
                break;
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "An error occurred in the chat interface: " << e.what() << std::endl;
    }

    std::cout << "Exited chat interface." << std::endl;
}

void Prich::run_Group()
{
    try
    {
        std::cout << "Start Chat !!!!" << std::endl;
        std::vector<std::string> OfflineMsg = GetOfflineMsg();
        int terminalWidth = getTerminalWidth();

        // Process each offline message in reverse order
        for (int i = OfflineMsg.size() - 1; i >= 0; i--)
        {
            try
            {
                nlohmann::json js = nlohmann::json::parse(OfflineMsg[i]);
                printRightAligned(js["send"], terminalWidth);
            }
            catch (const nlohmann::json::parse_error& e)
            {
                std::cerr << "Failed to parse JSON message: " << e.what() << std::endl;
            }
        }

        std::cout << "\033[31mChat interface (type your message and press Enter to send, press Esc to exit)\033[0m" << std::endl;
        startMessageReceiver();

        // Initialize chat interface for user input
        while (true)
        {
            std::string userInput;
            bool escPressed = false;

            while (true)
            {
                userInput.clear();  // Clear the buffer

                std::cin.clear();   // Clear any error flags on std::cin
                std::cin.sync();    // Synchronize the buffer

                char c;
                while (std::cin.get(c))
                {
                    if (c == '\n')
                    {
                        break; // End of message
                    }
                    else if (c == ' ')
                    {
                        userInput += '+'; // Convert space to '+'
                    }
                    else if (c == '\x1B') // ASCII for Esc
                    {
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        try
                        {
                            Sen s;
                            nlohmann::json js = {
                                {REQUEST, EXITCHAT},
                                {"id", m_id}
                            };
                            exitRequested = true;
                            s.send_cil(m_fd, js.dump());
                        }
                        catch (const std::exception& e)
                        {
                            std::cerr << "Failed to send exit message: " << e.what() << std::endl;
                        }

                        escPressed = true;
                        break;
                    }
                    else
                    {
                        userInput += c;
                    }
                }

                // Check if Esc was pressed
                if (escPressed)
                {
                    break;
                }

                // If input is not empty, process it
                if (!userInput.empty())
                {
                    // std::cout << userInput << std::endl;
                    if(userInput.size()>4096)
                    {
                        std::cout << "\033[31mYour message is too long!\033[0m" << std::endl;
                        continue;
                    }
                    std::cout << "Your : " << getCurrentTime() << " :" << userInput << std::endl;
                    bool allPlus = std::all_of(userInput.begin(), userInput.end(), [](char c) { return c == '+'; });
                    if (allPlus)
                    {
                        std::cout << "\033[31mDon't allow input all spaces!\033[0m" << std::endl;
                    }
                    else
                    {
                        try
                        {
                            Sen s;
                            nlohmann::json js = {
                                {"time", getCurrentTime()},
                                {"msg", userInput},
                                {REQUEST, GROUPCHAT},
                                {"friend_id", m_friend},
                                {"id", m_id},
                                
                            };
                            s.send_cil(m_fd, js.dump());
                        }
                        catch (const std::exception& e)
                        {
                            std::cerr << "Failed to send message: " << e.what() << std::endl;
                        }
                    }
                }
            }

            // Exit the outer loop if Esc was pressed
            if (escPressed)
            {
                exitRequested = true;
                messageReceiverThread.join();
                break;
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "An error occurred in the chat interface: " << e.what() << std::endl;
    }

    std::cout << "Exited chat interface." << std::endl;
}
