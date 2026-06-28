"""
Updated generation script to fix "floating" effect
Horizon must be strictly at t=0.5 to align with the camera's eye level.
Ground must extend seamlessly visually.
"""
from PIL import Image, ImageFilter
import math, os, random

SIZE = 512
random.seed(42)

def noise_2d(x, y, seed=0):
    n = int(x * 73 + y * 179 + seed * 311) & 0xFFFF
    n = (n << 13) ^ n
    return 1.0 - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7FFFFFFF) / 1073741824.0

def fbm(x, y, octaves=4, seed=0):
    val = 0.0; amp = 0.5; freq = 1.0
    for _ in range(octaves):
        val += amp * noise_2d(x * freq, y * freq, seed)
        amp *= 0.5; freq *= 2.0
    return val

def lerp_color(c1, c2, t):
    t = max(0.0, min(1.0, t))
    return tuple(int(c1[i] + (c2[i] - c1[i]) * t) for i in range(3))

def make_sky_face(filename, is_top=False, is_bottom=False, is_night=False, seed=0):
    img = Image.new('RGB', (SIZE, SIZE))
    pixels = img.load()
    
    grass_color = (35, 60, 25) # Darker distant green to separate the fair from the background
    
    if is_night:
        sky_top    = (5, 8, 30)
        sky_mid    = (15, 22, 60)
        horizon    = (20, 25, 45)
        ground     = (10, 16, 10)
        cloud_col  = (20, 28, 55)
        cloud_base = 0.12
    else:
        sky_top    = (70, 130, 215)
        sky_mid    = (125, 180, 235)
        horizon    = (190, 215, 230)
        ground     = grass_color
        cloud_col  = (235, 245, 255)
        cloud_base = 0.35

    for y in range(SIZE):
        t = y / (SIZE - 1)  # 0=top, 1=bottom
        
        for x in range(SIZE):
            if is_top:
                base = lerp_color(sky_top, sky_mid, t * 0.5)
                cloud_val = fbm(x / 80.0, y / 80.0, 4, seed)
                cloud_fac = max(0.0, (cloud_val - cloud_base) * 2.0)
                color = lerp_color(base, cloud_col, min(1.0, cloud_fac * 0.6))
            elif is_bottom:
                base = ground
                gn = fbm(x / 40.0, y / 40.0, 3, seed + 100)
                variation = int(gn * 12)
                color = tuple(max(0, min(255, base[i] + variation)) for i in range(3))
            else:
                # SIDE FACES: Horizon lifted to t=0.45 to make it feel closer
                if t < 0.35:
                    st = t / 0.35
                    base = lerp_color(sky_top, sky_mid, st)
                elif t < 0.45:
                    st = (t - 0.35) / 0.10
                    base = lerp_color(sky_mid, horizon, st)
                elif t < 0.47: # short blend into ground
                    st = (t - 0.45) / 0.02
                    base = lerp_color(horizon, ground, st)
                else: 
                    st = (t - 0.47) / 0.53
                    dark_ground = (ground[0]-20, ground[1]-25, ground[2]-15)
                    base = lerp_color(ground, tuple(max(0, c) for c in dark_ground), st)

                # Clouds only above horizon
                if t < 0.42:
                    cloud_val = fbm(x / 90.0, y / 90.0, 5, seed)
                    cloud_fac = max(0.0, (cloud_val - cloud_base) * 2.5)
                    cloud_fac = min(1.0, cloud_fac) * (1.0 - t / 0.42) 
                    color = lerp_color(base, cloud_col, cloud_fac * 0.7)
                else:
                    color = base
                    
                # Silhouette Trees and Distant Villages (approx t=0.38 to 0.47)
                if 0.38 < t < 0.48 and not is_night:
                    tree_val = fbm(x / 10.0, 0, 3, seed + 50) # scaled up x for closer look
                    
                    house_base = fbm(x / 25.0, 0, 2, seed + 200) # scaled up x
                    house_val = 0.0
                    if house_base > 0.6:
                        house_shape = math.sin(x / 12.0) + math.cos(x / 5.0)
                        if house_shape > 0.5:
                            house_val = 0.8
                        elif house_shape < -0.5:
                            house_val = 0.6 + (x % 8)*0.03
                            
                    # Taller elements to make them look closer
                    combined_height = 0.45 - (tree_val * 0.02) - (house_val * 0.04)
                    
                    if t > combined_height:
                        if house_val > 0.0:
                            obj_color = (40, 45, 35) # distant houses
                        else:
                            obj_color = (25 + int(tree_val * 10), 45 + int(tree_val * 15), 20 + int(tree_val * 5))
                            
                        blend = min(1.0, (t - combined_height) * 40.0)
                        if t > 0.45: blend = 1.0 - min(1.0, (t - 0.45) * 40.0) # fade out into ground
                        color = lerp_color(color, obj_color, blend * 0.85)

                elif 0.38 < t < 0.48 and is_night:
                    tree_val = fbm(x / 10.0, 0, 3, seed + 50)
                    house_base = fbm(x / 25.0, 0, 2, seed + 200)
                    house_val = 0.0
                    is_window = False
                    
                    if house_base > 0.6:
                        house_shape = math.sin(x / 12.0) + math.cos(x / 5.0)
                        if house_shape > 0.5:
                            house_val = 0.8
                            # Larger windows: wider x condition, taller y condition
                            if (x % 25 < 8) and (0.422 < t < 0.435):
                                is_window = True
                        elif house_shape < -0.5:
                            house_val = 0.6 + (x % 8)*0.03
                            
                    combined_height = 0.45 - (tree_val * 0.02) - (house_val * 0.04)
                    
                    if t > combined_height:
                        if is_window:
                            obj_color = (255, 210, 100) # glowing warm windows
                            blend = 1.0 # no fading for light
                        else:
                            obj_color = (5, 8, 8) # dark silhouette
                            blend = min(1.0, (t - combined_height) * 40.0)
                            
                        if t > 0.45 and not is_window: blend = 1.0 - min(1.0, (t - 0.45) * 40.0)
                        color = lerp_color(color, obj_color, blend * 0.9)

            pixels[x, y] = tuple(max(0, min(255, c)) for c in color)
    
    # Removed GaussianBlur to make the background clear and crisp
    img.save(filename)
    print(f'  Created: {filename}')

print("Generating DAY skybox faces...")
os.makedirs('skybox', exist_ok=True)
faces = [('right', False, False, 10),('left', False, False, 20),('top', True, False, 30),('bottom', False, True,  40),('front', False, False, 50),('back', False, False, 60)]
for name, is_top, is_bottom, seed in faces:
    make_sky_face(f'skybox/{name}.png', is_top, is_bottom, False, seed)

print("Generating NIGHT skybox faces...")
os.makedirs('skybox_night', exist_ok=True)
for name, is_top, is_bottom, seed in faces:
    make_sky_face(f'skybox_night/{name}.png', is_top, is_bottom, True, seed + 100)
    
print("\nSkybox faces successfully updated to align with the camera ground plane!")
