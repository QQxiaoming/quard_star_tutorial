(: Name: ReturnExpr010 :)
(: Description: FLWOR expression returns empty sequence literal :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<empty>
{
for $file in ($input-context//Folder)[1]/File
return ()
}
</empty>