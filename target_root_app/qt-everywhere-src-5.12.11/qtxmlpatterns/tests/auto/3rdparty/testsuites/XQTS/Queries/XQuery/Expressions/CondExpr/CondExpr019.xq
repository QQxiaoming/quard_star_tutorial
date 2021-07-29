(: FileName: CondExpr019 :)
(: Purpose: Test with test expression not contained in ( ... ) :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

if $input-context//File[1]
then "true"
else "false"