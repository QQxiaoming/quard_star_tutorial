(: Name: ForExprType004 :)
(: Description: FLWOR expressions with type declaration (comment type) :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $comment as comment() in $input-context//comment()
return $comment