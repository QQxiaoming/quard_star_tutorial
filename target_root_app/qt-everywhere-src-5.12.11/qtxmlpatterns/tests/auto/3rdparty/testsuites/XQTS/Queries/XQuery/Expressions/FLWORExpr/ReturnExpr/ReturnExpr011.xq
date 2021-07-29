(: Name: ReturnExpr011 :)
(: Description: FLWOR expression returns a constructed sequence :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $folder in $input-context//Folder, $file in $folder/File
return ( $folder/FolderName, $file/FileName )