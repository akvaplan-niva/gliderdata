param (
  [string] $root = ".",
  [string] $algo = "SHA256",
  [string] $shafilename = "shasum.txt",
  [string] $exclude = $shafilename,
  [switch] $force
)

function Count-File-Lines {
  param(
    $path
  )
  [int]$lines = 0
  $reader = New-Object IO.StreamReader $path
  while($reader.ReadLine() -ne $null) { $lines++ }
  return $lines;
}

function shasum {
  param(
    $path,
    $algo
  )
  $shasum = Get-FileHash -Algorithm $algo -Path $path | ForEach-Object {
  Write-Host -NoNewLine "."
  "$($_.Hash)  $($_.Path | Resolve-Path -Relative)";
  }
  return $shasum;
}

function subdir-sums {
  param(
    $root,
    $algo,
    $exclude
  )
  [int]$lines = 0

  $sums = Get-ChildItem $root -File -Recurse -Exclude $exclude
    | ForEach-Object { shasum -path $_ -algo $algo }
  return $sums;
}

$source = Resolve-Path $root
$pwd = Get-Location

Write-Host "Generating $algo checksums in sub-directories of $source"

Get-ChildItem $source -Directory | ForEach-Object {
  $dir = $_
  Set-Location $dir # Location changed in order to have relative paths in the shasum-file
  $dirname=$dir.Name
  $shapath="$source/$dirname/$shafilename"

  Write-Host -NoNewline "$dirname "

  if ((Test-Path $shapath) -And !$force) {
    $verb = "retained"
  } else {
    $verb = "generated"
    subdir-sums -root $dir -algo $algo -exclude $exclude > $shapath
  }
  $lines = Count-File-Lines -path $shapath
  Write-Host " [$lines checksums $verb]"

}
Set-Location $pwd
