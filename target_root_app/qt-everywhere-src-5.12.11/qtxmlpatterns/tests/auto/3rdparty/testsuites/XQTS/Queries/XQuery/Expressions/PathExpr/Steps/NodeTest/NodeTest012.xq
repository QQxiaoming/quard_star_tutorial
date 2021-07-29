(: FileName: NodeTest012 :)
(: Purpose: processing-instruction() NodeTest can apply under root :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<out>{fn:count($input-context//processing-instruction())}</out>