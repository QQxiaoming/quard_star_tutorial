(: Name: step-expr-si-2 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-17 :)
(: Description: Ensure that static type of the child axis on a restricted schema type is computed correctly. :)

import schema default element namespace "http://www.example.com/XQueryTest/extendedTypes";

(: insert-begin :)
declare variable $input-context external;
(: insert-end :)

for $extended as element(*, restrictedType) in $input-context//element(*, restrictedType)
let $baseElement as element()+ := $extended/node()
return $baseElement
