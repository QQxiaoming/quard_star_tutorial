(: Name: fn-doc-37 :)
(: Description: Use an xml:id attribute that has whitespace, whitespace normalization is performed. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

let $result := id("idABC", doc($input-context))
return (empty($result), $result)
