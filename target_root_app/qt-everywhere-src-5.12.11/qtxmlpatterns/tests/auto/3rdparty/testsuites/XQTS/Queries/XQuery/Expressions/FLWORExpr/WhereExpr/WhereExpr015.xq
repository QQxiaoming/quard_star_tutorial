(: Name: WhereExpr015 :)
(: Description: Test 'where' expression returning a sequence with a constant boolean value :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<empty>
{
for $file in ($input-context//Folder)[1]/File
where false()
return $file/FileName
}
</empty>