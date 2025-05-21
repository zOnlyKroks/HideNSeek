# ImageCryptoApp

A comprehensive tool for image cryptography and steganography operations.

## Features

- **Image Encryption/Decryption**: Apply various cryptographic algorithms to transform images
    - XOR encryption
    - Pixel bit manipulation
    - Channel swapping
    - Pixel permutation
    - ROT-N transformation
    - Bitwise NOT operation

- **Steganography**: Hide and extract data within images
    - LSB (Least Significant Bit) steganography
    - DCT (Discrete Cosine Transform) steganography

## Usage

### Encryption/Decryption Mode

```
./ImageCryptoApp --inputFile input.jpg --outputFile output.png --steps xor:3 --masterPassword secret
```

For decryption:

```
./ImageCryptoApp --inputFile encrypted.png --outputFile decrypted.png --decrypt --masterPassword secret
```

### Steganography Mode

#### Hiding Data in an Image

Hide text data:

```
./ImageCryptoApp --steg hide --algo lsb --inputFile carrier.jpg --outputFile stego.png --data "secret message" --pass optional_password
```

Hide text from a file:

```
./ImageCryptoApp --steg hide --algo lsb --inputFile carrier.jpg --outputFile stego.png --data secret.txt --pass optional_password
```

Hide an image within another image:

```
./ImageCryptoApp --steg hide --algo lsb --inputFile carrier.jpg --outputFile stego.png --data hidden.jpg --image --pass optional_password
```

#### Extracting Data from an Image

Extract text data:

```
./ImageCryptoApp --steg extract --algo lsb --inputFile stego.png --outputFile extracted.txt --pass optional_password
```

Extract an image:

```
./ImageCryptoApp --steg extract --algo lsb --inputFile stego.png --outputFile extracted.png --image --pass optional_password
```

## Steganography Algorithms

### LSB (Least Significant Bit)

The LSB algorithm replaces the least significant bits of pixel values with bits from the data to be hidden. This method is simple but effective and has minimal impact on the visual appearance of the image.

Options:
- Default implementation uses 1 bit per channel
- Can be modified to use more bits (higher capacity but more visible)

### DCT (Discrete Cosine Transform)

The DCT algorithm embeds data in the frequency domain by modifying the DCT coefficients