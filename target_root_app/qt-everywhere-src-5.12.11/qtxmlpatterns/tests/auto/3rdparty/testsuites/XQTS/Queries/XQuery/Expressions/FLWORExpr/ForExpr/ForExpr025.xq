(: Name: ForExpr025 :)
(: Description: Redefine existing bound variable. Second binding overrides first :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file in ($input-context//Folder)[1]/File, $file in ($input-context//Folder)[2]/File
return $file/FileName