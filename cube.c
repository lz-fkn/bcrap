// === cube.c - UEFI Rotating Cube (No Dependencies) ===
typedef unsigned long long UINT64;
typedef unsigned int UINT32;
typedef unsigned short UINT16;
typedef unsigned char UINT8;

typedef struct { float x, y, z; } Vec3;
typedef struct { UINT8 b, g, r, a; } Pixel;

// Cube vertices and edges
Vec3 cube[] = {
    {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
    {-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1}
};

int edges[][2] = {
    {0,1}, {1,2}, {2,3}, {3,0}, // Bottom face
    {4,5}, {5,6}, {6,7}, {7,4}, // Top face
    {0,4}, {1,5}, {2,6}, {3,7}  // Connecting edges
};

// Fast sine/cosine approximations
float sin_approx(float x) {
    while (x > 3.14159f) x -= 6.28318f;
    while (x < -3.14159f) x += 6.28318f;
    float x2 = x * x;
    return x * (1 - x2/6 + x2*x2/120);
}

float cos_approx(float x) { return sin_approx(x + 1.5708f); }

// Rotate point around X/Y axes
Vec3 rotate(Vec3 p, float ax, float ay) {
    Vec3 r = p;
    // X rotation
    float y = cos_approx(ax)*r.y - sin_approx(ax)*r.z;
    float z = sin_approx(ax)*r.y + cos_approx(ax)*r.z;
    r.y = y; r.z = z;
    // Y rotation
    float x = cos_approx(ay)*r.x + sin_approx(ay)*r.z;
    z = -sin_approx(ay)*r.x + cos_approx(ay)*r.z;
    r.x = x; r.z = z;
    return r;
}

// Main UEFI entry point
void __attribute__((ms_abi)) efi_main(void *h, void *st) {
    // Set up 800x600 framebuffer
    Pixel *fb = (Pixel*)0xA0000;
    int width = 800, height = 600;
    
    float angle = 0.0f;
    while (1) {
        // Clear screen (black)
        for (int i = 0; i < width * height; i++)
            fb[i] = (Pixel){0, 0, 0, 0};
        
        // Draw rotating cube
        for (int i = 0; i < 12; i++) {
            Vec3 p1 = rotate(cube[edges[i][0]], angle, angle*0.7f);
            Vec3 p2 = rotate(cube[edges[i][1]], angle, angle*0.7f);
            
            // Simple perspective projection
            int x1 = (int)(p1.x * 100 / (p1.z + 4) + width/2);
            int y1 = (int)(p1.y * 100 / (p1.z + 4) + height/2);
            int x2 = (int)(p2.x * 100 / (p2.z + 4) + width/2);
            int y2 = (int)(p2.y * 100 / (p2.z + 4) + height/2);
            
            // Draw line (Bresenham's algorithm)
            int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
            int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
            int err = dx + dy, e2;
            
            while (1) {
                if (x1 >= 0 && x1 < width && y1 >= 0 && y1 < height) {
                    // Rainbow colors based on edge index
                    fb[y1 * width + x1] = (Pixel){
                        (i*50)%255,  // Blue
                        (i*70)%255,  // Green
                        (i*90)%255,  // Red
                        0            // Alpha
                    };
                }
                if (x1 == x2 && y1 == y2) break;
                e2 = 2*err;
                if (e2 >= dy) { err += dy; x1 += sx; }
                if (e2 <= dx) { err += dx; y1 += sy; }
            }
        }
        angle += 0.05f;
    }
}
