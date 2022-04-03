# Temperature-and-humidity-monitoring-system
使用stm32mp157aaa开发板，在开发者模式下进行，使用tftp服务来挂载内核、设备树，使用nfs服务挂载根文件系统。编写温湿度芯片、数码管、按键、LED灯、蜂鸣器各模块设备驱动，编写设备树文件。
应用层通过ioctl函数下发读命令给温湿度设备驱动读取温湿度，温湿度设备驱动通过i2c_transfer函数发送消息结构体给I2C总线驱动，温湿度设备驱动将得到的信息传回给应用层，应用层再通过ioctl函数下发写命令同时把温湿度数据发送给数码管设备驱动，数码管设备驱动得到温湿度数据，将温湿度数据转化后，通过spi_write函数发送数据给spi总线驱动，最终数码显示相应温湿度数据。
在按键驱动中使用中断子系统和异步通知IO来管理三枚按键，当按键按下时，按键驱动通知应用层来读取准备好的值，在应用层中执行对应按键操作。
设定key1为增加报警温度阈值，key2为降低报警温度阈值，同时应用层也可以自定义报警温度阈值。将key3进行复用，短按时清除报警警告，长按两秒以上时切换数码管温湿度显示。
应用层通过多线程显示温湿度、打印功能选择、温湿度阈值报警，使用条件变量将温湿度阈值报警的线程在温湿度没达到阈值时进行休眠。
