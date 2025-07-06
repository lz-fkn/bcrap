#include <efi.h>
#include <efilib.h>
#define WIDTH 800
#define HEIGHT 600
static float sin_approx(float x) {
    while (x > 3.14159f) x -= 6.28318f;
    while (x < -3.14159f) x += 6.28318f;
    float x2 = x * x;
    return x * (1 - x2 / 6 + x2 * x2 / 120);
}
static float cos_approx(float x) {
    return sin_approx(x + 1.5708f);
}
typedef struct {
    float x, y, z;
} Vec3;
EFI_GRAPHICS_OUTPUT_BLT_PIXEL colors[] = {
    {255, 0, 0, 0}, {255, 127, 0, 0}, {255, 255, 0, 0},
    {0, 255, 0, 0}, {0, 0, 255, 0}, {75, 0, 130, 0}
};
Vec3 cube[] = {
    {-1, -1, -1}, { 1, -1, -1}, { 1,  1, -1}, {-1,  1, -1},
    {-1, -1,  1}, { 1, -1,  1}, { 1,  1,  1}, {-1,  1,  1},
};
int edges[][2] = {
    {0,1}, {1,2}, {2,3}, {3,0},
    {4,5}, {5,6}, {6,7}, {7,4},
    {0,4}, {1,5}, {2,6}, {3,7}
};
Vec3 rotate(Vec3 p, float angleX, float angleY) {
    Vec3 r = p;
    float y = cos_approx(angleX) * r.y - sin_approx(angleX) * r.z;
    float z = sin_approx(angleX) * r.y + cos_approx(angleX) * r.z;
    r.y = y; r.z = z;
    float x = cos_approx(angleY) * r.x + sin_approx(angleY) * r.z;
    z = -sin_approx(angleY) * r.x + cos_approx(angleY) * r.z;
    r.x = x; r.z = z;
    return r;
}
void draw_pixel(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, int x, int y, EFI_GRAPHICS_OUTPUT_BLT_PIXEL color) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
    gop->Blt(gop, &color, EfiBltVideoFill, 0, 0, x, y, 1, 1, 0);
}
void draw_line(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, int x0, int y0, int x1, int y1, EFI_GRAPHICS_OUTPUT_BLT_PIXEL color) {
    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int dy = (y1 > y0) ? (y1 - y0) : (y0 - y1);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    while (1) {
        draw_pixel(gop, x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx)  { err += dx; y0 += sy; }
    }
}
EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    uefi_call_wrapper(BS->LocateProtocol, 3, &gEfiGraphicsOutputProtocolGuid, NULL, (void**)&gop);
    UINT32 cx = gop->Mode->Info->HorizontalResolution;
    UINT32 cy = gop->Mode->Info->VerticalResolution;
    float angleX = 0.0f, angleY = 0.0f;
    while (1) {
        EFI_GRAPHICS_OUTPUT_BLT_PIXEL black = {0, 0, 0, 0};
        gop->Blt(gop, &black, EfiBltVideoFill, 0, 0, 0, 0, cx, cy, 0);
        Vec3 projected[8];
        for (int i = 0; i < 8; ++i) {
            Vec3 r = rotate(cube[i], angleX, angleY);
            float scale = 200 / (r.z + 4);
            projected[i].x = r.x * scale + cx / 2;
            projected[i].y = r.y * scale + cy / 2;
        }
        for (int i = 0; i < 12; ++i) {
            Vec3 p1 = projected[edges[i][0]];
            Vec3 p2 = projected[edges[i][1]];
            draw_line(gop, (int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y, colors[i % 6]);
        }
        angleX += 0.03f;
        angleY += 0.02f;
    }
    return EFI_SUCCESS;
}
