(: Name: step-expr-si-3 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-17 :)
(: Description: Ensure that the child axis on a restricted schema type is computed correctly :)

import schema default element namespace "http://www.example.com/XQueryTest/extendedTypes";

(: insert-begin :)
declare variable $input-context external;
(: insert-end :)

for $extended as element(*, anotherBaseType) in $input-context//element(restrictedAttribute, anotherBaseType)
let $baseElement as element(baseElement) := $extended/baseElement
return $baseElement
