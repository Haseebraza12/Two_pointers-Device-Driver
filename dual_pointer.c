#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/usb/input.h>
#include <linux/input/mt.h>

#define DRIVER_AUTHOR "Haseeb"
#define DRIVER_DESC "Dual Mouse Pointer Driver"

// Virtual input devices for the two pointers
static struct input_dev *pointer1_dev;
static struct input_dev *pointer2_dev;

// Physical mouse input handlers
static struct input_handler mouse_handler1;
static struct input_handler mouse_handler2;

// Device IDs for YSPRINGTECH USB OPTICAL MOUSE and PixArt USB Mouse
static const struct input_device_id trackpoint_ids[] = {
    { .vendor = 0x1C4F, .product = 0x1234 }, // Example YSPRINGTECH USB OPTICAL MOUSE IDs (adjust as per your device)
    { },
};

static const struct input_device_id mouse_ids[] = {
    { .vendor = 0x046d, .product = 0xc534 }, // Example PixArt USB Mouse IDs (adjust as per your device)
    { },
};

MODULE_DEVICE_TABLE(input, trackpoint_ids);
MODULE_DEVICE_TABLE(input, mouse_ids);

// Input event callback for pointer1 (TrackPoint -> Virtual Pointer 1)
static void handle_pointer1_event(struct input_handle *handle, unsigned int type, unsigned int code, int value) {
    if (type == EV_REL) {
        // Handle relative movement for the TrackPoint
        if (code == REL_X || code == REL_Y) {
            input_event(pointer1_dev, type, code, value);
            input_sync(pointer1_dev);
        }
    }
    // Handle absolute touch events for more precise movements
    else if (type == EV_ABS) {
        if (code == ABS_X || code == ABS_Y) {
            input_event(pointer1_dev, type, code, value);
        } else if (code == ABS_PRESSURE) {
            // For touch pressure (if needed)
            input_event(pointer1_dev, type, code, value);
        }
        input_sync(pointer1_dev);
    }
}

// Input event callback for pointer2 (Mouse -> Virtual Pointer 2)
static void handle_pointer2_event(struct input_handle *handle, unsigned int type, unsigned int code, int value) {
    if (type == EV_REL) {
        // Handle relative movement for the mouse
        if (code == REL_X || code == REL_Y) {
            input_event(pointer2_dev, type, code, value);
            input_sync(pointer2_dev);
        }
    }
    input_sync(pointer2_dev);
}

// Connect callback for pointer1 (TrackPoint)
static int pointer1_connect(struct input_handler *handler, struct input_dev *dev, const struct input_device_id *id) {
    struct input_handle *handle;

    if (!id) return -ENODEV;  // Check device id match

    handle = kzalloc(sizeof(struct input_handle), GFP_KERNEL);
    if (!handle)
        return -ENOMEM;

    handle->dev = dev;
    handle->handler = handler;
    handle->name = "pointer1_handle";

    int ret = input_register_handle(handle);
    if (ret)
        return ret;

    input_open_device(handle);
    return 0;
}

// Connect callback for pointer2 (Mouse)
static int pointer2_connect(struct input_handler *handler, struct input_dev *dev, const struct input_device_id *id) {
    struct input_handle *handle;

    if (!id) return -ENODEV;  // Check device id match

    handle = kzalloc(sizeof(struct input_handle), GFP_KERNEL);
    if (!handle)
        return -ENOMEM;

    handle->dev = dev;
    handle->handler = handler;
    handle->name = "pointer2_handle";

    int ret = input_register_handle(handle);
    if (ret)
        return ret;

    input_open_device(handle);
    return 0;
}

// Disconnect callback (shared)
static void mouse_disconnect(struct input_handle *handle) {
    input_close_device(handle);
    input_unregister_handle(handle);
    kfree(handle);
}

// Register the driver
static int __init dual_pointer_init(void) {
    int ret;

    // Create the virtual devices
    pointer1_dev = input_allocate_device();
    pointer2_dev = input_allocate_device();

    if (!pointer1_dev || !pointer2_dev) {
        pr_err("Failed to allocate virtual input devices.\n");
        ret = -ENOMEM;
        goto err_alloc;
    }

    pointer1_dev->name = "Virtual Pointer 1";
    pointer1_dev->id.bustype = BUS_USB;

    pointer2_dev->name = "Virtual Pointer 2";
    pointer2_dev->id.bustype = BUS_USB;

    // Register the virtual devices
    ret = input_register_device(pointer1_dev);
    if (ret) {
        pr_err("Failed to register pointer1 device.\n");
        goto err_register1;
    }

    ret = input_register_device(pointer2_dev);
    if (ret) {
        pr_err("Failed to register pointer2 device.\n");
        goto err_register2;
    }

    // Setup the input handlers
    mouse_handler1.event = handle_pointer1_event;
    mouse_handler1.connect = pointer1_connect;
    mouse_handler1.disconnect = mouse_disconnect;
    mouse_handler1.name = "mouse_handler1";
    mouse_handler1.id_table = trackpoint_ids;  // Matching YSPRINGTECH USB OPTICAL MOUSE

    mouse_handler2.event = handle_pointer2_event;
    mouse_handler2.connect = pointer2_connect;
    mouse_handler2.disconnect = mouse_disconnect;
    mouse_handler2.name = "mouse_handler2";
    mouse_handler2.id_table = mouse_ids;  // Matching PixArt USB Mouse

    // Register the input handlers
    ret = input_register_handler(&mouse_handler1);
    if (ret) {
        pr_err("Failed to register mouse_handler1.\n");
        goto err_register_handler1;
    }

    ret = input_register_handler(&mouse_handler2);
    if (ret) {
        pr_err("Failed to register mouse_handler2.\n");
        goto err_register_handler2;
    }

    pr_info("Dual pointer driver loaded.\n");
    return 0;

err_register_handler2:
    input_unregister_handler(&mouse_handler1);
err_register_handler1:
    input_unregister_device(pointer1_dev);
err_register2:
    input_unregister_device(pointer2_dev);
err_register1:
    input_free_device(pointer1_dev);
    input_free_device(pointer2_dev);
err_alloc:
    return ret;
}

static void __exit dual_pointer_exit(void) {
    input_unregister_handler(&mouse_handler1);
    input_unregister_handler(&mouse_handler2);
    input_unregister_device(pointer1_dev);
    input_unregister_device(pointer2_dev);
    pr_info("Dual pointer driver unloaded.\n");
}

module_init(dual_pointer_init);
module_exit(dual_pointer_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
