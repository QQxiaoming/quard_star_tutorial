(: Name: step-expr-si-1 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-17 :)
(: Description: Ensure that the type of the child axis on an extended schema type is computed correctly. :)

import schema default element namespace "http://www.example.com/XQueryTest/extendedTypes";

(: insert-begin :)
declare variable $input-context external;
(: insert-end :)

for $extended as element(*, extendedWithElementType) in $input-context//element(*, extendedWithElementType)
let $baseElement as empty-sequence() := $extended/baseElement
return $baseElement
