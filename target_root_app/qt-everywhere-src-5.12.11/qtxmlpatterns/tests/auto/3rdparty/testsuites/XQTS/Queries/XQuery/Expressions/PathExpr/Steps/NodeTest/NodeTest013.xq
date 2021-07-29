(: FileName: NodeTest013 :)
(: Purpose: processing-instruction('name') matches only the given name :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<out>{fn:count($input-context//center/processing-instruction('a-pi'))}</out>