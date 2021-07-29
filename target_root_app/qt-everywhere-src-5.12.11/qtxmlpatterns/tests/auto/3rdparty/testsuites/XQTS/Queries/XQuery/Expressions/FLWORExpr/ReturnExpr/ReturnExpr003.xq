(: Name: ReturnExpr003 :)
(: Description: For+Return - one For+Return expr contains another one For+Return expr :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $folder in $input-context/MyComputer/Drive1/Folder[1]
return 
<folder1>
  {$folder/FolderName}
  <files>
  { for $file in $folder/File
      return $file/FileName
  }
  </files>
</folder1>
