/*
 * macmasq
 * Copyright (C) 2025 Vasu Makadia
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Constant to enable GNU extensions
#define _GNU_SOURCE  

// Including required C Header files
#include <stdio.h>         // for standard input/output functions
#include <stdlib.h>        // for standard library functions (rand, srand, EXIT_SUCCESS, EXIT_FAILURE, etc.)
#include <stdbool.h>       // for Boolean type support
#include <unistd.h>        // for POSIX API functions (getpid, close, etc.)
#include <string.h>        // for string manipulation functions (strncpy, memcpy, etc.)
#include <errno.h>         // for error number definitions for error handling
#include <assert.h>        // for assert function for debugging
#include <sys/ioctl.h>     // for ioctl function for device control operations
#include <sys/socket.h>    // for socket programming functions and definitions
#include <net/if.h>        // for network interface definitions (struct ifreq, etc.)
#include <net/if_arp.h>    // for ARP protocol definitions (hardware types, etc.)
#include <netinet/in.h>    // for definitions for internet operations

// Defining required data types (for 64 bit systems)
typedef unsigned char int8;                // Define int8 as unsigned char
typedef unsigned short int int16;          // Define int16 as unsigned short int
typedef unsigned int int32;                // Define int32 as unsigned int
typedef unsigned long int int64;           // Define int64 as unsigned long int

/**
* @brief A structure to store a MAC address
*/
typedef struct store_mac {               
    unsigned char bytes[6];              // Array to store 6 bytes representing the MAC address
} MacAddress;                            // Alias the structure to MacAddress

/**
 * @brief Generates a random MAC address.
 *
 * This function creates a random MAC address by generating 6 random bytes.
 * It then sets the locally administered bit and clears the multicast bit in the first byte.
 *
 * @param void (nothing)
 * @return MacAddress The generated random MAC address.
 */
MacAddress generate_mac_address(void) {
    MacAddress mac_addr;                    // Declare a variable to hold the MAC address
    for (int i = 0; i < 6; i++) {           // Loop through each of the 6 bytes
        mac_addr.bytes[i] = rand() % 256;   // Assign a random value (0-255) to each byte
    }
    // Set the locally administered bit and clear the multicast bit in the first byte
    mac_addr.bytes[0] = (mac_addr.bytes[0] & 0xFE) | 0x02;  // Adjust first byte according to MAC standards
    // Return the generated MAC address
    return mac_addr;                     
}

/**
 * @brief Changes the MAC address of the specified network interface.
 *
 * This function brings the network interface down, changes its MAC address,
 * and then brings it back up.
 *
 * @param interface_name A string representing the network interface name.
 * @param new_mac The new MAC address to apply.
 * @return true if the MAC address was changed successfully, false otherwise.
 */
bool change_mac_address(const char *interface_name, MacAddress new_mac) {
    struct ifreq interface_request;      // Declare structure to hold interface request parameters
    int socket_fd;                       // Declare variable to hold the socket file descriptor
    int original_flags;                  // Declare variable to store original interface flags

    // Create a socket for performing ioctl operations
    socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);  // Open a socket with IPv4, datagram type, and IP protocol
    if (socket_fd < 0) {                // Check if socket creation failed
        perror("socket");               // Print error message to stderr
        return false;                   // Return false indicating failure
    }

    // Set the interface name in the request structure
    strncpy(interface_request.ifr_name, interface_name, IFNAMSIZ - 1);  // Copy the interface name into the structure
    interface_request.ifr_name[IFNAMSIZ - 1] = '\0';                    // Ensure the interface name is null-terminated

    // Retrieve current interface flags using ioctl
    if (ioctl(socket_fd, SIOCGIFFLAGS, &interface_request) < 0) {
        perror("ioctl (SIOCGIFFLAGS)");     // Print error message if retrieval fails
        close(socket_fd);                   // Close the socket
        return false;                       // Return false indicating failure
    }
    // Store the original interface flags
    original_flags = interface_request.ifr_flags;  

    // Bring the interface down by clearing the IFF_UP flag
    interface_request.ifr_flags &= ~IFF_UP;  // Modify flags to disable (bring down) the interface
    // Apply the new flags to bring the interface down
    if (ioctl(socket_fd, SIOCSIFFLAGS, &interface_request) < 0) {  
        perror("ioctl (SIOCSIFFLAGS - down)");  // Print error message if operation fails
        close(socket_fd);                       // Close the socket
        return false;                           // Return false indicating failure
    }

    // Set the hardware address family to Ethernet and copy the new MAC address bytes
    interface_request.ifr_hwaddr.sa_family = ARPHRD_ETHER;              // Set the hardware address family to Ethernet
    memcpy(interface_request.ifr_hwaddr.sa_data, new_mac.bytes, 6);     // Copy the new MAC address bytes into the request

    // Change the MAC address using ioctl
    if (ioctl(socket_fd, SIOCSIFHWADDR, &interface_request) < 0) {
        perror("ioctl (SIOCSIFHWADDR)");                        // Print error message if MAC address change fails
        interface_request.ifr_flags = original_flags;           // Restore original interface flags
        ioctl(socket_fd, SIOCSIFFLAGS, &interface_request);     // Bring the interface back up before exiting
        close(socket_fd);                                       // Close the socket
        return false;                                           // Return false indicating failure
    }

    // Bring the interface back up by restoring the original flags
    interface_request.ifr_flags = original_flags;   // Reset the interface flags to their original state
    // Apply the flags to bring the interface up
    if (ioctl(socket_fd, SIOCSIFFLAGS, &interface_request) < 0) {   
        perror("ioctl (SIOCSIFFLAGS - up)");    // Print error message if operation fails
        close(socket_fd);                       // Close the socket
        return false;                           // Return false indicating failure
    }

    close(socket_fd);                  // Close the socket as it is no longer needed
    return true;                       // Return true indicating the MAC address was changed successfully
}

/**
 * @brief Main function to change the MAC address of a specified network interface.
 *
 * This program generates a random MAC address and attempts to apply it to the given
 * network interface. It prints the new MAC address if successful, or an error message otherwise.
 *
 * @param argc The number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @return (int) EXIT_SUCCESS if successful, EXIT_FAILURE otherwise.
 */
int main(int argc, char **argv) {
    // Check if the required interface argument is provided
    if (argc < 2) {     
        // Print usage message to stderr               
        fprintf(stderr, "Usage: %s INTERFACE\n", argv[0]);  
        // Exit with failure code if interface is not provided
        return EXIT_FAILURE;           
    }

    // Seed the random number generator with the process ID
    srand(getpid());                   

    // Generate a new random MAC address
    MacAddress new_mac = generate_mac_address();  

      // Attempt to change the MAC address of the specified interface
    if (change_mac_address(argv[1], new_mac)) {
        // Print the new MAC address in standard hexadecimal format
        printf("New MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
               new_mac.bytes[0], new_mac.bytes[1], new_mac.bytes[2],
               new_mac.bytes[3], new_mac.bytes[4], new_mac.bytes[5]);
    } else {
        // Print error message if MAC address change failed
        fprintf(stderr, "Failed to change MAC address.\n");  
        // Exit with failure code
        return EXIT_FAILURE;           
    }

    // Exit with success code
    return EXIT_SUCCESS;
}