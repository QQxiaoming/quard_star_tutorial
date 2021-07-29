(: Name: Constr-namespace-14 :)
(: Written by: Andreas Behm :)
(: Description: Namespace Declarations - use undeclared parent prefix in child :)

<foo:elem xmlns:foo="http://www.example.com/parent"><child xmlns:foo=""><foo:grand-child/></child></foo:elem>