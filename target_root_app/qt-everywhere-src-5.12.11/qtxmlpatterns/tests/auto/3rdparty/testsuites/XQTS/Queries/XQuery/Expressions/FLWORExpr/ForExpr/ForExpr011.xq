(: Name: ForExpr011 :)
(: Description: FLWOR expression with multiple, interdependent variables :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $folder in $input-context//Folder, $file in $folder//File
return 
<File folder="{$folder/FolderName}">{string($file/FileName[1])}</File>
