(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-24                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that the correct default element namespace is used, with an attribute name test. :)
(:*******************************************************:)
<e a="{<e b="data"/>/@b}" xmlns="http://www.example.com/"/>