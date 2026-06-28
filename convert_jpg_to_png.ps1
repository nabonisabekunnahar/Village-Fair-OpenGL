Add-Type -AssemblyName System.Drawing

$basePath = 'C:\Users\Win-10\source\repos\g_project'
$jpgFiles = @('grass.jpg', 'fabric.jpg', 'wood.jpg', 'brick.jpg')

foreach ($jpg in $jpgFiles) {
    $jpgPath = Join-Path $basePath $jpg
    $pngName = $jpg.Replace('.jpg', '.png')
    $pngPath = Join-Path $basePath $pngName
    
    if (Test-Path $jpgPath) {
        $image = [System.Drawing.Image]::FromFile($jpgPath)
        $image.Save($pngPath, [System.Drawing.Imaging.ImageFormat]::Png)
        $image.Dispose()
        
        $jpgSize = (Get-Item $jpgPath).Length
        $pngSize = (Get-Item $pngPath).Length
        Write-Host "Converted: $jpg ($(($jpgSize/1KB).ToString('F0')) KB) -> $pngName ($(($pngSize/1KB).ToString('F0')) KB)"
    }
}
