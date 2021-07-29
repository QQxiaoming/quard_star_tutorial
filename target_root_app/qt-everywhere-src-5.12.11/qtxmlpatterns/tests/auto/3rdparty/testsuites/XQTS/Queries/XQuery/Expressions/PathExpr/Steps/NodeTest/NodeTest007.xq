(: FileName: NodeTest007 :)
(: Purpose: processing-instruction('name') NodeTest can apply under root :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<out>{fn:count($input-context//processing-instruction('a-pi'))}</out>