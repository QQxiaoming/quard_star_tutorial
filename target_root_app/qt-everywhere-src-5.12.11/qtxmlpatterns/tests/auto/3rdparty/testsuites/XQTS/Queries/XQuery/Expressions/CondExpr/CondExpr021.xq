(: FileName: CondExpr021 :)
(: Purpose: Test case where else-expression raises a dynamic error, but test-expression selects then-expression so no error is raised :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

if( true() )
then 10 cast as xs:double ?
else $input-context/Folder[1] cast as xs:double ?