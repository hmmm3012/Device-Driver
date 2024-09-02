so I already develop read write method to comunicate to a real hardware, lets say an UART. What if I want to configure its baudrate ? I cant just use write method, it will be out of the context of write function.
Fortunately, kernel offers me a way to solve this problem - IOCTL.

ioctl (file desriptor, ioctl number, long para);
