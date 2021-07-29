(: FileName: NodeTest010 :)
(: Purpose: comment() NodeTest can apply under root :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<out>{fn:count($input-context//comment())}</out>