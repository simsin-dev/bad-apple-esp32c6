package main

import (
	"fmt"
	"image"
	_ "image/jpeg"
	_ "image/png"
	"log"
	"net"
	"os"
	"strings"
	"time"
)

const address string = "192.168.1.192:10"
const maxNetPaylaodSize int = 1400

func main() {
	entries, err := os.ReadDir("./vid/frames/")
	if err != nil {
		log.Fatal(err)
	}

	var images [][]byte
	start := time.Now()

	for i := range entries {
		if !entries[i].IsDir() {
			info, err := entries[i].Info()
			if err != nil {
				log.Print(err)
			}
			name := info.Name()

			if strings.HasSuffix(name, ".png") || strings.HasSuffix(name, ".jpg") || strings.HasSuffix(name, ".jpeg") {
				bytes := getImg("./vid/frames/" + name)
				images = append(images, bytes)
			}

		}
	}

	elapsed := time.Since(start)
	fmt.Printf("images converted in: %s\n", elapsed.String())

	fmt.Printf("Image byte size: %d\n", len(images[0]))

	tcpaddr, err := net.ResolveTCPAddr("tcp4", address)
	if err != nil {
		log.Fatal(err)
	}

	conn, err := net.DialTCP("tcp4", nil, tcpaddr)
	if err != nil {
		log.Fatal(err)
	}

	skipNextFrame := false
	start = time.Now()
	for i := range images {
		if skipNextFrame {
			fmt.Printf("Skipping: %d\n", i)
			skipNextFrame = false
			start = time.Now()
			continue
		}

		transferImg(images[i], conn)

		elapsed = time.Since(start)
		ms := elapsed.Milliseconds()
		if ms < 44 {
			time.Sleep(time.Millisecond * time.Duration(ms-43))
		} else if ms > 60 {
			skipNextFrame = true
		}

		start = time.Now()
	}

	conn.Close()
}

func transferImg(img []byte, conn *net.TCPConn) {

	for i := 0; i < len(img); i += maxNetPaylaodSize {
		end := i + maxNetPaylaodSize
		if end > len(img) {
			end = len(img)
		}

		chunk := img[i:end]

		_, err := conn.Write(chunk)
		if err != nil {
			log.Fatal(err)
		}

		//fmt.Printf("Transfered: %d\n", i)
	}

	buff := make([]byte, 10) // read a byte so we dont spam
	read, err := conn.Read(buff)
	if err != nil || read < 2 {
		log.Fatal(err)
	}
}

func getImg(path string) []byte {
	reader, err := os.Open(path)
	if err != nil {
		log.Fatal(err)
	}

	defer reader.Close()

	m, _, err := image.Decode(reader)
	if err != nil {
		log.Fatal(err)
	}

	size := m.Bounds().Size()

	var convertedBytes []byte

	if size.X == 240 && size.Y == 320 {
		convertedImg := convertImage(m, 240, 320)

		for i := range 320 {
			for j := range 30 {
				convertedBytes = append(convertedBytes, convertedImg[i][j])
			}
		}

	} else {
		fmt.Printf("Incorrect size: %dx%d\n", size.X, size.Y)
	}

	return convertedBytes
}

func convertImage(img image.Image, width, height int) [][]uint8 {
	var convertedImage [][]uint8

	for i := range height {
		var colorX []uint8

		for j := 0; j < width; j += 8 {
			var colors8 uint8 = 0
			for k := 0; k < 8; k++ {
				r1, _, _, _ := img.At(j+k, i).RGBA()
				colors8 = colors8 << 1
				colors8 |= colorToBit(r1)
			}

			colorX = append(colorX, colors8)
		}

		convertedImage = append(convertedImage, colorX)
	}

	return convertedImage
}

func colorToBit(color uint32) uint8 {
	if color > 32768 {
		return 1
	} else {
		return 0
	}
}
