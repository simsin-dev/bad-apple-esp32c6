# no #! since i dont want you to run this wihtout reading
# to change the scaling of the original video:
ffmpeg -i bad_apple.webm -vf "scale=320:240" bad_apple_correct.webm
# to rotate it sideways
ffmpeg -i bad_apple_correct.webm -vf "transpose=1" bad_apple_rotated.webm
# to convert into .png frames
ffmpeg -i bad_apple_rotated.webm -pix_fmt rgba frames/%04d.png
