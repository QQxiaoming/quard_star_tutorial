(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-33                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use a namespace binding in range variables that appears after them in query order. :)
(:*******************************************************:)
<a attr="{let $p:name := 3 return $p:name}" xmlns:p="http://www.example.com/"/>