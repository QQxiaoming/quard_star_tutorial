(: FileName: NodeTest008 :)
(: Purpose: text() as a NodeTest :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<out>{fn:count($input-context//center/text())}</out>