import numpy as np
import scipy.ndimage
from PIL import Image
import math
import os

def create_cubemap_faces(panorama_path, output_dir, face_size=1024):
    img = Image.open(panorama_path).convert('RGB')
    
    # We apply a strong gradient blur to the bottom "floor" area of the 2D image
    # so it softly blends into the 3D ground plane.
    pano = np.array(img).astype(np.float32)
    H, W, _ = pano.shape
    
    # 1. Seamless Horizontal Wrapping (Blend right edge into left edge)
    # This prevents the 'inkblot/mirror maze' problem while still keeping the panorama 360-seamless.
    blend_w = int(0.12 * W)
    left_chunk = pano[:, :blend_w, :].copy()
    for i in range(blend_w):
        alpha = i / blend_w
        # Right edge fades precisely into the very start of the left edge
        pano[:, -blend_w + i, :] = pano[:, -blend_w + i, :] * (1 - alpha) + left_chunk[:, i, :] * alpha
        
    # 2. Seamless Sky Top (Blend top 15% to solid sky blue)
    # This prevents parallel stretching lines at the zenith when we clip the sky!
    sky_blue = np.array([130, 180, 235], dtype=np.float32)
    y_coords = np.arange(H)[:, None, None]
    top_blend = np.clip((0.15 * H - y_coords) / (0.15 * H), 0, 1)
    pano = pano * (1 - top_blend) + sky_blue * top_blend
    
    pano_blurred = scipy.ndimage.gaussian_filter(pano, sigma=(20.0, 20.0, 0))
    y_coords = np.arange(H)[:, None, None]
    blend_factor = np.clip((y_coords - 0.76 * H) / (0.08 * H), 0, 1) # Slowly becomes fully blurry below 0.84H
    pano = pano * (1 - blend_factor) + pano_blurred * blend_factor
    pano = pano.astype(np.uint8)

    faces = {
        'right':  {'front': [ 1,  0,  0], 'right': [ 0,  0, -1], 'down': [ 0, -1,  0]},
        'left':   {'front': [-1,  0,  0], 'right': [ 0,  0,  1], 'down': [ 0, -1,  0]},
        'top':    {'front': [ 0,  1,  0], 'right': [ 1,  0,  0], 'down': [ 0,  0,  1]},
        'bottom': {'front': [ 0, -1,  0], 'right': [ 1,  0,  0], 'down': [ 0,  0, -1]},
        'front':  {'front': [ 0,  0,  1], 'right': [ 1,  0,  0], 'down': [ 0, -1,  0]},
        'back':   {'front': [ 0,  0, -1], 'right': [-1,  0,  0], 'down': [ 0, -1,  0]},
    }

    os.makedirs(output_dir, exist_ok=True)
    os.makedirs(output_dir + "_night", exist_ok=True)

    U, V = np.meshgrid(np.linspace(-1, 1, face_size), np.linspace(-1, 1, face_size))
    
    for face_name, dParams in faces.items():
        F = np.array(dParams['front'])
        R = np.array(dParams['right'])
        D = np.array(dParams['down'])
        
        X = F[0] + U * R[0] + V * D[0]
        Y = F[1] + U * R[1] + V * D[1]
        Z = F[2] + U * R[2] + V * D[2]
        
        norm = np.sqrt(X*X + Y*Y + Z*Z)
        X /= norm
        Y /= norm
        Z /= norm
        
        theta = np.arctan2(Z, X) 
        phi = np.arcsin(Y)      
        
        # Zoom out horizontally (tile more times) to make houses look further away
        zoom = 2.2
        # Use simple continuous tracking. 'wrap' mode will naturally connect W back to 0 seamlessly!
        pano_x = (theta / (np.pi * 2.0) + 0.5) * W * zoom
        
        # Vertical mapping: Linear stretch to keep horizon houses perfectly proportioned!
        pano_y = (0.78 - phi * zoom) * (H - 1)
        pano_y = np.clip(pano_y, 0, H - 1)

        face_img = np.zeros((face_size, face_size, 3), dtype=np.uint8)
        for c in range(3):
            # 'wrap' mode effortlessly creates an infinite rotating world without mirroring
            face_img[:, :, c] = scipy.ndimage.map_coordinates(
                pano[:, :, c], [pano_y, pano_x], order=1, mode='wrap'
            )
            
        pil_img = Image.fromarray(face_img)
        day_path = os.path.join(output_dir, f"{face_name}.png")
        pil_img.save(day_path)
        print(f"Created {day_path}")
        
        # Night mode
        night_arr = face_img.astype(np.float32)
        night_arr[:, :, 0] *= 0.1  
        night_arr[:, :, 1] *= 0.2  
        night_arr[:, :, 2] *= 0.35 
        
        np.random.seed(hash(face_name) % 100000)
        stars = np.random.rand(face_size, face_size) > 0.999
        is_sky = (night_arr[:,:,2] < 80) & (face_img[:,:,2] > 100)
        star_mask = stars & is_sky
        night_arr[star_mask] = [200, 220, 255]
        
        night_img = Image.fromarray(np.clip(night_arr, 0, 255).astype(np.uint8))
        night_path = os.path.join(output_dir + "_night", f"{face_name}.png")
        night_img.save(night_path)
        print(f"Created {night_path}")

if __name__ == "__main__":
    pano_path = r"skybox.png"
    create_cubemap_faces(pano_path, "skybox", 1024)
