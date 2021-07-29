(: Name: ForExpr010 :)
(: Description: FLWOR expression with multiple, interdependent variables :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $folder in $input-context//Folder
	for $file in $folder//File
return 
<File folder="{$folder/FolderName}">{string($file/FileName[1])}</File>
