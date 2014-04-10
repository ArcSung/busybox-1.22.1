
ROOT_DIR=..
KERNEL_RELEASE=3.4.6
XORG_DRIVER_DIR=$ROOT_DIR/usr/lib/xorg/modules/drivers
KMD_DRIVER_DIR=$ROOT_DIR/lib/modules/$KERNEL_RELEASE/kernel/drivers/gpu/drm/s3g
UDEV_RULES_DIR=$ROOT_DIR/lib/udev/rules.d

echo "install s3g_drv.so"

rm -f $XORG_DRIVER_DIR/s3g_drv.so
cp -f s3g_drv.so $XORG_DRIVER_DIR/

echo "install kernel and mdoules"
cp -f uImage $ROOT_DIR/
mkdir -p $KMD_DRIVER_DIR
cp -f s3g_core.ko $KMD_DRIVER_DIR/
cp -f s3g.ko $KMD_DRIVER_DIR/
cp -f s3g_ipc.ko $KMD_DRIVER_DIR/
cp -f xorg.conf $ROOT_DIR/etc/X11/
cp -f 40-graphics-s3g.rules $UDEV_RULES_DIR/
cp -f s3g_drv_video.so $ROOT_DIR/usr/lib/dri/
