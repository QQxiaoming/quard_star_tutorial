(: Name: WhereExpr013 :)
(: Description: Test 'where' expression returning a sequence with a single boolean value :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file in ($input-context//Folder)[1]/File
where $file/SecurityObject/Denies/Deny/security/right="Change"
return $file/FileName