(: Name: WhereExpr008 :)
(: Description: Test 'where' expression with the empty sequence literal :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<empty>
{
for $file in ($input-context//Folder)[1]/File
where ()
return $file/FileName
}
</empty>