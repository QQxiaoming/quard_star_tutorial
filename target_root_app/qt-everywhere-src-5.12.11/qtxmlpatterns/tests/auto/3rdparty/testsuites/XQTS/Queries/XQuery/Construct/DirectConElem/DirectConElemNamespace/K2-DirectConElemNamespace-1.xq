(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-1                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A namespace declaration inside a direct element constructor is not in-scope for the next operand of the comma operator. :)
(:*******************************************************:)
<name xmlns:ns="http://example.com/NS"/>, ns:nametest