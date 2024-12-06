import evdev
from evdev import InputDevice, UInput, ecodes
import threading
import pygame

# Replace these paths with your mouse device paths
mouse1 = InputDevice('/dev/input/event7')  # First mouse device
mouse2 = InputDevice('/dev/input/event21')  # Second mouse device

# Initialize Pygame
pygame.init()
screen = pygame.display.set_mode((800, 600))  # Create a window
pygame.display.set_caption("Two Mouse Pointers")

# Define two pointers (initially placed at the center)
pointer1 = pygame.Surface((10, 10))
pointer1.fill((255, 0, 0))  # Red color for pointer1
pointer2 = pygame.Surface((10, 10))
pointer2.fill((0, 255, 0))  # Green color for pointer2

# Initial positions of the pointers
pos1 = [screen.get_width() // 3, screen.get_height() // 2]
pos2 = [2 * screen.get_width() // 3, screen.get_height() // 2]

ui1 = UInput()  # Virtual pointer for mouse1
ui2 = UInput()  # Virtual pointer for mouse2

def handle_mouse(mouse, ui, pointer_num):
    global pos1, pos2
    for event in mouse.read_loop():
        if event.type == ecodes.EV_REL:
            if pointer_num == 1:
                if event.code == ecodes.REL_X:
                    pos1[0] += event.value
                elif event.code == ecodes.REL_Y:
                    pos1[1] += event.value
            elif pointer_num == 2:
                if event.code == ecodes.REL_X:
                    pos2[0] += event.value
                elif event.code == ecodes.REL_Y:
                    pos2[1] += event.value

            # Log for debugging
            print(f"Pointer {pointer_num} moved to: {pos1 if pointer_num == 1 else pos2}")

            ui.write(event.type, event.code, event.value)
            ui.syn()

        elif event.type == ecodes.EV_KEY:
            # Handle button events (left click, etc.)
            ui.write(event.type, event.code, event.value)
            ui.syn()

# Create threads for both mice
thread1 = threading.Thread(target=handle_mouse, args=(mouse1, ui1, 1))
thread2 = threading.Thread(target=handle_mouse, args=(mouse2, ui2, 2))

thread1.start()
thread2.start()

running = True
while running:
    screen.fill((0, 0, 0))  # Clear screen

    # Draw both pointers
    screen.blit(pointer1, pos1)
    screen.blit(pointer2, pos2)

    # Update the screen
    pygame.display.update()

    # Handle Pygame events
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

# Stop the threads
thread1.join()
thread2.join()

pygame.quit()
