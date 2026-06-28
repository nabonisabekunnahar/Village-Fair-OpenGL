$basePath = 'C:\Users\Win-10\source\repos\g_project'
$jpgFiles = @('grass.jpg', 'fabric.jpg', 'wood.jpg', 'brick.jpg')

foreach ($jpg in $jpgFiles) {
    $jpgPath = Join-Path $basePath $jpg
    $pngName = $jpg.Replace('.jpg', '.png')
    $pngPath = Join-Path $basePath $pngName
    
    if (Test-Path $jpgPath) {
        # Remove existing PNG if it exists
        if (Test-Path $pngPath) {
            Remove-Item $pngPath -Force
            Write-Host "Deleted old: $pngName"
        }
        
        # Rename JPG to PNG
        Rename-Item -Path $jpgPath -NewName $pngName
        
        $size = (Get-Item $pngPath).Length / 1KB
        Write-Host "Renamed: $jpg -> $pngName ($('{0:F0}' -f $size) KB)" -ForegroundColor Green
    }
}

Write-Host "`nRenaming complete!" -ForegroundColor Green
Get-Item "$basePath\*.png" | Select-Object Name, @{Name="SizeKB";Expression={[math]::Round($_.Length/1KB,1)}}
