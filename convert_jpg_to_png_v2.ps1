Add-Type -AssemblyName System.Drawing

$basePath = 'C:\Users\Win-10\source\repos\g_project'
$jpgFiles = @('grass.jpg', 'fabric.jpg', 'wood.jpg', 'brick.jpg')

foreach ($jpg in $jpgFiles) {
    $jpgPath = Join-Path $basePath $jpg
    $pngName = $jpg.Replace('.jpg', '.png')
    $pngPath = Join-Path $basePath $pngName
    
    try {
        if (Test-Path $jpgPath) {
            Write-Host "Loading: $jpg"
            $stream = [System.IO.File]::OpenRead($jpgPath)
            $image = [System.Drawing.Image]::FromStream($stream, $true)
            
            Write-Host "Converting to PNG..."
            $image.Save($pngPath, [System.Drawing.Imaging.ImageFormat]::Png)
            $image.Dispose()
            $stream.Close()
            $stream.Dispose()
            
            $jpgSize = (Get-Item $jpgPath).Length / 1KB
            $pngSize = (Get-Item $pngPath).Length / 1KB
            Write-Host "Success: $pngName ($('{0:F0}' -f $pngSize) KB)" -ForegroundColor Green
        }
    }
    catch {
        Write-Host "Error on $jpg : $_" -ForegroundColor Red
    }
    
    [System.GC]::Collect()
}

Write-Host "Conversion complete!"
