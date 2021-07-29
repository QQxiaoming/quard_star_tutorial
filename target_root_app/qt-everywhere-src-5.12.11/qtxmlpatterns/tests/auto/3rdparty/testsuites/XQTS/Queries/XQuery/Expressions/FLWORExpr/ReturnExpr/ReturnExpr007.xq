(: Name: ReturnExpr007 :)
(: Description: FLWOR expression return parent of select nodes :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file in ($input-context//Folder)[1]/File
return $file/parent::Folder/FolderName