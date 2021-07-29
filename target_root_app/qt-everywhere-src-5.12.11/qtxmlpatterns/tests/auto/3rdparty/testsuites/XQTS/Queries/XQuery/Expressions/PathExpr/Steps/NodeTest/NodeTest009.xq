(: FileName: NodeTest009 :)
(: Purpose: comment() as a NodeTest :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<out>{fn:count($input-context//center/comment())}</out>