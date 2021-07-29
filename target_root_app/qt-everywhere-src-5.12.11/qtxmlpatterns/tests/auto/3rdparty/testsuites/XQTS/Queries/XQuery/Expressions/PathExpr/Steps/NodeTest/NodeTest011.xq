(: FileName: NodeTest011 :)
(: Purpose: processing-instruction() as a NodeTest :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<out>{fn:count($input-context//center/processing-instruction())}</out>