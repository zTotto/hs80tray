# Corsair HS80 RGB Wireless (0x0A6B) - Linux HID Protocol

## Device Info
- **Vendor ID**: 0x1B1C
- **Product ID**: 0x0A6B
- **Primary Interface**: Interface 3 (Handles both RGB and Status)

## 1. Handshake (Software Mode)
Send these to Interface 3 to enable custom control:
1. `02 09 01 03 00 02` (Software Mode Enable)
2. `02 09 0D 00 01` (Open Lighting Endpoint)

*Note: Repeat these periodically (every 1-5s) to prevent the firmware from reverting to Hardware Mode.*

## 2. LED Control (Interleaved RGB)
**Report ID**: 02, **Wireless Mode**: 09, **Command**: 06
Format: `02 09 06 00 09 00 00 00 [R0 R1 R2] [G0 G1 G2] [B0 B1 B2]`

### Verified Zone Mapping
- **Index 0**: Logo LED
- **Index 1**: Power LED
- **Index 2**: Microphone LED

### Example: Power Green, Others Off
- Rossi (0, 0, 0): `00 00 00`
- Verdi (0, 255, 0): `00 FF 00`
- Blu   (0, 0, 0): `00 00 00`
**Full Packet**: `02 09 06 00 09 00 00 00 00 00 00 00 FF 00 00 00 00`

## 3. Status Monitoring (Mute/Battery)
**Report ID**: 03
**Event Code**: A6 (Microphone Status)
- **Byte 6 (Index 5)**: `01` (Muted/Up), `00` (Active/Down)

**Event Code**: 0F (Battery Level)
- **Byte 6-7 (Index 5-6)**: 16-bit Little Endian (Value / 10 = %)
